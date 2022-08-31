; sgboot microkernel - by segsy uwu

.org 0xF800

.equ sgcomp_page_table, 0xF400

.equ sgboot_init_stack,    0x0800
.equ sgboot_task_list,     0x0800
.equ sgboot_syscall_list,  0x0C00
.equ sgboot_task_id,       0x1000
.equ sgboot_syscall_level, 0x1001

.macro ldd_task_map task_id, is_allocated, is_swappable, virtual, physical=0x00000
  ldd #((\task_id * 256) | (\virtual & 0xE000) | \is_allocated | (\is_swappable * 2) | ((\physical & 0x1E000) / 512))
.endm

sgboot_entry:
  ; set up a single 8 KiB page at 0x0000, we'll properly set this in a sec
  lda #0x0D
  sta sgcomp_page_table
  
  ; initialize the temporal stack pointer
  lds #sgboot_init_stack
  
  ; initialize the task list
  bsr task_init
  
  ; make a task for the kernel, using the third page as before
  ldx #sgboot_task_name
  bsr task_create
  
  ldd_task_map 0x00, 0, 0, 0x0000, 0x04000
  bsr task_map
  ldd_task_map 0x00, 0, 0, 0xA000, 0x00000
  bsr task_map
  ldd_task_map 0x00, 0, 0, 0xC000, 0x02000
  bsr task_map
  
  ; fill the task stack with all the cute stuff we want
  ldu (sgboot_task_list + 0x08)
  ldx #sgboot_task
  pshu x
  ldx #0x0000
  pshu x
  pshu x
  pshu x
  pshu x
  pshu x
  
  ; save the new stack pointer
  stu (sgboot_task_list + 0x08)
  
  ; finally, jump to the newly created task
  clra
  bra task_switch

sgboot_task:
  
  
  ; initialize the timer (don't worry, the timer won't switch tasks as long as we are in task 0)
  bsr timer_init
  
  ; initialize the syscall list
  bsr syscall_init
  
  ; TODO: setup filesystem syscalls and load init task
  
  ; (ok let's say the task ID is in a)
  ; switch to the init task, starting schedy too!
  bra task_switch

sgboot_task_name:
  .byte "(sgboot)"

.include "syscall.inc"
.include "timer.inc"
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
