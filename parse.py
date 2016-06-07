#!/usr/bin/python3

import numpy as np

svg = []

for l in open('path.txt'):
	l = l.replace('\n', '')
	svg.append(l.split(' '))

# print(svg)


def vec(ln, p):
	return np.array([float(ln[p]), float(ln[p + 1])])

crvs = []

cp = None
sp = None
crv = None
for i in range(len(svg)):
	ln = svg[i]
	cmd = ln[0]
	if cmd == 'M':
		cp = vec(ln, 1)
		bp = cp
	elif cmd == 'm':
		cp = cp + vec(ln, 1)
		bp = cp
	elif cmd == 'c':
		crv = [cp]
		crv.append(cp + vec(ln, 1))
		crv.append(cp + vec(ln, 3))
		cp = cp + vec(ln, 5)
		crv.append(cp)
		crvs.append(crv)
	elif cmd == 'C':
		crv = [cp]
		crv.append(vec(ln, 1))
		crv.append(vec(ln, 3))
		cp = vec(ln, 5)
		crv.append(cp)
		crvs.append(crv)
	elif cmd == 's':
		crv = [cp]
		crv.append(cp - (sp - cp))
		crv.append(cp + vec(ln, 1))
		cp = cp + vec(ln, 3)
		crv.append(cp)
		crvs.append(crv)
	elif cmd == 'S':
		crv = [cp]
		crv.append(cp - (sp - cp))
		crv.append(vec(ln, 1))
		cp = vec(ln, 3)
		crv.append(cp)
		crvs.append(crv)
	elif cmd == 'l':
		crv = [cp, cp]
		cp = cp + vec(ln, 1)
		crv.append(cp)
		crv.append(cp)
		crvs.append(crv)
	elif cmd == 'L':
		crv = [cp, cp]
		cp = vec(ln, 1)
		crv.append(cp)
		crv.append(cp)
		crvs.append(crv)
	elif cmd == 'z':
		crv = [cp, cp]
		cp = bp
		crv.append(cp)
		crv.append(cp)
	else:
		print('unknown command: ' + cmd)

	if crv is not None:
		sp = crv[2]

for crv in crvs:
	print('%f, %f, %f, %f, %f, %f, %f, %f,' % (crv[3][0], crv[3][1], crv[2][0], crv[2][1], crv[1][0], crv[1][1], crv[0][0], crv[0][1]))
