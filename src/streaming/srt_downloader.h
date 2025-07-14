#ifndef SRT_DOWNLOADER_H
#define SRT_DOWNLOADER_H
#include <functional>
#include <string>

#include "srt_input.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace medi_cloud::streaming::in
{
    // 流上下文结构体
    struct StreamContext
    {
        AVFormatContext* input_ctx  = nullptr;
        AVFormatContext* output_ctx = nullptr;
        int64_t          start_time = 0;
    };

    enum class DownloadState
    {
        INIT        = 0,
        DOWNLOADING = 1,
        DONE        = 2,
        ERROR       = 3,
    };

    using ctx_provider = std::function<AVFormatContext*(AVFormatContext*)>;

    // 初始化FFmpeg网络库
    void init_ffmpeg();

    // 清理资源
    void cleanup(StreamContext& ctx);

    void download(
        const std::string&         url,
        const SrtConnectionParams& params,
        const ctx_provider&        output_ctx,
        DownloadState&             state);
}

#endif //SRT_DOWNLOADER_H
