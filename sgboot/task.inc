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
  pshs x
  ldx #sgboot_task_list
.0:
  cmpx #0x0C00
  bhs .1
  clr 0x0A, x
  leax 0x20, x
  bra .0
.1:
  clr sgboot_syscall_level
  puls x
  rts

; task_create: create a task and return its id
; 
; - x: task name (padded with NULL bytes to 8 bytes total)
; returns:
; - a: task id (0-31)
task_create:
  pshs b, x, y
  
  ; weird satanic magic to find a free task
  pshs x
  ldx #sgboot_task_list
  clra
.0:
  ldb 0x0A, x
  bitb 0b00000001
  beq .1
  inca
  leax 0x20, x
  bra .0
.1:
  incb
  stb 0x0A, x
  
  ; set the task pointer to 0x0800!
  ldb #0x08
  stb 0x08, x
  clr 0x09, x
  
  ; can't wait until someone optimizes this 8-byte copy to use less bytes, yay!
  puls y
  ldu , y++
  stu , x++
  ldu , y++
  stu , x++
  ldu , y++
  stu , x++
  ldu , y++
  stu , x++
  
  ; oh god, please forgive me
  clr 0x03, x
  clr 0x06, x
  clr 0x09, x
  clr 0x0C, x
  clr 0x0F, x
  clr 0x12, x
  clr 0x15, x
  
  ; restore all the registers :p
  puls b, x, y
  rts

; task_switch: load the appropiate page map, stack and registers for a given task ID
;
; - a: task ID (0-31)
task_switch:
  ; save new task ID in memory
  sta sgboot_task_id
  
  ; load task ID into d, shift left 5 times and add the starting address of the task list
  tfr a, b
  clra
  lslb
  lslb
  lslb
  lslb
  adca #0x00
  lsla
  lslb
  adca #0x00
  addd #sgboot_task_list
  tfr d, x
  
  ; now x contains the proper address we need
  ; TODO: load the page map and get all swapped pages from the SD card
  
  lds 0x08, x
  puls cc, d, dp, x, y, u, pc ; do the magic return in 6809 assembly

; task_map: maps a page to a task
; 
; - a (bit 0-4): task id (0-31)
; - a (bit 5-7): virtual address / 8192
; - b (bit 0): 1 if allocated (will ignore physical address then)
; - b (bit 1): 1 if swappable (bit 1 has to be 1)
; - b (bit 2-3): MUST be clear
; - b (bit 4-7): physical address / 8192
task_map:
  pshs d, x
  pshs d
  
  ; load task ID into d, shift left 5 times and add the starting address of the task list
  tfr a, b
  clra
  lslb
  lslb
  lslb
  lslb
  adca #0x00
  lsla
  lslb
  adca #0x00
  addd #(sgboot_task_list + 11)
  tfr d, x
  
  ; pop old a into b, to get the virtual address and add it as an offset to x
  puls b
  lsrb
  lsrb
  lsrb
  lsrb
  lsrb
  lda #0x03
  mul
  abx
  
  ; pop old b into b, to actually write the contents to its right place
  puls b
  tfr b, a
  lsrb
  lsrb
  lsrb
  lsrb
  anda #0x0F
  sta , x+
  
  ; TODO: if "is allocated" is set, load b with the first free page found right here
  
  clra
  std 0x00, x
  
  ; restore all the registers!
  puls d, x
  rts

task_unmap:
  ; TODO
  rts
