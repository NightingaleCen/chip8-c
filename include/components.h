#ifndef COMPONENT_H
#define COMPONENT_H

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

#include <SDL3/SDL.h>

#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH 64
#define DISPLAY_PIXEL_SIZE 10
/* Every 8 pixels in a row are stored as a byte */
#define DISPLAY_W_BYTES (DISPLAY_WIDTH / 8) // 64 bits (pixels) -> 8 bytes

#define TIMER_FREQUENCY_HZ 60
#define TIMER_WITH_SOUND true
#define TIMER_WITHOUT_SOUND false

#define KEYBOARD_BUFFER_SIZE 20

typedef struct Display {
  uint8_t *display_buffer;
  SDL_Renderer *renderer;
} Display;

typedef struct Timer {
  uint_fast8_t t;
  bool with_sound;
  atomic_bool is_running;

  pthread_mutex_t mutex;
  pthread_t thread;
} Timer;

/*----------Display----------*/

Display *init_display(void);
void clear_display(Display *display);
void render_display_once(Display *display);
void destory_display(Display *display);

/*-----------Timer-----------*/

Timer *init_timer(bool with_sound);
void *update_timer(void *arg);
void stop_timer(Timer *timer);

/*---------Keyboard----------*/
// TODO

#endif // COMPONENT_H