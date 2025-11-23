#include <SDL2/SDL.h>
#include <iostream>
#include "chip8.h"

const int VIDEO_SCALE = 10;
const int VIDEO_WIDTH = 64;
const int VIDEO_HEIGHT = 32;

int main(int argc, char* argv[]) {
  if(argc < 2) {
    std::cout << "Usage: " << argv[0] << " <ROM file>\n";
    return 1;
  }

  Chip8 chip8;
  chip8.loadROM(argv[1]);

  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    VIDEO_WIDTH * VIDEO_SCALE, VIDEO_HEIGHT * VIDEO_SCALE, SDL_WINDOW_SHOWN);

  if(!window){
    std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(!renderer) {
    std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  bool running = true;
  SDL_Event event;

  const int CYCLES_PER_FRAME = 8;
  const int FPS = 120;
  const int FRAME_DELAY = 1000 / FPS;

  while(running) {
    Uint32 frameStart = SDL_GetTicks();

    while(SDL_PollEvent(&event)) {
      if(event.type == SDL_QUIT)
        running = false;

      if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        bool pressed = event.type == SDL_KEYDOWN;

        switch(event.key.keysym.sym) {
          case SDLK_1: chip8.key[0x1] = pressed; break;
          case SDLK_2: chip8.key[0x2] = pressed; break;
          case SDLK_3: chip8.key[0x3] = pressed; break;
          case SDLK_4: chip8.key[0xC] = pressed; break;

          case SDLK_q: chip8.key[0x4] = pressed; break;
          case SDLK_w: chip8.key[0x5] = pressed; break;
          case SDLK_e: chip8.key[0x6] = pressed; break;
          case SDLK_r: chip8.key[0xD] = pressed; break;

          case SDLK_a: chip8.key[0x7] = pressed; break;
          case SDLK_s: chip8.key[0x8] = pressed; break;
          case SDLK_d: chip8.key[0x9] = pressed; break;
          case SDLK_f: chip8.key[0xE] = pressed; break;

          case SDLK_z: chip8.key[0xA] = pressed; break;
          case SDLK_x: chip8.key[0x0] = pressed; break;
          case SDLK_c: chip8.key[0xB] = pressed; break;
          case SDLK_v: chip8.key[0xF] = pressed; break;
        }
      }
    }

    for(int i = 0; i < CYCLES_PER_FRAME; i++){
      chip8.emulateCycle();
    }
    chip8.updateTimers();
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for(int y = 0; y < VIDEO_HEIGHT; ++y){
      for(int x = 0; x < VIDEO_WIDTH; ++x){
        if(chip8.gfx[y * VIDEO_WIDTH + x]) {
          SDL_Rect rect = { x * VIDEO_SCALE, y * VIDEO_SCALE, VIDEO_SCALE, VIDEO_SCALE };
          SDL_RenderFillRect(renderer, &rect);
        }
      }
    }
    SDL_RenderPresent(renderer);

    Uint32 frameTime = SDL_GetTicks() - frameStart;
    if(FRAME_DELAY > frameTime)
      SDL_Delay(FRAME_DELAY - frameTime);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
