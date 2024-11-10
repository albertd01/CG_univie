#include <iostream>
#include "NetworkHandler.h"
#include "VideoDecoder.h"
#include "FrameRenderer.h"
#include "GameClient.h"
#include "Logger.h"


int main(int argv, char** args)
{
    LOG_INFO("Application started.");
    WSADATA wsa;
    auto err = WSAStartup(MAKEWORD(2, 2), &wsa);
    NetworkHandler handler;
    FrameRenderer renderer{1280, 720};
    VideoDecoder decoder;
    GameClient client(handler, decoder, renderer);
    client.run();

    WSACleanup();
    return 0;
}
