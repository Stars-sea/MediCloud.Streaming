#ifndef OSTREAM_CTX_H
#define OSTREAM_CTX_H
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

namespace medi_cloud::streaming::out
{
    struct HlsParams
    {
        int         segment_time;    // Unit: Second
        int         list_size;       // Size of segment list
        bool        delete_segments; // Whether to delete old segments
        std::string hls_output;      // HLS output path
    };

    // 自定义IO上下文结构体
    struct OStreamIOContext
    {
        std::ostream* stream{};
        int64_t       bytes_written{};
        HlsParams     hls_params{};
    };

    // 设置输出到ostream
    AVFormatContext* setup_output_ostream(const AVFormatContext* input_ctx, std::ostream& stream);

    AVFormatContext* setup_output_file(const AVFormatContext* input_ctx, const std::string& filename);

    AVFormatContext* setup_output_hls(const AVFormatContext* input_ctx, const HlsParams& hls_params);
}

#endif //OSTREAM_CTX_H
