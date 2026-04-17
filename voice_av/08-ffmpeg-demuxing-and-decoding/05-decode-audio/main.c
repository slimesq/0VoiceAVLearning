/**
 * @projectName   voice_av_08_05_decode_audio
 * @brief         解码音频，主要的测试格式aac和mp3
 * @author        Liao Qingfu
 * @date          2020-01-16
 */
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libavcodec/codec.h"
#include "libavcodec/packet.h"

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

static char err_buf[128] = {0};
static char* av_get_err(int errnum) {
    av_strerror(errnum, err_buf, 128);
    return err_buf;
}

static void print_sample_format(AVFrame const* frame) {
    printf("ar-samplerate: %uHz\n", frame->sample_rate);
    printf("ac-channel: %d\n", frame->ch_layout.nb_channels);
    printf("f-format: %u\n", frame->format);  // 格式需要注意，实际存储到本地文件时已经改成交错模式
}

static void decode(AVCodecContext* dec_ctx, AVPacket* pkt, AVFrame* frame, FILE* outfile) {
    int ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret == AVERROR(EAGAIN)) {
        fprintf(stderr,
                "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
    } else if (ret < 0) {
        fprintf(stderr, "Error submitting the packet to the decoder, err:%s, pkt_size:%d\n",
                av_get_err(ret), pkt->size);
        return;
    }
    while (1) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return;
        } else if (ret < 0) {
            fprintf(stderr, "Error during decoding, err:%s\n", av_get_err(ret));
            return;
        }
        // printf("decoded frame, nb_samples:%d\n", frame->nb_samples);
        // print_sample_format(frame);

        int sampleSize = av_get_bytes_per_sample(frame->format);
        for (int i = 0; i < frame->nb_samples; ++i) {
            for (int j = 0; j < frame->ch_layout.nb_channels; ++j) {
                fwrite(frame->data[j] + i * sampleSize, 1, sampleSize, outfile);
            }
        }
        av_frame_unref(frame);
    }
}
// 播放范例：   ffplay -ar 48000 -ac 2 -f f32le believe.pcm
int main(int argc, char** argv) {
    char const* outfilename;
    char const* filename;
    if (argc <= 2) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(0);
    }
    filename = argv[1];
    outfilename = argv[2];
    FILE* infile = NULL;
    fopen_s(&infile, filename, "rb");
    if (!infile) {
        printf("fopen_s infile error\n");
        return -1;
    }
    FILE* outfile = NULL;
    fopen_s(&outfile, outfilename, "wb");
    if (!outfile) {
        printf("fopen_s outfile error\n");
        return -1;
    }

    uint8_t inbuf[AUDIO_INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];

    enum AVCodecID audio_codec_id = AV_CODEC_ID_AAC;
    if (strstr(filename, "aac") != NULL) {
        audio_codec_id = AV_CODEC_ID_AAC;
    } else if (strstr(filename, "mp3") != NULL) {
        audio_codec_id = AV_CODEC_ID_MP3;
    } else {
        printf("default codec id:%d\n", audio_codec_id);
    }

    AVCodecParserContext* parser = av_parser_init(audio_codec_id);
    if (!parser) {
        printf("av_parser_init error");
        return -1;
    }

    AVCodec const* decoder = avcodec_find_decoder(audio_codec_id);
    if (!decoder) {
        printf("avcodec_find_decoder error");
        return -1;
    }
    AVCodecContext* codec = avcodec_alloc_context3(decoder);
    if (!codec) {
        printf("avcodec_alloc_context3 error");
        return -1;
    }
    if (avcodec_open2(codec, decoder, NULL) < 0) {
        printf("avcodec_open2 error");
        return -1;
    }

    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        printf("av_packet_alloc error");
        return -1;
    }
    AVFrame* frame = av_frame_alloc();

    int data_size = fread(inbuf, 1, AUDIO_INBUF_SIZE, infile);
    while (1) {
        int parseSize = av_parser_parse2(parser, codec, &pkt->data, &pkt->size, inbuf, data_size,
                                         AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (parseSize < 0) {
            printf("av_parser_parse2 error");
            return -1;
        }
        if (!frame) {
            if (!(frame = av_frame_alloc())) {
                printf("av_frame_alloc error");
                return -1;
            }
        }
        if (pkt->size > 0) {
            decode(codec, pkt, frame, outfile);
        }

        data_size -= parseSize;
        memmove(inbuf, inbuf + parseSize, data_size);

        if (data_size < AUDIO_REFILL_THRESH) {
            int readSize = fread(inbuf + data_size, 1, AUDIO_INBUF_SIZE - data_size, infile);
            if (readSize <= 0) {
                break;
            }
            data_size += readSize;
        }
    }

    // 冲刷解码器
    pkt->data = NULL;
    pkt->size = 0;
    decode(codec, pkt, frame, outfile);
    av_packet_free(&pkt);
    av_frame_free(&frame);
    avcodec_free_context(&codec);
    av_parser_close(parser);
    fclose(infile);
    fclose(outfile);

    printf("main finish, please enter Enter and exit\n");
    return 0;
}
