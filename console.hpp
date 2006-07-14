#include <SDL.h>

#define TTY_MAJOR_VERSION  0;
#define TTY_MINOR_VERSION  0;
#define TTY_PATH_VERSION   0;

struct TTY_Font
{
  SDL_Surface* surface;

  int glyph_width;
  int glyph_height;
};

void TTY_init();

void TTY_deinit();

void TTY_version(Uint32* major, Uint32* minor, Uint32* patch);

void TTY_cursor_set_pos(int x, int y);
void TTY_cursor_get_pos(int* x, int* y);

void TTY_clear();

void TTY_write(const char* buffer, int len);

void TTY_print(const char* buffer);
void TTY_putchar(char chr);
void TTY_putchar_nomove(char chr);

void TTY_print_cursor(int i);

void TTY_draw(SDL_Surface* screen, int x, int y);

void TTY_get_glypth_rect(TTY_Font* font, char idx, SDL_Rect* rect);

/* EOF */
