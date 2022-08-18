#include <mos6522.h>
#include <stdint.h>

void mos6522_reset(mos6522_t *via) {
  // TODO
}

int mos6522_tick(mos6522_t *via) {
  return 0; // TODO
}

uint8_t mos6522_read(mos6522_t *via, uint16_t addr) {
  if (addr == 0x00) return via->irb;
  if (addr == 0x01) return via->ira;
  if (addr == 0x02) return via->ddrb;
  if (addr == 0x03) return via->ddra;
  
  return 0x00; // invalid!
}

void mos6522_write(mos6522_t *via, uint16_t addr, uint8_t data) {
  if (addr == 0x00) via->orb = data;
  if (addr == 0x01) via->ora = data;
  if (addr == 0x02) via->ddrb = data;
  if (addr == 0x03) via->ddra = data;
}
