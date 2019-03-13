#-*- coding: UTF-8 -*-
from collections import OrderedDict

def is_access_key(ch):
	return (ch >= '0' and ch <= '9') or (ch >= 'A' and ch <= 'Z') or (ch >= 'a' and ch <= 'z')

def find_access_key(menu):
	key = []
	index = 0
	while index < len(menu):
		ch = menu[index]
		index += 1
		if ch == '&':
			ch = menu[index]
			if is_access_key(ch):
				index += 1
				key.append(ch.upper())

	if len(key) == 0:
		raise ValueError('No access key: ' + menu)
	if len(key) > 1:
		raise ValueError('Multiply access key: ' + menu)

	return key[0]

def find_free_access_key(menu, path):
	all_key = {}
	used_key = set() # from frozen line

	lines = []
	for line in menu.splitlines():
		line = line.strip()
		if not line or line.startswith('//'):
			continue

		start = line.find('"')
		if start < 0:
			continue

		start += 1
		end_cut = line.find('\t', start)
		end = line.find('"', start)
		if end <= start:
			print('Error:', line)
			continue

		if end_cut > 0 and end_cut < end:
			end = end_cut

		used = line.endswith('//#')
		line = line[start:end].strip()

		if used:
			try:
				key = find_access_key(line)
				if key in used_key:
					print('duplicate access key:', line)
				else:
					used_key.add(key)
			except Exception as ex:
				print('find access key fail:', line, ex)
		else:
			lines.append(line)

	if not lines:
		return

	required = set()
	for line in lines:
		unique = set(ch.upper() for ch in line if is_access_key(ch))
		required |= unique
		for ch in unique:
			if ch not in used_key:
				if ch in all_key:
					all_key[ch].append(line)
				else:
					all_key[ch] = [line]

	if all_key:
		all_key = OrderedDict(sorted(all_key.items(), key=lambda m: m[0]))
		all_key = OrderedDict(sorted(all_key.items(), key=lambda m: len(m[1])))

		print('write:', path)
		with open(path, 'w', encoding='utf-8', newline='\n') as fd:
			for key, lines in all_key.items():
				fd.write(key + ':\n')
				for line in lines:
					fd.write('\t' + line + '\n')
	else:
		print('No free access key:', lines)
		print('required access key:', list(sorted(required)))

# line starts with '//' is line comment
# line ends with '//#' is frozen, access key should not changed
find_free_access_key('''
''', 'access_key.log')
