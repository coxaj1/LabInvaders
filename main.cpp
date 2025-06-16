/* Lab Invaders
 * By Andrew Cox
 * (For a directed study with Dr. Robert Marmorstein at Longwood University)
 * (c) 2025 Andrew Cox
 */

/* TODO
-add sound effects
-add 1 or 2 player choice at start
-add keybind tutorial at start
-add quit confirmation popup window
-add option to restart on game over
-add background texture
*/

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <chrono>
#include <string>
#include "lafuncs.h"
using namespace std;

int main(int argc, char* argv[]) {

  srand(time(nullptr));
  init();
  initPlayers();

  SDL_Event e;

  creditsScreen();
  SDL_RenderPresent(renderer);
  SDL_Delay(2000);

  startBox();
  SDL_RenderPresent(renderer); //display start box
  
  waitForKey(); //game won't start until a key is pressed

  auto startTime = chrono::steady_clock::now();
  int elTime = 0;

  while (!quit) {

    auto curTime = chrono::steady_clock::now();
    int elapsedTime = chrono::duration_cast<chrono::seconds>(curTime - startTime).count();

    if (elapsedTime > elTime) {
      timer = elapsedTime;
      elTime = elapsedTime;
    }

    while(SDL_PollEvent(&e) != 0) { //handle events
      if (e.type == SDL_QUIT) {
	quit = true;
      }   
    }

    Uint64 start = SDL_GetPerformanceCounter();
 
    handleInput();
    updateScreen();

    Uint64 end = SDL_GetPerformanceCounter();
   
    float frames = (end - start) / (float)SDL_GetPerformanceFrequency();

    cout << "FPS: " << to_string(1.0f / frames) << endl; //frame rate counter
    fps = (1.0f / frames);
  }

  endScreen();
  SDL_RenderPresent(renderer); //render game over screen
  SDL_Delay(3000);

  cleanup(); 

  return 0;
}
