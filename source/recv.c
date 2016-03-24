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
	Path *path;
	Buffer *buffer;
	int *done;
} Cookie;

void callback(void *cookie_data, struct can_frame *frame) {
	struct timespec ts;
	long ns;
	
	Cookie *cookie = (Cookie*)cookie_data;
	Time *time = cookie->time;
	Path *path = cookie->path;
	Buffer *buffer = cookie->buffer;
	
	clock_gettime(CLOCK_MONOTONIC, &ts);
	if(!time->init) {
		ns = 0;
		time->init = 1;
	} else {
		ns = get_ns_diff(&ts, &time->lts);
	}
	time->lts = ts;
	
	int chan = frame->data[0] & 0xf;
	int ival = ((int)frame->data[1] << 8) | (int)frame->data[2];
	double val = ((double)ival/0xFFFF)*(KOZ_DAC_MAX_VOLTAGE - KOZ_DAC_MIN_VOLTAGE) + KOZ_DAC_MIN_VOLTAGE;
	
	switch(chan) {
	case 0:
		path->x = val;
		break;
	case 1:
		path->y = val;
		break;
	}
	
	fprintf(stderr, "%ld\n", buffer->pos);
	
	buffer->coord[2*buffer->pos + 0] = path->x;
	buffer->coord[2*buffer->pos + 1] = path->y;
	buffer->time[buffer->pos] = time->t;		
	
	buffer->pos++;
	if(buffer->pos >= buffer->len)
		*(cookie->done) = 1;
	
	time->t += 1e-9*ns;
	
	++time->counter;
}

int main(int argc, char *argv[]) {
	int i;
	int status;
	int done = 0;
	CAN_Node node;
	
	//signal(SIGTERM, sig_handler);
	//signal(SIGINT, sig_handler);
	
	const char *ifname;
#ifdef __XENO__
	ifname = "rtcan1";
#else
	ifname = "can1";
#endif
	status = CAN_createNode(&node, ifname);
	if(status != 0)
		return 1;

	//printf("Node created\n");
	
	Time time;
	time.t = 0.0;
	time.init = 0;
	time.counter = 0;
	
	Path path;
	path.x = 0;
	path.y = 0;
	path.t = 0;
	
	Buffer buffer;
	buffer.len = 0x1000;
	buffer.pos = 0;
	buffer.coord = (double*)malloc(2*sizeof(double)*buffer.len);
	buffer.time = (double*)malloc(sizeof(double)*buffer.len);
	
	Cookie cookie;
	cookie.path = &path;
	cookie.time = &time;
	cookie.buffer = &buffer;
	cookie.done = &done;
	
#ifdef __REALTIME__
	struct sched_param param;
	param.sched_priority = 80;
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
#endif // __REALTIME__
	
	CAN_listen(&node, callback, &cookie, &done);
	
#ifdef __REALTIME__
	pthread_setschedparam(pthread_self(), SCHED_OTHER, &param);
#endif // __REALTIME__
	
	CAN_destroyNode(&node);
	
	for(i = 0; i < buffer.pos; ++i) {
		printf("%lf \t%lf \t%lf \n", buffer.coord[2*i + 0], buffer.coord[2*i + 1], buffer.time[i]);
	}
	
	free((void*)buffer.coord);
	free((void*)buffer.time);
	
	//printf("done in %ld steps\n", counter);
	
	//printf("exiting...\n");
	
	return 0;
}
