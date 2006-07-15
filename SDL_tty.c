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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <SDL.h>
#include <SDL_image.h>
#include "font8x12.h"
#include "SDL_tty.h"

#define TTY_CreateRGBSurface(name) SDL_CreateRGBSurfaceFrom( name##_data, \
                                      name##_width, name##_height, name##_bpp, name##_pitch, \
                                      name##_rmask, name##_gmask,  name##_bmask, name##_amask )

TTY_Font*
TTY_CreateFont(SDL_Surface* surface, int glyph_width, int glyph_height)
{
  TTY_Font* font = (TTY_Font*)malloc(sizeof(TTY_Font));

  font->surface = SDL_DisplayFormatAlpha(surface);

  if(!font->surface) 
    {
      printf("TTY_CreateFont: conversation of surface failed\n");
      return 0;
    }

  font->glyph_width  = glyph_width;
  font->glyph_height = glyph_height;

  return font;
}

void
TTY_FreeFont(TTY_Font* font)
{
  SDL_FreeSurface(font->surface);
  free(font);
}

TTY*
TTY_Create(int width, int height)
{
  int i;
  TTY* tty = (TTY*)malloc(sizeof(TTY));

  /* SDL_Surface* temp = TTY_CreateRGBSurface(font8x12); */
  SDL_Surface* temp = IMG_Load("c64_16x16.png");
      
  tty->font = TTY_CreateFont(temp, 16, 16);

  SDL_FreeSurface(temp);
  
  /* Create Framebuffer */
  tty->framebuffer = (char**)malloc(sizeof(char*) * height);
  for(i = 0; i < height; ++i)
    tty->framebuffer[i] = (char*)malloc(sizeof(char) * width);
  
  tty->width  = width;
  tty->height = height;

  tty->print_cursor = 0;

  TTY_Clear(tty);

  return tty;
}

void
TTY_Free(TTY* tty)
{
  int i;
  TTY_FreeFont(tty->font);

  for(i = 0; i < tty->height; ++i)
    free(tty->framebuffer[i]);
  free(tty->framebuffer);
  
  free(tty);
}

void TTY_SetCursor(TTY* tty, int x, int y)
{
  tty->cursor_x = ((x % tty->width)  + tty->width)  % tty->width;
  tty->cursor_y = ((y % tty->height) + tty->height) % tty->height;
}

void TTY_GetCursor(TTY* tty, int* x, int* y)
{
  *x = tty->cursor_x;
  *y = tty->cursor_y;
}

void TTY_Clear(TTY* tty)
{ 
  int y;

  tty->cursor_x = 0;
  tty->cursor_y = 0;

  for(y = 0; y < tty->height; ++y)
    memset(tty->framebuffer[y], 0, tty->width);
}

void TTY_putchar_nomove(TTY* tty, char chr)
{
  if (chr != '\n' && chr != '\r')
    tty->framebuffer[tty->cursor_y][tty->cursor_x] = chr;
}

void TTY_putchar(TTY* tty, char chr)
{
  if (chr == '\n')
    {
      tty->cursor_x = 0;
      tty->cursor_y += 1;

      if (tty->cursor_y == tty->height)
        {
          /* FIXME: insert scroll_one_line code here */
          tty->cursor_y = 0;
        }
    }
  else if (chr == '\r')
    {
      tty->cursor_x = 0;
    }
  else
    {      
      tty->framebuffer[tty->cursor_y][tty->cursor_x] = chr;

      tty->cursor_x += 1;
      if (tty->cursor_x == tty->width)
        {
          tty->cursor_x = 0;
          tty->cursor_y += 1;

          if (tty->cursor_y == tty->height)
            {
              /* FIXME: insert scroll_one_line code here */
              tty->cursor_y = 0;
            }
        }
    }
}

void TTY_write(TTY* tty, const char* buffer, int len)
{
  int i;
  for(i = 0; i < len; ++i)
    TTY_putchar(tty, buffer[i]);
}

void TTY_print(TTY* tty, const char* buffer)
{
  TTY_write(tty, buffer, strlen(buffer));
}

void
TTY_get_glypth_rect(TTY_Font* font, char idx, SDL_Rect* rect)
{
  rect->x = (idx % (font->surface->w / font->glyph_width)) * font->glyph_width;
  rect->y = (idx / (font->surface->w / font->glyph_width)) * font->glyph_height;

  rect->w = font->glyph_width;
  rect->h = font->glyph_height;
}

void TTY_print_cursor(TTY* tty, int i)
{
  tty->print_cursor = i;
}

