#ifndef MESSAGES_H
#define MESSAGES_H
#include <nlohmann/json_fwd.hpp>

namespace medi_cloud::messages
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
}

#endif //MESSAGES_H
