#include "messages.h"

#include <nlohmann/json.hpp>

namespace medi_cloud::messages
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

}
