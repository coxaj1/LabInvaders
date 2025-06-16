#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <algorithm>
#include <chrono>
#include <string>
#include "lafuncs.h"
using namespace std;

SDL_Texture* ship1Tex = nullptr;
SDL_Texture* ship2Tex = nullptr;
SDL_Texture* enemyTex = nullptr;
SDL_Texture* bulletTex = nullptr;
SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

int ship1x, ship1y, ship2x, ship2y, enemX, enemY, bulX, bulY;
int texW, texH, texW2, texH2, texW3, texH3, texW4, texH4;

int score = 0;

Mix_Music* blaster = nullptr;

static unsigned int enemyCounter = 0;
static unsigned int shootCounter = 0;

const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

class Enemy {
  public:
    int x, y;	//position
    int wd, ht;	//dimensions
    bool alive;	//state
    int speed;	//movement speed
    SDL_Texture* texture; //texture for enemy ship

    Enemy (int startX, int startY, int w, int h, int spd, SDL_Texture* tex) : x(startX), y(startY), wd(w), ht(h), speed(spd), texture(tex), alive(true) {} 

    void update() { //move downwards
      if (alive) {
	y += speed;
      }
    }

    void render(SDL_Renderer* enemRend) { //draw when alive
      if (alive && texture) {
	SDL_Rect rect = {x + 20, y, wd - 10, ht - 10};
	SDL_SetRenderDrawColor(enemRend, 0, 0, 0, 0); //black to blend in with background, will make transparent later
	SDL_RenderFillRect(enemRend, &rect);
	if (SDL_RenderCopy(enemRend, texture, nullptr, &rect) != 0) {
	  cout << "Could not render enemies, error: " << SDL_GetError() << endl;
	}
      }
    }
};

class Bullet {
  public:
    int x, y;	//position
    int wd, ht;	//dimensions
    bool alive;	//state
    int speed;	//movement speed
    SDL_Texture* texture; //texture for projectile

    Bullet (int startX, int startY, int w, int h, int spd, SDL_Texture* tex) : x(startX), y(startY), wd(w), ht(h), speed(spd), texture(tex), alive(true) {} 

    void update() { //bullets go upwards
      if (alive) {  
	y -= speed; 
      }
    }

    void render(SDL_Renderer* bulletRend) { //draw when alive
      if (alive && texture) {
	//cout << "bullet being rendered" << endl;
	SDL_Rect rect2 = {x, y, wd, ht};
	SDL_SetRenderDrawColor(bulletRend, 0, 0, 0, 0); //black to blend in with background, will make transparent later
	SDL_RenderFillRect(bulletRend, &rect2);
	if (SDL_RenderCopy(bulletRend, texture, nullptr, &rect2) != 0) {
	  cout << "Could not render bullet, error: " << SDL_GetError() << endl;
	}
      }
    }
};

vector<Enemy> enemies;
vector<Bullet> bullets;

float bound (float val, float min, float max) { //keep player ships in bounds of screen
  if (val < min) {
    return min;
  }
  if (val > max) {
    return max;
  }
  return val;
}

//get SDL started, create a window and renderer, load IMG and TTF
void init (void) {
  //start SDL with audio support loaded
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0 ) {
    cerr << "SDL could not start, error: " << SDL_GetError() << endl;
    SDL_Quit();
    exit(1);
  }

  //start SDL_IMAGE
  if (!(IMG_Init(IMG_INIT_PNG) && IMG_INIT_PNG)) {
    cerr << "SDL_IMG could not start, error: " << SDL_GetError() << endl;
    SDL_Quit();
    exit(1);
  }

  //start SDL_TTF
  if (TTF_Init() == -1) {
    cerr << "SDL_ttf could not start, error: " << TTF_GetError() << endl;
    SDL_Quit();
    exit(1);
  }

  //start SDL audio mixer
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    cerr << "Mix_OpenAudio could not start, error: " << Mix_GetError() << endl;
    SDL_Quit();
    exit(1);
  }

  //create window
  //For fullscreen, SDL_WINDOW_FULLSCREEN or SDL_WINDOW_FULLSCREEN_DESKTOP
  window = SDL_CreateWindow("LabInvaders!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    cerr << "Could not create window, error: " << SDL_GetError() << endl;
    TTF_Quit();
    SDL_Quit();
    exit(1);
  }

  //create renderer
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr) {
    cerr << "Could not create renderer, error: " << SDL_GetError() << endl;
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    exit(1);
  }
 
  //create font
  font = TTF_OpenFont("assets/fonts/Orbitron-VariableFont_wght.ttf", 30); //size 30 pt
  if (font == nullptr) {
    cerr << "Could not load font, error: " << TTF_GetError() << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    exit(1);
  }

  blaster = Mix_LoadMUS("assets/sounds/blaster.mp3");
  if (!blaster) {
    cerr << "failed to load blaster sound, error: " << Mix_GetError() << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    exit(1);
  }

  cout << "Init finished" << endl;
}

