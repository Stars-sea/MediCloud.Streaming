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


#include "ostream_ctx.h"

#include <iostream>

#include "util.h"

extern "C" {
#include <libavutil/error.h>
}

namespace medi_cloud::streaming::out
{
    // 自定义IO写回调函数
    static int ostream_write_packet(void* opaque, const uint8_t* buf, const int buf_size)
    {
        auto* ctx = static_cast<OStreamIOContext*>(opaque);
        if (!ctx || !ctx->stream)
            return AVERROR(EINVAL);

        try
        {
            ctx->stream->write(reinterpret_cast<const char*>(buf), buf_size);
            if (ctx->stream->fail())
                return AVERROR(EIO);

            ctx->bytes_written += buf_size;
            return buf_size;
        }
        catch (const std::exception& e)
        {
            std::println(std::cerr, "Error occurred while writing: {}", e.what());
            return AVERROR(EIO);
        }
    }

    AVFormatContext* setup_output_ostream(const AVFormatContext* input_ctx, std::ostream& stream)
    {
        // 创建自定义IO上下文
        constexpr int buffer_size = 4096; // 缓冲区大小
        auto*         buffer      = static_cast<uint8_t*>(av_malloc(buffer_size));
        if (!buffer)
            throw std::runtime_error("Failed to allocate buffer");

        // 创建自定义IO上下文
        auto* custom_io_ctx = new OStreamIOContext{&stream, 0};

        // 创建AVIO上下文
        AVIOContext* avio_ctx = avio_alloc_context(
            buffer, buffer_size,
            1, // Writeable: 1
            custom_io_ctx,
            nullptr,
            ostream_write_packet,
            nullptr
            );
        if (!avio_ctx)
        {
            av_free(buffer);
            delete custom_io_ctx;
            throw std::runtime_error("Failed to allocate AVIO context");
        }

        AVFormatContext* output_ctx = avformat_alloc_context();
        output_ctx->pb              = avio_ctx;
        output_ctx->oformat         = av_guess_format("mpegts", nullptr, nullptr);
        if (!output_ctx->oformat)
        {
            av_free(buffer);
            delete custom_io_ctx;
            avformat_free_context(output_ctx);
            throw std::runtime_error("Failed to allocate output format");
        }

        std::for_each_n(input_ctx->streams, input_ctx->nb_streams, [output_ctx](const AVStream* istream)
        {
            AVStream* av_ostream = avformat_new_stream(output_ctx, nullptr);
            if (!av_ostream || avcodec_parameters_copy(av_ostream->codecpar, istream->codecpar) < 0)
                throw std::runtime_error("Failed to allocate output stream");

            av_ostream->time_base = istream->time_base;
        });

        // 写入文件头
        if (const int ret = avformat_write_header(output_ctx, nullptr); ret < 0)
            throw std::runtime_error(get_err_msg(ret));

        return output_ctx;
    }

    AVFormatContext* setup_output_file(const AVFormatContext* input_ctx, const fs::path& path)
    {
        AVFormatContext* output_ctx = avformat_alloc_context();
        avformat_alloc_output_context2(&output_ctx, nullptr, "mpegts", path.c_str());

        std::for_each_n(input_ctx->streams, input_ctx->nb_streams, [output_ctx](const AVStream* istream)
        {
            AVStream* av_ostream = avformat_new_stream(output_ctx, nullptr);
            if (!av_ostream || avcodec_parameters_copy(av_ostream->codecpar, istream->codecpar) < 0)
                throw std::runtime_error("Failed to allocate output stream");

            av_ostream->time_base = istream->time_base;
        });

        if (!(output_ctx->oformat->flags & AVFMT_NOFILE))
        {
            if (avio_open(&output_ctx->pb, path.c_str(), AVIO_FLAG_WRITE) < 0)
                throw std::runtime_error("Failed to open output file");
        }

        if (avformat_write_header(output_ctx, nullptr) < 0)
            throw std::runtime_error("Failed to write output header");
        return output_ctx;
    }

    AVFormatContext* setup_output_hls(const AVFormatContext* input_ctx, const HlsParams& hls_params)
    {
        AVFormatContext* hls_output_ctx;
        avformat_alloc_output_context2(&hls_output_ctx, nullptr, "hls", hls_params.hls_output.c_str());

        AVDictionary* hls_options = nullptr;
        av_dict_set_int(&hls_options, "hls_time", hls_params.segment_time, 0);
        av_dict_set_int(&hls_options, "hls_list_size", hls_params.list_size, 0);
        if (hls_params.delete_segments)
            av_dict_set(&hls_options, "hls_flags", "delete_segments", 0);

        std::for_each_n(
            input_ctx->streams, input_ctx->nb_streams,
            [hls_output_ctx](const AVStream* istream)
            {
                AVStream* hls_ostream = avformat_new_stream(hls_output_ctx, nullptr);
                if (!hls_ostream || avcodec_parameters_copy(hls_ostream->codecpar, istream->codecpar) < 0)
                    throw std::runtime_error("Failed to allocate output stream");

                hls_ostream->time_base = istream->time_base;
            });

        if (!(hls_output_ctx->oformat->flags & AVFMT_NOFILE))
        {
            if (avio_open(&hls_output_ctx->pb, hls_params.hls_output.c_str(), AVIO_FLAG_WRITE) < 0)
                throw std::runtime_error("Failed to open output file");
        }

        if (avformat_write_header(hls_output_ctx, &hls_options) < 0)
            throw std::runtime_error("Failed to write output header");

        return hls_output_ctx;
    }
}
