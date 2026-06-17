#include "components.h"
#include "utils.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>

#define TARGET_CYCLE_NS (ONE_SEC_IN_NS / TIMER_FREQUENCY_HZ)

typedef enum Pixel {
  BLACK,
  WHITE,
} Pixel;

/*----------Display----------*/

Display *init_display(void) {
  Display *display = malloc(sizeof(Display));
  uint8_t *display_buffer =
      calloc(sizeof(uint8_t), DISPLAY_HEIGHT * DISPLAY_W_BYTES);
  if (display_buffer == NULL || display == NULL) {
    PANIC("Init display failed!");
  }

  display->display_buffer = display_buffer;
  display->renderer = NULL;

  return display;
}

void clear_display(Display *display) {
  memset(display->display_buffer, 0,
         sizeof(uint8_t) * DISPLAY_HEIGHT * DISPLAY_W_BYTES);
}

void render_display_once(Display *display) {
  /* Draw the black background */
  SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(display->renderer);

  /* Draw the white pixels */
  SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255);
  SDL_FRect pixels[DISPLAY_HEIGHT * DISPLAY_WIDTH];
  SDL_FRect current_pixel;
  size_t n_pixel = 0;

  current_pixel.h = current_pixel.w = DISPLAY_PIXEL_SIZE;
  current_pixel.x = current_pixel.y = 0;

  for (size_t i = 0; i < DISPLAY_HEIGHT; i++) {
    for (size_t j = 0; j < DISPLAY_W_BYTES; j++) {
      uint8_t data = display->display_buffer[i * DISPLAY_W_BYTES + j];

      for (int k = 7; k >= 0; k--) {
        int bit = (data >> k) & 1;
        switch (bit) {
        case WHITE:
          pixels[n_pixel++] = current_pixel;
          break;
        default:
          break;
        }
        current_pixel.x += DISPLAY_PIXEL_SIZE;
      }
    }
    current_pixel.x = 0;
    current_pixel.y += DISPLAY_PIXEL_SIZE;
  }

  SDL_RenderFillRects(display->renderer, pixels, n_pixel);
  SDL_RenderPresent(display->renderer);
}

void destroy_display(Display *display) {
  SDL_DestroyRenderer(display->renderer);
  free(display->display_buffer);
  free(display);
}

/*-----------Timer-----------*/

Timer *init_timer(bool with_sound) {
  Timer *timer = malloc(sizeof(Timer));
  if (timer == NULL) {
    PANIC("Init timer failed!");
  }

  timer->t = 0;
  timer->with_sound = with_sound;
  timer->is_running = true;

  pthread_mutex_init(&(timer->mutex), NULL);
  pthread_create(&(timer->thread), NULL, update_timer, timer);

  return timer;
}

void *update_timer(void *arg) {
  Timer *timer = arg;
  struct timespec next_frame_time, current_time, sleep_time;
  clock_gettime(CLOCK_MONOTONIC, &next_frame_time);

  while (timer->is_running) {
    if (timer->t > 0 && timer->t <= 0xff) {
      if (timer->with_sound) {
        // TODO: some beep here
      }
      pthread_mutex_lock(&(timer->mutex));

      timer->t--;

      pthread_mutex_unlock(&(timer->mutex));
      add_ns_to_timespec(&next_frame_time, TARGET_CYCLE_NS);
      clock_gettime(CLOCK_MONOTONIC, &current_time);

      // Calculate the time length and sleep
      long long remaining_ns =
          (next_frame_time.tv_sec - current_time.tv_sec) * ONE_SEC_IN_NS +
          (next_frame_time.tv_nsec - current_time.tv_nsec);

      if (remaining_ns > 0) {
        sleep_time.tv_sec = remaining_ns / ONE_SEC_IN_NS;
        sleep_time.tv_nsec = remaining_ns % ONE_SEC_IN_NS;
        nanosleep(&sleep_time, NULL);
      }

    } else if (timer->t > 0xff) {
      // this might happen since we use uint_fast8_t for timer_t
      PANIC("Timer t larger than 25535(two bytes)!");
    }
  }

  return NULL;
}

void stop_timer(Timer *timer) {
  timer->is_running = false;
  pthread_join(timer->thread, NULL);
  pthread_mutex_destroy(&(timer->mutex));
}

/*---------Keyboard----------*/

bool is_key_pressed(Keys key) {
  const bool *key_states = SDL_GetKeyboardState(NULL);
  SDL_Scancode scancode = key_to_scancode(key);
  if (scancode == SDL_SCANCODE_UNKNOWN) {
    PANIC("Unknown key detected in is_key_pressed!");
  }
  return key_states[scancode];
}

SDL_Scancode key_to_scancode(Keys key) {
  switch (key) {
  case KEY_0:
    return SDL_SCANCODE_X;
  case KEY_1:
    return SDL_SCANCODE_1;
  case KEY_2:
    return SDL_SCANCODE_2;
  case KEY_3:
    return SDL_SCANCODE_3;
  case KEY_4:
    return SDL_SCANCODE_Q;
  case KEY_5:
    return SDL_SCANCODE_W;
  case KEY_6:
    return SDL_SCANCODE_E;
  case KEY_7:
    return SDL_SCANCODE_A;
  case KEY_8:
    return SDL_SCANCODE_S;
  case KEY_9:
    return SDL_SCANCODE_D;
  case KEY_A:
    return SDL_SCANCODE_Z;
  case KEY_B:
    return SDL_SCANCODE_C;
  case KEY_C:
    return SDL_SCANCODE_4;
  case KEY_D:
    return SDL_SCANCODE_R;
  case KEY_E:
    return SDL_SCANCODE_F;
  case KEY_F:
    return SDL_SCANCODE_V;
  default:
    return SDL_SCANCODE_UNKNOWN;
  }
}

Keys scancode_to_key(SDL_Scancode scancode) {
  switch (scancode) {
  case SDL_SCANCODE_X:
    return KEY_0;
  case SDL_SCANCODE_1:
    return KEY_1;
  case SDL_SCANCODE_2:
    return KEY_2;
  case SDL_SCANCODE_3:
    return KEY_3;
  case SDL_SCANCODE_Q:
    return KEY_4;
  case SDL_SCANCODE_W:
    return KEY_5;
  case SDL_SCANCODE_E:
    return KEY_6;
  case SDL_SCANCODE_A:
    return KEY_7;
  case SDL_SCANCODE_S:
    return KEY_8;
  case SDL_SCANCODE_D:
    return KEY_9;
  case SDL_SCANCODE_Z:
    return KEY_A;
  case SDL_SCANCODE_C:
    return KEY_B;
  case SDL_SCANCODE_4:
    return KEY_C;
  case SDL_SCANCODE_R:
    return KEY_D;
  case SDL_SCANCODE_F:
    return KEY_E;
  case SDL_SCANCODE_V:
    return KEY_F;
  default:
    return KEY_UNKNOWN;
  }
}