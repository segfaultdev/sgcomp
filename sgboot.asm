.org 0xF800

.set cursor_x, 0x4000
.set cursor_y, 0x4001
.set back_color, 0x4002

.set ps2_first, 0x4003
.set ps2_second, 0x4004

.set via_orb, 0xF000
.set via_ora, 0xF001
.set via_ddrb, 0xF002
.set via_ddra, 0xF003

.set via_t2_latch, 0xF008
.set via_t2_count_low, 0xF008
.set via_t2_count_high, 0xF009

.set via_sr, 0xF00A
.set via_acr, 0xF00B
.set via_ifr, 0xF00D
.set via_ier, 0xF00E

start:
  lds #0x6000
  ldx #0xF400
  ldd #0x0F0E
  std 0, x
  ldd #0x0D0C
  std 2, x
  lda #0x55
  sta back_color
  bsr clear_screen
  bsr via_init
  ldd #0x0000
  std cursor_x
  ldx #welcome_str
  bsr print_string
  ldd #0x0002
  std cursor_x
  lda #0x39
  bsr print_hex
.0:
  ldd cursor_x
  pshs a, b
  ldd #0x2F23
  std cursor_x
  lda #'#'
  bsr print_char
  puls a, b
  std cursor_x
  lda #50
  bsr delay_cents
  ldd cursor_x
  pshs a, b
  ldd #0x2F23
  std cursor_x
  lda #'_'
  bsr print_char
  puls a, b
  std cursor_x
  lda #50
  bsr delay_cents
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

print_hex:
  pshs b
  tfr a, b
  andb #0x0F
  lsra
  lsra
  lsra
  lsra
  adda #'0'
  cmpa #':'
  blo .0
  adda #('A' - ':')
.0:
  pshs b
  bsr print_char
  puls a
  adda #'0'
  cmpa #':'
  blo .1
  adda #('A' - ':')
.1:
  bsr print_char
  puls b
  rts

; initializes the via: set timer 2's counter to 11, clear SR, etc.
via_init:
  lda #0b00101100
  sta via_acr
  lda #11
  sta via_t2_latch
  clra
  sta via_t2_count_high
  lda via_sr
  lda #0b01111111
  sta via_ier
  sta via_ifr
  lda #0b10100100
  sta via_ier
  andcc #0b10101111
  rts
  
via_handle:
  pshs a
  lda via_ifr
  bita #0b00000100 ; shift register overflow, means we've read the first 8 bits
  beq .0
  lda via_sr
  sta ps2_first
  puls a
  rti
.0:
  bita #0b00100000 ; timer 2 interrupt, means we just read the last bit of the 11 total
  beq .1
  lda via_sr
  sta ps2_second
  lda #11
  sta via_t2_latch
  clra
  sta via_t2_count_high
  
  pshs a, b, x, y
  lda ps2_first
  bsr print_hex
  lda #','
  bsr print_char
  lda ps2_second
  bsr print_hex
  lda #' '
  bsr print_char
  puls a, b, x, y
  
  puls a
  rti
.1:
  puls a
  rti ; generic handler in case its not a PS/2 related thing

welcome_str:
  .byte "SGBOOT r01, by segfaultdev", 0x00

font_bin:
  .incbin font.bin

.balign 0xFFF0, 0x00

.2byte 0x0000     ; reserved
.2byte 0x0000     ; software interrupt 3
.2byte 0x0000     ; software interrupt 2
.2byte via_handle ; fast interrupt request
.2byte 0x0000     ; interrupt request
.2byte 0x0000     ; software interrupt 1
.2byte 0x0000     ; non-maskable interrupt
.2byte start      ; reset
