#ifndef NETWORKHANDLER_H
#define NETWORKHANDLER_H

#include "UDPSend6.h"
#include "UDPReceive6.h"
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include "Logger.h"
#define INBUF_SIZE 65000

struct NetworkStatistics {
    int lastFrame = 0;
    int receivedFrames = 0;
    int droppedFrames = 0;
    int receivedBytes = 0;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    void resetStats() {
        lastFrame = 0;
        receivedFrames = 0;
        droppedFrames = 0;
        receivedBytes = 0;
        start = std::chrono::steady_clock::now();
    }
};

struct EncodedFrame{
    uint8_t* data;
    int buf_size;
};


class NetworkHandler{
    typedef struct Input {
        int		value;
    } Input_t;
    UDPSend6 input_sender;

    UDPReceive6 receiver;
    void init_input_sender(){
        input_sender.init((char*)"::1", 50001);

    }

    void init_receiver(){
        receiver.init(50000);
    }
    public:
    void initialize(){
        LOG_INFO("Initializing network communication");
        init_input_sender();
        init_receiver();
    }

    EncodedFrame receive_frame(){
        LOG_INFO("Trying to receive a frame");
        EncodedFrame ret;

        char buf[INBUF_SIZE];
        double ptime;
        auto buf_size = receiver.receive(buf, sizeof buf, &ptime);
        uint8_t* data = reinterpret_cast<uint8_t*>(buf);
        ret.buf_size=buf_size;
        ret.data=data;
        LOG_INFO("Received Frame: buf_size: " + std::to_string(ret.buf_size));
        return ret;
    }
    void send_input(int input_value)
    {
        char buf[sizeof(Input_t)];
        Input_t input;
        input.value = input_value;
        memcpy(buf, &input, sizeof(input));
        input_sender.send(buf, sizeof(input));
        LOG_INFO("Sending key with value " + std::to_string(input_value) + " to server");
    }
};

#endif