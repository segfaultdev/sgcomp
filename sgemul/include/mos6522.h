#ifndef __MOS_6522_H__
#define __MOS_6522_H__

#include <stdint.h>

typedef struct mos6522_t mos6522_t;

struct mos6522_t {
  uint8_t ddra, ora, ira;
  uint8_t ddrb, orb, irb;
};

void mos6522_reset(mos6522_t *via);
int  mos6522_tick(mos6522_t *via); // return 1 if interrupt

uint8_t mos6522_read(mos6522_t *via, uint16_t addr);
void    mos6522_write(mos6522_t *via, uint16_t addr, uint8_t data);

#endif
