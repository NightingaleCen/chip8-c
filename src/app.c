#include <stdlib.h>
#include <time.h>

#include "emulator.h"
#include "utils.h"

#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

static const struct timespec sleep_duration = {
    .tv_sec = 0, .tv_nsec = ONE_SEC_IN_NS / OP_PER_SEC};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
  /*Set up the components*/
  AppState *ctx = malloc(sizeof(AppState));
  if (ctx == NULL) {
    SDL_Log("Couldn't allocate app state!: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  ctx->memory = init_memory();
  ctx->registers = init_registers();
  ctx->stack = init_stack();
  ctx->display = init_display();
  ctx->delay_timer = init_timer(TIMER_WITHOUT_SOUND);
  ctx->sound_timer = init_timer(TIMER_WITH_SOUND);

  if (argc != 2) {
    PANIC("You should specify ONE rom file to load!");
    return SDL_APP_FAILURE;
  }

  load_rom(ctx->memory, argv[1]);
  ctx->PC = 0x200;

  /*Set up random number seed*/
  srand((unsigned)time(NULL));

  /*Create the window*/
  if (!SDL_CreateWindowAndRenderer("CHIP-8", DISPLAY_WIDTH * DISPLAY_PIXEL_SIZE,
                                   DISPLAY_HEIGHT * DISPLAY_PIXEL_SIZE,
                                   SDL_WINDOW_BORDERLESS, &(ctx->window),
                                   &(ctx->display->renderer))) {
    SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  *appstate = ctx;

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
  (void)appstate;
  if ((event->type == SDL_EVENT_KEY_DOWN &&
       event->key.scancode == SDL_SCANCODE_ESCAPE) ||
      event->type == SDL_EVENT_QUIT) {
    return SDL_APP_SUCCESS; /* end the program, reporting success to the OS. */
  }
  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
  AppState *ctx = appstate;

  /*----------Fetch----------*/
  uint16_t instruction = fetch_instruction(ctx);

  /*----------Decode---------*/
  DecodedInstruction decoded_instruction = decode_instruction(instruction);

  /*----------Execute--------*/
  exec_instruction(ctx, decoded_instruction);

  nanosleep(&sleep_duration, NULL);

  return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
  AppState *ctx = appstate;
  if (ctx != NULL) {

    SDL_DestroyWindow(ctx->window);
    destory_display(ctx->display);

    stop_timer(ctx->sound_timer);
    stop_timer(ctx->delay_timer);
    free(ctx->stack);
    free(ctx->registers);
    free(ctx->memory);
    free(ctx);
  }

  (void)result; // we don't really use this
}