; task:
; 0x0000   - 0x0800    -> stack
; 0x0800   - PROG_END  -> loaded program
; PROG_END - BSS_END   -> bss

; task strucutre:
; | offset | size  | description                           |
; | 0      | 2     | virtual stack address                 |
; | 2      | 1     | flags(bitmask): 1=present, 2=sleeping |
; | 3      | 7 * 3 | pagemap                               |

; page structure:
; | offset | size | description                                                 |
; | 0      | 1    | type(bitmask): 1=allocated, 2=swapped                       |
; | 1      | 2    | address / 8192 (inside swap if swapped, physical otherwise) |

.equ task_start 0x0000

; task_init: initialize the task manager, alongside all its 64 wonderful tasks
; 
task_init:
  ; TODO
  rts

; task_create: create a task and return its id
; 
; returns:
; - a: task id (1-64, task 0 is sgboot)
task_create:
  ; TODO
  rts

; task_switch: load the appropiate page map and stack for a given task id
;
; - a: task id (1-64, or 0 for the sgboot task)
; returns:
; - s: new stack (i mean, of course it does)
task_switch:

; task_map: maps 