//clean up after quitting
void cleanup(void) {
  TTF_CloseFont(font);
  Mix_CloseAudio();
  SDL_DestroyTexture(ship1Tex);
  SDL_DestroyTexture(ship2Tex);
  SDL_DestroyTexture(enemyTex);
  SDL_DestroyTexture(bulletTex);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  IMG_Quit();
  SDL_Quit();
  exit(0);
}

void initPlayers(void) {
  //load ship image for p1
  SDL_Surface* ship1Img = IMG_Load("assets/images/ship.png");
  if (ship1Img == nullptr) { //load image
    cerr << "Could not load image, error: " << IMG_GetError() << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  //load ship image for p2
  SDL_Surface* ship2Img = IMG_Load("assets/images/ship2.png");
  if (ship2Img == nullptr) {
    cerr << "Could not load ship2, error: " << IMG_GetError() << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  //load image for enemies
  SDL_Surface* enemyImg = IMG_Load("assets/images/enemy.png");
  if (enemyImg == nullptr)  {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  //load image for bullets
  SDL_Surface* bulletImg = IMG_Load("assets/images/bullet.png");
  if (bulletImg == nullptr)  {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  //put ship 1 image onto a surface
  ship1Tex = SDL_CreateTextureFromSurface(renderer, ship1Img);
  SDL_FreeSurface(ship1Img); //free after creating texture
  if (ship1Tex == nullptr) {
    cerr << "Could not create ship 1 texture, error: " << SDL_GetError() << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  //put ship 2 image onto a surface
  ship2Tex = SDL_CreateTextureFromSurface(renderer, ship2Img);
  SDL_FreeSurface(ship2Img); //free after creating texture
  if (ship2Tex == nullptr) {
    cerr << "Could not create ship 2 texture, error: " << SDL_GetError() << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  //put enemy image onto a surface
  enemyTex = SDL_CreateTextureFromSurface(renderer, enemyImg);
  SDL_FreeSurface(enemyImg); //free after creating texture
  if (enemyTex == nullptr) {
    cerr << "Could not create enemy texture, error: " << SDL_GetError() << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  //put bullet image onto a surface
  bulletTex = SDL_CreateTextureFromSurface(renderer, bulletImg);
  SDL_FreeSurface(bulletImg); //free after creating texture
  if (bulletTex == nullptr) {
    cerr << "Could not create bullet texture, error: " << SDL_GetError() << endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(1);
  }

  //starting position of ships
  ship1x = (SCREEN_WIDTH - 857);
  ship1y = (SCREEN_HEIGHT - 200);
  ship2x = (SCREEN_WIDTH - 422);
  ship2y = (SCREEN_HEIGHT - 200);

  //grab info about textures for later
  SDL_QueryTexture(ship1Tex, nullptr, nullptr, &texW, &texH);
  SDL_QueryTexture(ship2Tex, nullptr, nullptr, &texW2, &texH2);
  SDL_QueryTexture(enemyTex, nullptr, nullptr, &texW3, &texH3);
  SDL_QueryTexture(bulletTex, nullptr, nullptr, &texW4, &texH4);

  cout << "Init players finished" << endl;
}

//handle keyboard input
void handleInput(void) {
  //player 1
  if (currentKeyStates[SDL_SCANCODE_W]) {
    ship1y -= MSPEED;
  }

  if (currentKeyStates[SDL_SCANCODE_S]) {
    ship1y += MSPEED;
  }

  if (currentKeyStates[SDL_SCANCODE_A]) {
    ship1x -= MSPEED;
  }

  if (currentKeyStates[SDL_SCANCODE_D]) {
    ship1x += MSPEED;
  }
  
  //player 2
  if (currentKeyStates[SDL_SCANCODE_UP]) {
    ship2y -= MSPEED;
  }

  if (currentKeyStates[SDL_SCANCODE_LEFT]) {
    ship2x -= MSPEED;
  }

  if (currentKeyStates[SDL_SCANCODE_DOWN]) {
    ship2y += MSPEED;
  }

  if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
    ship2x += MSPEED;
  }

  //shooting for both players
  if (currentKeyStates[SDL_SCANCODE_SPACE]) {
    shoot(ship1x, ship1y, bulletTex);
  }

  if (currentKeyStates[SDL_SCANCODE_RETURN]) {
    shoot(ship2x, ship2y, bulletTex);
  }
  
  //can quit with P at any time
  if (currentKeyStates[SDL_SCANCODE_P]) {
    quit = true;
    /*
    quitConfirm();
    SDL_RenderPresent(renderer);
    SDL_Delay(1000);
    if (currentKeyStates[SDL_SCANCODE_P]) { //quit confirm that doesn't work
      quit = true;
    }
    else {
      
    }
    */
  }
}

void updateScreen (void) {
  //keep ships inside bounds of screen
  ship1x = bound(ship1x, 0, SCREEN_WIDTH - 1 - texW);
  ship1y = bound(ship1y, 0, SCREEN_HEIGHT - 1 - texH);
  ship2x = bound(ship2x, 0, SCREEN_WIDTH - 1 - texW2);
  ship2y = bound(ship2y, 0, SCREEN_HEIGHT - 1 - texH2);

  //clear screen
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer);
 
  //shape for texture to go on to
  SDL_Rect destRect = {ship1x, ship1y, texW, texH};
  SDL_Rect destRect2 = {ship2x, ship2y, texW2, texH2};

  //render textures
  SDL_RenderCopy(renderer, ship1Tex, nullptr, &destRect);
  SDL_RenderCopy(renderer, ship2Tex, nullptr, &destRect2);

  makeEnemies();

  for (auto& enemy : enemies) {
    enemy.update();
    if (enemy.y > SCREEN_HEIGHT) { //end game if any enemies get below window
      enemy.alive = false;
      quit = true;
    }
    enemy.render(renderer);
  }

  for (auto& bullet: bullets) {
    bullet.update();
    if (bullet.y < 0) { //remove bullets that go above the window
      bullet.alive = false;
    }
    bullet.render(renderer);
  }
 
  for (auto& bullet : bullets) { //collsion check between bullets and enemies
    for (auto& enemy : enemies) {
      if (enemy.x < bullet.x + bullet.wd && enemy.x + enemy.wd > bullet.x && enemy.y < bullet.y + bullet.ht && enemy.y + enemy.ht > bullet.y) {
	bullet.alive = false;
	enemy.alive = false;
	if (score <= 999) { //score is capped at 1000 for now
	  score++;
	}
      }
    }
  }

  //delete removed bullets and enemies
  bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) {return !b.alive; }), bullets.end());
  enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) {return !e.alive; }), enemies.end());

  for (auto& enemy : enemies) { //collision check between enemies and players 
    if (enemy.x < ship1x + texW && enemy.x + enemy.wd > ship1x && enemy.y < ship1y + texW && enemy.y + enemy.ht > ship1y ||
	enemy.x < ship2x + texW && enemy.x + enemy.wd > ship2x && enemy.y < ship2y + texW && enemy.y + enemy.ht > ship2y) { 
      quit = true;
    }
  }

  scoreCounter();
  timerBox();
  frameCounter();

  //update screen
  SDL_RenderPresent(renderer);
}

