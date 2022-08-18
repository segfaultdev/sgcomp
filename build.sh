#!/usr/bin/sh

vasm6809_std sgboot.asm -6809 -opt-branch -opt-offset -Fbin -pic -o sgboot.bin
