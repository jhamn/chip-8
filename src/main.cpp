#include <SDL2/SDL.h>
#include <chrono>
#include "chip8.h"

int main() {
  // Init SDL
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* window = SDL_CreateWindow(
      "CHIP-8 Emulator",
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      640, 320,
      SDL_WINDOW_SHOWN
  );

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

  Chip8 chip;
  chip.loadROM("../roms/PONG.ch8");

  const int SCALE = 10;
  bool running = true;
  SDL_Event e;

  auto last_timer_update = std::chrono::high_resolution_clock::now();

  while (running) {
    // SDL events (quit)
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) running = false;
    }

    // Run one CPU cycle
    chip.emulateCycle();

    // Timer at 60 Hz
    auto now = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(now - last_timer_update).count();
    if (ms > 16.6f) {
      chip.updateTimers();
      last_timer_update = now;
    }

    // Draw screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for(int y = 0; y < 32; y++) {
      for(int x = 0; x < 64; x++) {
        if (chip.gfx[y * 64 + x]) {
          SDL_Rect r { x * SCALE, y * SCALE, SCALE, SCALE };
          SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
          SDL_RenderFillRect(renderer, &r);
        }
      }
    }

    SDL_RenderPresent(renderer);

    SDL_Delay(1); // Small delay to avoid 100% CPU
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
