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
