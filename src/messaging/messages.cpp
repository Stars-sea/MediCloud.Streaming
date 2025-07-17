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


#include "messages.h"

#include <nlohmann/json.hpp>
#include <uuid/uuid.h>

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

}
