#ifndef FRAME_RENDERER
#define FRAME_RENDERER

#include "SDL2/include/SDL.h"
#include <vector>
#include <iostream>

class FrameRenderer {
    int width;
    int height;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
public:
    FrameRenderer(int width, int height): width(width), height(height), renderer(nullptr), window(nullptr), texture(nullptr){};
    ~FrameRenderer(){
        cleanup();
    };

    void initialize(){
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        }

        window = SDL_CreateWindow("Cloud Game",SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
        if (!window) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!renderer) {
            std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        }

        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, width, height);
        if (!texture) {
            std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
        }
    };
    
    void renderFrame(DecodedFrame frame){
        int xsize = frame.w;
        int ysize = frame.h;

        LOG_INFO("Rendering frame - width: " + std::to_string(xsize) + " height: " + std::to_string(ysize));
        SwsContext* sws_ctx = sws_getContext(
            xsize, ysize, AV_PIX_FMT_YUV420P, xsize, ysize, AV_PIX_FMT_RGBA, SWS_BILINEAR, nullptr, nullptr, nullptr);

        if (!sws_ctx) {
            LOG_ERROR("Failed to create SWS context");
            return;
        }
        uint8_t* dst_data = new uint8_t[xsize * ysize * 4];

        int stride[1] = { xsize * 4 };

        //sws_scale(sws_ctx, (const uint8_t* const*)frame->data, frame->linesize, 0, ysize, &dst_data, stride);
        sws_scale(sws_ctx, frame.data, frame.linesize, 0, ysize, &dst_data, stride);

        SDL_Texture* texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            xsize, ysize);

        if (!texture) {
            std::cerr << "SDL_CreateTexture Error: " << SDL_GetError() << std::endl;
            delete[] dst_data;
            sws_freeContext(sws_ctx);
            return;
        }

        SDL_UpdateTexture(texture, nullptr, dst_data, xsize * 4);

        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, texture, nullptr, nullptr);

        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(texture);

        sws_freeContext(sws_ctx);

        delete[] dst_data;
    };

    void cleanup(){
        if (texture) {
            SDL_DestroyTexture(texture);
        }
        if (renderer) {
            SDL_DestroyRenderer(renderer);
        }
        if (window) {
            SDL_DestroyWindow(window);
        }
        SDL_Quit();
    };
};

#endif