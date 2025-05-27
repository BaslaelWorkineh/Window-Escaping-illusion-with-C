#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>

#define NUM_SPRITES 6
const int ANIMATION_DELAY_MS = 150;

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Texture* imgTexture = NULL;
    SDL_Surface* loadedSurface = NULL;
    SDL_Texture* windowImgTexture = NULL;
    SDL_Surface* windowLoadedSurface = NULL;
    SDL_Texture* spriteTextures[NUM_SPRITES] = {NULL};
    char spritePath[256];
    int currentSpriteFrame = 0;
    Uint32 lastAnimationTime = 0;
    int spriteX = 0;
    const int SPRITE_SPEED = 2;
    int firstSpriteWidth = 0;

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

    for (int i = 0; i < NUM_SPRITES; ++i) {
        snprintf(spritePath, sizeof(spritePath), "sprites/%d.png", i + 1);
        SDL_Surface* spriteSurface = IMG_Load(spritePath);
        if (spriteSurface == NULL) {
            printf("Unable to load sprite %s! SDL_image Error: %s\\n", spritePath, IMG_GetError());
        } else {
            spriteTextures[i] = SDL_CreateTextureFromSurface(renderer, spriteSurface);
            if (spriteTextures[i] == NULL) {
                printf("Unable to create texture from sprite %s! SDL Error: %s\\n", spritePath, SDL_GetError());
            }
            SDL_FreeSurface(spriteSurface);
        }
    }

    int windowImgWidth, windowImgHeight;
    SDL_QueryTexture(windowImgTexture, NULL, NULL, &windowImgWidth, &windowImgHeight);

    SDL_Rect windowDestRect;
    windowDestRect.w = windowImgWidth;
    windowDestRect.h = windowImgHeight;
    windowDestRect.x = (dm.w - windowDestRect.w) / 2;
    windowDestRect.y = (dm.h - windowDestRect.h) / 2;

    if (spriteTextures[0] != NULL) {
        SDL_QueryTexture(spriteTextures[0], NULL, NULL, &firstSpriteWidth, NULL);
        spriteX = windowDestRect.x + (windowDestRect.w - firstSpriteWidth) / 2;
        if (firstSpriteWidth > windowDestRect.w) {
            spriteX = windowDestRect.x;
        }
    } else {
        spriteX = 0;
        firstSpriteWidth = 0;
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, imgTexture, NULL, NULL);
    SDL_RenderCopy(renderer, windowImgTexture, NULL, &windowDestRect);
    SDL_RenderPresent(renderer);

    int quit = 0;
    SDL_Event e;
    lastAnimationTime = SDL_GetTicks();

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    quit = 1;
                }
            }
        }

        Uint32 currentTime = SDL_GetTicks();
        if (currentTime > lastAnimationTime + ANIMATION_DELAY_MS) {
            currentSpriteFrame = (currentSpriteFrame + 1) % NUM_SPRITES;
            lastAnimationTime = currentTime;
        }

        if (firstSpriteWidth > 0) {
            spriteX += SPRITE_SPEED;
            if (spriteX > dm.w) {
                spriteX = 0 - firstSpriteWidth;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, imgTexture, NULL, NULL);
        SDL_RenderCopy(renderer, windowImgTexture, NULL, &windowDestRect);

        if (spriteTextures[currentSpriteFrame] != NULL) {
            int spriteWidth, spriteHeight;
            SDL_QueryTexture(spriteTextures[currentSpriteFrame], NULL, NULL, &spriteWidth, &spriteHeight);

            SDL_Rect spriteDestRect;
            spriteDestRect.w = spriteWidth;
            spriteDestRect.h = spriteHeight;
            spriteDestRect.x = spriteX;
            spriteDestRect.y = windowDestRect.y + (windowDestRect.h - spriteHeight) / 2;

            SDL_RenderCopy(renderer, spriteTextures[currentSpriteFrame], NULL, &spriteDestRect);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(10);
    }

    SDL_DestroyTexture(imgTexture);
    SDL_DestroyTexture(windowImgTexture);
    for (int i = 0; i < NUM_SPRITES; ++i) {
        if (spriteTextures[i] != NULL) {
            SDL_DestroyTexture(spriteTextures[i]);
        }
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
