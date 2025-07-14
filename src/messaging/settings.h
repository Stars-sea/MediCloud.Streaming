#ifndef SETTINGS_H
#define SETTINGS_H

#include <nlohmann/json.hpp>

namespace medi_cloud::messaging::settings
{
    using json = nlohmann::json;

    struct ConnectionSettings
    {
        std::string host;
        int         port;
        std::string vhost;
        std::string user;
        std::string password;
    };

    void from_json(const json& j, ConnectionSettings& settings);

    struct ChannelSettings
    {
        std::string consumer_queue_name;
        std::string producer_queue_name;
        std::string producer_exchange_name;
        std::string producer_routing_key;
        uint16_t    channel;;
    };

    void from_json(const json& j, ChannelSettings& settings);

    struct RabbitMQSettings
    {
        ConnectionSettings connection;
        ChannelSettings    channel;
    };

    void read_from(const std::string& file, RabbitMQSettings& settings);

}

#endif //SETTINGS_H
