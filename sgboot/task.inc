; task:
; 0x0000   - 0x0800    -> stack
; 0x0800   - PROG_END  -> loaded program
; PROG_END - BSS_END   -> bss

; task strucutre:
; | offset | size  | description                           |
; | 0      | 8     | task name (padded with NULL bytes)    |
; | 8      | 2     | virtual stack address                 |
; | 10     | 1     | flags(bitmask): 1=present, 2=sleeping |
; | 11     | 7 * 3 | pagemap                               |

; page structure:
; | offset | size | description                                                 |
; | 0      | 1    | type(bitmask): 1=allocated, 2=swappable, 4=swapped          |
; | 1      | 2    | address / 8192 (inside swap if swapped, physical otherwise) |

; task_init: initialize the task manager, alongside all its 32 wonderful tasks
; 
task_init:
  pshs b
  ldd #0x2020
  ldx #(sgboot_task_list + 0x0A)
.0:
  cmpa #0x00
  beq .1
  clr 0, x
  abx ; hey abbix
  deca
  bra .0
.1:
  puls b
  rts

; task_create: create a task and return its id
; 
; - x: task name (padded with NULL bytes to 8 bytes total)
; returns:
; - a: task id (0-31)
task_create:
  ; TODO
  rts

; task_switch: load the appropiate page map and stack for a given task id
;
; - a: task id (0-31)
; returns:
; - s: new stack (i mean, of course it does)
task_switch:
  ; TODO
  ; (we don't expect to return back, duh)

; task_map: maps a page to a task
; 
; - a (bit 0-4): task id (0-31)
; - a (bit 5-7): virtual address / 8192
; - b (bit 0): 1 if allocated (will ignore physical address then)
; - b (bit 1): 1 if swappable (bit 1 has to be 1)
; - b (bit 4-7): physical address / 8192
task_map:
  ; TODO
  rts

task_unmap:
  ; TODO
  rts
