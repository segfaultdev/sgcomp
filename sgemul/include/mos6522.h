#ifndef __MOS_6522_H__
#define __MOS_6522_H__

#include <stdint.h>

typedef struct mos6522_t mos6522_t;

struct mos6522_t {
  uint8_t ddra, ora, ira;
  uint8_t ddrb, orb, irb;
  
  uint16_t t1_count, t1_latch;
  
  uint16_t t2_count;
  uint8_t t2_latch;
  int t2_trigger;
  
  uint8_t acr, pcr, ifr, ier;
  
  uint8_t sr;
  int sr_mod;
  
  int ca1, ca2, cb1, cb2;
};

void mos6522_reset(mos6522_t *via);
int  mos6522_tick(mos6522_t *via); // return 1 if interrupt

uint8_t mos6522_read(mos6522_t *via, uint16_t addr);
void    mos6522_write(mos6522_t *via, uint16_t addr, uint8_t data);

void mos6522_input(mos6522_t *via, int pin, int data); // 0-7 -> port A, 8-9 -> control A, 10-17 -> port B, 18-19 -> control B

#endif
