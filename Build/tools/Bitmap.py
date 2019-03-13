#-*- coding: UTF-8 -*-
import sys
import os
import struct
import math
from enum import IntEnum
from PIL import Image

__all__ = ['Bitmap']

# https://en.wikipedia.org/wiki/BMP_file_format
class BitmapFileHeader(object):
	StructureSize = 14

	def __init__(self):
		self.size = 0
		self.reserved1 = 0
		self.reserved2 = 0
		self.offset = 54

	def read(self, fd):
		magic = fd.read(2)
		assert magic == b'BM'
		self.size = struct.unpack('<I', fd.read(4))[0]
		self.reserved1, self.reserved2 = struct.unpack('<HH', fd.read(2*2))
		self.offset = struct.unpack('<I', fd.read(4))[0]
		assert self.offset == 54

	def write(self, fd):
		fd.write(b'BM')
		fd.write(struct.pack('<I', self.size))
		fd.write(struct.pack('<HH', self.reserved1, self.reserved2))
		fd.write(struct.pack('<I', self.offset))

	def __str__(self):
		return f'''BitmapFileHeader {{
	size: {self.size :08X} {self.size}
	reserved: {self.reserved1 :04X} {self.reserved2 :04X}
	offset: {self.offset :08X} {self.offset}
}}'''

_InchesPerMetre = 0.0254
_TransparentColor = (0, 0, 0, 0)

class CompressionMethod(IntEnum):
	BI_RGB = 0
	BI_RLE8 = 1
	BI_RLE4 = 2
	BI_BITFIELDS = 3
	BI_JPEG = 4
	BI_PNG = 5
	BI_ALPHABITFIELDS = 6
	BI_CMYK = 11
	BI_CMYKRLE8 = 12
	BI_CMYKRLE4 = 13

	@staticmethod
	def getName(value):
		try:
			return CompressionMethod(value).name
		except ValueError:
			return f'Unknown-{value}'

class BitmapInfoHeader(object):
	StructureSize = 40

	def __init__(self):
		self.width = 0
		self.height = 0
		self.planes = 1
		self.bitsPerPixel = 32
		self.compression = 0
		self.sizeImage = 0
		self.resolutionX = 96
		self.resolutionY = 96
		self.colorUsed = 0
		self.colorImportant = 0

	def read(self, fd):
		magic, self.width, self.height = struct.unpack('<III', fd.read(4*3))
		assert magic == BitmapInfoHeader.StructureSize
		self.planes, self.bitsPerPixel = struct.unpack('<HH', fd.read(2*2))
		assert self.planes == 1
		self.compression, self.sizeImage = struct.unpack('<II', fd.read(4*2))
		self._resolutionX, self._resolutionY = struct.unpack('<II', fd.read(4*2))
		self.colorUsed, self.colorImportant = struct.unpack('<II', fd.read(4*2))

	def write(self, fd):
		fd.write(struct.pack('<III', BitmapInfoHeader.StructureSize, self.width, self.height))
		fd.write(struct.pack('<HH', self.planes, self.bitsPerPixel))
		fd.write(struct.pack('<II', self.compression, self.sizeImage))
		fd.write(struct.pack('<II', self._resolutionX, self._resolutionY))
		fd.write(struct.pack('<II', self.colorUsed, self.colorImportant))

	@property
	def size(self):
		return (self.width, self.height)

	@property
	def resolutionX(self):
		return round(self._resolutionX * _InchesPerMetre)

	@resolutionX.setter
	def resolutionX(self, value):
		self._resolutionX = round(value / _InchesPerMetre)

	@property
	def resolutionY(self):
		return round(self._resolutionY * _InchesPerMetre)

	@resolutionY.setter
	def resolutionY(self, value):
		self._resolutionY = round(value / _InchesPerMetre)

	@property
	def resolution(self):
		return (self.resolutionX, self.resolutionY)

	@resolution.setter
	def resolution(self, value):
		self.resolutionX = value[0]
		self.resolutionY = value[1]

	def __str__(self):
		return f'''BitmapInfoHeader {{
	width: {self.width :08X} {self.width}
	height: {self.height :08X} {self.height}
	planes: {self.planes :04X} {self.planes}
	bitsPerPixel: {self.bitsPerPixel :04X} {self.bitsPerPixel}
	compression: {self.compression :08X} {self.compression} {CompressionMethod.getName(self.compression)}
	sizeImage: {self.sizeImage :08X} {self.sizeImage}
	resolutionX: {self._resolutionX :08X} {self._resolutionX} {self.resolutionX} DPI
	resolutionY: {self._resolutionY :08X} {self._resolutionY} {self.resolutionY} DPI
	colorUsed: {self.colorUsed :08X} {self.colorUsed}
	colorImportant: {self.colorImportant :08X} {self.colorImportant}
}}'''

