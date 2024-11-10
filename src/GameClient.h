#ifndef GAME_CLIENT
#define GAME_CLIENT

#include <condition_variable>
#include <utility>

#include "NetworkHandler.h"
#include "VideoDecoder.h"
#include "FrameRenderer.h"
#include "Logger.h"
#include<queue>
#include <mutex>

class GameClient
{
public:
    GameClient(NetworkHandler  handler, VideoDecoder decoder, const FrameRenderer& renderer) :
    networkHandler(std::move(handler)), videoDecoder(decoder), videoRenderer(renderer), running(true)
    {
        networkHandler.initialize();
        videoRenderer.initialize();
        videoDecoder.initialize();
    };

    ~GameClient(){
        running = false;
        frameAvailable.notify_all();

        if (receiverThread.joinable()) {
            receiverThread.join();
        }

        videoDecoder.cleanup();
        videoRenderer.cleanup();
    }

    void run() {
        LOG_INFO("Client running.");

        receiverThread = std::thread(&GameClient::receiveAndDecodeFrames, this);

        while (running) {
            handleInput();
            renderFrames();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Small sleep to reduce CPU usage
        }
    }
    void handleInput() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                LOG_INFO("Received SDL Quit event - exiting now");
                running = false;
                exit(1);
                frameAvailable.notify_all();  // Wake up other threads - needed for other inputs
                break;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case SDLK_0:
                    LOG_INFO("Key 0 pressed");
                    networkHandler.send_input(0);
                    break;
                case SDLK_1:
                    LOG_INFO("Key 1 pressed");
                    networkHandler.send_input(1);
                    break;
                case SDLK_2:
                    LOG_INFO("Key 2 pressed");
                    networkHandler.send_input(2);
                    break;
                case SDLK_3:
                    LOG_INFO("Key 3 pressed");
                    networkHandler.send_input(3);
                    break;
                case SDLK_4:
                    LOG_INFO("Key 4 pressed");
                    networkHandler.send_input(4);
                    break;
                case SDLK_5:
                    LOG_INFO("Key 5 pressed");
                    networkHandler.send_input(5);
                    break;
                case SDLK_6:
                    LOG_INFO("Key 6 pressed");
                    networkHandler.send_input(6);
                    break;
                case SDLK_7:
                    LOG_INFO("Key 7 pressed");
                    networkHandler.send_input(7);
                    break;
                case SDLK_8:
                    LOG_INFO("Key 8 pressed");
                    networkHandler.send_input(8);
                    break;
                case SDLK_9:
                    LOG_INFO("Key 9 pressed");
                    networkHandler.send_input(9);
                    break;
                default:
                    break;
                }
            }
        }
    }
    void receiveAndDecodeFrames() {
        LOG_INFO("Receiving and decoding thread started");
        while (running) {
            EncodedFrame packet = networkHandler.receive_frame();  // Ensure this is non-blocking or times out
            videoDecoder.decode_packet(packet, frameQueue, queueMutex, frameAvailable);
        }
    }
    void renderFrames() {
        std::unique_lock<std::mutex> lock(queueMutex);
        if (frameQueue.empty()) {
            frameAvailable.wait_for(lock, std::chrono::milliseconds(10));
        }

        if (!frameQueue.empty()) {
            DecodedFrame frame = frameQueue.front();
            frameQueue.pop();
            lock.unlock();
            videoRenderer.renderFrame(frame);
        }
    }

private:
    NetworkHandler networkHandler;
    VideoDecoder videoDecoder;
    FrameRenderer videoRenderer;
    std::atomic<bool> running;
    std::condition_variable frameAvailable;

    std::thread receiverThread;
    std::queue<DecodedFrame> frameQueue;
    std::mutex queueMutex;
};

#endif