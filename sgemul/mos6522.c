#include <mos6522.h>
#include <stdint.h>
#include <stdio.h>

void mos6522_reset(mos6522_t *via) {
  via->ddrb = 0x00;
  via->ddra = 0x00;
  via->ier = 0x00;
  
  via->t1_count = 0x0000;
  via->t2_count = 0x0000;
  
  via->t2_trigger = 0;
  via->sr_mod = 0;
}

int mos6522_tick(mos6522_t *via) {
  if (via->sr_mod == 8 && !(via->ifr & 0b00000100)) {
    // printf("generating shift register FIRQ!\n");
    
    via->ifr |= 0b00000100;
    if (via->ier & 0b00000100) return 1;
  }
  
  if (via->t2_trigger && !(via->ifr & 0b00100000)) {
    via->t2_trigger = 0;
    // printf("generating timer 2 FIRQ!\n");
    
    via->ifr |= 0b00100000;
    if (via->ier & 0b00100000) return 1;
  }
  
  via->sr_mod &= 7;
  return 0;
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
  if (addr == 0x0A) {via->sr_mod = 0; via->ifr &= 0b11111011; return via->sr;}
  if (addr == 0x0B) return via->acr;
  if (addr == 0x0C) return via->pcr;
  if (addr == 0x0D) return via->ifr;
  if (addr == 0x0E) return via->ier;
  if (addr == 0x0F) return (via->ora & via->ddra) | (via->ira & ~via->ddra);
  
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
  if (addr == 0x09) {via->t2_count = (uint16_t)(via->t2_latch) | ((uint16_t)(data) << 8); via->ifr &= 0b11011111;}
  if (addr == 0x0A) {via->sr_mod = 0; via->sr = data; via->ifr &= 0b11111011;}
  if (addr == 0x0B) via->acr = data;
  if (addr == 0x0C) via->pcr = data;
  if (addr == 0x0D) via->ifr &= ~data;
  if (addr == 0x0F) via->ora = data;
  
  if (addr == 0x0E) {
    if (data & 0x80) {
      via->ier |= (data & 0x7F);
    } else {
      via->ier &= ~(data & 0x7F);
    }
  }
}

void mos6522_input(mos6522_t *via, int pin, int data) {
  if (pin == 10 + 6 && via->irb & 0x40 && !data) {
    via->t2_count--;
    if (!via->t2_count) via->t2_trigger = 1;
  }
  
  if (pin == 18 + 0 && via->cb1 && !data) {
    via->sr = (via->sr << 1) | via->cb2;
    via->sr_mod++;
  }
  
  if (pin >= 0 && pin < 8) {
    if (!(via->ddra & (1 << pin))) {
      via->ira = ((via->ira & ~(1 << (pin - 0))) | (data << (pin - 0)));
    }
  } else if (pin >= 8 && pin < 10) {
    if (pin == 8 + 0) via->ca1 = data;
    else via->ca2 = data;
  } else if (pin >= 10 && pin < 18) {
    if (!(via->ddrb & (1 << pin))) {
      via->irb = ((via->irb & ~(1 << (pin - 10))) | (data << (pin - 10)));
    }
  } else if (pin >= 18 && pin < 20) {
    if (pin == 18 + 0) via->cb1 = data;
    else via->cb2 = data;
  }
}
