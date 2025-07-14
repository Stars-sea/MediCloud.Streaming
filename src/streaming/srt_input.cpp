#include "srt_input.h"

#include <stdexcept>

#include "util.h"

namespace medi_cloud::streaming::in
{
    AVFormatContext* open_srt_input(const std::string& input_url, const SrtConnectionParams& params)
    {
        AVFormatContext* input_ctx = nullptr;
        AVDictionary*    options   = nullptr;

        // 设置SRT参数
        av_dict_set_int(&options, "timeout", params.timeout, 0);
        av_dict_set_int(&options, "latency", params.latency, 0);
        av_dict_set_int(&options, "ffs", params.ffs, 0);
        if (params.passphrase)
            av_dict_set(&options, "passphrase", params.passphrase, 0);

        // 打开输入流
        int ret = avformat_open_input(&input_ctx, input_url.c_str(), nullptr, &options);
        av_dict_free(&options);

        if (ret < 0)
            throw std::runtime_error(streaming::get_err_msg(ret));

        // 获取流信息
        if ((ret = avformat_find_stream_info(input_ctx, nullptr)) < 0)
        {
            avformat_close_input(&input_ctx);
            throw std::runtime_error(streaming::get_err_msg(ret));
        }

        // 打印输入流信息
        av_dump_format(input_ctx, 0, input_url.c_str(), 0);

        return input_ctx;
    }
}
