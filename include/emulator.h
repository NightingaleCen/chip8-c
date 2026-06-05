#ifndef EMULATOR_H
#define EMULATOR_H

#include "components.h"
#include "memory.h"

#include <stdint.h>

/*Masks for decoding instructions*/
#define INS_MASK_INDICATOR 0xf000
#define INS_MASK_X 0x0f00
#define INS_MASK_Y 0x00f0
#define INS_MASK_N 0x000f
#define INS_MASK_NN 0x00ff
#define INS_MASK_NNN 0x0fff

#define OP_PER_SEC 700 // Operations executed in one sec

typedef struct AppState {
  uint8_t *memory;
  Registers *registers;
  Stack *stack;
  Display *display;
  Timer *delay_timer;
  Timer *sound_timer;
  SDL_Window *window;
  uint16_t PC;
  // TODO: add a context member for error messages etc.
} AppState;

typedef enum InstructionType {
  CLEAR_SCREEN,
  JUMP,
  SET_VX,
  ADD_TO_VX,
  SET_I,
  DISPLAY,
  UNIDENTIFIED,
} InstructionType;

typedef struct DecodedInstruction {
  uint16_t indicator; // the first nibble
  uint16_t X;
  uint16_t Y;
  uint16_t N;
  uint16_t NN;
  uint16_t NNN;
  InstructionType type;
} DecodedInstruction;

uint16_t fetch_instruction(AppState *ctx);

DecodedInstruction decode_instruction(uint16_t instruction);

void exec_instruction(AppState *ctx, DecodedInstruction decoded_instruction);

#endif // EMULATOR_H