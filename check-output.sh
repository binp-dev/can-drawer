#!/bin/sh

FILE='stats/spiral-recv.txt'

rm $FILE
./build/can-send &
./build/can-recv > $FILE
pkill -TERM can-send
