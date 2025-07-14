#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>

#include "messages.h"
#include "rabbitmq_client.h"
#include "settings.h"
#include "srt_downloader.h"

using namespace medi_cloud;

using json = nlohmann::json;

std::mutex                                 client_mtx;
settings::RabbitMQChannelSettings          channel_settings;
std::unique_ptr<rabbitmq::rabbitmq_client> client_ptr;

void consumer_thread(messages::PullStreamCommand command)
{
    recvsrt::SrtConnectionParams params{
        command.timeout,
        command.latency,
        command.ffs
    };
    if (!command.passphrase.empty())
        memcpy(params.passphrase, command.passphrase.c_str(), command.passphrase.size());

    recvsrt::DownloadState state;

    std::ofstream output_file{command.path, std::ios_base::binary|std::ios_base::out};

    std::println(std::cout, "[Streaming] Begin to pull stream {} -> {}", command.url, command.path);
    // recvsrt::download(command.url, params, command.path, state);
    recvsrt::download(command.url, params, output_file, state);
    std::println(std::cout, "[Streaming] Stream retrieved {} -> {}", command.url, command.path);

    if (!client_ptr)
        return;

    messages::StreamRetrievedResponse message{
        command.id,
        command.url,
        command.path
    };
    if (state == recvsrt::DownloadState::INIT)
        message.code = "init";
    else if (state == recvsrt::DownloadState::DOWNLOADING)
        message.code = "downloading";
    else if (state == recvsrt::DownloadState::DONE)
        message.code = "done";
    else if (state == recvsrt::DownloadState::ERROR)
        message.code = "error";

    json json_message = message;

    std::lock_guard lock(client_mtx);
    if (client_ptr->Publish(
        json_message.dump(),
        channel_settings.producer_exchange_name,
        channel_settings.producer_routing_key) < 0)
    {
        std::println(std::cerr, "[RabbitMQ] Failed to publish message StreamRetrievedResponse");
    }
}

auto rabbitmq_declare() -> int
{
    if (client_ptr->QueueDeclare(channel_settings.consumer_queue_name) < 0 ||
        client_ptr->QueueDeclare(channel_settings.producer_queue_name) < 0)
    {
        std::println(std::cerr, "Failed to queue declare");
        return -1;
    }

    if (client_ptr->ExchangeDeclare(channel_settings.producer_exchange_name, "fanout") < 0)
    {
        std::println(std::cerr, "Failed to exchange declare");
        return -2;
    }

    if (client_ptr->QueueBind(channel_settings.producer_queue_name,
                              channel_settings.producer_exchange_name,
                              channel_settings.producer_routing_key) < 0)
    {
        std::println(std::cerr, "Failed to queue bind");
        return -3;
    }

    return 0;
}

auto main(int argc, char* argv[]) -> int
{
    settings::RabbitMQSettings settings;
    settings::read_from("settings.json", settings);

    client_ptr = std::make_unique<rabbitmq::rabbitmq_client>(
        settings.connection,
        settings.channel);

    client_ptr->Connect();

    channel_settings = settings.channel;
    if (rabbitmq_declare())
    {
        std::println(std::cerr, "Failed to setup RabbitMQ declares");
        return -1;
    }

    std::queue<std::thread> threads;

    std::queue<std::string> messages;
    while (true)
    {
        client_mtx.lock();
        if (client_ptr->Consumer(messages, settings.channel.consumer_queue_name) < 0)
        {
            client_ptr = nullptr;
            break;
        }
        client_mtx.unlock();

        std::string msg = messages.front();
        messages.pop();

        auto command = json::parse(msg)["message"].get<messages::PullStreamCommand>();
        threads.emplace(consumer_thread, command);
    }

    client_mtx.unlock();
    while (!threads.empty())
    {
        if (threads.front().joinable())
            threads.front().join();

        threads.pop();
    }
}
