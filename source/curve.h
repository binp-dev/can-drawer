#include <math.h>

#define sizearr(arr) (sizeof(arr)/sizeof(arr[0]))

typedef struct Point {
	float x, y;
} Point;

Point pmul(Point p, float f) {
	Point r;
	r.x = p.x*f;
	r.y = p.y*f;
	return r;
}

Point padd(Point a, Point b) {
	Point c;
	c.x = a.x + b.x;
	c.y = a.y + b.y;
	return c;
}

float plen(Point p) {
	return sqrt(p.x*p.x + p.y*p.y);
}

typedef struct Curve {
	Point p[4];
} Curve;

Point get_pos(const Curve *c, float t) {
	float s = 1.0 - t;
	return padd(padd(pmul(c->p[0], t*t*t), pmul(c->p[1], 3*t*t*s)), padd(pmul(c->p[2], 3*t*s*s), pmul(c->p[3], s*s*s)));
}

Point get_vel(const Curve *c, float t) {
	float s = 1.0 - t;
	return padd(padd(pmul(c->p[0], 3*t*t), pmul(c->p[1], 3*t*(2 - 3*t))), padd(pmul(c->p[2], 3*(3*t*t - 4*t + 1)), pmul(c->p[3], -3*s*s)));
}

float step(Curve *cs, int size, float t, float spd) {
	int i, len = 0x10;
	for(i = 0; i < 0x10 && t >= 0 && t <= size; ++i) {
		int k = (int) t;
		Curve *c = cs + k;
		float f = t - k;
		t += spd/len/(plen(get_vel(c, f)) + 1e-1);
	}
	return t;
}
