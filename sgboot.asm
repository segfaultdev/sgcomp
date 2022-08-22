.org 0xF800

.set cursor_x, 0x4000
.set cursor_y, 0x4001
.set back_color, 0x4002

.set ps2_temp, 0x4003
.set ps2_stack, 0x4004
.set ps2_state, 0x4006

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
  ldd #0x0F0E
  std 0xF400
  ldd #0x0D0C
  std 0xF402
  ldd #0x5800
  std ps2_stack
  lda #0b00000001
  sta ps2_state
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
.0:
  ldu ps2_stack
  cmpu #0x5800
  beq .0
  pulu a
  stu ps2_stack
  bsr print_char
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

; a: byte to flip (ps.: hi bitflip!)
bit_flip:
  pshs b
  ldb #0x00
  bita #0b00000001
  beq .0
  orb #0b10000000
.0:
  bita #0b00000010
  beq .1
  orb #0b01000000
.1:
  bita #0b00000100
  beq .2
  orb #0b00100000
.2:
  bita #0b00001000
  beq .3
  orb #0b00010000
.3:
  bita #0b00010000
  beq .4
  orb #0b00001000
.4:
  bita #0b00100000
  beq .5
  orb #0b00000100
.5:
  bita #0b01000000
  beq .6
  orb #0b00000010
.6:
  bita #0b10000000
  beq .7
  orb #0b00000001
.7:
  tfr b, a
  puls b
  rts

; a: new scancode
ps2_update:
  pshs b, u
  cmpa #0xF0
  beq .1
  ; TODO: perform scancode translation, though not needed for the emulator *as of now*
  ldb ps2_state
  bitb #0b00000001
  beq .0
  ldu ps2_stack
  pshu a
  stu ps2_stack
.0:
  lda #0b00000001
  sta ps2_state
  bra .2
.1:
  lda ps2_state
  anda #0b11111110
  sta ps2_state
.2:
  puls b, u
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
  lsla
  anda #0b11000000
  sta ps2_temp
  puls a
  rti
.0:
  bita #0b00100000 ; timer 2 interrupt, means we just read the last bit of the 11 total
  beq .1
  lda via_sr
  lsra
  lsra
  ora ps2_temp
  bsr bit_flip
  bsr ps2_update
  lda #11
  sta via_t2_latch
  clra
  sta via_t2_count_high
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
