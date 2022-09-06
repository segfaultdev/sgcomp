; sgboot kernel

.org 0xF800

sgboot_entry:
  ldd #0x0F0E
  std 0xF401
  lds #0x4000
  
  bsr task_init
  ldx #sgboot_test_str
  bsr task_create
  
  ldb #0x60
  bsr task_map
  ldb #0x0F
  bsr task_map
  
  ldb #(sgboot_task & 255)
  bsr task_push
  ldb #(sgboot_task / 256)
  bsr task_push
  
  ldb #0x00
  bsr task_push
  bsr task_push
  bsr task_push
  bsr task_push
  bsr task_push
  bsr task_push
  bsr task_push
  bsr task_push
  bsr task_push
  bsr task_push
  
  bsr task_switch
  
.0:
  bra .0

sgboot_task:
  lda #0xFF
  sta 0xC000
  
  exg b, b
  
  bra $

sgboot_test_str:
  .byte "(sgboot)", 0x00

.include "task.inc"

.balign 0xFFF0, 0x00

.2byte 0x0000       ; reserved
.2byte 0x0000       ; software interrupt 3
.2byte 0x0000       ; software interrupt 2
.2byte 0x0000       ; fast interrupt request
.2byte 0x0000       ; interrupt request
.2byte 0x0000       ; software interrupt 1
.2byte 0x0000       ; non-maskable interrupt
.2byte sgboot_entry ; reset
