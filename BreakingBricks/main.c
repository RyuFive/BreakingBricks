#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include "SDL.h"
#include <SDL_image.h>
#include <SDL_TTF.h>

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
SDL_Event event;
SDL_Texture * textures[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
TTF_Font * font = NULL;
int running = 1;

struct Ball{
    int posX;
    int posY;
    int velX;
    int velY;
    int width;
    int height;
    SDL_Texture * pic;
};

struct Screen{
    int posX;
    int posY;
    int width;
    int height;
    SDL_Texture * pic;
};

struct Ball * ball;
struct Screen * screen;

void logSDLError(const char *  msg){
    printf("%sError: %s\n", msg, SDL_GetError());
    return;
}
SDL_Texture* loadTexture(const char * file){
    SDL_Texture * texture = IMG_LoadTexture(renderer, file);
    if (texture == NULL){
        logSDLError("CreateTexture");
        return NULL;
    }
    return texture;
}
SDL_Texture* renderText(const char * message, const char * fontFile, SDL_Color color, int fontSize, SDL_Renderer * renderer){
	font = TTF_OpenFont(fontFile, fontSize);
	if (font == NULL){
		logSDLError("TTF_OpenFont");
		return NULL;
	}
	SDL_Surface * surface = TTF_RenderText_Blended(font, message, color);
	if (surface == NULL){
		TTF_CloseFont(font);
		logSDLError("TTF_RenderText");
		return NULL;
	}
	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (texture == NULL){
		logSDLError("CreateTexture");
	}
	SDL_FreeSurface(surface);
	return texture;
}
void renderTextureScale(SDL_Texture * texture, int x, int y, int w, int h){
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}
void renderTexture(SDL_Texture * texture, int x, int y){
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}
void keyboardMovement(SDL_Event event){
    switch(event.key.keysym.sym){
        case SDLK_UP:{
            break;
        }
        case SDLK_DOWN:{
            break;
        }
        case SDLK_LEFT:{
            break;
        }
        case SDLK_RIGHT:{
            break;
        }
        case SDLK_x:{
            running = 0;
            break;
        }
        default:{
        }
    }
}
void handleEvents(){
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT){
            running = 0;
        }
        if (event.type == SDL_KEYDOWN){
            keyboardMovement(event);
        }
//      event.type == SDL_MOUSEBUTTONDOWN
    }
}
void loadTextures(){
    textures[0] = loadTexture("res/ball.png");
    textures[1] = loadTexture("res/bat.png");
    textures[2] = loadTexture("res/brick.png");
}
void collisionDetection(){
    if (ball->posX == screen->posX || ball->posX == screen->width)
    {
        ball->velX = ball->velX * -1;
    }
    if (ball->posY == screen->posY || ball->posY == screen->height)
    {
        ball->velY = ball->velY * -1;
    }
}
void moveBall(){
    ball->posX = ball->posX + ball->velX;
    ball->posY = ball->posY + ball->velY;
}
void update(){
    collisionDetection();
    moveBall();
}
void render(){
    SDL_RenderClear(renderer);
    renderTextureScale(textures[0], ball->posX, ball->posY, ball->width, ball->height);
    SDL_RenderPresent(renderer);
}
void initSDL(){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        logSDLError("Init");
        return;
    }
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG){
        logSDLError("IMG_Init");
        SDL_Quit();
        return;
    }
    if (TTF_Init() != 0){
        logSDLError("TTF_Init");
        SDL_Quit();
    }
    window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
    if (window == NULL){
        logSDLError("CreateWindow");
        SDL_Quit();
        return;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL){
        SDL_DestroyWindow(window);
        logSDLError("CreateRenderer");
        SDL_Quit();
        return;
    }
}
void initScreen(){
    screen = (struct Screen *)malloc(sizeof(struct Screen));
    screen->posX = 0;
    screen->posY = 0;
    screen->width = 640;
    screen->height = 480;
    screen->pic = NULL;
}
void initBall(){
    ball = (struct Ball *)malloc(sizeof(struct Ball));
    ball->width = 16;
    ball->height = 16;
    ball->posX = screen->width / 2-ball->width/2;
    ball->posY = screen->height / 2-ball->height/2;
    ball->velX = 1;
    ball->velY = 1;
    ball->pic = textures[0];
}
void initEverything(){
    time_t t;
    srand((unsigned) time(&t));
    initSDL(); //INIT SDL,WINDOW,RENDERER
    initScreen();
    initBall();
    loadTextures();
}
void killEverything(){
    for(int i=0; textures[i] != NULL; i++){
        SDL_DestroyTexture(textures[i]);
    }
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]){
    initEverything();

    while(running){
        handleEvents();
        update();
        render();
    }

    killEverything();
    return 0;
}
