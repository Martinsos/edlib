#!/usr/bin/env python

import random
import sys
import fileinput

if (len(sys.argv) < 2):
	print("Missing first argument: chance of mutation.")
	sys.exit()

chance = float(sys.argv[1])

seq = sys.stdin.readline()

#alphabet = ['A','R','N','D','C','Q','E','G','H','I','L','K','M','F','P','S','T','W','Y','V','B','J','Z','X']
alphabet = ['A', 'C', 'T', 'G']

seq = list(seq)

for i in range(0, len(seq)):
        if random.random() < chance:
                seq[i] = random.choice(alphabet)

seq = "".join(seq)

print(seq)
