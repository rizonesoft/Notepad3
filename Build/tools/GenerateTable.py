#!/usr/bin/env python3

def GenerateEOLTable():
	table = [0] * 16
	table[ord('\n')] = 1
	table[ord('\r')] = 2
	line = ', '.join(str(c) for c in table)
	line = line + ', // %02X - %02X' % (0, 15)
	print('EOLTable:', line)

if __name__ == '__main__':
	GenerateEOLTable()
