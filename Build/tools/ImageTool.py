#!/usr/bin/env python3
#-*- coding: UTF-8 -*-
import os.path
import re
from Bitmap import Bitmap

def save_bitmap(bmp, path):
	ext = os.path.splitext(path)[1].lower()
	if ext == '.bmp':
		bmp.save(path)
	else:
		img = bmp.toImage()
		img.save(path)

def dump_bitmap(path):
	print('dump bitmap:', path)
	bmp = Bitmap.fromFile(path)
	print(bmp.fileHeader)
	print(bmp.infoHeader)

	data_path = path + '.data'
	print('write:', data_path, len(bmp.data))
	with open(data_path, 'wb') as fd:
		fd.write(bmp.data)

	dump_path = os.path.splitext(path)[0] + '-dump.bmp'
	print('write:', dump_path)
	bmp.save(dump_path)

def convert_image(path, out_path=None):
	if not out_path:
		name, ext = os.path.splitext(path)
		if ext.lower() == 'bmp':
			out_path = name + '-converted' + '.bmp'
		else:
			out_path = name + '.bmp'

	print('convert image:', path, '=>', out_path)
	bmp = Bitmap.fromFileEx(path)
	#bmp.resolution = (96, 96)
	save_bitmap(bmp, out_path)


def concat_images(horizontal, paths, out_path):
	if horizontal:
		print('concat horizontal:', ', '.join(paths), '=>', out_path)
	else:
		print('concat vertical:', ', '.join(paths), '=>', out_path)

	bmps = []
	for path in paths:
		bmp = Bitmap.fromFileEx(path)
		bmps.append(bmp)

	if horizontal:
		bmp = Bitmap.concatHorizontal(bmps)
	else:
		bmp = Bitmap.concatVertical(bmps)
	save_bitmap(bmp, out_path)

def concat_horizontal(paths, out_path):
	concat_images(True, paths, out_path)

def concat_vertical(paths, out_path):
	concat_images(False, paths, out_path)


def save_bitmap_list(bmps, out_path, ext):
	if not os.path.exists(out_path):
		os.makedirs(out_path)
	for index, bmp in enumerate(bmps):
		path = os.path.join(out_path, f'{index}{ext}')
		save_bitmap(bmp, path)

def _parse_split_dims(item):
	items = item.split()
	dims = []
	for item in items:
		m = re.match(r'(\d+)(x(\d+))?', item)
		if m:
			g = m.groups()
			size = int(g[0])
			count = g[2]
			if count:
				dims.extend([size] * int(count))
			else:
				dims.append(size)
		else:
			break
	return dims

def split_image(horizontal, path, dims=None, out_path=None, ext=None):
	name, old_ext = os.path.splitext(path)
	if not out_path:
		out_path = name + '-split'
	if not ext:
		ext = old_ext

	if isinstance(dims, str):
		dims = _parse_split_dims(dims)
	if horizontal:
		print('split horizontal:', path, dims, '=>', out_path)
	else:
		print('split vertical:', path, dims, '=>', out_path)

	bmp = Bitmap.fromFileEx(path)
	if horizontal:
		bmps = bmp.splitHorizontal(dims)
	else:
		bmps = bmp.splitVertical(dims)
	save_bitmap_list(bmps, out_path, ext)

def split_horizontal(path, dims=None, out_path=None, ext=None):
	split_image(True, path, dims, out_path, ext)

def split_vertical(path, dims=None, out_path=None, ext=None):
	split_image(False, path, dims, out_path, ext)


def flip_image(horizontal, path, out_path=None):
	if not out_path:
		name, ext = os.path.splitext(path)
		out_path = name + '-flip' + ext
	if horizontal:
		print('flip horizontal:', path, '=>', out_path)
	else:
		print('flip vertical:', path, '=>', out_path)

	bmp = Bitmap.fromFileEx(path)
	if horizontal:
		bmp = bmp.flipHorizontal()
	else:
		bmp = bmp.flipVertical()
	save_bitmap(bmp, out_path)

def flip_horizontal(path, out_path=None):
	flip_image(True, path, out_path)

def flip_vertical(path, out_path=None):
	flip_image(False, path, out_path)


def make_matapath_toolbar_bitmap():
	concat_horizontal([
		'images/Previous_16x.png',				# IDT_HISTORY_BACK
		'images/Next_16x.png',					# IDT_HISTORY_FORWARD
		'images/Upload_16x.png',				# IDT_UP_DIR
		'images/OneLevelUp_16x.png',			# IDT_ROOT_DIR
		'images/Favorite_16x.png',				# IDT_VIEW_FAVORITES
		'images/NextDocument_16x.png',			# IDT_FILE_PREV
		'images/NextDocument_16x.png',			# IDT_FILE_NEXT
		'images/Run_16x.png',					# IDT_FILE_RUN
		'images/PrintPreview_16x.png',			# IDT_FILE_QUICKVIEW
		'images/Save_16x.png',					# IDT_FILE_SAVEAS
		'images/CopyItem_16x.png',				# IDT_FILE_COPYMOVE
		'images/RestoreFromRecycleBin_16x.png',	# IDT_FILE_DELETE_RECYCLE
		'images/RedCrossMark_16x.png',			# IDT_FILE_DELETE_PERM
		'images/DeleteFilter_16x.png',			# IDT_VIEW_FILTER TB_DEL_FILTER_BMP
		'images/AddFilter_16x.png',				# IDT_VIEW_FILTER TB_ADD_FILTER_BMP
	], 'Toolbar.bmp')

#make_matapath_toolbar_bitmap()
#split_horizontal('Toolbar.bmp', '16x40')
