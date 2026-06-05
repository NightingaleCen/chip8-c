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

void destory_display(Display *display) {
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