void startBox (void) {

  SDL_Rect textBoxBorder = { //create box that servers as the border
    (SCREEN_WIDTH - START_SCREEN_WIDTH) / 2,
    (SCREEN_HEIGHT - START_SCREEN_HEIGHT) / 2,
    START_SCREEN_WIDTH,
    START_SCREEN_HEIGHT
  };

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //border is white
  SDL_RenderFillRect(renderer, &textBoxBorder);

  SDL_Rect textBox = { //create inner box
    textBoxBorder.x + 2,
    textBoxBorder.y + 2,
    textBoxBorder.w - 4,
    textBoxBorder.h - 4
  };

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1); //inner box is blue
  SDL_RenderFillRect(renderer, &textBox);


  SDL_Color textColor = {255, 255, 255, 255}; //box text is white
  SDL_Surface* textSurface = TTF_RenderText_Blended_Wrapped(font, "Press any key to start...", textColor, 300);
  
  if (textSurface == nullptr) {
    cerr << "Unable to create text surface, error: " << TTF_GetError() << endl;
  }

  else { 
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (textTex == nullptr) {
      cerr << "Unable to create text texture, error: " << SDL_GetError() << endl;
    }
    else { 
      int textWidth, textHeight;
      SDL_QueryTexture(textTex, nullptr, nullptr, &textWidth, &textHeight); //grab text attributes and put them into width/height

      SDL_Rect textBox2 = { //where on box to draw text
      textBoxBorder.x + (textBoxBorder.w - textWidth) / 2,
      textBoxBorder.y + (textBoxBorder.h - textHeight) / 2,
      textWidth,
      textHeight
      };

      SDL_RenderCopy(renderer, textTex, nullptr, &textBox2); //draw text onto box
      SDL_DestroyTexture(textTex);

    }
  }
}

