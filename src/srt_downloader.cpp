#include "srt_downloader.h"

#include <thread>

#include "ostream_ctx.h"
#include "srt_input.h"
#include "util.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

namespace medi_cloud::recvsrt
{
    void init_ffmpeg()
    {
        avformat_network_init();
    }

    bool process_packet(const StreamContext& ctx, AVPacket* packet)
    {
        // 读取数据包
        int ret = av_read_frame(ctx.input_ctx, packet);
        if (ret < 0)
        {
            // 处理读取错误
            if (ret == AVERROR_EOF || avio_feof(ctx.input_ctx->pb))
            {
                std::cout << "到达文件结尾" << std::endl;
                return false;
            }

            // 处理超时
            if (ret == AVERROR(EAGAIN))
            {
                std::cerr << "超时重连中..." << std::endl;
                av_usleep(100000); // 等待100ms重试
                return true;
            }

            throw std::runtime_error(util::get_err_msg(ret));
        }

        // // 重设流索引
        // const AVStream* in_stream  = ctx.input_ctx->streams[packet.stream_index];
        // const AVStream* out_stream = ctx.output_ctx->streams[packet.stream_index];
        //
        // // 时间基转换
        // packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base,
        //                               static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        // packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base,
        //                               static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        // packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
        // packet.pos      = -1;

        const AVStream* istream = ctx.input_ctx->streams[packet->stream_index];
        const AVStream* ostream = ctx.output_ctx->streams[packet->stream_index];
        av_packet_rescale_ts(packet, istream->time_base, ostream->time_base);

        // 写入数据包
        if ((ret = av_interleaved_write_frame(ctx.output_ctx, packet)) < 0)
            throw std::runtime_error(util::get_err_msg(ret));

        av_packet_unref(packet);

        // 显示录制时长
        const int64_t current_time = (av_gettime() - ctx.start_time) / 1000000;
        std::cout << "\r录制时长: " << current_time << "秒" << std::flush;

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
            // // 写入文件尾
            // av_write_trailer(ctx.output_ctx);
            //
            // // 清理自定义IO上下文
            // if (ctx.output_ctx->pb)
            // {
            //     // 获取自定义IO上下文
            //     if (ctx.output_ctx->pb->opaque)
            //     {
            //         const auto* custom_io_ctx = static_cast<OStreamIOContext*>(ctx.output_ctx->pb->opaque);
            //         delete custom_io_ctx;
            //     }
            //
            //     // 释放缓冲区
            //     if (ctx.output_ctx->pb->buffer)
            //         av_free(ctx.output_ctx->pb->buffer);
            //            //
            //            //     // 释放AVIO上下文
            //            //     avio_context_free(&ctx.output_ctx->pb);
            //            // }
            avformat_free_context(ctx.output_ctx);
            ctx.output_ctx = nullptr;
        }
        avformat_network_deinit();
    }

    void download(
        const std::string&         url,
        const SrtConnectionParams& params,
        std::ostream&              output,
        DownloadState&             state)
    {
        state = DownloadState::INIT;

        StreamContext ctx;
        try
        {
            init_ffmpeg();

            ctx.input_ctx  = open_srt_input(url, params);
            ctx.output_ctx = setup_output_ostream(ctx.input_ctx, output);

            std::println(std::cout, "[{}] Start pulling from {}", std::this_thread::get_id(), url);
            state = DownloadState::DOWNLOADING;

            ctx.start_time = av_gettime();
            // 主循环：处理数据包
            AVPacket packet;
            while (true)
            {
                if (!process_packet(ctx, &packet))
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

    void download(
        const std::string&         url,
        const SrtConnectionParams& params,
        const std::string&         path,
        DownloadState&             state)
    {
        state = DownloadState::INIT;

        StreamContext ctx;
        try
        {
            init_ffmpeg();

            ctx.input_ctx  = open_srt_input(url, params);
            ctx.output_ctx = setup_output_file(ctx.input_ctx, path);

            std::println(std::cout, "[{}] Start pulling from {}", std::this_thread::get_id(), url);
            state = DownloadState::DOWNLOADING;

            ctx.start_time = av_gettime();
            // 主循环：处理数据包
            AVPacket packet;
            while (true)
            {
                if (!process_packet(ctx, &packet))
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
