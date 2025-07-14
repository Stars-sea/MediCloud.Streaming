#include "messages.h"

#include <nlohmann/json.hpp>
#include <uuid/uuid.h>

#include "settings.h"

namespace medi_cloud::messaging::messages
{
    void to_json(json& j, const PullStreamCommand& command)
    {
        j = json{
            {"id", command.id},
            {"url", command.url},
            {"path", command.path},
            {"passphrase", command.passphrase},
            {"timeout", command.timeout},
            {"latency", command.latency},
            {"ffs", command.ffs}
        };
    }

    void from_json(const json& j, PullStreamCommand& command)
    {
        j.at("id").get_to(command.id);
        j.at("url").get_to(command.url);
        j.at("path").get_to(command.path);
        j.at("passphrase").get_to(command.passphrase);
        j.at("timeout").get_to(command.timeout);
        j.at("latency").get_to(command.latency);
        j.at("ffs").get_to(command.ffs);
    }

    void to_json(json& j, const StreamRetrievedResponse& command)
    {
        j = json{
            {"id", command.id},
            {"url", command.url},
            {"path", command.path},
            {"code", command.code}
        };
    }

    void from_json(const json& j, StreamRetrievedResponse& command)
    {
        j.at("id").get_to(command.id);
        j.at("url").get_to(command.url);
        j.at("path").get_to(command.path);
        j.at("code").get_to(command.code);
    }

    std::string generate_uuid()
    {
        uuid_t uuid;
        uuid_generate_random(uuid);

        std::string uuid_str;
        uuid_unparse_lower(uuid, uuid_str.data());

        return uuid_str;
    }

    template <typename Message>
    std::string envelop_message(const Message& response)
    {
        settings::RabbitMQSettings settings;
        settings::read_from("settings.json", settings);

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