//game over screen
void endScreen (void) {

  SDL_Rect textBoxBorder = { //create box that servers as the border
    (SCREEN_WIDTH - START_SCREEN_WIDTH) / 2,
    (SCREEN_HEIGHT - START_SCREEN_HEIGHT) / 2,
    START_SCREEN_WIDTH,
    START_SCREEN_HEIGHT
  };

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //border is white
  SDL_RenderFillRect(renderer, &textBoxBorder);

  SDL_Rect textBox = { //create inner box
    textBoxBorder.x + 2,
    textBoxBorder.y + 2,
    textBoxBorder.w - 4,
    textBoxBorder.h - 4
  };

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1); //inner box is blue
  SDL_RenderFillRect(renderer, &textBox);


  SDL_Color textColor = {255, 255, 255, 255}; //box text is white
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, "GAME OVER", textColor);

  if (textSurface == nullptr) {
    cerr << "Unable to create text surface, error: " << TTF_GetError() << endl;
  }

  else { 
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (textTex == nullptr) {
      cerr << "Unable to create text texture, error: " << SDL_GetError() << endl;
    }
    else { 
      int textWidth, textHeight;
      SDL_QueryTexture(textTex, nullptr, nullptr, &textWidth, &textHeight); //grab text attributes and put them into width/height

      SDL_Rect textBox2 = { //where on box to draw text
	textBoxBorder.x + (textBoxBorder.w - textWidth) / 2,
	textBoxBorder.y + (textBoxBorder.h - textHeight) / 2,
	textWidth,
	textHeight
      };

      SDL_RenderCopy(renderer, textTex, nullptr, &textBox2); //draw text onto box
      SDL_DestroyTexture(textTex);

    }
  }
}

void scoreCounter(void) {
  SDL_Rect textBoxBorder = { //create box that servers as the border
    (SCREEN_WIDTH / 2) - 100,
    885,
    200,
    75
  };

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //border is white
  SDL_RenderFillRect(renderer, &textBoxBorder);

  SDL_Rect textBox = { //create inner box
    textBoxBorder.x + 2,
    textBoxBorder.y + 2,
    textBoxBorder.w - 4,
    textBoxBorder.h - 4
  };

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1); //inner box is blue
  SDL_RenderFillRect(renderer, &textBox); 

  string score_text = "Score: " + to_string(score);

  SDL_Color textColor = {255, 255, 255, 255}; //box text is white
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, score_text.c_str(), textColor);

  if (textSurface == nullptr) {
    cerr << "Unable to create text surface, error: " << TTF_GetError() << endl;
  }

  else { 
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (textTex == nullptr) {
      cerr << "Unable to create text texture, error: " << SDL_GetError() << endl;
    }
    else { 
      int textWidth, textHeight;
      SDL_QueryTexture(textTex, nullptr, nullptr, &textWidth, &textHeight); //grab text attributes and put them into width/height

      SDL_Rect textBox2 = { //where on box to draw text
	textBoxBorder.x + (textBoxBorder.w - textWidth) / 2,
	textBoxBorder.y + (textBoxBorder.h - textHeight) / 2,
	textWidth,
	textHeight
      };

      SDL_RenderCopy(renderer, textTex, nullptr, &textBox2); //draw text onto box
      SDL_DestroyTexture(textTex);

    }
  }
}

