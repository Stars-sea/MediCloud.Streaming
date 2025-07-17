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


#ifndef MESSAGES_H
#define MESSAGES_H
#include <nlohmann/json_fwd.hpp>

#include "settings.h"

namespace medi_cloud::messaging::messages
{
    using json = nlohmann::json;

    struct PullStreamCommand
    {
        std::string id;
        std::string url;
        std::string path;
        std::string passphrase;

        int timeout;
        int latency;
        int ffs;
    };

    void to_json(json& j, const PullStreamCommand& command);

    void from_json(const json& j, PullStreamCommand& command);

    struct StreamRetrievedResponse
    {
        std::string id;
        std::string url;
        std::string path;

        std::string code;
    };

    void to_json(json& j, const StreamRetrievedResponse& command);

    void from_json(const json& j, StreamRetrievedResponse& command);

    std::string generate_uuid();

    template <typename Message>
    std::string envelop_message(const Message& response)
    {
        using namespace medi_cloud::messaging::settings;

        RabbitMQSettings settings;
        read_from("settings.json", settings);

        std::string sourceAddress = std::format(
            "rabbitmq://{}:{}/{}/{}",
            settings.connection.host,
            settings.connection.port,
            settings.connection.vhost,
            settings.channel.consumer_queue_name);

        std::string destinationAddress = std::format(
            "rabbitmq://{}:{}/{}/{}",
            settings.connection.host,
            settings.connection.port,
            settings.connection.vhost,
            settings.channel.producer_exchange_name);

        std::string messageType =
            "urn:message:MediCloud.Application.Live.Contracts:StreamRetrievedResponse";

        const json envelope = {
            {"messageId", generate_uuid()},
            {"conversationId", generate_uuid()},
            {"sourceAddress", sourceAddress},
            {"destinationAddress", destinationAddress},
            {"messageType", json::array({messageType})},
            {"message", response}
        };
        return envelope.dump();
    }
}

#endif //MESSAGES_H
