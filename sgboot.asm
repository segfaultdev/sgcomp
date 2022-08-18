.org 0xF800

.set cursor_x, 0x4000
.set cursor_y, 0x4001

start:
  lds #0x5000
  ldx #0xF400
  ldd #0x0F0E
  std 0, x
  ldd #0x0D0C
  std 2, x
  lda #0x55
  bsr clear_screen
  ldd #0x0000
  std cursor_x
  ldx #test_1_str
  bsr print_string
  ldd #0x0002
  std cursor_x
  ldx #test_2_str
  bsr print_string
  ldd #0x0003
  std cursor_x
  ldx #test_3_str
  bsr print_string
  ldd #0x0004
  std cursor_x
  ldx #test_4_str
  bsr print_string
  ldd #0x0005
  std cursor_x
  ldx #test_5_str
  bsr print_string
  ldd #0x0006
  std cursor_x
  ldx #test_6_str
  bsr print_string
.0:
  bra .0

; a: 4-pixel pattern to clear the screen with
clear_screen:
  ldb #0x01
  ldx #0x0000
.0:
  cmpx #0x3600
  bhs .1
  sta 0, x
  abx
  bra .0
.1:
  rts

; a: ascii char to print
print_char:
  ldb #0x06
  mul
  addd #font_bin
  tfr d, x
  ldb cursor_y
  lda #0x06
  mul
  lda #0x40
  mul
  addb cursor_x
  adca #0x00
  tfr d, y
  lda 0, x
  sta 0x0000, y
  lda 1, x
  sta 0x0040, y
  lda 2, x
  sta 0x0080, y
  lda 3, x
  sta 0x00C0, y
  lda 4, x
  sta 0x0100, y
  lda #0x00
  sta 0x0140, y
  lda cursor_x
  inca
  cmpa #0x30
  blo .0
  clra
  ldb cursor_y
  incb
  stb cursor_y
.0:
  sta cursor_x
  rts

; x: string to print
print_string:
  lda 0, x
  cmpa #0x00
  beq .0
  pshs x, y
  bsr print_char
  puls x, y
  ldb #0x01
  abx
  bra print_string
.0:
  rts

test_1_str:
  .byte "Hey, this is running on my own computer!", 0x00
test_2_str:
  .byte "Featuring:", 0x00
test_3_str:
  .byte "- Motorola 6809 @ 1MHz", 0x00
test_4_str:
  .byte "- Custom GPU w/ 192x216, 4 colors", 0x00
test_5_str:
  .byte "- 128 KiB RAM (16 KiB shared with GPU)", 0x00
test_6_str:
  .byte "- Dozens of hours well spent :D", 0x00

font_bin:
  .incbin font.bin

.balign 0xFFF0, 0x00

.2byte 0x0000 ; reserved
.2byte 0x0000 ; software interrupt 3
.2byte 0x0000 ; software interrupt 2
.2byte 0x0000 ; fast interrupt request
.2byte 0x0000 ; interrupt request
.2byte 0x0000 ; software interrupt 1
.2byte 0x0000 ; non-maskable interrupt
.2byte start  ; reset
