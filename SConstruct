# -*- python -*-

env = Environment(CXXFLAGS=['-Wall', '-O2'],
                CPPPATH=['../SDL_tty'],
                  LIBS=['SDL_image'])
env.ParseConfig("sdl-config --cflags --libs")

env.Program('jumpnrun', ['jumpnrun.cpp', 'SDL_tty/SDL_tty.c'])

# EOF #
