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

void sg_fault(mc6809__t *cpu, mc6809fault__t fault) {
  if (fault == MC6809_FAULT_INTERNAL_ERROR) {
    printf("sgemul: Internal error!\n\n");
  } else if (fault == MC6809_FAULT_INSTRUCTION) {
    printf("sgemul: Unknown opcode!\n\n");
  } else if (fault == MC6809_FAULT_ADDRESS_MODE) {
    printf("sgemul: Invalid address mode!\n\n");
  } else if (fault == MC6809_FAULT_EXG) {
    printf("sgemul: Invalid EXG operand!\n\n");
  } else if (fault == MC6809_FAULT_TFR) {
    printf("sgemul: Invalid TFR operand!\n\n");
  } else if (fault == MC6809_FAULT_user) {
    printf("sgemul: Breakpoint!\n\n");
  }
  
  printf("PC=0x%04X, D=0x%04X, X=0x%04X, Y=0x%04X, S=0x%04X, U=0x%04X\n\n", cpu->pc, cpu->d.w, cpu->X, cpu->Y, cpu->S, cpu->U);
  
  printf("S stack: ");
  
  for (int i = 0; i < 32; i++) {
    uint8_t byte = sg_read(cpu, cpu->S.w + i, 0);
    printf("%02X", byte);
  }
  
  printf("\nU stack: ");
  
  for (int i = 0; i < 32; i++) {
    uint8_t byte = sg_read(cpu, cpu->U.w + i, 0);
    printf("%02X", byte);
  }
  
  printf("\n\n");
  
  printf("Page table:\n");
  
  for (int i = 0; i < 7; i++) {
    printf("- 0x%04X -> 0x%05X\n", i * 0x2000, ((buffer_page[i] & 0x0F) ^ 0x0F) * 0x2000);
  }
  
  if (fault != MC6809_FAULT_user) {
    exit(1);
  }
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
  cpu.fault = sg_fault;
  
  mos6522_reset(&via);
  mc6809_reset(&cpu);
  
  int blob_size = (192 * SCALE_X) / 16;
  
  SetTraceLogLevel(LOG_NONE);
  InitWindow(192 * SCALE_X, 216 * SCALE_Y + blob_size, "sgemul");
  
  int ps2_cycle = 0;
  int cycles = 0;
  
  uint16_t ps2_queue[16];
  int ps2_reader = 0, ps2_offset = 0;
  
  uint16_t ps2_data = 0;
  int ps2_left = 0;
  
  #define ps2_send(scan) do { \
    ps2_queue[ps2_offset++] = scan; \
    ps2_offset %= 16; \
  } while (0)
  
  #define ps2_press(scan) do { \
    ps2_send(scan); \
  } while (0)
  
  #define ps2_release(scan) do { \
    ps2_send(0xF0); \
    ps2_send(scan); \
  } while (0)
  
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
      int in_y_bounds = (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && GetMouseY() >= 216 * SCALE_Y && GetMouseY() < 216 * SCALE_Y + blob_size);
      
      if ((via.ddrb >> (7 - i)) & 1) {
        if ((via.orb >> (7 - i)) & 1) final_color = ORANGE;
        else final_color = BROWN;
        
        DrawRectangle((i + 0) * blob_size, 216 * SCALE_Y, blob_size, blob_size, final_color);
      } else {
        if ((via.irb >> (7 - i)) & 1) final_color = GREEN;
        else final_color = DARKGREEN;
        
        DrawRectangle((i + 0) * blob_size, 216 * SCALE_Y, blob_size, blob_size, final_color);
        
        if (in_y_bounds && GetMouseX() >= (i + 0) * blob_size && GetMouseX() < (i + 1) * blob_size) {
          mos6522_input(&via, 10 + (7 - i), 1 - ((via.irb >> (7 - i)) & 1));
        }
      }
      
      if ((via.ddra >> (7 - i)) & 1) {
        if ((via.ora >> (7 - i)) & 1) final_color = ORANGE;
        else final_color = BROWN;
        
        DrawRectangle((i + 8) * blob_size, 216 * SCALE_Y, blob_size, blob_size, final_color);
      } else {
        if ((via.ira >> (7 - i)) & 1) final_color = GREEN;
        else final_color = DARKGREEN;
        
        DrawRectangle((i + 8) * blob_size, 216 * SCALE_Y, blob_size, blob_size, final_color);
        if (in_y_bounds && GetMouseX() >= (i + 8) * blob_size && GetMouseX() < (i + 9) * blob_size) via.ira ^= (1 << (7 - i));
      }
    }
    
    DrawFPS(6, 216 * SCALE_Y - 20);
    EndDrawing();
    
    for (;;) {
      int key = GetCharPressed();
      if (!key) break;
      
      ps2_press(key);
      ps2_release(key);
    }
    
    for (int i = 0; i < 1000000 * GetFrameTime(); i++) {
      if (!ps2_left && ps2_reader != ps2_offset) {
        uint8_t data = ps2_queue[ps2_reader++];
        uint8_t parity = ((1 ^ (data >> 0) ^ (data >> 1) ^ (data >> 2) ^ (data >> 3) ^ (data >> 4) ^ (data >> 5) ^ (data >> 6) ^ (data >> 7)) & 1);
        
        ps2_data = ((uint16_t)(data) << 1) | ((uint16_t)(parity) << 9) | 0b0000010000000000;
        ps2_reader %= 16;
        
        ps2_cycle = cycles;
        ps2_left = 11;
      }
      
      if (ps2_left && (cycles - ps2_cycle) >= 2173 && ((cycles - ps2_cycle) - 2173) % 59 == 0) {
        mos6522_input(&via, 10 + 6, 0);
        mos6522_input(&via, 18 + 1, ps2_data & 1);
        mos6522_input(&via, 18 + 0, 0);
        
        ps2_data >>= 1;
        ps2_left--;
      } else {
        mos6522_input(&via, 10 + 6, 1);
        mos6522_input(&via, 18 + 0, 1);
      }
      
      if (mos6522_tick(&via)) cpu.firq = 1;
      if (cpu.cycles <= cycles || cpu.irq || cpu.firq || cpu.nmi) mc6809_step(&cpu);
      
      cycles++;
    }
  }
  
  CloseWindow();
  
  return 0;
}
