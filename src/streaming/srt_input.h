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
