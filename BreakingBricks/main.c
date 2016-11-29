#include "SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL_image.h>

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
SDL_Texture * textures[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

void logSDLError(const char *  msg){
    printf("%sError: %s\n", msg, SDL_GetError());
}
SDL_Texture* loadTexture(const char * file){
    SDL_Texture * texture = IMG_LoadTexture(renderer, file);
    if (texture == NULL){
        logSDLError("CreateTexture");
    }
    return texture;
}
void renderTexture(SDL_Texture * texture, int x, int y){
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
}
void loadTextures(){
    textures[0] = loadTexture("res/ball.png");
    textures[1] = loadTexture("res/bat.png");
    textures[2] = loadTexture("res/brick.png");
}
void initSDL(){
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0){
        logSDLError("Init");
    }
    window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_OPENGL);
    if (window == NULL){
        logSDLError("CreateWindow");
        SDL_Quit();
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL){
        SDL_DestroyWindow(window);
        logSDLError("CreateRenderer");
        SDL_Quit();
    }
}
void initEverything(){
    time_t t;
    srand((unsigned) time(&t));
    initSDL(); //INIT SDL,WINDOW,RENDERER
    loadTextures();
}

int main(int argc, char* argv[]) {
    initEverything();

    SDL_Delay(500);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
