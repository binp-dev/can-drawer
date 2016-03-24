XENO_CFLAGS=-g -D__XENO__ $(shell xeno-config --skin=posix --cflags)
XENO_LDFLAGS=-g $(shell xeno-config --skin=posix --ldflags)

CF=${XENO_CFLAGS} -Isource
LF=${XENO_LDFLAGS} -lm

SRC_PATH=source
CANDEV_PATH=${SRC_PATH}/candev
CANDEV_HDRS=${CANDEV_PATH}/node.h ${CANDEV_PATH}/device.h ${CANDEV_PATH}/kozak.h ${CANDEV_PATH}/util.h

.phony: all clean

all: build build/candev build/can-control build/can-monitor

build:
	mkdir -p $@

build/candev: build
	mkdir -p $@

build/can-control: build/control.o build/path.o build/candev/node_init.o build/candev/node_use.o 
	gcc ${LF} -o $@ $^

build/control.o: source/control.c source/path.h ${CANDEV_HDRS}
	gcc ${CF} -c -o $@ $<

build/can-monitor: build/monitor.o build/path.o build/candev/node_init.o build/candev/node_use.o 
	gcc ${LF} -o $@ $^

build/monitor.o: source/monitor.c source/path.h ${CANDEV_HDRS}
	gcc ${CF} -c -o $@ $<

build/path.o: source/path.c source/path.h
	gcc ${CF} -c -o $@ $<

build/candev/node_init.o: source/candev/node_init.c ${CANDEV_PATH}/node.h
	gcc ${CF} -c -o $@ $<
	
build/candev/node_use.o: source/candev/node_use.c ${CANDEV_PATH}/node.h
	gcc ${CF} -c -o $@ $<
