/*
 *  MediCloud.LiveStreaming
 *  Copyright (C) 2025  Stars sea<Stars_sea@outlook.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */



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
