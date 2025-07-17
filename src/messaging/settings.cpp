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
