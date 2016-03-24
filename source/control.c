#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>

#include <candev/node.h>
#include <candev/kozak.h>

#include "path.h"

void func(double t, double *x, double *y) {
	const int mr = 7.4;
	const int s = 4;
	t *= s;
	int ft = (int)floor(t);
	double r = mr*((ft%s) + (t - ft))/s;
	*x = r*cos(2*M_PI*t);
	*y = r*sin(2*M_PI*t);
}

#define __REALTIME__
#define __PRINT__

long get_ns_diff(const struct timespec *ts, const struct timespec *lts) {
	return 1000000000*(ts->tv_sec - lts->tv_sec) + ts->tv_nsec - lts->tv_nsec;
}

int main(int argc, char *argv[]) {
	int status;
	int done = 0;
	CAN_Node node;
	KOZ dev;
	
	//signal(SIGTERM, sig_handler);
	//signal(SIGINT, sig_handler);
	
	const char *ifname;
#ifdef __XENO__
	ifname = "rtcan0";
#else
	ifname = "can0";
#endif
	status = CAN_createNode(&node, ifname);
	if(status != 0)
		return 1;

	printf("Node created\n");
	
	KOZ_setup(&dev, 0x0F, &node);
	
	dev.cb_cookie = (void *) &dev;
	
	Path path;
	pathInit(&path, func);
	
#ifdef __REALTIME__
	struct sched_param param;
	param.sched_priority = 80;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
#endif // __REALTIME__
	
	double t = 0.0;
	struct timespec lts;
	double speed = 1.0; // V/s
	long delay = 10000000; // ns
	
	long ns;
	long counter = 0;
	
	Path old_path = path;
	
	clock_gettime(CLOCK_MONOTONIC, &lts);
	while(!done) {
		struct timespec ts;
		double dt;
		
		KOZ_DACWriteProp wp;
		wp.use_code = 0;
		
		// X channels
		wp.voltage = path.x;
		{
			wp.channel_number = 2;
			if(KOZ_dacWrite(&dev, &wp) != 0) {
				fprintf(stderr, "error dac write\n");
				return 2;
			}
			
			wp.channel_number = 4;
			if(KOZ_dacWrite(&dev, &wp) != 0) {
				fprintf(stderr, "error dac write\n");
				return 2;
			}
		}
		
		// Y channels
		wp.voltage = path.y;
		{
			wp.channel_number = 3;
			if(KOZ_dacWrite(&dev, &wp) != 0) {
				fprintf(stderr, "error dac write\n");
				return 2;
			}
			
			wp.channel_number = 5;
			if(KOZ_dacWrite(&dev, &wp) != 0) {
				fprintf(stderr, "error dac write\n");
				return 2;
			}
		}
		
		clock_gettime(CLOCK_MONOTONIC, &ts);
		ns = get_ns_diff(&ts, &lts);
		
		if(ns < delay) {
			ts.tv_sec = 0;
			ts.tv_nsec = delay - ns;
			nanosleep(&ts, NULL);
		}
		
		clock_gettime(CLOCK_MONOTONIC, &ts);
		ns = get_ns_diff(&ts, &lts);
		lts = ts;
		
		old_path = path;
		
		dt = 1e-9*ns;
		double dpar = speed*dt;
		pathStep(&path, dpar);
		t += dt;
		
		++counter;
	}
	
#ifdef __REALTIME__
	pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif // __REALTIME__
	
	CAN_destroyNode(&node);
	
	printf("done in %ld steps\n", counter);
	
	printf("exiting...\n");
	
	return 0;
}
