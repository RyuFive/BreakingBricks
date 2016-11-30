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
struct Bat{
    int posX;
    int poxY;
    int width;
    int height;
    int speed;
    int move[4];
    SDL_Texture * pic;

};

struct Ball * ball;
struct Screen * screen;
struct Bat * bat;


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
void handleEvents(){
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT){
            running = 0;
        }
        if (event.type == SDL_KEYDOWN){
            switch(event.key.keysym.sym){
                case SDLK_LEFT:{
                    bat->move[2] = bat->speed;
                    break;
                }
                case SDLK_RIGHT:{
                    bat->move[3] = bat->speed;
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
        if (event.type == SDL_KEYUP){
            switch(event.key.keysym.sym){
                case SDLK_LEFT:{
                    bat->move[2] = 0;
                    break;
                }
                case SDLK_RIGHT:{
                    bat->move[3] = 0;
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
//      event.type == SDL_MOUSEBUTTONDOWN
    }
}
void loadTextures(){
    textures[0] = loadTexture("res/ball.png");
    textures[1] = loadTexture("res/bat.png");
    textures[2] = loadTexture("res/brick.png");
}
void collisionDetection(){
    //Collision with walls
    if (ball->posX == screen->posX || ball->posX == screen->width)
    {
        ball->velX = ball->velX * -1;
    }
    if (ball->posY == screen->posY || ball->posY == screen->height)
    {
        ball->velY = ball->velY * -1;
    }
    //Collision with bat
    if (ball->posY + ball->height == bat->posX){
        if (ball->posX + ball->width > bat->posX && ball->posX + ball->width < bat->posX + bat->width){
            ball->velY *= -1;
        }
        if (ball->posX > bat->posX && ball->posX < bat->posX + bat->width){
            ball->velY *= -1;
        }
    }

}
void moveBall(){
    ball->posX = ball->posX + ball->velX;
    ball->posY = ball->posY + ball->velY;

}
void moveBat(){
    if (bat->posX - bat->move[2] < screen->posX || bat->posX + bat->move[3] + bat->width > screen->width){
        return;
    }
    bat->posX -= bat->move[2];
    bat->posX += bat->move[3];
}
void update(){
    collisionDetection();
    moveBall();
    moveBat();
}
void render(){
    SDL_RenderClear(renderer);
    renderTextureScale(ball->pic, ball->posX, ball->posY, ball->width, ball->height);
    renderTextureScale(bat->pic, bat->posX, bat->poxY, bat->width, bat->height);
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
    window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen->width, screen->height, SDL_WINDOW_OPENGL);
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
void initBat(){
    bat = (struct Bat *)malloc(sizeof(struct Bat));
    bat->width = 100;
    bat->height = 15;
    bat->posX = screen->width / 2 - bat->width / 2;
    bat->poxY = screen->height - 30;
    bat->speed = 1;
    bat->move[0] = 0;
    bat->move[1] = 0;
    bat->move[2] = 0;
    bat->move[3] = 0;
    bat->pic = textures[1];
}
void initScreen(){
    screen = (struct Screen *)malloc(sizeof(struct Screen));
    screen->posX = 0;
    screen->posY = 0;
    screen->width = 800;
    screen->height = 600;
    screen->pic = NULL;
}
void initBall(){
    ball = (struct Ball *)malloc(sizeof(struct Ball));
    ball->width = 8;
    ball->height = 8;
    ball->posX = screen->width / 2 - ball->width / 2;
    ball->posY = screen->height / 2 - ball->height / 2;
    ball->velX = 1;
    ball->velY = 1;
    ball->pic = textures[0];
}
void initEverything(){
    time_t t;
    srand((unsigned) time(&t));
    initScreen();
    initSDL(); //INIT SDL,WINDOW,RENDERER
    loadTextures();
    initBall();
    initBat();
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
