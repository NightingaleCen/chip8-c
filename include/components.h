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

  SDL_AudioStream *audio_stream;
  uint32_t audio_phase;

  pthread_mutex_t mutex;
  pthread_t thread;
} Timer;

typedef enum Keys {
  KEY_0 = 0x0,
  KEY_1 = 0x1,
  KEY_2 = 0x2,
  KEY_3 = 0x3,
  KEY_4 = 0x4,
  KEY_5 = 0x5,
  KEY_6 = 0x6,
  KEY_7 = 0x7,
  KEY_8 = 0x8,
  KEY_9 = 0x9,
  KEY_A = 0xA,
  KEY_B = 0xB,
  KEY_C = 0xC,
  KEY_D = 0xD,
  KEY_E = 0xE,
  KEY_F = 0xF,
  KEY_UNKNOWN = 0xFF,
} Keys;

/*----------Display----------*/

Display *init_display(void);
void clear_display(Display *display);
void render_display_once(Display *display);
void destroy_display(Display *display);

/*-----------Timer-----------*/

Timer *init_timer(bool with_sound);
void *update_timer(void *arg);
void stop_timer(Timer *timer);

/*---------Keyboard----------*/
bool is_key_pressed(Keys key);
SDL_Scancode key_to_scancode(Keys key);
Keys scancode_to_key(SDL_Scancode scancode);

#endif // COMPONENT_H