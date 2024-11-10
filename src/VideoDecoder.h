#ifndef VIDEODECODER_H
#define VIDEODECODER_H
#include <condition_variable>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

extern "C" {
#include <stdio.h>
#include <time.h>
#include "libswscale/swscale.h"
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/frame.h"
}

struct DecodedFrame {
    uint8_t* data[AV_NUM_DATA_POINTERS];
    int linesize[AV_NUM_DATA_POINTERS];
    int w;
    int h;
    AVPixelFormat format;
    DecodedFrame(const AVFrame *f) {
        w = f->width;
        h = f->height;
        format = static_cast<AVPixelFormat>(f->format);
        av_image_alloc(data, linesize, w, h, format, 1);
        av_image_copy(data, linesize, (const uint8_t**)f->data, f->linesize, format, w, h);
    }
};


class VideoDecoder{
    const AVCodec* codec;
    AVCodecParserContext* parser;
    AVCodecContext* c = NULL;
    AVFrame* frame;
    uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
    uint8_t* data;
    size_t   data_size;
    int ret;
    int eof;
    AVPacket* pkt;

    public:
    VideoDecoder() {
        codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
        parser = av_parser_init(codec->id);
        c = avcodec_alloc_context3(codec);
        avcodec_open2(c, codec, nullptr);
        pkt = av_packet_alloc();
        frame = av_frame_alloc();
    }
    void initialize(){
        pkt = av_packet_alloc();
        if (!pkt)
            exit(1);

        /* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
        memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);

        /* find the MPEG-1 video decoder */
        codec = avcodec_find_decoder(AV_CODEC_ID_MPEG4);
        if (!codec) {
            fprintf(stderr, "Codec not found\n");
            exit(1);
        }

        parser = av_parser_init(codec->id);
        if (!parser) {
            fprintf(stderr, "parser not found\n");
            exit(1);
        }

        c = avcodec_alloc_context3(codec);
        if (!c) {
            fprintf(stderr, "Could not allocate video codec context\n");
            exit(1);
        }

        /* For some codecs, such as msmpeg4 and mpeg4, width and height
        MUST be initialized there because this information is not
        available in the bitstream. */

        /* open it */
        if (avcodec_open2(c, codec, NULL) < 0) {
            fprintf(stderr, "Could not open codec\n");
            exit(1);
        }

        frame = av_frame_alloc();
        if (!frame) {
            fprintf(stderr, "Could not allocate video frame\n");
            exit(1);
        }
    }

    void decode_packet(EncodedFrame& packet, std::queue<DecodedFrame>& frameQueue, std::mutex& lock, std::condition_variable &frameAvailable) {
        const uint8_t* data = packet.data;
        while(packet.buf_size)
        {
            int ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                data, packet.buf_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
            if (ret < 0) {
                std::cerr << "Error parsing the packet" << std::endl;
                LOG_ERROR("Error parsing the packet");
                return;
            }
            packet.buf_size-=ret;
            data += ret;
            if(pkt->size)
            {
                int send_result = avcodec_send_packet(c, pkt);
                if(send_result < 0)
                {
                    std::cerr << "Error sending a packet for decoding" << std::endl;
                    LOG_ERROR("Error sending a packet for decoding");
                    return;
                }
                while (send_result >= 0) {
                    ret = avcodec_receive_frame(c, frame);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    {
                        break;
                    }
                    if (ret < 0) {
                        std::cerr << "Error during decoding" << std::endl;
                        LOG_ERROR("Error during decoding");
                        return;
                    }
                    LOG_INFO("Trying to add frame to queue");
                    DecodedFrame cp(av_frame_clone(frame));
                    {
                        std::lock_guard<std::mutex> guard(lock);
                        frameQueue.push(cp);
                    }
                    frameAvailable.notify_one();
                    LOG_INFO("Queue size: " + std::to_string(frameQueue.size()));
                }
            }
        }
    }
    void cleanup()
    {
        //flush the decoder
        int ret;
        ret = avcodec_send_packet(c, nullptr);
        while (ret >= 0) {
            ret = avcodec_receive_frame(c, frame);
            fflush(stdout);
        }

        //free resources
        av_parser_close(parser);
        avcodec_free_context(&c);
        av_frame_free(&frame);
        av_packet_free(&pkt);
    }
};



#endif