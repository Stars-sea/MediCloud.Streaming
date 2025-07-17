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


#include "srt_downloader.h"

#include <iostream>
#include <thread>

#include "ostream_ctx.h"
#include "srt_input.h"
#include "util.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

namespace medi_cloud::streaming::in
{
    void init_ffmpeg()
    {
        avformat_network_init();
    }

    static bool read_frame(AVFormatContext* input_ctx, AVPacket* packet)
    {
        const int ret = av_read_frame(input_ctx, packet);

        // 处理读取错误
        if (ret == AVERROR_EOF || avio_feof(input_ctx->pb))
        {
            std::println(std::cout, "Read EOF");
            return false;
        }

        // 处理超时
        if (ret == AVERROR(EAGAIN))
        {
            std::println(std::cerr, "Timeout, reconnecting...");
            av_usleep(100000); // 等待100ms重试
            return true;
        }

        if (ret < 0)
            throw std::runtime_error(get_err_msg(ret));

        return true;
    }

    static bool process_packet(const StreamContext& ctx)
    {
        AVPacket packet;
        if (!read_frame(ctx.input_ctx, &packet))
            return false;

        const AVStream* istream = ctx.input_ctx->streams[packet.stream_index];
        const AVStream* ostream = ctx.output_ctx->streams[packet.stream_index];

        av_packet_rescale_ts(&packet, istream->time_base, ostream->time_base);

        // 写入数据包
        if (const int ret = av_interleaved_write_frame(ctx.output_ctx, &packet); ret < 0)
            throw std::runtime_error(get_err_msg(ret));

        return true;
    }

    void cleanup(StreamContext& ctx)
    {
        if (ctx.input_ctx)
        {
            avformat_close_input(&ctx.input_ctx);
            ctx.input_ctx = nullptr;
        }

        if (ctx.output_ctx)
        {
            avformat_free_context(ctx.output_ctx);
            ctx.output_ctx = nullptr;
        }
        avformat_network_deinit();
    }

    void download(
        const std::string&         url,
        const SrtConnectionParams& params,
        const ctx_provider&        output_ctx,
        DownloadState&             state)
    {
        state = DownloadState::INIT;

        StreamContext ctx;
        try
        {
            init_ffmpeg();

            ctx.input_ctx  = open_srt_input(url, params);
            ctx.output_ctx = output_ctx(ctx.input_ctx);

            std::println(std::cout, "[{}] Start pulling from {}", std::this_thread::get_id(), url);
            state = DownloadState::DOWNLOADING;

            ctx.start_time = av_gettime();
            // 主循环：处理数据包
            while (true)
            {
                if (!process_packet(ctx))
                    break; // 流结束
            }
            av_write_trailer(ctx.output_ctx);

            std::println(std::cout, "[{}] Completed pulling from {}", std::this_thread::get_id(), url);
            state = DownloadState::DONE;
        }
        catch (const std::exception&)
        {
            state = DownloadState::ERROR;
            cleanup(ctx);
        }

        cleanup(ctx);
    }
}
