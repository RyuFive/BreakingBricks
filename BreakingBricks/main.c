#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "SDL.h"
#include <SDL_image.h>
#include <SDL_TTF.h>
#define null NULL

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
SDL_Event event;
SDL_Texture * textures[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
TTF_Font * font = NULL;
bool running = true;
int collision = 0;
int score = 0;
int framesPassed = 0;
float frequency = 10;
float waitTime;

struct Ball{
    SDL_Rect box;
    int velX;
    int velY;
    SDL_Texture * pic;
    struct Ball * next;
    struct Ball * prev;
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

struct Brick * brickHead = NULL;
struct Brick * brickTail = NULL;
struct Ball * ballHead = NULL;
struct Ball * ballTail = NULL;
struct Bat * bat = NULL;
struct Screen * screen = NULL;

void logSDLError(const char *  msg){
    printf("%sError: %s\n", msg, SDL_GetError());
    return;
}
SDL_Texture * loadTexture(const char * file){
    SDL_Texture * texture = IMG_LoadTexture(renderer, file);
    if (texture == NULL){
        logSDLError("CreateTexture");
        return NULL;
    }
    return texture;
}
SDL_Texture * renderText(const char * message, const char * fontFile, SDL_Color color, int fontSize, SDL_Renderer * renderer){
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
void initBall(int x, int y, int w, int h){
    SDL_Rect box;
    box.x = x;
    box.y = y;
    box.w = w;
    box.h = h;
    struct Ball * temp = ballHead;
    if (temp != NULL){
        if (SDL_HasIntersection(&box, &temp->box)){
            return;
        }
        temp = temp->next;
    }
    struct Ball * ptr = (struct Ball *)malloc(sizeof(struct Ball));
    ptr->box = box;
    ptr->pic = textures[0];
    ptr->velX = rand() % 11 - 5;
    ptr->velY = rand() % 11 - 5;
    while (ptr->velX == 0 || ptr->velY == 0){
        ptr->velX = rand() % 3 - 1;
        ptr->velY = rand() % 3 - 1;
    }
	if (ballHead != NULL){
		ballHead->prev = ptr;
	}
	ptr->next = ballHead;
	ptr->prev = NULL;
	ballHead = ptr;
	if (ballTail == NULL){
		ballTail = ptr;
	}
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
                case SDLK_b:{
                    initBall(screen->box.w / 2 - 8, screen->box.h / 2 - 8, 16, 16);
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
void scoreUp(){
    score += 100;
}
void deleteBrick(struct Brick * brick){
    if (brick->next == NULL && brick->prev == NULL){
        brickHead = NULL;
    }
    else if (brick->prev == NULL){
        brickHead = brick->next;
        brick->next->prev = NULL;
    }
    else if (brick->next == NULL){
        brick->prev->next = NULL;
    }
    else{
        brick->prev->next = brick->next;
        brick->next->prev = brick->prev;
    }
    scoreUp();
    free(brick);
}
void deleteBall(struct Ball * ball){
    if (ball->next == NULL && ball->prev == NULL){
        ballHead = NULL;
        ballTail = NULL;
        free(ball);
    }
    else if (ball->next == NULL){
        ballTail = ball->prev;
        ballTail->next = NULL;
        free(ball);
    }
    else if (ball->prev == NULL){
        ballHead = ball->next;
        ballHead->prev = NULL;
        free(ball);
    }
}
bool checkAlready(struct Ball * ball){
    if (SDL_HasIntersection(&(ball->box), &(bat->box))){
        return true;
    }
    return false;
}
bool checkHorizontal(struct Ball * ball){
    ball->box.x = ball->box.x + ball->velX;
    if (SDL_HasIntersection(&(ball->box), &(bat->box))){
        return true;
    }
    struct Brick * temp = brickHead;
    while (temp != NULL){
        if (SDL_HasIntersection(&(ball->box), &(temp->box))){
            struct Brick * del = temp;
            deleteBrick(del);
            return true;
        }
        temp = temp->next;
    }
    ball->box.x = ball->box.x - ball->velX;
    return false;
}
bool checkVertical(struct Ball * ball){
    ball->box.y = ball->box.y + ball->velY;
    if (SDL_HasIntersection(&(ball->box), &(bat->box))){
        return true;
    }
    struct Brick * temp = brickHead;
    temp = brickHead;
    while (temp != NULL){
        if (SDL_HasIntersection(&(ball->box), &(temp->box))){
            struct Brick * del = temp;
            deleteBrick(del);
            return true;
        }
        temp = temp->next;
    }
    ball->box.y = ball->box.y - ball->velY;
    return false;
}
bool checkBoth(struct Ball * ball){
    ball->box.x = ball->box.x + ball->velX;
    ball->box.y = ball->box.y + ball->velY;
    if (SDL_HasIntersection(&(ball->box), &(bat->box))){
        return true;
    }
    struct Brick * temp = brickHead;
    temp = brickHead;
    while (temp != NULL){
        if (SDL_HasIntersection(&(ball->box), &(temp->box))){
            struct Brick * del = temp;
            deleteBrick(del);
            return true;
        }
        temp = temp->next;
    }
    ball->box.x = ball->box.x - ball->velX;
    ball->box.y = ball->box.y - ball->velY;
    return false;
}
bool checkHorizontalBall(struct Ball * ball){
    ball->box.x = ball->box.x + ball->velX;
    struct Ball * temp = ballHead;
    while (temp != NULL && temp != ball){
        if (SDL_HasIntersection(&(ball->box), &(temp->box))){
            int temporary = temp->velX;
            temp->velX = ball->velX;
            ball->velX = temporary;
            return true;
        }
        temp = temp->next;
    }
    ball->box.x = ball->box.x - ball->velX;
    return false;
}
bool checkVerticalBall(struct Ball * ball){
    ball->box.y = ball->box.y + ball->velY;
    struct Ball * temp = ballHead;
    while (temp != NULL && temp != ball){
        if (SDL_HasIntersection(&(ball->box), &(temp->box))){
            int temporary = temp->velY;
            temp->velY = ball->velY;
            ball->velY = temporary;
            return true;
        }
        temp = temp->next;
    }
    ball->box.y = ball->box.y - ball->velY;
    return false;
}
bool checkBothBall(struct Ball * ball){
    ball->box.x = ball->box.x + ball->velX;
    ball->box.y = ball->box.y + ball->velY;
    struct Ball * temp = ballHead;
    while (temp != NULL && temp != ball){
        if (SDL_HasIntersection(&(ball->box), &(temp->box))){
            int temporary = temp->velX;
            temp->velX = ball->velX;
            ball->velX = temporary;
            temporary = temp->velY;
            temp->velY = ball->velY;
            ball->velY = temporary;
            return true;
        }
        temp = temp->next;
    }
    ball->box.x = ball->box.x - ball->velX;
    ball->box.y = ball->box.y - ball->velY;
    return false;
}
void collisionWalls(struct Ball * ball){
    if (ball->box.x <= screen->box.x || ball->box.x + ball->box.w >= screen->box.w)
    {
        ball->velX = ball->velX * -1;
    }
    if (ball->box.y <= screen->box.y)
    {
        ball->velY = ball->velY * -1;
    }
    if (ball->box.y >= screen->box.h){
        deleteBall(ball);
    }
}
void collisionBlocks(struct Ball * ball){
    if (checkAlready(ball)){
        ball->velX = bat->speed;
        ball->box.x = ball->box.x - bat->move[2];
        ball->box.x = ball->box.x + bat->move[3];
        if (ball->velX > 0 && bat->move[3] == 0){
            ball->velX *= -1;
        }
        if (ball->velX < 0 && bat->move[2] == 0){
            ball->velX *= -1;
        }
    }
    else if (checkHorizontal(ball)){
        ball->velX *= -1;
    }
    else if (checkVertical(ball)){
        ball->velY *= -1;
    }
    else if (checkBoth(ball)){
        ball->velX *= -1;
        ball->velY *= -1;
    }
}
void collisionBalls(struct Ball * ball){
    if (checkHorizontalBall(ball)){
    }
    else if (checkVerticalBall(ball)){
    }
    else if (checkBothBall(ball)){
    }
}
void collisionCorrection(struct Ball * ball){
    collisionWalls(ball);
    collisionBlocks(ball);
    collisionBalls(ball);
}
void moveBall(struct Ball * ball){
    if (framesPassed % 1500 == 0){
        if (ball->velX < 0){
            ball->velX -= 1;
        }
        else {
            ball->velX += 1;
        }
        if (ball->velY < 0){
            ball->velY -= 1;
        }
        else {
            ball->velY += 1;
        }
    }
    printf("velocity: %d, %d\n", ball->velX, ball->velY);
    ball->box.x = ball->box.x + ball->velX;
    ball->box.y = ball->box.y + ball->velY;
}
void moveBat(){
    if (bat->box.x - bat->move[2] < screen->box.x || bat->box.x + bat->move[3] + bat->box.w > screen->box.w){
        return;
    }
    bat->box.x -= bat->move[2];
    bat->box.x += bat->move[3];
}
void correctVel(struct Ball * temp){
    while(temp->velX == 0){
        temp->velX = rand() % 3 - 1;
    }
    while(temp->velY == 0){
        temp->velY = rand() % 3 - 1;
    }
}
void update(){
    struct Ball * temp = ballHead;
    while (temp != null){
        correctVel(temp);
        moveBall(temp);
        collisionCorrection(temp);
        temp = temp->next;
    }
    moveBat();
}
void render(){
    SDL_RenderClear(renderer);
    struct Brick * temp = brickHead;
    while (temp != NULL){
        renderTextureScale(temp->pic, temp->box.x, temp->box.y, temp->box.w, temp->box.h);
        temp = temp->next;
    }
    renderTextureScale(bat->pic, bat->box.x, bat->box.y, bat->box.w, bat->box.h);
    struct Ball * temp2 = ballHead;
    while (temp2 != NULL){
        renderTextureScale(temp2->pic, temp2->box.x, temp2->box.y, temp2->box.w, temp2->box.h);
        temp2 = temp2->next;
    }
    SDL_RenderPresent(renderer);
}
void fps(){
    framesPassed++;
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), waitTime)) {
    }
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
    bat->speed = 8;
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
void initBrick(int x, int y, int w, int h){
    struct Brick * ptr = (struct Brick *)malloc(sizeof(struct Brick));
	ptr->box.x = x;
    ptr->box.y = y;
    ptr->box.w = w;
    ptr->box.h = h;
    ptr->pic = textures[2];
	if (brickHead != NULL){
		brickHead->prev = ptr;
	}
	ptr->next = brickHead;
	ptr->prev = NULL;
	brickHead = ptr;
	if (brickTail == NULL){
		brickTail = ptr;
	}
}
void initEverything(){
    time_t t;
    srand((unsigned) time(&t));
    initScreen();
    initSDL(); //INIT SDL,WINDOW,RENDERER
    loadTextures();
    for (int i = 40; i < 760; i += 80 ){
        for (int j = 10; j < 250; j += 20){
            initBrick(i, j, 80, 20);
            j += 10;
        }
    }
    initBall(screen->box.w / 2 - 8, screen->box.h / 2 - 8, 16, 16);
    initBall(screen->box.w / 2 - 8, screen->box.h / 2 - 8, 16, 16);
    initBall(screen->box.w / 2 - 8, screen->box.h / 2 - 8, 16, 16);
    initBat();
}
void killEverything(){
    for(int i=0; textures[i] != NULL; i++){
        SDL_DestroyTexture(textures[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    IMG_Quit();
    SDL_Quit();
}
void gameLoop(){
    while(running){
        waitTime = SDL_GetTicks() + frequency;
        update();
        render();
        handleEvents();
        fps();
    }
}

int main(int argc, char* argv[]){
    initEverything();
    gameLoop();
    killEverything();
    return 0;
}

