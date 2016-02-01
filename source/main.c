#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>

#include <candev/node.h>
#include <candev/ceac124.h>

#define __REALTIME__

long get_ns_diff(const struct timespec *ts, const struct timespec *lts) {
	return 1000000000*(ts->tv_sec - lts->tv_sec) + ts->tv_nsec - lts->tv_nsec;
}

int main(int argc, char *argv[]) {
	int status;
	int done = 0;
	CAN_Node node;
	CEAC124 dev;
	
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
	
	CEAC124_setup(&dev, 0x1F, &node);
	
	dev.cb_cookie = (void *) &dev;
	
#ifdef __REALTIME__
	struct sched_param param;
	param.sched_priority = 80;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
#endif // __REALTIME__
	
	double t = 0.0;
	struct timespec lts;
	double freq = 0.5; // Hz
	long delay = 10000000; // ns
	
	long ns;
	
	clock_gettime(CLOCK_MONOTONIC, &lts);
	while(!done) {
		struct timespec ts;
		
		CEAC124_DACWriteProp wp;
		wp.use_code = 0;
		
		wp.channel_number = 0;
		wp.voltage = 7.4*cos(t);
		if(CEAC124_dacWrite(&dev, &wp) != 0) {
			fprintf(stderr, "error dac write\n");
			return 2;
		}
		
		wp.channel_number = 2;
		wp.voltage = 7.4*sin(t);
		if(CEAC124_dacWrite(&dev, &wp) != 0) {
			fprintf(stderr, "error dac write\n");
			return 2;
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
		
		t += freq*(2*M_PI*1e-9*ns);
	}
	
	CAN_destroyNode(&node);
	
	printf("exiting...\n");
	
	return 0;
}