void creditsScreen (void) {

  SDL_Rect textBoxBorder = { //create box that servers as the border
    (SCREEN_WIDTH - START_SCREEN_WIDTH) / 2,
    (SCREEN_HEIGHT - START_SCREEN_HEIGHT) / 2,
    START_SCREEN_WIDTH,
    START_SCREEN_HEIGHT
  };

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //border is white
  SDL_RenderFillRect(renderer, &textBoxBorder);

  SDL_Rect textBox = { //create inner box
    textBoxBorder.x + 2,
    textBoxBorder.y + 2,
    textBoxBorder.w - 4,
    textBoxBorder.h - 4
  };

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1); //inner box is blue
  SDL_RenderFillRect(renderer, &textBox);


  SDL_Color textColor = {255, 255, 255, 255}; //box text is white
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Lab Invaders! By Andrew Cox (c) 2025", textColor);

  if (textSurface == nullptr) {
    cerr << "Unable to create text surface, error: " << TTF_GetError() << endl;
  }

  else { 
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (textTex == nullptr) {
      cerr << "Unable to create text texture, error: " << SDL_GetError() << endl;
    }
    else { 
      int textWidth, textHeight;
      SDL_QueryTexture(textTex, nullptr, nullptr, &textWidth, &textHeight); //grab text attributes and put them into width/height

      SDL_Rect textBox2 = { //where on box to draw text
	textBoxBorder.x + (textBoxBorder.w - textWidth) / 2,
	textBoxBorder.y + (textBoxBorder.h - textHeight) / 2,
	textWidth,
	textHeight
      };

      SDL_RenderCopy(renderer, textTex, nullptr, &textBox2); //draw text onto box
      SDL_DestroyTexture(textTex);

    }
  }
}

void createRow (int windW, int enemyW, int enemyH, int speed, SDL_Texture* texture) {
  int numEnemies = windW / enemyW; //number of enemies is dynamic to screen width
  for (int i = 0; i < numEnemies; ++i) {
    int startX = i * enemyW;
    enemies.emplace_back(startX, 0, enemyW, enemyH, speed, texture); //add new enemies to the vector
    //cout << "spawned enemy at x: " << startX << " width: " << enemyW << " height: " << enemyH << endl; //XXX DEBUG
  }
}

void makeEnemies (void) { //ensures rows of enemies are spaced apart evenly
  ++enemyCounter;
  if (enemyCounter % 100 == 0) { //spawn rows 100 pixels apart
    createRow(SCREEN_WIDTH, 50, 50, 1, enemyTex);
  }
}

void shoot (int playerX, int playerY, SDL_Texture* bulletTexture) {
  ++shootCounter;
  if (shootCounter % 5 == 0) { //slows down fire rate
    Mix_PlayMusic(blaster, 0);
    bullets.emplace_back(playerX + 15, playerY, 10, 15, 5, bulletTexture);
    //Mix_PlayMusic(blaster, 0);
  }
}

void waitForKey(void) {
  SDL_Event e2;
  bool keyPressed = false;

  while (!keyPressed) {
    while(SDL_PollEvent(&e2)) {
      if (e2.type == SDL_KEYDOWN || e2.type == SDL_QUIT) {
	keyPressed = true;
      }
    }
  }
}

