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
