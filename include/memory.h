#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>

#define MEMORY_SIZE 4096     // Max index: 0xfff
#define FONT_OFFSET 0x50     // Address for font data
#define PROGRAM_OFFSET 0x200 // Address for ROM program

#define STACK_SIZE 16

typedef struct Stack {
  uint16_t *base_ptr;
  uint16_t *top_ptr;
} Stack;

typedef struct Registers {
  uint8_t VX[16];
  uint16_t I;
} Registers;

/*----------Memory----------*/

uint8_t *init_memory(void);
uint8_t read_byte_from_memory(uint8_t *memory, uint16_t index);
void load_rom(uint8_t *memory, char *rom_path);

/*----------Stack----------*/

Stack *init_stack(void);
bool is_stack_empty(Stack *stack);
bool is_stack_full(Stack *stack);
void push_stack(Stack *stack, uint16_t value);
uint16_t pop_stack(Stack *stack);

/*---------Register---------*/

Registers *init_registers(void);

#endif // MEMORY_H