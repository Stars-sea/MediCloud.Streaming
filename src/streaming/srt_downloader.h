#ifndef SRT_DOWNLOADER_H
#define SRT_DOWNLOADER_H
#include <string>

#include "srt_input.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace medi_cloud::recvsrt
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

    // 初始化FFmpeg网络库
    void init_ffmpeg();

    // 处理数据包（读取、转换、写入）
    bool process_packet(const StreamContext& ctx);

    // 清理资源
    void cleanup(StreamContext& ctx);

    // 开始下载
    void download(
        const std::string&         url,
        const SrtConnectionParams& params,
        std::ostream&              output,
        DownloadState&             state);

    void download(
        const std::string&         url,
        const SrtConnectionParams& params,
        const std::string&         path,
        DownloadState&             state);
}

#endif //SRT_DOWNLOADER_H
