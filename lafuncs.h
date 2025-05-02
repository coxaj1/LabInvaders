#ifndef LA_FUNCS_H_
#define LA_FUNCS_H_

inline const int SCREEN_WIDTH = 1280;
inline const int SCREEN_HEIGHT = 960;

inline const int START_SCREEN_WIDTH = 700;
inline const int START_SCREEN_HEIGHT = 200;

inline const int MSPEED = 2;
inline bool quit = false;
inline bool gameOver = false;
inline bool gameStarted = false;
inline int timer = 0;
inline int fps = 0;

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern SDL_Texture* ship1Tex;
extern SDL_Texture* ship2Tex;
extern SDL_Texture* enemyTex;
extern SDL_Texture* bulletTex;

extern TTF_Font* font;

extern int ship1x, ship1y, ship2x, ship2y, enemX, enemY, bulX, bulY;
extern int texW, texH, texW2, texH2, texW3, texH3, texW4, texH4;

extern const Uint8* currentKeyStates;

extern Mix_Music* blaster;

float bound (float val, float min, float max);

void init(void);
void initPlayers(void);
void cleanup(void);
void handleInput(void);
void updateScreen(void);
void startBox(void);
void endScreen(void);
void createRow(int windW, int enemyW, int enemyH, int speed, SDL_Texture* texture);
void makeEnemies(void);
void shoot(int playerX, int playerY, SDL_Texture* bulletTexture);
void scoreCounter(void);
void waitForKey(void);
void creditsScreen(void);
void timerBox(void);
void quitConfirm(void);
void frameCounter(void);

#endif
