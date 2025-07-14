#ifndef SRT_INPUT_H
#define SRT_INPUT_H
#include <string>

extern "C" {
#include <libavformat/avformat.h>
}

namespace medi_cloud::streaming::in
{
    // SRT连接参数结构体
    struct SrtConnectionParams
    {
        int   timeout    = 5000000; // 超时时间（微秒）
        int   latency    = 200000;  // 延迟（微秒）
        int   ffs        = 1000000; // 文件结束等待时间
        char* passphrase = nullptr; // 密码（可选）
    };

    // 打开SRT输入流
    AVFormatContext* open_srt_input(const std::string& input_url, const SrtConnectionParams& params);
}
#endif //SRT_INPUT_H
