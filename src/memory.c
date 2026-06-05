#include "memory.h"
#include "utils.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t *init_memory(void) {

  uint8_t *memory = calloc(MEMORY_SIZE, sizeof(uint8_t));
  if (memory == NULL) {
    PANIC("Init memory failed!");
  }

  // write in fonts data
  uint8_t fonts[] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  size_t font_count = 16;
  size_t font_size = 5;

  memcpy(memory + FONT_OFFSET, fonts, sizeof(uint8_t) * font_count * font_size);
  return memory;
}

uint8_t read_byte_from_memory(uint8_t *memory, uint16_t index) {
  if (index > 0x0fff) {
    PANIC("Invalid memory access (index out of bound)!");
  }
  return memory[index];
}

void load_rom(uint8_t *memory, char *rom_path) {
  FILE *file = fopen(rom_path, "rb");
  if (file == NULL) {
    PANIC("Fail to open rom file %s!", rom_path);
  }

  fseek(file, 0, SEEK_END);
  unsigned long file_size = ftell(file);
  rewind(file);

  uint8_t *buffer = malloc(file_size);
  if (buffer == NULL) {
    fclose(file);
    PANIC("Fail to allocate buffer for rom %s!", rom_path);
  }

  size_t bytes_read = fread(buffer, 1, file_size, file);
  if (bytes_read != file_size) {
    printf("Fail to read rom file!\n %ld bytes are expected, but only %zu "
           "bytes were read\n",
           file_size, bytes_read);
  } else {
    memcpy(memory + PROGRAM_OFFSET, buffer, file_size);
  }

  free(buffer);
  fclose(file);
}

Stack *init_stack(void) {
  Stack *stack = malloc(sizeof(Stack));
  uint16_t *stack_ptr = calloc(STACK_SIZE, sizeof(uint16_t));
  if (stack_ptr == NULL) {
    PANIC("Init stack failed!");
  }
  stack->base_ptr = stack_ptr;
  stack->top_ptr = stack_ptr;
  return stack;
}

bool is_stack_empty(Stack *stack) {
  return (stack->top_ptr == stack->base_ptr);
}

bool is_stack_full(Stack *stack) {
  return (stack->top_ptr == (stack->base_ptr + STACK_SIZE));
}

void push_stack(Stack *stack, uint16_t entry) {
  if (is_stack_full(stack)) {
    PANIC("Stack Overflow!");
  } else {
    *(stack->top_ptr) = entry;
    stack->top_ptr++;
  }
}

uint16_t pop_stack(Stack *stack) {
  if (is_stack_empty(stack)) {
    PANIC("Stack Underflow!");
  } else {
    stack->top_ptr--;
    return *(stack->top_ptr);
  }
}

Registers *init_registers(void) {
  Registers *registers = malloc(sizeof(Registers));
  if (registers == NULL) {
    PANIC("Init registers failed!");
  }

  memset(registers, 0, 18 * sizeof(uint8_t)); // TODO: check if this works

  return registers;
}
