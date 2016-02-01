#pragma once

typedef struct Path {
	double x, y;
	double t;
	void (*f)(double, double*, double*);
} Path;

void pathInit(Path *path, void (*f)(double, double*, double*));
void pathStep(Path *path, double dt);
