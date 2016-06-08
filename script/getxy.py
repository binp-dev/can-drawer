#!/usr/bin/python3

data = open('stats/mes4.txt')

p1,p2 = ([],[])

(x1,y1,x2,y2) = (0,0,0,0)
for line in data:
	line.split('\t')
