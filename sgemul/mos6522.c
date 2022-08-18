#include <mos6522.h>
#include <stdint.h>
#include <stdio.h>

void mos6522_reset(mos6522_t *via) {
  via->ddrb = 0x00;
  via->ddra = 0x00;
  via->ier = 0x00;
}

int mos6522_tick(mos6522_t *via) {
  return 0; // TODO
}

uint8_t mos6522_read(mos6522_t *via, uint16_t addr) {
  if (addr == 0x00) return (via->orb & via->ddrb) | (via->irb & ~via->ddrb);
  if (addr == 0x01) return (via->ora & via->ddra) | (via->ira & ~via->ddra);
  if (addr == 0x02) return via->ddrb;
  if (addr == 0x03) return via->ddra;
  if (addr == 0x04) return (uint8_t)(via->t1_count >> 0);
  if (addr == 0x05) return (uint8_t)(via->t1_count >> 8);
  if (addr == 0x06) return (uint8_t)(via->t1_latch >> 0);
  if (addr == 0x07) return (uint8_t)(via->t1_latch >> 8);
  if (addr == 0x08) return (uint8_t)(via->t2_count >> 0);
  if (addr == 0x09) return (uint8_t)(via->t2_count >> 8);
  if (addr == 0x0A); // TODO: shift register
  if (addr == 0x0B) return via->acr;
  if (addr == 0x0C) return via->pcr;
  if (addr == 0x0D) return via->ifr;
  if (addr == 0x0E) return via->ier;
  if (addr == 0x0F) return via->ira;
  
  printf("warning: 6522 register 0x%02X not implemented!\n", addr);
  return 0x00;
}

void mos6522_write(mos6522_t *via, uint16_t addr, uint8_t data) {
  if (addr == 0x00) via->orb = data;
  if (addr == 0x01) via->ora = data;
  if (addr == 0x02) via->ddrb = data;
  if (addr == 0x03) via->ddra = data;
  if (addr == 0x04) via->t1_latch = (via->t1_latch & 0xFF00) | ((uint16_t)(data) << 0);
  if (addr == 0x05) via->t1_count = (via->t1_count & 0x00FF) | ((uint16_t)(data) << 8);
  if (addr == 0x06) via->t1_latch = (via->t1_latch & 0xFF00) | ((uint16_t)(data) << 0);
  if (addr == 0x07) via->t1_latch = (via->t1_latch & 0x00FF) | ((uint16_t)(data) << 8);
  if (addr == 0x08) via->t2_latch = data;
  if (addr == 0x09) via->t2_count = (via->t2_count & 0x00FF) | ((uint16_t)(data) << 8);
  if (addr == 0x0A) printf("warning: 6522 register 0x0A (shift register) not implemented!\n", addr);
  if (addr == 0x0B) via->acr = data;
  if (addr == 0x0C) via->pcr = data;
  if (addr == 0x0D) via->ifr = data;
  if (addr == 0x0E) via->ier = data;
  if (addr == 0x0F) via->ora = data;
}
