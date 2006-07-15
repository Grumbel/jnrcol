/** 
 ** Copyright (c) 2006 Ingo Ruhnke <grumbel@gmx.de>
 ** 
 ** This software is provided 'as-is', without any express or implied
 ** warranty. In no event will the authors be held liable for any
 ** damages arising from the use of this software.
 ** 
 ** Permission is granted to anyone to use this software for any
 ** purpose, including commercial applications, and to alter it and
 ** redistribute it freely, subject to the following restrictions:
 ** 
 **   1. The origin of this software must not be misrepresented; you
 **      must not claim that you wrote the original software. If you
 **      use this software in a product, an acknowledgment in the
 **      product documentation would be appreciated but is not
 **      required.
 ** 
 **   2. Altered source versions must be plainly marked as such, and
 **      must not be misrepresented as being the original software.
 ** 
 **   3. This notice may not be removed or altered from any source
 **      distribution.
 ** 
 */

#ifndef _SDL_TTY_H
#define _SDL_TTY_H

#include <SDL.h>

/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif


#define TTY_MAJOR_VERSION  0;
#define TTY_MINOR_VERSION  0;
#define TTY_PATH_VERSION   0;

#define TTY_SetError	SDL_SetError
#define TTY_GetError	SDL_GetError

typedef struct TTY_Font
{
  SDL_Surface* surface;

  char transtbl[256];

  int  glyph_width;
  int  glyph_height;
}  TTY_Font;

typedef struct TTY
{
  char** framebuffer;

  struct TTY_Font* font;

  int width;
  int height;

  int cursor_x;
  int cursor_y;

  int cursor_character;
  int print_cursor;

}  TTY;

/**
 *  Creates a font from an SDL_Surface. The letter with index 0 is at
 *  the top/left of the image
 *
 *  @param surface      The SDL_Surface that contains all letters
 *  @param glyph_width  The width of a glyph
 *  @param glyph_height The height of a glyph
 *  @param letters      The letters that are present in the font
 */
TTY_Font* TTY_CreateFont(SDL_Surface* surface, int glyph_width, int glyph_height, char* letters);
void      TTY_FreeFont(TTY_Font* font);

TTY* TTY_Create(int width, int height);
void TTY_Free(TTY* tty);

/**
 *  The the current cursor position to \a x, \a y, if x or y are
 *  outside the range of the TTY, they automatically wrap around 
 */
void TTY_SetCursor(TTY* tty, int x, int y);

/**
 *  Write the current cursor position to \a x and \a y 
 */
void TTY_GetCursor(TTY* tty, int* x, int* y);

void TTY_SetCursorCharacter(TTY* tty, int chr);

void TTY_EnableVisibleCursor(TTY* tty, int i);

/** 
 *  Clear the tty's framebuffer
 */
void TTY_Clear(TTY* tty);

void TTY_write(TTY* tty, const char* buffer, int len);

void TTY_print(TTY* tty, const char* buffer);
void TTY_putchar(TTY* tty, char chr);
void TTY_putchar_nomove(TTY* tty, char chr);

void TTY_printf(TTY* tty, const char *fmt, ...)  __attribute__ ((format (printf, 2, 3)));

void TTY_Blit(TTY* tty, SDL_Surface* screen, int x, int y);

void TTY_GetGlypth(TTY_Font* font, char idx, SDL_Rect* rect);

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

#endif

/* EOF */