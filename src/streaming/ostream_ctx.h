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


#ifndef OSTREAM_CTX_H
#define OSTREAM_CTX_H
#include <filesystem>

extern "C" {
#include <libavformat/avformat.h>
}

namespace medi_cloud::streaming::out
{
    namespace fs = std::filesystem;

    struct HlsParams
    {
        int      segment_time;    // Unit: Second
        int      list_size;       // Size of segment list
        bool     delete_segments; // Whether to delete old segments
        fs::path hls_output;      // HLS output path
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

    AVFormatContext* setup_output_file(const AVFormatContext* input_ctx, const fs::path& path);

    AVFormatContext* setup_output_hls(const AVFormatContext* input_ctx, const HlsParams& hls_params);
}

#endif //OSTREAM_CTX_H
