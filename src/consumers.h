#ifndef CONSUMERS_H
#define CONSUMERS_H

#include "messaging/messages.h"

namespace medi_cloud::consumers
{
    namespace messages = messaging::messages;

    std::string pull_stream_consumer(const messages::PullStreamCommand& command);
}

#endif //CONSUMERS_H
