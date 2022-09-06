; syscall:
; | offset | size | description                   |
; | 0      | 11   | name (padded with NULL bytes) |
; | 11     | 1    | task to jump to               |
; | 12     | 2    | name hash                     |
; | 14     | 2    | virtual address to jump to    |

; tiny note: tasks with syscalls must be destroyed in the opposite order they were created, as
; destroying a task with loaded syscalls will set the syscall count to whatever it was set to
; prior to it being created

; syscall_init: initialize the syscall table
; 
syscall_init:
  pshs a
  clra
  sta sgboot_syscall_level
  sta sgboot_syscall_count
  puls a
  rts

; syscall_handle: syscall interrupt handler
; 
; - d: syscall name hash
syscall_handle:
  ; all registers have already been pushed to the stack, perfect!
  ; we should free up d for now though
  tfr d, u
  
  ; let's just map the sgboot page in a slightly hacky way
  lda #0x0D
  sta (sgcomp_page_table + 1)
  
  ; now QUICK, INCREMENT OUR SYSCALL NESTING LEVEL!
  inc sgboot_syscall_level
  
  ; we'll also push the current task ID to be able to recover it later
  lda sgboot_task_id
  pshs a
  
  ; next, let's just try finding the syscall
  ldx #sgboot_syscall_list
  clra
  
.0:
  cmpa sgboot_syscall_count
  beq .2
  cmpu 0x0C, x
  bne .1
  
  ; call the actual syscall handler!
  lda 0x0B, x
  ldx 0x0E, x
  bsr task_call
  
  ; save return value (inside d)
  std 0x02, s
  
  bra .2
  
.1:
  leax 0x10, x
  inca
  bra .0
  
.2:
  ; let's decrement the nesting level before exiting...
  dec sgboot_syscall_level
  
  ; and now we'll return to the caller using task_switch
  puls a
  bra task_switch
