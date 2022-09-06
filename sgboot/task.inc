; task:
; 0x0000   - 0x0800    -> stack
; 0x0800   - PROG_END  -> task code and data
; PROG_END - BSS_END   -> bss
; 
; task strucutre:
; | offset | size | description                                                    |
; | 0      | 9    | task name (padded with NULL bytes)                             |
; | 9      | 1    | (bit 0-3): parent task ID, (bit 6): sleeping, (bit 7): present |
; | 10     | 4    | page table (each nibble is a page, big endian, LSB first)      |
; | 14     | 2    | virtual stack address                                          |
; 
; 0x2030 - 0x237F -> task list (last 16 bytes of every 64 byte block, as it's part of VRAM)
; 0x23B0          -> current task
; 0x2430 - 0x243D -> page allocator bitmap (0xFF if free, task ID otherwise, 14 pages total)

; task_init: initialize the task manager and page allocator
; 
task_init:
  pshs a, x
  ldx #0x2039
  clra
  sta 0x23B0
.0:
  clr 0x00, x
  leax 0x40, x
  inca
  cmpa #0x0E
  blo .0
  ldx #0x2430
  lda #0xFF
.1:
  sta , x+
  cmpx #0x243E
  blo .1
  puls a, x
  rts

; task_create: create a task and return its ID
; 
; - x: task name (padded with NULL bytes to 9 bytes total)
; returns:
; - a: task ID (0-13, or 0xFF if failed to create one)
task_create:
  pshs b, x, y
  ldx #0x2030
  clra
.0:
  ldb 0x09, x
  bitb #0x80
  beq .1
  leax 0x0040, x
  inca
  cmpa #0x0E
  blo .0
  puls b, x, y
  lda #0xFF
  rts
.1:
  ldy 0x01, s
  ; TODO: copy 9 bytes from y to x
  ldb 0x23B0
  orb #0x80
  stb 0x09, x
  ldy #0x0800
  sty 0x0E, x
  puls b, x, y
  rts

; task_push: pushes b onto a task's stack
; 
; - a: task ID
; - b: value to push
task_push:
  pshs d, x, u
  pshs b
  ldb #0x40
  mul
  addd #0x203A
  tfr d, x
  lda 0x00, x
  coma
  sta 0xF400
  ldu 0x04, x
  puls b
  pshu b
  stu 0x04, x
  puls d, x, u
  rts

; task_enter: load a page table in x and y and do the magic jump
; 
; - x: first 4 pages in page table (first low nibble of high byte, then high nibble, then same with low byte)
; - y: last 4 pages in page table (same as with x)
task_enter:
  tfr x, d
  coma
  comb
  sta 0xF400
  lsra
  lsra
  lsra
  lsra
  std 0xF401
  lsrb
  lsrb
  lsrb
  lsrb
  stb 0xF403
  tfr y, d
  coma
  comb
  sta 0xF404
  lsra
  lsra
  lsra
  lsra
  std 0xF405
  puls cc, d, dp, x, y, u, pc

; task_switch: load the appropiate page table, stack and registers for a given task ID
; 
; - a: task ID (0-13)
task_switch:
  ldb #0x40
  mul
  addd #0x203A
  tfr d, u
  pulu x, y, s
  bra task_enter

; task_call: load the appropiate page table for a given task ID, but use the registers in the stack
; 
; - a: task ID (0-13)
; - x: address to jump to
task_call:
  ; TODO
  rts

; task_map: maps a page to a task
; 
; - a: task ID (0-13)
; - b (bit 0-3): physical page
; - b (bit 4-6): virtual page
; - b (bit 7): allocate page if 1 (try finding a free page in the bitmap, and ignore the specified physical page)
task_map:
  pshs d, x
  pshs d
  ldb #0x40
  mul
  addd #0x203A
  tfr d, x
  puls d
  bitb #0x80
  beq .0
  bsr task_alloc
  andb #0x70
  pshs a
  orb 0x00, s
  puls a
.0:
  bitb #0x40
  beq .1
  leax 0x02, x
.1:
  bitb #0x20
  beq .2
  leax 0x01, x
.2:
  lda 0x00, x
  bitb #0x10
  beq .3
  lslb
  lslb
  lslb
  lslb
  anda #0x0F
  bra .4
.3:
  andb #0x0F
  anda #0xF0
.4:
  pshs b
  ora 0x00, s
  puls b
  sta 0x00, x
  puls d, x
  rts

; task_alloc: allocates a free page
; 
; - a: the task ID the page is allocated to (0-13)
; returns:
; - a: allocated page index
task_alloc:
  pshs b, x
  pshs a
  ldx #0x2430
  clra
.0:
  ldb , x+
  cmpb #0xFF
  beq .1
  inca
  bra .0
.1:
  puls b
  stb , -x
  puls b, x
  rts

; task_clean: frees all pages given a task ID, and marks it as non-present
; 
; - a: task ID (0-13)
task_clean:
  pshs d, x
  ldx #0x2430
  clrb
  comb
.0:
  cmpa 0x00, x
  bne .1
  stb 0x00, x
.1:
  leax 0x01, x
  cmpx #0x243E
  blo .0
  ldb #0x40
  mul
  addd #0x2039
  tfr d, x
  clr 0x00, x
  puls d, x
  rts
