; syscall:
; | offset | size | description                   |
; | 0      | 11   | name (padded with NULL bytes) |
; | 11     | 1    | task to jump to               |
; | 12     | 2    | name hash                     |
; | 14     | 2    | virtual address to jump to    |

; syscall_init: initialize the syscall table
; 
syscall_init:
  rts
