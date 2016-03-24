#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>

#include <candev/node.h>
#include <candev/kozak.h>

#include "path.h"
#define __REALTIME__

long get_ns_diff(const struct timespec *ts, const struct timespec *lts) {
	return 1000000000*(ts->tv_sec - lts->tv_sec) + ts->tv_nsec - lts->tv_nsec;
}

typedef struct {
	double t;
	struct timespec lts;
	int init;
	long counter;
} Time;

typedef struct {
	double *coord;
	double *time;
	long len;
	long pos;
} Buffer;

typedef struct {
	Time *time;
	int *done;
} Cookie;

void callback(void *cookie_data, const KOZ_ADCReadResult *result) {
	struct timespec ts;
	long ns;
	
	//printf("callback\n");
	
	Cookie *cookie = (Cookie*)cookie_data;
	Time *time = cookie->time;
	
	clock_gettime(CLOCK_MONOTONIC, &ts);
	if(!time->init) {
		ns = 0;
		time->init = 1;
	} else {
		ns = get_ns_diff(&ts, &time->lts);
	}
	time->lts = ts;
	
	printf("channel: %d,\tvoltage: %lf,\ttime: %lf\n", result->channel_number, result->voltage, time->t);
	
	time->t += 1e-9*ns;
	
	++time->counter;
}

int done = 0;

void sighandler(int sig)
{
	done = 1;
}

int main(int argc, char *argv[]) {
	int i;
	int status;
	CAN_Node node;
	KOZ dev;
	
	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);
	
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
	
	status = KOZ_setup(&dev, 0x0f, &node);
	if(status != 0)
		return 2;
	
	dev.cb_adc_read_m = callback;
	dev.cb_adc_read_s = callback;
	
	printf("Device initialized\n");
	
	Time time;
	time.t = 0.0;
	time.init = 0;
	time.counter = 0;
	
	Cookie cookie;
	cookie.time = &time;
	cookie.done = &done;
	
	dev.cb_cookie = (void *) &cookie;
	
#ifdef __REALTIME__
	struct sched_param param;
	param.sched_priority = 80;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
#endif // __REALTIME__
	
	KOZ_ADCReadMProp prop;
	prop.channel_begin = 4;
	prop.channel_end = 9;
	prop.mode = 0x30;
	prop.time = KOZ_ADC_READ_TIME_1MS;
	KOZ_adcReadM(&dev, &prop);
	
	KOZ_listen(&dev, &done);
	
	KOZ_adcStop(&dev);
	
#ifdef __REALTIME__
	pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif // __REALTIME__
	
	CAN_destroyNode(&node);
	
	printf("exiting...\n");
	
	return 0;
}
