#-*- coding: UTF-8 -*-
from __future__ import print_function
import sys
from math import sqrt

def is_prime_odd(n):
	m = int(sqrt(n))
	for k in range(3, m + 1, 2):
		if (n % k) == 0:
			return False
	return True

def find_prime():
	n = 2
	count = 1
	if len(sys.argv) > 1:
		n = max(2, int(sys.argv[1]))
		if len(sys.argv) > 2:
			count = max(1, int(sys.argv[2]))

	result = []
	if n == 2:
		result.append(2)
		count -= 1
	if (n & 1) == 0:
		n += 1

	while count != 0:
		while not is_prime_odd(n):
			n += 2

		result.append(n)
		n += 2
		count -= 1

	print('next prime:', result)

if __name__ == '__main__':
	find_prime()