class Bitmap(object):
	def __init__(self, width=None, height=None, bitsPerPixel=32):
		self.fileHeader = BitmapFileHeader()
		self.infoHeader = BitmapInfoHeader()
		self.rows = [] # RGBA tuple
		self.data = None

		self.infoHeader.bitsPerPixel = bitsPerPixel
		if width and height:
			for y in range(height):
				row = [_TransparentColor] * width
				self.rows.append(row)
			self.infoHeader.width = width
			self.infoHeader.height = height

	def read(self, fd):
		start = fd.tell()
		self.fileHeader.read(fd)
		self.infoHeader.read(fd)

		curret = fd.tell()
		# enable reading from stream
		offset = self.fileHeader.offset - (curret - start)
		if offset != 0:
			fd.seek(offset, os.SEEK_CUR)
		# TODO: sizeImage maybe zero
		self.data = fd.read()
		self.decode()

	def write(self, fd):
		self.encode()
		self.fileHeader.write(fd)
		self.infoHeader.write(fd)
		fd.write(self.data)

	def decode(self):
		if not self.data:
			return
		bitsPerPixel = self.bitsPerPixel
		if bitsPerPixel == 32:
			self._decode_32bit()
		elif bitsPerPixel == 24:
			self._decode_24bit()
		else:
			print(f'Warning: decode not implemented for {bitsPerPixel} bit bitmap', file=sys.stderr)

	def encode(self):
		if not self.rows:
			return
		bitsPerPixel = self.bitsPerPixel
		if bitsPerPixel == 32:
			self._encode_32bit()
		elif bitsPerPixel == 24:
			self._encode_24bit()
		else:
			print(f'Warning: encode not implemented for {bitsPerPixel} bit bitmap', file=sys.stderr)

	def _set_data(self, buf):
		self.data = bytes(buf)
		size = len(buf)
		self.infoHeader.sizeImage = size
		self.fileHeader.size = size + BitmapFileHeader.StructureSize + BitmapInfoHeader.StructureSize

	def _decode_32bit(self):
		width, height = self.size
		self.rows.clear()

		offset = 0
		buf = self.data
		rows = []
		for y in range(height):
			row = []
			for x in range(width):
				blue = buf[offset]
				green = buf[offset + 1]
				red = buf[offset + 2]
				alpha = buf[offset + 3]
				offset += 4
				row.append((red, green, blue, alpha))
			rows.append(row)
		self.rows.extend(reversed(rows))

	def _encode_32bit(self):
		width, height = self.size

		buf = []
		for y in range(height - 1, -1, -1):
			row = self.rows[y]
			for x in range(width):
				red, green, blue, alpha = row[x]
				buf.append(blue)
				buf.append(green)
				buf.append(red)
				buf.append(alpha)
		self._set_data(buf)

	def _decode_24bit(self):
		width, height = self.size
		self.rows.clear()

		offset = 0
		buf = self.data
		padding = math.ceil(24*width/32)*4 - width*3
		rows = []
		for y in range(height):
			row = []
			for x in range(width):
				blue = buf[offset]
				green = buf[offset + 1]
				red = buf[offset + 2]
				offset += 3
				row.append((red, green, blue, 0xFF))

			offset += padding
			rows.append(row)
		self.rows.extend(reversed(rows))

	def _encode_24bit(self):
		width, height = self.size

		padding = math.ceil(24*width/32)*4 - width*3
		paddingBytes = [0] * padding
		buf = []
		for y in range(height - 1, -1, -1):
			row = self.rows[y]
			for x in range(width):
				red, green, blue, alpha = row[x]
				buf.append(blue)
				buf.append(green)
				buf.append(red)
			if paddingBytes:
				buf.extend(paddingBytes)
		self._set_data(buf)

	@property
	def width(self):
		return self.infoHeader.width

	@property
	def height(self):
		return self.infoHeader.height

	@property
	def size(self):
		return self.infoHeader.size

	@property
	def resolutionX(self):
		return self.infoHeader.resolutionX

	@resolutionX.setter
	def resolutionX(self, value):
		self.infoHeader.resolutionX = value

	@property
	def resolutionY(self):
		return self.infoHeader.resolutionY

	@resolutionX.setter
	def resolutionY(self, value):
		self.infoHeader.resolutionY = value

	@property
	def resolution(self):
		return self.infoHeader.resolution

	@resolution.setter
	def resolution(self, value):
		self.infoHeader.resolution = value

	@property
	def bitsPerPixel(self):
		return self.infoHeader.bitsPerPixel

	@bitsPerPixel.setter
	def bitsPerPixel(self, value):
		self.infoHeader.bitsPerPixel = value

	def __getitem__(self, key):
		return self.getColor(key[0], key[1])

	def __setitem__(self, key, value):
		self.setColor(key[0], key[1], value)

	def getColor(self, x, y):
		return self.rows[y][x]

	def setColor(self, x, y, color):
		if len(color) == 3:
			color = (color[0], color[1], color[2], 0xFF)
		elif len(color) != 4:
			raise ValueError('Invalid color:' + str(color))
		self.rows[y][x] = color

	def save(self, path):
		if hasattr(path, 'write'):
			self.write(path)
		else:
			with open(path, 'wb') as fd:
				self.write(fd)

	@staticmethod
	def fromFile(path):
		bmp = Bitmap()
		if hasattr(path, 'read'):
			bmp.read(path)
		else:
			with open(path, 'rb') as fd:
				bmp.read(fd)
		return bmp

	@staticmethod
	def fromImage(image):
		if image.mode != 'RGB' and image.mode != 'RGBA':
			image = image.convert('RGBA')

		width, height = image.size
		data = image.load()
		if image.mode == 'RGB':
			bmp = Bitmap(width, height, 24)
			for y in range(height):
				for x in range(width):
					bmp[x, y] = data[x, y]
		else:
			bmp = Bitmap(width, height, 32)
			for y in range(height):
				for x in range(width):
					color = data[x, y]
					# TODO: fix transparent color
					if color[3] == 0:
						bmp[x, y] = _TransparentColor
					else:
						bmp[x, y] = color
		return bmp

	def toImage(self):
		image = None
		width, height = self.size
		bitsPerPixel = self.bitsPerPixel
		if bitsPerPixel == 24:
			image = Image.new('RGB', (width, height))
			data = []
			for row in self.rows:
				for red, green, blue, alpha in row:
					data.append((red, green, blue))
			image.putdata(data)
		elif bitsPerPixel == 32:
			image = Image.new('RGBA', (width, height))
			data = []
			for row in self.rows:
				data.extend(row)
			image.putdata(data)
		else:
			print(f'Warning: toImage not implemented for {bitsPerPixel} bit bitmap', file=sys.stderr)
		return image

	@staticmethod
	def fromFileEx(path):
		image = Image.open(path)
		if image.format == 'BMP':
			try:
				bmp = Bitmap.fromFile(path)
				if bmp.bitsPerPixel == 24 or bmp.bitsPerPixel == 32:
					return bmp
			except Exception:
				pass
		return Bitmap.fromImage(image)

	@staticmethod
	def concatHorizontal(bmps):
		width = 0
		height = 0
		for bmp in bmps:
			width += bmp.width
			if height == 0:
				height = bmp.height
			elif height != bmp.height:
				raise ValueError(f'Invalid image height {bmp.height}, requre {height}!')

		out_bmp = Bitmap(width, height)
		rows = out_bmp.rows
		for y in range(height):
			rows[y].clear()
		for bmp in bmps:
			for y in range(height):
				rows[y].extend(bmp.rows[y])

		return out_bmp

	def splitHorizontal(self, dims=None):
		width, height = self.size
		if not dims:
			dims = [height] * (width // height)
			w = width % height
			if w:
				dims.append(w)
		else:
			used = []
			total = 0
			for w in dims:
				total += w
				if total > width:
					total -= w
					break
				used.append(w)
			w = width - total
			if w:
				used.append(w)
			dims = used

		total = 0
		bmps = []
		for w in dims:
			bmp = Bitmap(w, height)
			for y in range(height):
				bmp.rows[y].clear()
				bmp.rows[y].extend(self.rows[y][total:total + w])
			total += w
			bmps.append(bmp)

		return bmps

	@staticmethod
	def concatVertical(bmps):
		width = 0
		height = 0
		for bmp in bmps:
			height += bmp.height
			if width == 0:
				width = bmp.width
			elif width != bmp.width:
				raise ValueError(f'Invalid image width {bmp.width}, requre {width}!')

		out_bmp = Bitmap(width, height)
		rows = out_bmp.rows
		rows.clear()
		for bmp in bmps:
			for row in bmp.rows:
				rows.append(row[:])

		return out_bmp

	def splitVertical(self, dims=None):
		width, height = self.size
		if not dims:
			dims = [width] * (height // width)
			h = height % width
			if h:
				dims.append(h)
		else:
			used = []
			total = 0
			for h in dims:
				total += h
				if total > height:
					total -= h
					break
				used.append(h)
			h = height - total
			if h:
				used.append(h)
			dims = used

		total = 0
		bmps = []
		for h in dims:
			bmp = Bitmap(width, h)
			bmp.rows.clear()
			for row in self.rows[total:total + h]:
				bmp.rows.append(row[:])
			total += h
			bmps.append(bmp)
		return bmps

	def flipHorizontal(self):
		width, height = self.size
		bmp = Bitmap(width, height)
		bmp.rows.clear()
		for row in self.rows:
			copy = row[:]
			copy.reverse()
			bmp.rows.append(copy)
		return bmp

	def flipVertical(self):
		width, height = self.size
		bmp = Bitmap(width, height)
		bmp.rows.clear()
		for row in reversed(self.rows):
			bmp.rows.append(row[:])
		return bmp
