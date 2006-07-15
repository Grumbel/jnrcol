#include <SDL.h>
#include <iostream>
#include "SDL_tty.h"
#include "jumpnrun.hpp"

char* level[] = {
  "                    ",
  "                    ",
  "                    ",
  "       #### ####    ",
  "                    ",
  "                    ",
  "                    ",
  "#        ######     ",
  "                    ",
  "                    ",
  "#                   ",
  "      ####      #   ",
  "                    ",
  "                    ",
  "                    ",
  "####################"
};

char get_tile(float x, float y)
{
  if (x < 0 || x >= 20*32 ||
      y < 0 || y >= 16*32)
    {
      return 'X';
    }
  else
    {
      return level[int(y/32)][int(x/32)];
    }
}

SDL_Surface *screen;
TTY* tty;

void draw_rect(int x, int y, int w, int h, unsigned char r, unsigned char b, unsigned char g, bool down = false)
{
  SDL_Rect rect;

  rect.x = x;
  rect.y = y;
  rect.w = w;

  Uint32 normal    = SDL_MapRGB(screen->format, r, g, b);
  Uint32 highlight = SDL_MapRGB(screen->format, r + 50, g + 50, b + 50);
  Uint32 shadow    = SDL_MapRGB(screen->format, r - 50, g - 50, b - 50);

  /*  if (down)
    {
      Uint32 tmp = highlight;
      highlight = shadow;
      shadow = tmp;
    }*/

  rect.h = h;
        
  SDL_FillRect(screen, &rect, normal);

  rect.w = 2;
  SDL_FillRect(screen, &rect, highlight);

  rect.w = w;
  rect.h = 2;
  SDL_FillRect(screen, &rect, highlight);

  rect.x = x + w - 2;
  rect.w = 2;
  rect.h = h;
  SDL_FillRect(screen, &rect, shadow);

  rect.x = x;
  rect.w = w;
  rect.y = y + h - 2;
  rect.h = 2;
  SDL_FillRect(screen, &rect, shadow);
}

class Player
{
public:
  float x;
  float y;
  
  float vel_x;
  float vel_y;
  bool jump;
  bool duck;

  enum Direction { LEFT, RIGHT, NONE } direction;

  Player()
  {
    x = 100;
    y = 100;

    vel_x = 0;
    vel_y = 0;
    
    jump = false;
    duck = false;
    direction = NONE;
  }

  bool clean()
  {
    return 
      get_tile(x-16, y)      != ' ' ||
      get_tile(x-16, y - 31) != ' ' ||
      get_tile(x+16, y)      != ' ' ||
      get_tile(x+16, y - 31) != ' ' ||
      (!duck &&
       (get_tile(x-16, y - 63) != ' ' ||
        get_tile(x+16, y - 63) != ' '));
  }

  void update(float delta) 
  {
    //std::cout << "Delta: " << delta << std::endl;
    vel_y += 10 * delta;

    if (jump)
      vel_y = -5;

    TTY_SetCursor(tty, 0, 28);
    TTY_printf(tty, "Velocity: %f %d       \r",  vel_x, get_tile(x, y));

    float last_x = x;
    float last_y = y;

    x += vel_x;

    if (clean())
      {
        x = last_x;
        vel_x = 0;
      }

    y += vel_y;
    if (clean())
      {
        y = last_y;
        vel_y = 0;
      }

    switch(direction)
      {
      case LEFT:
        if (vel_x > -5.0f)
          vel_x -= 10 * delta;
        break;

      case RIGHT:
        if (vel_x < 5.0f)
          vel_x += 10 * delta;
        break;

      case NONE:
        vel_x -= vel_x * delta * 10.0f;
        break;
      }
  }

  void draw()
  {
    if (duck)
      draw_rect(int(x - 16), int(y - 32) - 16, 32, 32, 150, 200, 150);
    else
      draw_rect(int(x - 16), int(y - 64) - 16, 32, 64, 150, 200, 150);

    TTY_Print(tty->font, screen, (int)x, (int)y-16, 0, "Hello\nWorld");
  }
  
  void left()
  {
    direction = LEFT;
  }

  void stop()
  {
    direction = NONE;
  }

  void right()
  {
    direction = RIGHT;
  }
};

class JumpnRun
{
private:

public:  
  JumpnRun()
  {
    screen = 0;
  }

  void init()
  {
    if( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) <0 )
      {
        printf("Unable to init SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
      }
    atexit(SDL_Quit);
    
    screen = SDL_SetVideoMode(640, 480, 0,
                              SDL_HWSURFACE|SDL_DOUBLEBUF);
    
    if ( screen == NULL )
      {
        printf("Unable to set 640x480 video: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
      }

    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = 640;
    rect.h = 480;

    SDL_SetClipRect(screen, &rect); 

    tty = TTY_Create(40, 30);

    TTY_printf(tty, "\n    **** COMMODORE 64 BASIC V2 ****\n\n");
    TTY_printf(tty, " 64k RAM SYSTEM  38911 BASIC BYTES FREE\n\n");
    TTY_printf(tty, "READY.\n\n");
  }

  void run()
  {
    bool quit = false;
    SDL_Event event;
    Uint32 last_tick = 0;
    Player player;
    while(!quit)
      {
        while(SDL_PollEvent(&event))
          {
            switch(event.type) 
              { 
              case SDL_QUIT: 
                quit = true;
                break;
              }
          }
        
        Uint8 *keystates = SDL_GetKeyState( NULL );

        if (keystates[SDLK_LEFT])
          player.left();
        
        else if (keystates[SDLK_RIGHT])
          player.right();

        else 
          player.stop();

        if (keystates[SDLK_SPACE])
          player.jump = true;  
        else
          player.jump = false;  

        if (keystates[SDLK_DOWN])
          player.duck = true;
        else
          player.duck = false;

        for(int y =  0; y < 16; ++y)
          for(int x = 0; x < 20; ++x)
            {
              if (level[y][x] == ' ')
                {
                  draw_rect(x*32, y*32 - 16, 32, 32, 50, 50, 50, true);
                }
              else if (level[y][x] == '#')
                {
                  draw_rect(x*32, y*32 - 16, 32, 32, 200, 200, 200);
                }
            }

        Uint32 tick = SDL_GetTicks();
        float delta = (tick - last_tick)/1000.0f;
        last_tick = tick;

        while (delta > 0.0f)
          {
            player.update(0.01f);
            delta -= 0.01f;
          }
        player.draw();
        TTY_Blit(tty, screen, 0, 0);
        SDL_Flip(screen);      
      }
  }

  void deinit()
  {
    TTY_Free(tty);
  }
};

int main(int argc, char** argv)
{
  JumpnRun app;
  app.init();
  app.run();
  app.deinit();
  return 0;
}

/* EOF */
