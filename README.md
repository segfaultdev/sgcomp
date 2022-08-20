# sgcomp

TODO: make beautiful readme :D

## SPI bus

PA0: 

## PS/2 bus

- we can use the PS/2 clock directly, feeding it into CB1 and PB6 at the SAME TIME, while feeding the data to CB2
- for this we use both the shift register in external input mode and the timer in pulse counting mode

initialization setps:
- timer 2 is set to count 11 negative-going pulses in PB6
- read SR to clear the shift register counter

after shift register interrupt:
- save SR byte somewhere in memory where it's safe

after timer 2 interrupt:
- save SR byte somewhere else in memory where it's safe
- process the scancode from the two saved bytes into a proper value and save to PS/2 queue
- reset timer 2 to count for 11 more pulses (no need to clear the shift register, it has already been done when saving SR somewhere)
