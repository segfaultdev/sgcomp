; sgboot microkernel - by segsy uwu

.org 0xF800

sgboot_entry:
  ; set up a single 8 KiB page at 0x0000
  
  
  
  bra sgboot_entry

sgboot_firq:
  rti

sgboot_syscall:
  rti

.include "task.inc"

.balign 0xFFF0, 0x00

.2byte 0x0000         ; reserved
.2byte 0x0000         ; software interrupt 3
.2byte 0x0000         ; software interrupt 2
.2byte sgboot_firq    ; fast interrupt request
.2byte 0x0000         ; interrupt request
.2byte sgboot_syscall ; software interrupt 1
.2byte 0x0000         ; non-maskable interrupt
.2byte sgboot_entry   ; reset
