#include <SDL_image.h>
#include <assert.h>
#include "console.hpp"

struct TTY
{
  char** framebuffer;

  TTY_Font* font;

  int width;
  int height;

  int cursor_x;
  int cursor_y;

  int print_cursor;
};

TTY* tty;

void
TTY_init(int width, int height)
{
  tty = (TTY*)malloc(sizeof(TTY));

  // Create TTY_Font
  tty->font = (TTY_Font*)malloc(sizeof(TTY_Font));

  // convert font data to SDL_Surface
  tty->font->surface = IMG_Load("16x16font.png");
  if(!tty->font->surface) 
    {
      printf("IMG_Load: %s\n", IMG_GetError());
    }

  tty->font->glyph_width  = 16;
  tty->font->glyph_height = 16;
  
  // Create Framebuffer 
  tty->framebuffer = (char**)malloc(sizeof(char*) * height);
  for(int i = 0; i < height; ++i)
    tty->framebuffer[i] = (char*)malloc(sizeof(char) * width);
  
  tty->width  = width;
  tty->height = height;

  tty->print_cursor = 0;

  TTY_clear();
}

void
TTY_deinit()
{
  // deallocate SDL_Surface
}

void TTY_version(Uint32* major, Uint32* minor, Uint32* patch)
{
  major = TTY_MAJOR_VERSION;
  minor = TTY_MINOR_VERSION;
  patch = TTY_PATH_VERSION;
}

void TTY_cursor_set_pos(int x, int y)
{
  assert(tty);
  assert(x >= 0 && x < tty->width);
  assert(y >= 0 && y < tty->height);

  tty->cursor_x = x;
  tty->cursor_y = y;
}

void TTY_cursor_get_pos(int* x, int* y)
{
  *x = tty->cursor_x;
  *y = tty->cursor_y;
}

void TTY_clear()
{ 
  tty->cursor_x = 0;
  tty->cursor_y = 0;

  for(int y = 0; y < tty->height; ++y)
    memset(tty->framebuffer[y], 0, tty->width);
}

void TTY_putchar_nomove(char chr)
{
  if (chr != '\n' && chr != '\r')
    tty->framebuffer[tty->cursor_y][tty->cursor_x] = chr;
}

void TTY_putchar(char chr)
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

void TTY_write(const char* buffer, int len)
{
  for(int i = 0; i < len; ++i)
    TTY_putchar(buffer[i]);
}

void TTY_print(const char* buffer)
{
  TTY_write(buffer, strlen(buffer));
}

void
TTY_get_glypth_rect(TTY_Font* font, char idx, SDL_Rect* rect)
{
  rect->x = (idx % (font->surface->w / font->glyph_width)) * font->glyph_width;
  rect->y = (idx / (font->surface->w / font->glyph_width)) * font->glyph_height;

  rect->w = font->glyph_width;
  rect->h = font->glyph_height;
}

void TTY_print_cursor(int i)
{
  tty->print_cursor = i;
}

void TTY_draw(SDL_Surface* screen, int screen_x, int screen_y)
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

  TTY_init(640/16, 480/16);

  TTY_print_cursor(1);

  TTY_print("Hello World\n");
  TTY_print("Welcome to console Version 0.0.0\n\n");

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
                  TTY_putchar('\n');
                }
              else if (event.key.keysym.sym == SDLK_BACKSPACE)
                {
                  int cx, cy;
                  TTY_cursor_get_pos(&cx, &cy);
              
                  if (cx == 0 && cy != 0)
                    cy -= 1;
                  else if (cx != 0)
                    cx -= 1;
                  else
                    cy = tty->height - 1;
              
                  TTY_cursor_set_pos(cx, cy);
                  TTY_putchar_nomove(0);
                }
              else if (event.key.keysym.sym == SDLK_LEFT)
                {
                  tty->cursor_x = (tty->cursor_x - 1 + tty->width) % tty->width;
                }
              else if (event.key.keysym.sym == SDLK_RIGHT)
                {
                  tty->cursor_x = (tty->cursor_x + 1) % tty->width;
                }
              else if (event.key.keysym.sym == SDLK_UP)
                {
                  tty->cursor_y = (tty->cursor_y - 1 + tty->height) % tty->height;
                }
              else if (event.key.keysym.sym == SDLK_DOWN)
                {
                  tty->cursor_y = (tty->cursor_y + 1) % tty->height;
                }
              else if (event.key.keysym.unicode && (event.key.keysym.unicode & 0xFF80) == 0) 
                {
                  //printf("keydown: %c %d\n", event.key.keysym.unicode & 0x7f, event.key.keysym.unicode);
                  TTY_putchar(event.key.keysym.unicode & 0x7f);
                }
              break;
            }
        }

      //TTY_print("Hello World\n");
      SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 150, 20, 50));
      TTY_draw(screen, 0, 0);
      SDL_Flip(screen);
    }

  TTY_deinit();
  return 0;
}
#endif

/* EOF */
