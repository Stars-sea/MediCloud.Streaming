#include "settings.h"

#include <fstream>

namespace medi_cloud::messaging::settings
{

    void from_json(const json& j, ConnectionSettings& settings)
    {
        j.at("host").get_to(settings.host);
        j.at("port").get_to(settings.port);
        j.at("vhost").get_to(settings.vhost);
        j.at("user").get_to(settings.user);
        j.at("password").get_to(settings.password);
    }

    void from_json(const json& j, ChannelSettings& settings)
    {
        j.at("consumer_queue_name").get_to(settings.consumer_queue_name);
        j.at("producer_queue_name").get_to(settings.producer_queue_name);
        j.at("producer_exchange_name").get_to(settings.producer_exchange_name);
        j.at("producer_routing_key").get_to(settings.producer_routing_key);
        j.at("channel").get_to(settings.channel);
    }

    void read_from(const std::string& file, RabbitMQSettings& settings)
    {
        std::ifstream file_stream(file);
        if (!file_stream.is_open())
            throw std::runtime_error("Could not open file " + file);

        json json_obj;
        file_stream >> json_obj;

        json_obj.at("connection").get_to(settings.connection);
        json_obj.at("channel").get_to(settings.channel);
    }
}
