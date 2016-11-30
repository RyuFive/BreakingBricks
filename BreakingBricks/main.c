#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>
#include "SDL.h"
#include <SDL_image.h>
#include <SDL_TTF.h>

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
SDL_Event event;
SDL_Texture * textures[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
TTF_Font * font = NULL;
bool running = true;
int collision = 0;

struct Ball{
    SDL_Rect box;
    int velX;
    int velY;
    SDL_Texture * pic;
};
struct Screen{
    SDL_Rect box;
    SDL_Texture * pic;
};
struct Bat{
    SDL_Rect box;
    int speed;
    int move[4];
    SDL_Texture * pic;

};
struct Brick{
    SDL_Rect box;
    SDL_Texture * pic;
    struct Brick * next;
    struct Brick * prev;
};

struct Ball * ball = NULL;
struct Screen * screen = NULL;
struct Bat * bat = NULL;
struct Brick * brick = NULL;


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
                    running = false;
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
                    running = false;
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
    if (ball->box.x == screen->box.x || ball->box.x + ball->box.w == screen->box.w)
    {
        ball->velX = ball->velX * -1;
    }
    if (ball->box.y == screen->box.y || ball->box.y + ball->box.h == screen->box.h)
    {
        ball->velY = ball->velY * -1;
    }
}
void deleteBrick(struct Brick * del){
    if (del->next == NULL && del->prev == NULL){
        brick = NULL;
    }
    else if (del->prev == NULL){
        brick = del->next;
        del->next->prev = NULL;
    }
    else if (del->next == NULL){
        del->prev->next = NULL;
    }
    else{
        del->prev->next = del->next;
        del->next->prev = del->prev;
    }
    free(del);
}
void moveBall(){
    ball->box.x = ball->box.x + ball->velX;
    if (SDL_HasIntersection(&(ball->box), &(bat->box))){
        ball->velX *= -1;
        ball->box.x = ball->box.x + ball->velX;
    }
    struct Brick * temp = brick;
    while (temp != NULL){
        if (SDL_HasIntersection(&(ball->box), &(temp->box))){
            ball->velX *= -1;
            ball->box.x = ball->box.x + ball->velX;
            struct Brick * del = temp;
            deleteBrick(del);
        }
        temp = temp->next;
    }
    ball->box.y = ball->box.y + ball->velY;
    if (SDL_HasIntersection(&(ball->box), &(temp->box))){
        ball->velY *= -1;
        ball->box.y = ball->box.y + ball->velY;
    }
    temp = brick;
    while (temp != NULL){
        if (SDL_HasIntersection(&(ball->box), &(temp->box))){
            ball->velY *= -1;
            ball->box.y = ball->box.y + ball->velY;
            struct Brick * del = temp;
            deleteBrick(del);
        }
        temp = temp->next;
    }
}
void moveBat(){
    if (bat->box.x - bat->move[2] < screen->box.x || bat->box.x + bat->move[3] + bat->box.w > screen->box.w){
        return;
    }
    bat->box.x -= bat->move[2];
    bat->box.x += bat->move[3];
}
void update(){
    collisionDetection();
    moveBall();
    moveBat();
}
void render(){
    SDL_RenderClear(renderer);
    struct Brick * temp = brick;
    while (temp != NULL){
        renderTextureScale(temp->pic, temp->box.x, temp->box.y, temp->box.w, temp->box.h);
        temp = temp->next;
    }
    renderTextureScale(bat->pic, bat->box.x, bat->box.y, bat->box.w, bat->box.h);
    renderTextureScale(ball->pic, ball->box.x, ball->box.y, ball->box.w, ball->box.h);
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
    window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen->box.w, screen->box.h, SDL_WINDOW_OPENGL);
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
    bat->box.w = 100;
    bat->box.h = 15;
    bat->box.x = screen->box.w / 2 - bat->box.w / 2;
    bat->box.y = screen->box.h - 30;
    bat->speed = 1;
    bat->move[0] = 0;
    bat->move[1] = 0;
    bat->move[2] = 0;
    bat->move[3] = 0;
    bat->pic = textures[1];
}
void initScreen(){
    screen = (struct Screen *)malloc(sizeof(struct Screen));
    screen->box.x = 0;
    screen->box.y = 0;
    screen->box.w = 800;
    screen->box.h = 600;
    screen->pic = NULL;
}
void initBall(){
    ball = (struct Ball *)malloc(sizeof(struct Ball));
    ball->box.w = 16;
    ball->box.h = 16;
    ball->box.x = screen->box.w / 2 - ball->box.w / 2;
    ball->box.y = screen->box.h / 2 - ball->box.h / 2;
    ball->velX = 1;
    ball->velY = 1;
    ball->pic = textures[0];
}
void initBrick(){
    for (int i = 10; i < 800; i += 100 ){
        for (int j = 10; j < 300; j += 30){
            struct Brick * ptr = (struct Brick *)malloc(sizeof(struct Brick));
            ptr->box.x = i;
            ptr->box.y = j;
            ptr->box.w = 80;
            ptr->box.h = 20;
            ptr->pic = textures[2];
            if (brick != NULL){
                brick->prev = ptr;
            }
            ptr->next = brick;
            ptr->prev = NULL;
            brick = ptr;
            j += 10;
        }

    }
}
void initEverything(){
    time_t t;
    srand((unsigned) time(&t));
    initScreen();
    initSDL(); //INIT SDL,WINDOW,RENDERER
    loadTextures();
    initBrick();
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
void gameLoop(){
    while(running){
        handleEvents();
        update();
        render();
    }
}

int main(int argc, char* argv[]){
    initEverything();
    gameLoop();
    killEverything();
    return 0;
}
