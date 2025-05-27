#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <math.h>

#define NUM_SPRITES 4 
const int ANIMATION_DELAY_MS = 150;

const float SPRITE_SCALE_FACTOR = 1.0f;
const float WINDOW_SCALE_FACTOR = 0.85f; 
const int SPRITE_VERTICAL_OFFSET = 0;  

typedef struct {
    int x, y;
    int active;
} Projectile;

// Hardcoded frame data from sprites/spritesheet.json
SDL_Rect spriteFrames[NUM_SPRITES] = {
    {1,   1,   128, 128}, 
    {131, 1,   128, 128},
    {1,   131, 128, 128}, 
    {131, 131, 128, 128} 
};

// Helper function to draw a filled circle  not used here tho
void SDL_RenderFillCircle(SDL_Renderer *renderer, int x, int y, int radius) {
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; 
            int dy = radius - h; 
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, x + dx, y + dy);
            }
        }
    }
}

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* imgTexture = NULL;
    SDL_Surface* loadedSurface = NULL;
    SDL_Texture* windowImgTexture = NULL;
    SDL_Surface* windowLoadedSurface = NULL;
    SDL_Texture* gameBgTexture = NULL;
    SDL_Surface* gameBgLoadedSurface = NULL; 
    SDL_Texture* spritesheetTexture = NULL;
    char spritePath[256];
    int currentSpriteFrame = 0;
    Uint32 lastAnimationTime = 0;
    int spriteX = 0;
    const int SPRITE_SPEED = 2;
    // Remove spriteTextures array and related code
    // int firstSpriteOriginalWidth = 0;
    int windowImgWidth = 0; // Declare windowImgWidth
    int windowImgHeight = 0; // Declare windowImgHeight

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_DisplayMode dm;
    if (SDL_GetCurrentDisplayMode(0, &dm) != 0) {
        printf("SDL_GetCurrentDisplayMode failed: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("Full Window Background Image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, dm.w, dm.h, SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    loadedSurface = IMG_Load("image.png");
    if (loadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\n", "image.png", IMG_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    imgTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if (imgTexture == NULL) {
        printf("Unable to create texture from %s! SDL Error: %s\n", "image.png", SDL_GetError());
        SDL_FreeSurface(loadedSurface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_FreeSurface(loadedSurface);

    windowLoadedSurface = IMG_Load("window.png");
    if (windowLoadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\\n", "window.png", IMG_GetError());
        SDL_DestroyTexture(imgTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    windowImgTexture = SDL_CreateTextureFromSurface(renderer, windowLoadedSurface);
    if (windowImgTexture == NULL) {
        printf("Unable to create texture from %s! SDL Error: %s\\n", "window.png", SDL_GetError());
        SDL_FreeSurface(windowLoadedSurface);
        SDL_DestroyTexture(imgTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
    SDL_FreeSurface(windowLoadedSurface);

    if (windowImgTexture != NULL) {
        SDL_QueryTexture(windowImgTexture, NULL, NULL, &windowImgWidth, &windowImgHeight);
    } else {
        printf("ERROR: windowImgTexture is NULL, cannot get dimensions for window.png.\n");
        windowImgWidth = 800;
        windowImgHeight = 600;
    }

    gameBgLoadedSurface = IMG_Load("gamebg.jpg");
    if (gameBgLoadedSurface == NULL) {
        printf("Unable to load image %s! SDL_image Error: %s\\n", "gamebg.jpg", IMG_GetError());
    } else {
        gameBgTexture = SDL_CreateTextureFromSurface(renderer, gameBgLoadedSurface);
        if (gameBgTexture == NULL) {
            printf("Unable to create texture from %s! SDL Error: %s\\n", "gamebg.jpg", SDL_GetError());
        }
        SDL_FreeSurface(gameBgLoadedSurface);
    }

    SDL_Rect windowDestRect;
    windowDestRect.w = windowImgWidth * WINDOW_SCALE_FACTOR;
    windowDestRect.h = windowImgHeight * WINDOW_SCALE_FACTOR;
    windowDestRect.x = (dm.w - windowDestRect.w) / 2;
    windowDestRect.y = (dm.h - windowDestRect.h) / 2;

    int firstSpriteOriginalWidth = spriteFrames[0].w;
    if (firstSpriteOriginalWidth > 0) {
        int scaledFirstSpriteWidth = firstSpriteOriginalWidth * SPRITE_SCALE_FACTOR;
        spriteX = windowDestRect.x + windowDestRect.w - scaledFirstSpriteWidth;
        printf("DEBUG: Sprite initial X (right edge): %d, First Sprite Original Width: %d, Scaled Width: %d\\n", spriteX, firstSpriteOriginalWidth, scaledFirstSpriteWidth);
    } else {
        spriteX = windowDestRect.x + windowDestRect.w / 2;
        firstSpriteOriginalWidth = 0;
        printf("DEBUG: spriteTextures[0] is NULL. Sprite initial X set to %d (center of window.png).\\n", spriteX);
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, imgTexture, NULL, NULL);
    SDL_RenderCopy(renderer, windowImgTexture, NULL, &windowDestRect);
    SDL_RenderPresent(renderer);

    printf("DEBUG: Screen dimensions: w=%d, h=%d\n", dm.w, dm.h);
    printf("DEBUG: window.png rect: x=%d, y=%d, w=%d, h=%d\n", windowDestRect.x, windowDestRect.y, windowDestRect.w, windowDestRect.h);

    int quit = 0;
    SDL_Event e;
    lastAnimationTime = 0;

    spritesheetTexture = IMG_LoadTexture(renderer, "sprites/spritesheet.png");
    if (!spritesheetTexture) {
        printf("Unable to load spritesheet! SDL_image Error: %s\n", IMG_GetError());
        SDL_DestroyTexture(imgTexture);
        SDL_DestroyTexture(windowImgTexture);
        if (gameBgTexture != NULL) SDL_DestroyTexture(gameBgTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    } else {
        printf("DEBUG: spritesheet.png loaded successfully!\n");
    }

    while (!quit) {
        Uint32 currentTime = SDL_GetTicks();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            if (e.type == SDL_KEYDOWN) {
                printf("DEBUG: Key pressed: %d\n", e.key.keysym.sym);
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = 1;
                }
            }
        }

        if (currentTime > lastAnimationTime + ANIMATION_DELAY_MS) {
            currentSpriteFrame = (currentSpriteFrame + 1) % NUM_SPRITES;
            lastAnimationTime = currentTime;
            printf("DEBUG: Animation frame: %d\n", currentSpriteFrame);
        }

        if (firstSpriteOriginalWidth > 0) {
            spriteX -= SPRITE_SPEED; 
            int scaledCurrentSpriteWidthForWrap = spriteFrames[currentSpriteFrame].w * SPRITE_SCALE_FACTOR;
            if (spriteX + scaledCurrentSpriteWidthForWrap < 0) { 
                spriteX = dm.w; 
            }
            printf("DEBUG: spriteX: %d\n", spriteX);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, imgTexture, NULL, NULL);
        SDL_RenderCopy(renderer, windowImgTexture, NULL, &windowDestRect);

        if (gameBgTexture != NULL) {
            SDL_Rect gameBgDestRect;
            int gameBgTopOffset = 30; 
            gameBgDestRect.x = windowDestRect.x;
            gameBgDestRect.y = windowDestRect.y + gameBgTopOffset;
            gameBgDestRect.w = windowDestRect.w; 
            gameBgDestRect.h = windowDestRect.h - gameBgTopOffset; 

            SDL_RenderCopy(renderer, gameBgTexture, NULL, &gameBgDestRect);
        }

        SDL_Rect srcRect = spriteFrames[currentSpriteFrame];
        SDL_Rect spriteDestRect;
        spriteDestRect.w = srcRect.w * SPRITE_SCALE_FACTOR;
        spriteDestRect.h = srcRect.h * SPRITE_SCALE_FACTOR;
        spriteDestRect.x = spriteX;
        spriteDestRect.y = windowDestRect.y + (windowDestRect.h - spriteDestRect.h) / 2 + SPRITE_VERTICAL_OFFSET;
        printf("DEBUG: Rendering frame %d at x=%d, y=%d, w=%d, h=%d\n", currentSpriteFrame, spriteDestRect.x, spriteDestRect.y, spriteDestRect.w, spriteDestRect.h);

        SDL_RenderCopyEx(renderer, spritesheetTexture, &srcRect, &spriteDestRect, 0, NULL, SDL_FLIP_HORIZONTAL);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderPresent(renderer);

        SDL_Delay(10);
    }

    SDL_DestroyTexture(imgTexture);
    SDL_DestroyTexture(windowImgTexture);
    if (gameBgTexture != NULL) { 
        SDL_DestroyTexture(gameBgTexture);
    }
    SDL_DestroyTexture(spritesheetTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
