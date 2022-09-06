#!/usr/bin/sh

gcc $(find . -name "*.c") -Llib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Iinclude -Ofast -s -o sgemul
