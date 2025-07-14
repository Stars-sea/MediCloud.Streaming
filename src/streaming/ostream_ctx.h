#ifndef OSTREAM_CTX_H
#define OSTREAM_CTX_H
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
}

namespace medi_cloud::streaming::out
{
    // 自定义IO上下文结构体
    struct OStreamIOContext
    {
        std::ostream* stream;
        int64_t       bytes_written;
    };

    // 设置输出到ostream
    AVFormatContext* setup_output_ostream(const AVFormatContext* input_ctx, std::ostream& stream);

    AVFormatContext* setup_output_file(const AVFormatContext* input_ctx, const std::string& filename);
}

#endif //OSTREAM_CTX_H
