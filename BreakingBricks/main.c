#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include "windows.h"
#include "SDL.h"
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_TTF.h>
#define SCREEN_WIDTH GetSystemMetrics(SM_CXSCREEN)
#define SCREEN_HEIGHT GetSystemMetrics(SM_CYSCREEN)

SDL_Window * window = NULL;
SDL_Renderer * renderer = NULL;
SDL_Event event;
SDL_Texture * textures[10] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
FILE * maps[100];
TTF_Font * font= NULL;
SDL_Rect clips[25];
int map = 22;
int mapLevel = 3;
bool running = true;
int collision = 0;
int score = 0;
int framesPassed = 0;
float frequency = 10;
float waitTime;
int ballSize = 20;
int batSize = 200;
int batSpeed = 8;
int brickWidth = 0;
int brickHeight = 0;
int brickColor = 0;
int mapNoX = -1;
int mapNoY = 1;
int lives = 3;
int objects = 0;
bool rev = false;
char buff[8];
enum states {game, pause, menu, options, help};
enum states gameState = game;

struct Ball{
    SDL_Rect box;
    int velX;
    int velY;
    SDL_Texture * pic;
    struct Ball * next;
    struct Ball * prev;
};
struct Status{
    SDL_Rect box;
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
    struct Ball * ball;
};
struct Brick{
    SDL_Rect box;
    int brickColor;
    SDL_Texture * pic;
    struct Brick * next;
    struct Brick * prev;
};
struct Powerup{
    SDL_Rect box;
    int velX;
    int velY;
    int ability;
    SDL_Texture * pic;
    struct Powerup * next;
    struct Powerup * prev;
};
struct Node{
    int data;
    struct Node * next;
    struct Node * prev;
};

