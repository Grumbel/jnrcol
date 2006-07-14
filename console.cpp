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

#include <iostream>
#include <SDL_image.h>
#include <assert.h>
#include "console.hpp"

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
  TTY* tty = (TTY*)malloc(sizeof(TTY));

  SDL_Surface* temp = IMG_Load("8x14font.png");
  tty->font = TTY_CreateFont(temp, 8, 14);
  SDL_FreeSurface(temp);
  
  // Create Framebuffer 
  tty->framebuffer = (char**)malloc(sizeof(char*) * height);
  for(int i = 0; i < height; ++i)
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
  TTY_FreeFont(tty->font);

  for(int i = 0; i < tty->height; ++i)
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
  tty->cursor_x = 0;
  tty->cursor_y = 0;

  for(int y = 0; y < tty->height; ++y)
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
          // FIXME: insert scroll_one_line code here
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
              // FIXME: insert scroll_one_line code here
              tty->cursor_y = 0;
            }
        }
    }
}

void TTY_write(TTY* tty, const char* buffer, int len)
{
  for(int i = 0; i < len; ++i)
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

  for(int y = 0; y < tty->height; ++y)
    {
      for(int x = 0; x < tty->width; ++x)
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

                      //printf("%c -> %d %d %d %d\n", chr, src_rect.x, src_rect.y, src_rect.w, src_rect.h);

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

                  //printf("%c -> %d %d %d %d\n", chr, src_rect.x, src_rect.y, src_rect.w, src_rect.h);

                  dst_rect.x = screen_x + x * tty->font->glyph_width;
                  dst_rect.y = screen_y + y * tty->font->glyph_height;

                  SDL_BlitSurface(tty->font->surface, &src_rect, screen, &dst_rect);
                }
            }
        }
    }
}

#ifdef __TEST__
int main()
{
  SDL_Surface* screen;
  bool quit = false;
  
  if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) <0 )
    {
      printf("Unable to init SDL: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }
  atexit(SDL_Quit);
  
  screen = SDL_SetVideoMode(640, 480, 0,
                            SDL_HWSURFACE|SDL_DOUBLEBUF);
  
  SDL_EnableUNICODE(1); 

  if ( screen == NULL )
    {
      printf("Unable to set 640x480 video: %s\n", SDL_GetError());
      exit(EXIT_FAILURE);
    }

  TTY* tty = TTY_Create(500/16, 240/16);

  TTY_print_cursor(tty, 1);

  TTY_print(tty, "Hello World\n");
  TTY_print(tty, "Welcome to console Version 0.0.0\n\n");

  puts("Init successfull");
  SDL_Event event;
  while (!quit)
    {
      while(SDL_PollEvent(&event))
        {
          switch(event.type) 
            { 
            case SDL_QUIT: 
              quit = true;
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
                  //printf("keydown: %c %d\n", event.key.keysym.unicode & 0x7f, event.key.keysym.unicode);
                  TTY_putchar(tty, event.key.keysym.unicode & 0x7f);
                }
              break;
            }
        }

      //TTY_print("Hello World\n");
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 150, 20, 50));

      {
        SDL_Rect rect;
        rect.x = 50;
        rect.y = 50;
        rect.w = tty->width * tty->font->glyph_width;
        rect.h = tty->height * tty->font->glyph_height;
        SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 250, 20, 50));
      }
      TTY_Blit(tty, screen, 50, 50);

      SDL_Flip(screen);
    }

  TTY_Free(tty);
  return 0;
}
#endif

/* EOF */
