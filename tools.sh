#!/usr/bin/sh

cd sgemul
gcc $(find . -name "*.c") -Llib -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Iinclude -Og -g -fsanitize=address -o sgemul
cd ..
