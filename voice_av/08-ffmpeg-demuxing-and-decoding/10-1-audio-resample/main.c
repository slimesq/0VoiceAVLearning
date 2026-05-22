#include <stdint.h>
#include <stdio.h>
#include "libavutil/mathematics.h"
#include "libavutil/mem.h"
#include "libavutil/samplefmt.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"

static int get_format_from_sample_fmt(char const** fmt, enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry
    {
        enum AVSampleFormat sample_fmt;
        char const *fmt_be, *fmt_le;
    } sample_fmt_entries[] = {
        {AV_SAMPLE_FMT_U8, "u8", "u8"},
        {AV_SAMPLE_FMT_S16, "s16be", "s16le"},
        {AV_SAMPLE_FMT_S32, "s32be", "s32le"},
        {AV_SAMPLE_FMT_FLT, "f32be", "f32le"},
        {AV_SAMPLE_FMT_DBL, "f64be", "f64le"},
    };
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++)
    {
        struct sample_fmt_entry* entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt)
        {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    fprintf(stderr,
            "Sample format %s not supported as output format\n",
            av_get_sample_fmt_name(sample_fmt));
    return AVERROR(EINVAL);
}

/**
 * Fill dst buffer with nb_samples, generated starting from t.
 */
static void fill_samples(double* dst, int nb_samples, int nb_channels, int sample_rate, double* t)
{
    int i, j;
    double tincr = 1.0 / sample_rate, *dstp = dst;
    double const c = 2 * M_PI * 440.0;

    /* generate sin tone with 440Hz frequency and duplicated channels */
    for (i = 0; i < nb_samples; i++)
    {
        *dstp = sin(c * *t);
        for (j = 1; j < nb_channels; j++)
            dstp[j] = dstp[0];
        dstp += nb_channels;
        *t += tincr;
    }
}

int main(int argc, char** argv)
{
    char const* dst_filename = NULL;
    FILE* dst_file;
    if (argc != 2)
    {
        fprintf(stderr,
                "Usage: %s output_file\n"
                "API example program to show how to resample an audio stream with libswresample.\n"
                "This program generates a series of audio frames, resamples them to a specified "
                "output format and rate and saves them to an output file named output_file.\n",
                argv[0]);
        exit(1);
    }
    dst_filename = argv[1];

    dst_file = fopen(dst_filename, "wb");
    if (!dst_file)
    {
        fprintf(stderr, "Could not open destination file %s\n", dst_filename);
        exit(1);
    }

    /* create resampler context */
    SwrContext* swr_ctx = swr_alloc();
    if (!swr_ctx)
    {
        printf("swr_alloc failed!\n");
        return -1;
    }

    AVChannelLayout const src_ch_layout = AV_CHANNEL_LAYOUT_STEREO,
                          dst_ch_layout = AV_CHANNEL_LAYOUT_SURROUND;
    int src_rate = 48000, dst_rate = 44100;
    /* set options */
    av_opt_set_chlayout(swr_ctx, "in_chlayout", &src_ch_layout, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_DBL, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", src_rate, 0);

    av_opt_set_chlayout(swr_ctx, "out_chlayout", &dst_ch_layout, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", dst_rate, 0);

    /* initialize the resampling context */
    int ret = swr_init(swr_ctx);
    if (ret < 0)
    {
        printf("swr_init failed!\n");
        return -1;
    }

    /* allocate source and destination samples buffers */
    uint8_t **src_data = NULL, **dst_data = NULL;
    int src_linesize, dst_linesize;
    int src_nb_samples = 1024;
    ret = av_samples_alloc_array_and_samples(
        &src_data, &src_linesize, src_ch_layout.nb_channels, src_nb_samples, AV_SAMPLE_FMT_DBL, 0);
    if (ret < 0)
    {
        printf("av_samples_alloc_array_and_samples src_data failed!\n");
        return -1;
    }

    double t = 0;
    int dst_max_sample_count, dst_sample_count;
    dst_max_sample_count = dst_sample_count =
        av_rescale_rnd(src_nb_samples,
                       dst_rate,
                       src_rate,
                       AV_ROUND_UP);  // The duration of each audio frame is the same.
    av_samples_alloc_array_and_samples(&dst_data,
                                       &dst_linesize,
                                       dst_ch_layout.nb_channels,
                                       dst_max_sample_count,
                                       AV_SAMPLE_FMT_S16,
                                       0);
    if (ret < 0)
    {
        printf("av_samples_alloc_array_and_samples dst_data failed!\n");
        return -1;
    }

    do
    {
        fill_samples((double*)src_data[0], src_nb_samples, src_ch_layout.nb_channels, src_rate, &t);

        dst_sample_count = av_rescale_rnd(
            swr_get_delay(swr_ctx, src_rate) + src_nb_samples, dst_rate, src_rate, AV_ROUND_UP);
        if (dst_max_sample_count < dst_sample_count)
        {
            dst_max_sample_count = dst_sample_count;
            av_freep(&(dst_data[0]));
            ret = av_samples_alloc(&(dst_data[0]),
                                   &dst_linesize,
                                   dst_ch_layout.nb_channels,
                                   dst_max_sample_count,
                                   AV_SAMPLE_FMT_S16,
                                   0);
            if (ret < 0)
            {
                printf("av_samples_alloc dst_data failed!\n");
                return -1;
            }
        }

        ret = swr_convert(swr_ctx,
                          (uint8_t* const*)dst_data,
                          dst_max_sample_count,
                          (uint8_t const* const*)src_data,
                          src_nb_samples);
        if (ret < 0)
        {
            printf("swr_convert failed!\n");
            return -1;
        }
        int bufsize =
            av_samples_get_buffer_size(NULL, dst_ch_layout.nb_channels, ret, AV_SAMPLE_FMT_S16, 0);
        if (bufsize < 0)
        {
            printf("av_samples_get_buffer_size failed!\n");
            return -1;
        }
        fwrite(dst_data[0], 1, bufsize, dst_file);
    } while (t < 10);

    char buf[64];
    char const* fmt;
    if ((ret = get_format_from_sample_fmt(&fmt, AV_SAMPLE_FMT_S16)) < 0)
        goto end;
    av_channel_layout_describe(&dst_ch_layout, buf, sizeof(buf));
    printf(
        "Resampling succeeded. Play the output file with the command:\n"
        "ffplay -f %s -ch_layout %s -ar %d %s\n",
        fmt,
        buf,
        dst_ch_layout.nb_channels,
        dst_filename);
end:

    if (dst_data)
        av_freep(&(dst_data[0]));
    av_freep(&dst_data);

    if (src_data)
        av_freep(&(src_data[0]));
    av_freep(&src_data);

    swr_free(&swr_ctx);

    fclose(dst_file);
    return 0;
}
