#include "ostream_ctx.h"

#include "util.h"

extern "C" {
#include <libavutil/error.h>
}

namespace medi_cloud::recvsrt
{
    // 自定义IO写回调函数
    static int ostream_write_packet(void* opaque, const uint8_t* buf, int buf_size)
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
            std::cerr << "Error occurred while writing: " << e.what() << std::endl;
            return AVERROR(EIO);
        }
    }

    AVFormatContext* setup_output_ostream(const AVFormatContext* input_ctx, std::ostream& stream)
    {
        // AVFormatContext* output_ctx = nullptr;

        // int ret;
        // // 创建输出上下文 - 使用mpegts格式，因为它是流式友好的
        // if ((ret = avformat_alloc_output_context2(&output_ctx, nullptr, "mpegts", nullptr)) < 0)
        //     throw std::runtime_error(util::get_err_msg(ret));

        // 复制输入流到输出
        // for (unsigned i = 0; i < input_ctx->nb_streams; i++)
        // {
        //     const AVStream* in_stream  = input_ctx->streams[i];
        //     AVStream*       out_stream = avformat_new_stream(output_ctx, nullptr);
        //     if (!out_stream)
        //         throw std::runtime_error("Failed to allocate output stream");
        //
        //     // 复制流参数
        //     if ((ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar)) < 0)
        //         throw std::runtime_error(util::get_err_msg(ret));
        //
        //     out_stream->time_base = in_stream->time_base;
        // }

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
            // ostream_seek // 不支持seek
            nullptr
            );
        if (!avio_ctx)
        {
            av_free(buffer);
            delete custom_io_ctx;
            throw std::runtime_error("Failed to allocate AVIO context");
        }

        // output_ctx->pb = avio_ctx;
        // output_ctx->flags |= AVFMT_FLAG_CUSTOM_IO;

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
            throw std::runtime_error(util::get_err_msg(ret));

        return output_ctx;
    }

    AVFormatContext* setup_output_file(const AVFormatContext* input_ctx, const std::string& filename)
    {
        AVFormatContext* output_ctx = avformat_alloc_context();
        avformat_alloc_output_context2(&output_ctx, nullptr, "mpegts", filename.c_str());

        std::for_each_n(input_ctx->streams, input_ctx->nb_streams, [output_ctx](const AVStream* istream)
        {
            AVStream* av_ostream = avformat_new_stream(output_ctx, nullptr);
            if (!av_ostream || avcodec_parameters_copy(av_ostream->codecpar, istream->codecpar) < 0)
                throw std::runtime_error("Failed to allocate output stream");

            av_ostream->time_base = istream->time_base;
        });

        if (!(output_ctx->oformat->flags & AVFMT_NOFILE))
        {
            if (avio_open(&output_ctx->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0)
                throw std::runtime_error("Failed to open output file");
        }

        avformat_write_header(output_ctx, nullptr);
        return output_ctx;
    }
}
