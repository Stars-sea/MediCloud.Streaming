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


#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <nlohmann/json.hpp>

#include "consumers.h"
#include "messaging/messages.h"
#include "messaging/rabbitmq_client.h"
#include "messaging/settings.h"

using namespace medi_cloud::messaging;

using json = nlohmann::json;

using rabbitmq_client_ptr = std::unique_ptr<rabbitmq::rabbitmq_client>;

std::mutex                client_mtx;
settings::ChannelSettings channel_settings;
rabbitmq_client_ptr       client_ptr;

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

    if (client_ptr->QueueBind(
        channel_settings.producer_queue_name,
        channel_settings.producer_exchange_name,
        channel_settings.producer_routing_key) < 0)
    {
        std::println(std::cerr, "Failed to queue bind");
        return -3;
    }

    return 0;
}

template <typename Message>
void consumer_wrapper(std::function<std::string(const Message&)> consumer, const Message& msg)
{
    try
    {
        const std::string message = consumer(msg);

        std::lock_guard lock(client_mtx);
        if (client_ptr == nullptr)
            return;
        if (client_ptr->Publish(
            message,
            channel_settings.producer_exchange_name,
            channel_settings.producer_routing_key) < 0)
        {
            std::println(std::cerr, "[RabbitMQ] Failed to publish message StreamRetrievedResponse");
        }
    }
    catch (const std::exception& e)
    {
        std::println(std::cerr, "{}", e.what());

        // TODO: Send error to rabbitmq
    }
}

auto main(int argc, char* argv[]) -> int
{
    using namespace medi_cloud::consumers;

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

    std::queue<std::thread> workers;

    std::queue<std::string> messages;

    constexpr timeval timeout{0, 0};
    while (true)
    {
        using namespace std::chrono;
        std::this_thread::sleep_for(10ms);

        {
            std::lock_guard lock{client_mtx};
            if (client_ptr->Consumer(messages, settings.channel.consumer_queue_name, 1, &timeout) < 0)
            {
                client_ptr = nullptr;
                break;
            }
        }

        if (messages.empty())
            continue;
        std::string msg = messages.front();
        messages.pop();

        auto command = json::parse(msg)["message"].get<messages::PullStreamCommand>();
        workers.emplace(
            consumer_wrapper<messages::PullStreamCommand>,
            pull_stream_consumer,
            command);
    }

    while (!workers.empty())
    {
        if (workers.front().joinable())
            workers.front().join();

        workers.pop();
    }
}
