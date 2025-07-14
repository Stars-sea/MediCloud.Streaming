#include "consumers.h"

#include <filesystem>
#include <iostream>

#include "messaging/rabbitmq_client.h"
#include "messaging/settings.h"
#include "streaming/ostream_ctx.h"
#include "streaming/srt_downloader.h"
#include "streaming/srt_input.h"

namespace settings = medi_cloud::messaging::settings;
using medi_cloud::messaging::rabbitmq::rabbitmq_client;

namespace medi_cloud::consumers
{
    std::string pull_stream_consumer(const messages::PullStreamCommand& command)
    {
        using namespace medi_cloud::streaming;
        namespace fs = std::filesystem;

        const in::SrtConnectionParams params{
            command.timeout,
            command.latency,
            command.ffs
        };
        if (!command.passphrase.empty())
            memcpy(params.passphrase, command.passphrase.c_str(), command.passphrase.size());

        in::DownloadState state;

        const fs::path path = command.path;
        if (!fs::is_directory(path) && !fs::create_directories(path))
        {
            const std::string msg = std::format("Failed to create directory '{}'", path.string());
            throw std::runtime_error(msg);
        }

        const out::HlsParams hls_params{
            5,
            10,
            false,
            path / "index.m3u8"
        };

        auto output_ctx_provider =
            [hls_params](const AVFormatContext* input_ctx)
        {
            return out::setup_output_hls(input_ctx, hls_params);
        };

        std::println(std::cout, "[Streaming] Begin to pull stream {} -> {}", command.url, command.path);
        in::download(command.url, params, output_ctx_provider, state);
        std::println(std::cout, "[Streaming] Stream retrieved {} -> {}", command.url, command.path);

        messages::StreamRetrievedResponse message{
            command.id,
            command.url,
            command.path
        };
        if (state == in::DownloadState::INIT)
            message.code = "init";
        else if (state == in::DownloadState::DOWNLOADING)
            message.code = "downloading";
        else if (state == in::DownloadState::DONE)
            message.code = "done";
        else if (state == in::DownloadState::ERROR)
            message.code = "error";

        return messages::envelop_message(message);
    }
}