void TTY_Blit(TTY* tty, SDL_Surface* screen, int screen_x, int screen_y)
{
  SDL_Rect src_rect;
  SDL_Rect dst_rect;

  int x;
  int y;
  for(y = 0; y < tty->height; ++y)
    {
      for(x = 0; x < tty->width; ++x)
        {
          if (tty->print_cursor)
            {
              if (x == tty->cursor_x && y == tty->cursor_y && (SDL_GetTicks()/200) % 2 == 0)
                {
                  TTY_get_glypth_rect(tty->font, 0, &src_rect);

                  dst_rect.x = screen_x + x * tty->font->glyph_width;
                  dst_rect.y = screen_y + y * tty->font->glyph_height;

                  SDL_BlitSurface(tty->font->surface, &src_rect, screen, &dst_rect);
                }
              else
                {
                  char chr = tty->framebuffer[y][x];
                  if (chr)
                    {
                      TTY_get_glypth_rect(tty->font, chr, &src_rect);

                      dst_rect.x = screen_x + x * tty->font->glyph_width;
                      dst_rect.y = screen_y + y * tty->font->glyph_height;

                      SDL_BlitSurface(tty->font->surface, &src_rect, screen, &dst_rect);
                    }
                }
            }
          else
            {          
              char chr = tty->framebuffer[y][x];
              if (chr)
                {
                  TTY_get_glypth_rect(tty->font, chr, &src_rect);

                  dst_rect.x = screen_x + x * tty->font->glyph_width;
                  dst_rect.y = screen_y + y * tty->font->glyph_height;

                  SDL_BlitSurface(tty->font->surface, &src_rect, screen, &dst_rect);
                }
            }
        }
    }
}

void TTY_printf(TTY* tty, const char *fmt, ...)
{
  /* Guess we need no more than 100 bytes. */
  int n, size = 100;
  char *p, *np;
  va_list ap;

  if ((p = malloc (size)) == NULL)
    return; /* FIXME: Error */

  while (1) {
    /* Try to print in the allocated space. */
    va_start(ap, fmt);
    n = vsnprintf (p, size, fmt, ap);
    va_end(ap);
    /* If that worked, return the string. */
    if (n > -1 && n < size)
      {
        TTY_print(tty, p);
        free(p);
        return; /* Success */
      }
    /* Else try again with more space. */
    if (n > -1)    /* glibc 2.1 */
      size = n+1; /* precisely what is needed */
    else           /* glibc 2.0 */
      size *= 2;  /* twice the old size */
    if ((np = realloc (p, size)) == NULL) {
      free(p);
      return; /* FIXME: Error */
    } else {
      p = np;
    }
  }
}

#ifdef __TEST__

/*  Compile via:
 *
 *   gcc -D__TEST__ -ansi -pedantic -Wall -g -O2 -o SDL_tty SDL_tty.c `sdl-config --cflags --libs` -lSDL_image 
 *
 */

int main()
{
  SDL_Surface* screen;
  int quit = 0;
  TTY* tty;
  SDL_Event event;

  if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) <0 )
    {
      printf("Unable to init SDL: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
  atexit(SDL_Quit);
  
  screen = SDL_SetVideoMode(800, 600, 0,
                            SDL_HWSURFACE|SDL_DOUBLEBUF);

  SDL_WM_SetCaption("C64 Look alike", NULL);
  SDL_EnableUNICODE(1); 
  SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

  if ( screen == NULL )
    {
      printf("Unable to set 640x480 video: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }

  tty = TTY_Create(40, 30);

  TTY_print_cursor(tty, 1);

  TTY_printf(tty, "\n    **** COMMODORE 64 BASIC V%d ****\n\n", 2);
  TTY_printf(tty, " %dk RAM SYSTEM  38911 BASIC BYTES FREE\n", 64);
  TTY_printf(tty, "READY.\n");


  while (!quit)
    {
      while(SDL_PollEvent(&event))
        {
          switch(event.type) 
            { 
            case SDL_QUIT: 
              quit = 1;
              break;

            case SDL_KEYDOWN:
              if (event.key.keysym.sym == SDLK_RETURN)
                {
                  TTY_putchar(tty, '\n');
                }
              else if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                  int cx, cy;
                  TTY_GetCursor(tty, &cx, &cy);
              
                  if (cx == 0 && cy != 0)
                    cy -= 1;
                  else if (cx != 0)
                    cx -= 1;
                  else
                    cy = tty->height - 1;
              
                  TTY_SetCursor(tty, cx, cy);
                  TTY_putchar_nomove(tty, 0);
                }
              else if (event.key.keysym.sym == SDLK_LEFT)
                {
                  int cx, cy; TTY_GetCursor(tty, &cx, &cy);
                  TTY_SetCursor(tty, cx - 1, cy);
                }
              else if (event.key.keysym.sym == SDLK_RIGHT)
                {
                  int cx, cy; TTY_GetCursor(tty, &cx, &cy);
                  TTY_SetCursor(tty, cx + 1, cy);
                }
              else if (event.key.keysym.sym == SDLK_UP)
                {
                  int cx, cy; TTY_GetCursor(tty, &cx, &cy);
                  TTY_SetCursor(tty, cx, cy - 1);
                }
              else if (event.key.keysym.sym == SDLK_DOWN)
                {
                  int cx, cy; TTY_GetCursor(tty, &cx, &cy);
                  TTY_SetCursor(tty, cx, cy + 1);
                }
              else if (event.key.keysym.unicode && (event.key.keysym.unicode & 0xFF80) == 0) 
                {
                  TTY_putchar(tty, event.key.keysym.unicode & 0x7f);
                }
              break;
            }
        }

      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 138, 138, 255));

      {
        SDL_Rect rect;
        rect.x = 80;
        rect.y = 60;
        rect.w = tty->width * tty->font->glyph_width;
        rect.h = tty->height * tty->font->glyph_height;

        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0, 0, 202));
      }
      TTY_Blit(tty, screen, 80, 60);

      SDL_Flip(screen);
    }

  TTY_Free(tty);
  return 0;
}
#endif

/* EOF */
