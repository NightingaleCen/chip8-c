#include "emulator.h"
#include "utils.h"

#include <stdlib.h>

uint16_t fetch_instruction(AppState *ctx) {
  uint16_t first_byte = read_byte_from_memory(ctx->memory, ctx->PC++);
  uint16_t second_byte = read_byte_from_memory(ctx->memory, ctx->PC++);
  return (first_byte << 8) | second_byte;
}

DecodedInstruction decode_instruction(uint16_t instruction) {
  uint16_t indicator = (instruction & INS_MASK_INDICATOR) >> 12;
  uint16_t X = (instruction & INS_MASK_X) >> 8;
  uint16_t Y = (instruction & INS_MASK_Y) >> 4;
  uint16_t N = instruction & INS_MASK_N;
  uint16_t NN = instruction & INS_MASK_NN;
  uint16_t NNN = instruction & INS_MASK_NNN;

  InstructionType instruction_type;

  switch (indicator) {
  case 0x0: // 2 cases
    switch (instruction) {
    case 0x00E0:
      instruction_type = CLEAR_SCREEN;
      break;

    default:
      instruction_type = UNIDENTIFIED;
      break;
    }
    break;

  case 0x1:
    instruction_type = JUMP;
    break;

  case 0x6:
    instruction_type = SET_VX;
    break;

  case 0x7:
    instruction_type = ADD_TO_VX;
    break;

  case 0xa:
    instruction_type = SET_I;
    break;

  case 0xd:
    instruction_type = DISPLAY;
    break;
  default:
    instruction_type = UNIDENTIFIED;
    break;
  }

  DecodedInstruction decoded_instruction = {indicator,       X, Y, N, NN, NNN,
                                            instruction_type};
  return decoded_instruction;
}

/* Functions for executing instructions */

typedef enum ExecuteResult {
  EXEC_SUCCESS,
  EXEC_FAILURE,
} ExecuteResult;

typedef ExecuteResult (*ExecFunc)(AppState *ctx, DecodedInstruction data);

static ExecuteResult exec_clear_screen(AppState *ctx, DecodedInstruction data) {
  (void)data;

  clear_display(ctx->display);
  render_display_once(ctx->display);

  return EXEC_SUCCESS;
}

static ExecuteResult exec_jump(AppState *ctx, DecodedInstruction data) {
  ctx->PC = data.NNN; // Stays in place, no need for -= 2
  return EXEC_SUCCESS;
}

static ExecuteResult exec_set_vx(AppState *ctx, DecodedInstruction data) {
  ctx->registers->VX[data.X] = data.NN;
  return EXEC_SUCCESS;
}

static ExecuteResult exec_add_to_vx(AppState *ctx, DecodedInstruction data) {
  uint16_t X = data.X;
  uint16_t NN = data.NN;

  // detect overflow
  uint8_t result = ctx->registers->VX[X] + NN;
  if (result < NN) {
    PANIC(
        "Number overflow when ADD_TO_VX"); // TODO: use Execute Result & context
  }

  ctx->registers->VX[X] = result;
  return EXEC_SUCCESS;
}

static ExecuteResult exec_set_I(AppState *ctx, DecodedInstruction data) {
  ctx->registers->I = data.NNN;
  return EXEC_SUCCESS;
}

static ExecuteResult exec_display(AppState *ctx, DecodedInstruction data) {
  ctx->registers->VX[0xf] = 0;

  uint8_t x = ctx->registers->VX[data.X] & (DISPLAY_WIDTH - 1);
  uint8_t y = ctx->registers->VX[data.Y] & (DISPLAY_HEIGHT - 1);

  uint8_t oringinal_x = x;
  for (size_t i = 0; i < data.N; i++) { // clip at edge
    x = oringinal_x;
    if (y >= DISPLAY_HEIGHT) {
      break;
    }
    uint8_t data_byte =
        read_byte_from_memory(ctx->memory, ctx->registers->I + i);
    for (int j = 7; j >= 0; j--) {
      if (x >= DISPLAY_WIDTH) { // clip at edge
        break;
      }

      int data_bit = (data_byte >> j) & 1;
      if (data_bit) {

        uint8_t buffer_byte =
            ctx->display->display_buffer[y * DISPLAY_W_BYTES + (x / 8)];

        int buffer_bit = (buffer_byte >> (7 - x % 8)) & 1;
        switch (buffer_bit) {
        case 1:
          ctx->registers->VX[0xf] = 1;
          break;
        default:
          break;
        }

        ctx->display->display_buffer[y * DISPLAY_W_BYTES + (x / 8)] ^=
            (1 << (7 - x % 8)); // "flip" the target bit in buffer
      }
      x++;
    }
    y++;
  }

  render_display_once(ctx->display);

  return EXEC_SUCCESS;
}

static ExecuteResult exec_unidentified(AppState *ctx, DecodedInstruction data) {
  (void)ctx;
  (void)data;
  // TODO:context
  return EXEC_FAILURE;
}

/* Execution entry point */
static ExecFunc func_table[UNIDENTIFIED] = {
    [CLEAR_SCREEN] = exec_clear_screen,
    [JUMP] = exec_jump,
    [SET_VX] = exec_set_vx,
    [ADD_TO_VX] = exec_add_to_vx,
    [SET_I] = exec_set_I,
    [DISPLAY] = exec_display,
};

void exec_instruction(AppState *ctx, DecodedInstruction decoded_instruction) {

  InstructionType type = decoded_instruction.type;
  ExecuteResult exec_result;

  if (type >= 0 && type < UNIDENTIFIED && func_table[type] != NULL) {
    exec_result = func_table[type](ctx, decoded_instruction);
  } else {
    exec_result = exec_unidentified(ctx, decoded_instruction);
  }

  switch (exec_result) {
  case EXEC_SUCCESS:
    // yay!
    break;

  case EXEC_FAILURE:
    // TODO: error context
    PANIC("Execution failed!");
    break;

  default:
    PANIC("Unrecognized exec result code %d!", exec_result);
    break;
  }
}