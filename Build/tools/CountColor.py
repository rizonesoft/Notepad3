#-*- coding: UTF-8 -*-
from __future__ import print_function
import sys
import os.path
from collections import OrderedDict
import operator
import re

kReColorHex = re.compile(r'#[0-9A-Fa-f]{6}')

def parse_key_value(line):
	line = line.strip()
	if not line or line[0] in ';#[':
		return None

	items = line.split('=', 2)
	if not items or len(items) != 2:
		return None

	items[0] = items[0].strip()
	items[1] = items[1].strip()
	if not items[0] or not items[1]:
		return None

	return items

def find_color_in_file(path, color_map):
	for line in open(path).readlines():
		items = parse_key_value(line)
		if not items:
			continue

		colors = kReColorHex.findall(items[1])
		if not colors:
			continue

		key = items[0]
		for color in colors:
			color = color.upper()
			if color in color_map:
				color_stat = color_map[color]
				color_stat['total_count'] += 1
				if key not in color_stat['usage']:
					color_stat['usage'][key] = 1
				else:
					color_stat['usage'][key] += 1
			else:
				color_stat = {
					'total_count': 1,
					'usage': {
						key: 1,
					},
				}
				color_map[color] = color_stat

def print_color_count(color_map):
	for color, color_stat in color_map.items():
		print('%s\t%d' % (color, color_stat['total_count']))
		usage = color_stat['usage']
		for key, count in usage.items():
			print('\t%d\t%s' % (count, key))

def count_color(path):
	# { color : { total_count: total_count, usage: { key: count}}}
	color_map = {}
	find_color_in_file(path, color_map)

	colors = sorted(color_map.items(), key=operator.itemgetter(0))
	colors = sorted(colors, key=lambda m: m[1]['total_count'], reverse=True)
	color_map = OrderedDict(colors)
	for color_stat in color_map.values():
		usage = color_stat['usage']
		usage = sorted(usage.items(), key=operator.itemgetter(0))
		usage = sorted(usage, key=operator.itemgetter(1), reverse=True)
		color_stat['usage'] = OrderedDict(usage)

	print_color_count(color_map)

if __name__ == '__main__':
	if len(sys.argv) > 1 and os.path.isfile(sys.argv[1]):
		count_color(sys.argv[1])
	else:
		print("""Usage: %s path""" % sys.argv[0])
