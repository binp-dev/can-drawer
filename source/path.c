#include "path.h"

#include <math.h>

double deriv(void (*f)(double, double *, double *), double t) {
	double dt = 1e-8;
	double x[2], y[2];
	f(t, x, y);
	f(t + dt, x + 1, y + 1);
	double dx = x[1] - x[0], dy = y[1] - y[0];
	return sqrt(dx*dx + dy*dy)/dt;
}

void pathInit(Path *path, void (*f)(double, double *, double *))  {
	path->f = f;
	path->t = 0.0;
	path->f(path->t, &path->x, &path->y);
}

void pathStep(Path *path, double dt) {
	path->t += dt/deriv(path->f, path->t);
	path->f(path->t, &path->x, &path->y);
}
