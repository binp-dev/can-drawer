XENO_CFLAGS=-g -D__XENO__ $(shell xeno-config --skin=posix --cflags)
XENO_LDFLAGS=-g $(shell xeno-config --skin=posix --ldflags)

CF=${XENO_CFLAGS} -Isource
LF=${XENO_LDFLAGS} -lm

.phony: all clean

all: build build/candev build/can-send build/can-recv

build:
	mkdir -p $@

build/candev: build
	mkdir -p $@

build/can-send: build/send.o build/path.o build/candev/node_init.o build/candev/node_use.o 
	gcc ${LF} -o $@ $^

build/send.o: source/send.c source/candev/node.h source/path.h
	gcc ${CF} -c -o $@ $<

build/can-recv: build/recv.o build/path.o build/candev/node_init.o build/candev/node_use.o 
	gcc ${LF} -o $@ $^

build/recv.o: source/recv.c source/candev/node.h source/path.h
	gcc ${CF} -c -o $@ $<

build/path.o: source/path.c source/path.h
	gcc ${CF} -c -o $@ $<

build/candev/node_init.o: source/candev/node_init.c
	gcc ${CF} -c -o $@ $<
	
build/candev/node_use.o: source/candev/node_use.c
	gcc ${CF} -c -o $@ $<

curve: build/curve

build/curve: source/curve.c
	gcc -o $@ $^
