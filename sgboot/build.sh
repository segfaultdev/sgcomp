#!/usr/bin/sh

../vbcc/bin/vbcchc12 -cpu=6809 -Iinclude -size -final -c99 -unsigned-char -merge-strings sgboot.c -o=sgboot.asm