struct Node * nodeHead = NULL;
struct Node * nodeTail = NULL;
struct Brick * brickHead = NULL;
struct Brick * brickTail = NULL;
struct Ball * ballHead = NULL;
struct Ball * ballTail = NULL;
struct Bat * bat = NULL;
struct Powerup * powerupHead = NULL;
struct Powerup * powerupTail = NULL;
struct Screen * screen = NULL;
struct Status * status = NULL;

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
void renderTextureClip(SDL_Texture * texture, int x, int y, int w, int h, SDL_Rect * clips){
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderCopy(renderer, texture, clips, &rect);
}
void renderFont(TTF_Font * font, char * msg, int r, int g, int b, int a, int x, int y){
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    SDL_Color color = {r, g, b, a};
    SDL_Surface * surface = TTF_RenderText_Solid(font, msg, color);
    SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureAlphaMod(texture, a);
    SDL_QueryTexture(texture, NULL, NULL, &rect.w, &rect.h);
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}
void sliceTexture(){
    int iW = 16, iH = 8;
    for (int i = 0; i < 25; ++i){
        clips[i].x = i / 5 * iW;
        clips[i].y = i % 5 * iH;
        clips[i].w = iW;
        clips[i].h = iH;
    }
}
void initBall(int x, int y, int w, int h){
    if (bat->ball != NULL){
        return;
    }
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
    bat->ball = ptr;
    ptr->velX = 0;
    ptr->velY = 0;
	if (ballHead != NULL){
		ballHead->prev = ptr;
	}
	ptr->next = ballHead;
	ptr->prev = NULL;
	ballHead = ptr;
	if (ballTail == NULL){
		ballTail = ptr;
	}
	objects++;
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
        return;
    }
    font = TTF_OpenFont("res/ttf/electro.ttf", 46);
    if (font == NULL){
        logSDLError("TextLoad");
        return;
    }
    window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
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
    bat->box.w = batSize;
    bat->box.h = 20;
    bat->box.x = screen->box.w / 2 - bat->box.w / 2;
    bat->box.y = status->box.h + screen->box.h - 30;
    bat->speed = batSpeed;
    bat->ball = NULL;
    bat->move[0] = 0;
    bat->move[1] = 0;
    bat->move[2] = 0;
    bat->move[3] = 0;
    bat->pic = textures[1];
    objects++;
}
void initGameScreen(){
    screen = (struct Screen *)malloc(sizeof(struct Screen));
    screen->box.x = status->box.x;
    screen->box.y = status->box.h;
    screen->box.w = SCREEN_WIDTH;
    screen->box.h = SCREEN_HEIGHT - status->box.h;
    screen->pic = textures[3];
    objects++;
}
void initStatus(){
    status = (struct Status *)malloc(sizeof(struct Status));
    status->box.x = 0;
    status->box.y = 0;
    status->box.w = SCREEN_WIDTH;
    status->box.h = 50;
    status->pic = textures[6];
    objects++;
}
void initPowerup(int x, int y, int w, int h, int ability){
    struct Powerup * ptr = (struct Powerup *)malloc(sizeof(struct Powerup));
	ptr->box.x = x-w/2;
    ptr->box.y = y;
    ptr->box.w = w;
    ptr->box.h = h;
    ptr->velX = 0;
    ptr->velY = 1;
    ptr->ability = ability;
    ptr->pic = textures[7];
	if (powerupHead != NULL){
		powerupHead->prev = ptr;
	}
	ptr->next = powerupHead;
	ptr->prev = NULL;
	powerupHead = ptr;
	if (powerupTail == NULL){
		powerupTail = ptr;
	}
	objects++;
}
void initBrick(int x, int y, int w, int h, int brickColor){
    struct Brick * ptr = (struct Brick *)malloc(sizeof(struct Brick));
	ptr->box.x = x;
    ptr->box.y = y + status->box.h;
    ptr->box.w = w;
    ptr->box.h = h;
    ptr->brickColor = brickColor;
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
	objects++;
}
void setSpeed(struct Ball * ball){
    ball->velX = 1;
    ball->velY = -3;
}
void handleEvents(){
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT){
            running = 0;
        }
        if (event.type == SDL_KEYDOWN){
            switch(event.key.keysym.sym){
                case SDLK_LEFT:{
                    if (!rev){
                        bat->move[2] = bat->speed;
                    }
                    else{
                        bat->move[3] = bat->speed;
                    }
                    break;
                }
                case SDLK_RIGHT:{
                    if (!rev){
                        bat->move[3] = bat->speed;
                    }
                    else{
                        bat->move[2] = bat->speed;
                    }
                    break;
                }
                case SDLK_x:{
                    running = false;
                    break;
                }
                case SDLK_p:{
                    if (gameState == game){
                        gameState = pause;
                    }
                    else if (gameState == pause){
                        gameState = game;
                    }
                    break;
                }
                case SDLK_b:{
                    initBall(bat->box.x + bat->box.w / 2 - ballSize / 2, bat->box.y - ballSize, ballSize, ballSize);
                    break;
                }
                case SDLK_a:{
                    printf("No. of objects are: %d\n", objects);
                    break;
                }
                case SDLK_SPACE:{
                    if (gameState == game){
                        if (bat->ball){
                            setSpeed(bat->ball);
                            bat->ball = NULL;
                        }
                    }
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
                default:{
                }
            }
        }
        if (event.type == SDL_MOUSEMOTION){

        }
//      event.type == SDL_MOUSEBUTTONDOWN
    }
}
void loadMaps(){
    maps[0] = fopen("res/map/map0.txt", "r");
    maps[1] = fopen("res/map/map1.txt", "r");
    maps[2] = fopen("res/map/map2.txt", "r");
    maps[3] = fopen("res/map/map3.txt", "r");
    maps[4] = fopen("res/map/map4.txt", "r");
    maps[20] = fopen("res/map/map20.txt", "r");
    maps[21] = fopen("res/map/map21.txt", "r");
    maps[22] = fopen("res/map/map22.txt", "r");
}
void loadTextures(){
    textures[0] = loadTexture("res/img/ball.png");
    textures[1] = loadTexture("res/img/bat.png");
    textures[2] = loadTexture("res/img/brick.png");
    textures[3] = loadTexture("res/img/back.png");
    textures[4] = loadTexture("res/img/pause.png");
    textures[5] = loadTexture("res/img/bricks.png");
    textures[6] = loadTexture("res/img/status.png");
    textures[7] = loadTexture("res/img/powerup.png");
}
void timer(int selection){
}
void applyPowerup(int ability){
    switch(ability){
        case 0:{
            printf("Ball Speed UP\n");
            struct Ball * temp = ballHead;
            while(temp != NULL){
                if(temp->velX > 0){
                    temp->velX++;
                }
                else{
                    temp->velX--;
                }
                if(temp->velY > 0){
                    temp->velY++;
                }
                else{
                    temp->velY--;
                }
                temp = temp->next;
            }
            break;
        }
        case 1:{
            printf("Ball Speed DOWN\n");
            struct Ball * temp = ballHead;
            while(temp != NULL){
                if(temp->velX > 1){
                    temp->velX--;
                }
                else if (temp->velX < -1){
                    temp->velX++;
                }
                if(temp->velY > 1){
                    temp->velY--;
                }
                else if (temp->velY < -1){
                    temp->velY++;
                }
                temp = temp->next;
            }
            break;
        }
        case 2:{
            printf("Ball Size UP\n");
            struct Ball * temp = ballHead;
            while(temp != NULL){
                temp->box.w += 2;
                temp->box.h += 2;
                temp = temp->next;
            }
            break;
        }
        case 3:{
            printf("Ball Size DOWN\n");
            struct Ball * temp = ballHead;
            while(temp != NULL){
                if (temp->box.w > 8){
                    temp->box.w -= 2;
                    temp->box.h -= 2;
                }
                temp = temp->next;
            }
            break;
        }
        case 4:{
            printf("Bat Speed UP\n");
            bat->speed *= 1.25;
            break;
        }
        case 5:{
            printf("Bat Speed DOWN\n");
            if (bat->speed > 4){
                bat->speed *= 0.75;
            }
            break;
        }
        case 6:{
            printf("Bat Size UP\n");
            bat->box.w += 50;
            break;
        }
        case 7:{
            printf("Bat Size DOWN\n");
            if (bat->box.w > 100){
                bat->box.w -= 50;
            }
            break;
        }
        case 8:{
            printf("Lives UP\n");
            lives++;
            break;
        }
        case 9:{
            printf("Lives DOWN\n");
            lives--;
            break;
        }
        case 10:{
//            reversed controls
            printf("Reversing Controls\n");
            rev = true;
//            timer(1);
            break;
        }
        case 11:{
//            gun paddle
            printf("Gun Paddle\n");
            break;
        }
        case 12:{
//            split balls
            printf("Split Balls\n");
            break;
        }
        case 13:{
//            hyper balls
            printf("Hyper Balls\n");
            break;
        }
        case 14:{
            printf("Score UP\n");
            score += 1000;
            break;
        }
        case 15:{
            printf("Score DOWN\n");
            score -= 1000;
            break;
        }
    }
}
void scoreUp(){
    score += 100;
}
void convertToList(){
    mapNoX = -1;
    mapNoY = 1;
    brickWidth = 0;
    brickHeight = 0;
    int ch;
    while ((ch = fgetc(maps[map])) != EOF) {
        if (brickWidth == 0){
            while(ch != (char)10){
                brickWidth *= 10;
                brickWidth += ch - '0';
                ch = fgetc(maps[map]);
            }
            continue;
        }
        else if (brickHeight == 0){
            while(ch != (char)10){
                brickHeight *= 10;
                brickHeight += ch - '0';
                ch = fgetc(maps[map]);
            }
            continue;
        }
        if (mapNoY == 1){
            mapNoX++;
        }
        if (ch == (char)10){
            mapNoY++;
        }
        struct Node * ptr = (struct Node *)malloc(sizeof(struct Node));
        ptr->data = ch - '0';
        if (nodeHead != NULL){
            nodeHead->prev = ptr;
        }
        ptr->next = nodeHead;
        ptr->prev = NULL;
        nodeHead = ptr;
        if (nodeTail == NULL){
            nodeTail = ptr;
        }
    }
}
int dequeue(int n){
    int returnValue = 0;
    struct Node * temp = NULL;
    while (n > 0){
        temp = nodeTail;
        if (temp->data >= 0){
            returnValue *= 10;
            returnValue += temp->data;
            n--;
        }
        nodeTail = nodeTail->prev;
        if (nodeTail != NULL){
            nodeTail->next = NULL;
        }
        free(temp);
        if (nodeTail == NULL){
            nodeHead = NULL;
        }
    }
    return returnValue;
}
void generateMap(){
    int brickInit = 0, iW = 0, iH = 0;
    if(mapLevel == 1){
        iW = SCREEN_WIDTH / (mapNoX + 1) / 2;
        iH = status->box.h + SCREEN_HEIGHT / (mapNoX + 1) / 2;
    }
    else if (mapLevel == 3){
        mapNoX = mapNoX / 3;
        iW = 0;
        iH = status->box.h;
    }
    brickColor = rand() % 25;
    for (int i = 0;i < mapNoX * mapNoY; i++){
        brickInit = dequeue(mapLevel);
        if (mapLevel == 3){
            if (brickInit / 100 == 1){
                initBrick(iW, iH, brickWidth, brickHeight, brickInit - 100);
            }
            iW += SCREEN_WIDTH / (mapNoX);
            if(iW > SCREEN_WIDTH / mapNoX / 5 * 100){
                brickColor = rand() % 25;
                iW = 0;
                iH += SCREEN_HEIGHT / mapNoY / 2;
            }
        }
        if (mapLevel == 1){
            if (brickInit == 1){
                initBrick(iW, iH, brickWidth, brickHeight, brickColor);
            }
            iW += SCREEN_WIDTH / (mapNoX + 1);
            if(iW > SCREEN_WIDTH / (mapNoX + 1) / 10 * 95){
                brickColor = rand() % 25;
                iW = SCREEN_WIDTH / (mapNoX + 1) / 2;
                iH += SCREEN_HEIGHT / (mapNoY + 1) / 2;
            }
        }
    }
}
void randomPowerup(struct Brick * brick){
    if (rand() % 100 < 15){
        initPowerup(brick->box.x + brick->box.w / 2, brick->box.y, 32, 32, rand() % 16);
    }
}
void deleteBrick(struct Brick * brick){
    randomPowerup(brick);
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
    objects--;
}
void deletePowerup(struct Powerup * powerup){
    if (powerup->next == NULL && powerup->prev == NULL){
        powerupHead = NULL;
        powerupTail = NULL;
    }
    else if (powerup->next == NULL){
        powerupTail = powerup->prev;
        powerupTail->next = NULL;
    }
    else if (powerup->prev == NULL){
        powerupHead = powerup->next;
        powerupHead->prev = NULL;
    }
    else{
        powerup->prev->next = powerup->next;
        powerup->next->prev = powerup->prev;
    }
    free(powerup);
    objects--;
}
void deleteBall(struct Ball * ball){
    if (ball->next == NULL && ball->prev == NULL){
        ballHead = NULL;
        ballTail = NULL;
    }
    else if (ball->next == NULL){
        ballTail = ball->prev;
        ballTail->next = NULL;
    }
    else if (ball->prev == NULL){
        ballHead = ball->next;
        ballHead->prev = NULL;
    }
    else{
        ball->prev->next = ball->next;
        ball->next->prev = ball->prev;
    }
    free(ball);
    objects--;
}
void movePowerup(struct Powerup * powerup){
    powerup->box.x += powerup->velX;
    powerup->box.y += powerup->velY;
}
void collisionPowerup(struct Powerup * powerup){
    if (SDL_HasIntersection(&(powerup->box), &(bat->box))){
        applyPowerup(powerup->ability);
        deletePowerup(powerup);
    }
    if (powerup->box.y >= screen->box.h + status->box.h){
        deletePowerup(powerup);
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
void adjustTilt(struct Ball * ball){
    ball->box.y = ball->box.y - ball->velY;
    int cen = bat->box.w / 2 + bat->box.x;
    int cenB = ball->box.w / 2 + ball->box.x;
    if (cenB > cen + 55){
        if (ball->velX < 0){
            ball->velX *= -1;
            return;
        }
    }
    else if (cenB < cen - 55){
        if (ball->velX > 0){
            ball->velX *= -1;
            return;
        }
    }
    if (ball->velX == 0){
        if (bat->move[2] > 0){
            ball->velX = -1;
        }
        else if (bat->move[3] > 0){
            ball->velX = 1;
        }
        else{
            ball->velX = rand() % 3 - 1;
        }
        ball->velY -= 1;
    }
    else if (bat->move[2] > 0){
        if (ball->velX > 0){
            ball->velX -= 1;
            ball->velY += 1;
        }
        else if (ball->velX < 0){
            if (ball->velY - 1 == 0){
                return;
            }
            ball->velX -= 1;
            ball->velY -= 1;
        }

    }
    else if (bat->move[3] > 0){
        if (ball->velX > 0){
            if (ball->velY - 1 == 0){
                return;
            }
            ball->velX += 1;
            ball->velY -= 1;
        }
        else if (ball->velX < 0){
            ball->velX += 1;
            ball->velY += 1;
        }
    }
    ball->box.y = ball->box.y + ball->velY;
}
bool checkVertical(struct Ball * ball){
    ball->box.y = ball->box.y + ball->velY;
    if (SDL_HasIntersection(&(ball->box), &(bat->box))){
        adjustTilt(ball);
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
void updateLives(int n){
    lives += n;
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
    if (ball->box.y >= screen->box.h + status->box.h){
        deleteBall(ball);
        if (lives > 0 && ballHead == NULL){
            updateLives(-1);
            initBall(bat->box.x + bat->box.w / 2 - ballSize / 2, bat->box.y - ballSize, ballSize, ballSize);
        }
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
void speedUpBallWithTime(struct Ball * ball){
    if (framesPassed % 2500 == 0){
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
}
void moveBall(struct Ball * ball){
//    speedUpBallWithTime(ball);
    if (bat->ball == ball){
        ball->box.x = bat->box.x + bat->box.w / 2 - ballSize / 2;
        ball->box.y = bat->box.y - ballSize;
    }
    else{
        ball->box.x = ball->box.x + ball->velX;
        ball->box.y = ball->box.y + ball->velY;
    }
}
void moveBat(){
    if (bat->box.x - bat->move[2] < screen->box.x || bat->box.x + bat->move[3] + bat->box.w > screen->box.w){
        return;
    }
    bat->box.x -= bat->move[2];
    bat->box.x += bat->move[3];
}
void correctVel(struct Ball * ball){
    if (bat->ball != NULL){
        while(ball->velY == 0){
            ball->velY = rand() % 3 - 1;
        }
    }
}
void fps(float frequency){
    framesPassed++;
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), waitTime)) {
    }
    waitTime = SDL_GetTicks() + frequency;
}
void update(){
    fps(frequency);
    if (gameState == menu){
    }
    if (gameState == game){
        struct Ball * temp = ballHead;
        while (temp != NULL){
            moveBall(temp);
            collisionCorrection(temp);
            temp = temp->next;
        }
        struct Powerup * temp2 = powerupHead;
        while (temp2 != NULL){
            movePowerup(temp2);
            collisionPowerup(temp2);
            temp2 = temp2->next;
        }
        if (brickHead == NULL){
            while (ballHead != NULL){
                deleteBall(ballHead);
            }
            map++;
            convertToList();
            generateMap();
            lives++;
            initBall(bat->box.x + bat->box.w / 2 - ballSize / 2, bat->box.y - ballSize, ballSize, ballSize);
        }
        moveBat();
    }
    if (gameState == pause){
    }
}
void render(){
    SDL_RenderClear(renderer);
    if (gameState == menu){

    }
    if (gameState == game || gameState == pause){
        renderTextureScale(status->pic, status->box.x, status->box.y, status->box.w, status->box.h);
        sprintf(buff, "Lives: %d", lives);
        renderFont(font, buff, 150, 150, 255, 200, 1100, 0);
        sprintf(buff, "Score: %d", score);
        renderFont(font, buff, 150, 150, 255, 200, 80, 0);
        renderTextureScale(screen->pic, screen->box.x, screen->box.y, screen->box.w, screen->box.h);
        struct Brick * temp = brickHead;
        while (temp != NULL){
            renderTextureClip(textures[5], temp->box.x, temp->box.y, temp->box.w, temp->box.h, &clips[temp->brickColor]);
            temp = temp->next;
        }
        renderTextureScale(bat->pic, bat->box.x, bat->box.y, bat->box.w, bat->box.h);
        struct Ball * temp2 = ballHead;
        while (temp2 != NULL){
            renderTextureScale(temp2->pic, temp2->box.x, temp2->box.y, temp2->box.w, temp2->box.h);
            temp2 = temp2->next;
        }
        struct Powerup * temp3 = powerupHead;
        while (temp3 != NULL){
            renderTextureScale(temp3->pic, temp3->box.x, temp3->box.y, temp3->box.w, temp3->box.h);
            temp3 = temp3->next;
        }
    }
    if (gameState == pause){
        renderTextureScale(textures[4], screen->box.x, screen->box.y, screen->box.w, screen->box.h);
    }
    SDL_RenderPresent(renderer);
}
void initEverything(){
    time_t t;
    srand((unsigned) time(&t));
    initSDL(); //INIT SDL,WINDOW,RENDERER
    loadTextures();
    sliceTexture(textures[5]);
    loadMaps();
    initStatus();
    initGameScreen();
    convertToList();
    generateMap();
    initBat();
    initBall(bat->box.x + bat->box.w / 2 - ballSize / 2, bat->box.y - ballSize, ballSize, ballSize);
}
void killEverything(){
    for(int i=0; textures[i] != NULL; i++){
        SDL_DestroyTexture(textures[i]);
    }
    for(int i=0; maps[i] != NULL; i++){
        fclose(maps[i]);
    }
    free(screen);
    free(status);
    free(bat);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    IMG_Quit();
    SDL_Quit();
}
void gameLoop(){
    while(running){
        update();
        render();
        handleEvents();
    }
}

int main(int argc, char* argv[]){
    initEverything();
    gameLoop();
    killEverything();
    return 0;
}
