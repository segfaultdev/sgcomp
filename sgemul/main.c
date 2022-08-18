#include <mos6522.h>
#include <mc6809.h>
#include <raylib.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define SCALE_X 4
#define SCALE_Y 3

uint8_t *buffer_page; // 16 bytes, but higher 4 bits are unwritable, also cannot read ):
uint8_t *buffer_ram; // 128 KiB
uint8_t *buffer_rom; // 2 KiB

mos6522_t via;
mc6809__t cpu;

uint8_t sg_read(mc6809__t *cpu, uint16_t addr, _Bool ifetch) {
  if (addr >= 0xE000) {
    addr |= 0x1000;
    
    if (addr >= 0xF800) {
      return buffer_rom[addr - 0xF800];
    } else if (addr >= 0xF400) {
      return 0x00; // we cannot read the page table in real hardware lol
    } else if (addr >= 0xF000) {
      if (addr & 0x0010) {
        return 0x00; // TODO: SN76489
      } else {
        return mos6522_read(&via, addr & 0x000F);
      }
    }
  }
  
  uint8_t real_page = (0x0F ^ (buffer_page[addr >> 13] & 0x0F));
  return buffer_ram[(addr & 0x01FFF) + (real_page << 13)];
}

void sg_write(mc6809__t *cpu, uint16_t addr, uint8_t data) {
  if (addr >= 0xE000) {
    addr |= 0x1000;
    
    if (addr >= 0xF800) {
      return; // we cannot write to ROM, lmao
    } else if (addr >= 0xF400) {
      buffer_page[addr & 0x000F] = (data & 0x0F);
    } else if (addr >= 0xF000) {
      if (addr & 0x0010) {
        return; // TODO: SN76489
      } else {
        mos6522_write(&via, addr & 0x000F, data);
      }
    }
  }
  
  uint8_t real_page = (0x0F ^ (buffer_page[addr >> 13] & 0x0F));
  buffer_ram[(addr & 0x01FFF) + (real_page << 13)] = data;
}

int main(int argc, const char **argv) {
  if (argc < 2) {
    printf("usage: %s <ROM>\n", argv[0]);
    return 1;
  }
  
  buffer_page = malloc(16);
  buffer_ram = malloc(128 * 1024);
  buffer_rom = malloc(2 * 1024);
  
  FILE *rom = fopen(argv[1], "rb");
  
  if (!rom) {
    printf("error: cannot open file '%s'\n", argv[1]);
    return 1;
  }
  
  fseek(rom, 0, SEEK_END);
  int size = ftell(rom);
  
  if (size != 2 * 1024) {
    printf("error: rom file must be exactly 2 kilobytes\n");
    return 1;
  }
  
  rewind(rom);
  fread(buffer_rom, 1, 2 * 1024, rom);
  
  fclose(rom);
  
  if (buffer_rom[2 * 1024 - 2] < 0xF8) {
    printf("warning: reset vector is outside rom\n");
  }
  
  cpu.read = sg_read;
  cpu.write = sg_write;
  
  mos6522_reset(&via);
  mc6809_reset(&cpu);
  
  int blob_size = (192 * SCALE_X) / 16;
  
  InitWindow(192 * SCALE_X, 216 * SCALE_Y + 1 * blob_size, "sgemul");
  
  int cycles = 0;
  
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);
    
    for (int i = 0; i < 216; i++) {
      for (int j = 0; j < 48; j++) {
        uint8_t data = buffer_ram[j + i * 64];
        
        for (int k = 0; k < 4; k++) {
          int color = ((data >> (2 * k)) & 3);
          
          Color final_color;
          
          if (color == 0) final_color = BLACK;
          else if (color == 1) final_color = BLUE;
          else if (color == 2) final_color = ORANGE;
          else final_color = WHITE;
          
          DrawRectangle((j * 4 + k) * SCALE_X, i * SCALE_Y, SCALE_X, SCALE_Y, final_color);
        }
      }
    }
    
    for (int i = 0; i < 8; i++) {
      Color final_color;
      
      if ((via.orb >> (7 - i)) & 1) final_color = RED;
      else final_color = BLACK;
      
      DrawRectangle((i + 0) * blob_size, 216 * SCALE_Y, blob_size, blob_size, final_color);
      
      if ((via.ora >> (7 - i)) & 1) final_color = RED;
      else final_color = BLACK;
      
      DrawRectangle((i + 8) * blob_size, 216 * SCALE_Y, blob_size, blob_size, final_color);
    }
    
    DrawFPS(10, 10);
    EndDrawing();
    
    for (int i = 0; i < 1000000 * GetFrameTime(); i++) {
      if (mos6522_tick(&via)) cpu.firq = 1;
      if (cpu.cycles <= cycles) mc6809_step(&cpu);
      
      cycles++;
    }
  }
  
  CloseWindow();
  
  return 0;
}
