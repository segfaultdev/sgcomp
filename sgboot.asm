.org 0xF800

.set cursor_x, 0x4000
.set cursor_y, 0x4001
.set back_color, 0x4002

.set reg_orb, 0xF000
.set reg_ora, 0xF001
.set reg_ddrb, 0xF002
.set reg_ddra, 0xF003

start:
  lds #0x5000
  ldx #0xF400
  ldd #0x0F0E
  std 0, x
  ldd #0x0D0C
  std 2, x
  lda #0x55
  sta back_color
  bsr clear_screen
  ldd #0x0000
  std cursor_x
  ldx #welcome_str
  bsr print_string
  ldd #0xACD0
  std reg_ddrb
  ldd #0xFFFF
  std reg_orb
  ldd #0x0001
  std cursor_x
  ldd reg_ddrb
  cmpd #0xACD0
  bne via_error
  ldx #via_success_str
  bsr print_string
.0:
  ldd #0x33AA
  std reg_orb
  lda #50
  bsr delay_cents
  ldd #0xCC55
  std reg_orb
  lda #50
  bsr delay_cents
  bra .0

via_error:
  ldx #via_error_str
  bsr print_string
.0:
  bra .0

; a: cents of a second to wait for
delay_cents:
  ldb #0xD7 ; crude calibration, maybe use the 6522's timers?
  mul
.0:
  cmpd #0x0000
  beq .1
  exg d, x
  ldy [0, x]
  exg d, x
  ldx [0, y]
  subd #0x0001
  bra .0
.1:
  rts

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
  ora back_color
  sta 0x0000, y
  lda 1, x
  ora back_color
  sta 0x0040, y
  lda 2, x
  ora back_color
  sta 0x0080, y
  lda 3, x
  ora back_color
  sta 0x00C0, y
  lda 4, x
  ora back_color
  sta 0x0100, y
  lda 5, x
  ora back_color
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

welcome_str:
  .byte "SGBOOT r01, by segfaultdev", 0x00
via_success_str:
  .byte "MOS6522 found!", 0x00
via_error_str:
  .byte "MOS6522 not found!", 0x00

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