void timerBox(void) {
  SDL_Rect textBoxBorder = { //create box that servers as the border
    (SCREEN_WIDTH / 2) - 100,
    0,
    200,
    75
  };

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //border is white
  SDL_RenderFillRect(renderer, &textBoxBorder);

  SDL_Rect textBox = { //create inner box
    textBoxBorder.x + 2,
    textBoxBorder.y + 2,
    textBoxBorder.w - 4,
    textBoxBorder.h - 4
  };

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1); //inner box is blue
  SDL_RenderFillRect(renderer, &textBox);

  string timer_text = "Time: " + to_string(timer);


  SDL_Color textColor = {255, 255, 255, 255}; //box text is white
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, timer_text.c_str(), textColor);

  if (textSurface == nullptr) {
    cerr << "Unable to create text surface, error: " << TTF_GetError() << endl;
  }

  else { 
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (textTex == nullptr) {
      cerr << "Unable to create text texture, error: " << SDL_GetError() << endl;
    }
    else { 
      int textWidth, textHeight;
      SDL_QueryTexture(textTex, nullptr, nullptr, &textWidth, &textHeight); //grab text attributes and put them into width/height

      SDL_Rect textBox2 = { //where on box to draw text
	textBoxBorder.x + (textBoxBorder.w - textWidth) / 2,
	textBoxBorder.y + (textBoxBorder.h - textHeight) / 2,
	textWidth,
	textHeight
      };

      SDL_RenderCopy(renderer, textTex, nullptr, &textBox2); //draw text onto box
      SDL_DestroyTexture(textTex);

    }
  }
}

void quitConfirm(void) {
  SDL_Rect textBoxBorder = { //create box that servers as the border
    (SCREEN_WIDTH - START_SCREEN_WIDTH) / 2,
    (SCREEN_HEIGHT - START_SCREEN_HEIGHT) / 2,
    START_SCREEN_WIDTH,
    START_SCREEN_HEIGHT
  };

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //border is white
  SDL_RenderFillRect(renderer, &textBoxBorder);

  SDL_Rect textBox = { //create inner box
    textBoxBorder.x + 2,
    textBoxBorder.y + 2,
    textBoxBorder.w - 4,
    textBoxBorder.h - 4
  };

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1); //inner box is blue
  SDL_RenderFillRect(renderer, &textBox);


  SDL_Color textColor = {255, 255, 255, 255}; //box text is white
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, "Are you sure you want to quit? (Y/N)", textColor);

  if (textSurface == nullptr) {
    cerr << "Unable to create text surface, error: " << TTF_GetError() << endl;
  }

  else { 
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (textTex == nullptr) {
      cerr << "Unable to create text texture, error: " << SDL_GetError() << endl;
    }
    else { 
      int textWidth, textHeight;
      SDL_QueryTexture(textTex, nullptr, nullptr, &textWidth, &textHeight); //grab text attributes and put them into width/height

      SDL_Rect textBox2 = { //where on box to draw text
	textBoxBorder.x + (textBoxBorder.w - textWidth) / 2,
	textBoxBorder.y + (textBoxBorder.h - textHeight) / 2,
	textWidth,
	textHeight
      };

      SDL_RenderCopy(renderer, textTex, nullptr, &textBox2); //draw text onto box
      SDL_DestroyTexture(textTex);

    }
  }
}

void frameCounter(void) {
  SDL_Rect textBoxBorder = { //create box that servers as the border
    0,
    0,
    100,
    75
  };

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //border is white
  SDL_RenderFillRect(renderer, &textBoxBorder);

  SDL_Rect textBox = { //create inner box
    textBoxBorder.x + 2,
    textBoxBorder.y + 2,
    textBoxBorder.w - 4,
    textBoxBorder.h - 4
  };

  SDL_SetRenderDrawColor(renderer, 0, 0, 255, 1); //inner box is blue
  SDL_RenderFillRect(renderer, &textBox);

  string fps_text = to_string(fps);


  SDL_Color textColor = {255, 255, 255, 255}; //box text is white
  SDL_Surface* textSurface = TTF_RenderText_Solid(font, fps_text.c_str(), textColor);

  if (textSurface == nullptr) {
    cerr << "Unable to create text surface, error: " << TTF_GetError() << endl;
  }

  else { 
    SDL_Texture* textTex = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);

    if (textTex == nullptr) {
      cerr << "Unable to create text texture, error: " << SDL_GetError() << endl;
    }
    else { 
      int textWidth, textHeight;
      SDL_QueryTexture(textTex, nullptr, nullptr, &textWidth, &textHeight); //grab text attributes and put them into width/height

      SDL_Rect textBox2 = { //where on box to draw text
	textBoxBorder.x + (textBoxBorder.w - textWidth) / 2,
	textBoxBorder.y + (textBoxBorder.h - textHeight) / 2,
	textWidth,
	textHeight
      };

      SDL_RenderCopy(renderer, textTex, nullptr, &textBox2); //draw text onto box
      SDL_DestroyTexture(textTex);

    }
  }
}

