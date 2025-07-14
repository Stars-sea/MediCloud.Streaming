#include "rabbitmq_client.h"

#include <iostream>
#include <queue>
#include <utility>
#include <rabbitmq-c/tcp_socket.h>

namespace medi_cloud::messaging::rabbitmq
{
    rabbitmq_client::rabbitmq_client(
        settings::RabbitMQConnectionSettings connection_settings,
        settings::RabbitMQChannelSettings    channel_settings) :
        connection_settings_(std::move(connection_settings)),
        channel_settings_(std::move(channel_settings)),
        sock_(nullptr),
        conn_(nullptr)
    {}

    rabbitmq_client::~rabbitmq_client()
    {
        if (nullptr != conn_)
        {
            Disconnect();
            conn_ = nullptr;
        }
    }

    int rabbitmq_client::Connect()
    {
        conn_ = amqp_new_connection();
        if (nullptr == conn_)
        {
            std::println(std::cerr, "amqp new connection failed");
            return -1;
        }

        sock_ = amqp_tcp_socket_new(conn_);
        if (nullptr == sock_)
        {
            std::println(std::cerr, "amqp tcp new socket failed");
            return -2;
        }

        if (amqp_socket_open(sock_, connection_settings_.host.c_str(), connection_settings_.port) < 0)
        {
            std::println(std::cerr, "amqp socket open failed");
            return -3;
        }

        // amqp_login(amqp_connection_state_t state,char const *vhost, int channel_max, int frame_max, int heartbeat, amqp_sasl_method_enum sasl_method, ..)
        const amqp_rpc_reply_t reply = amqp_login(
            conn_,
            connection_settings_.vhost.c_str(),
            0,
            131072,
            0,
            AMQP_SASL_METHOD_PLAIN,
            connection_settings_.user.c_str(),
            connection_settings_.password.c_str());
        if (0 != ErrorMsg(reply, "Logging in"))
            return -4;

        return 0;
    }

    int rabbitmq_client::Disconnect()
    {
        if (nullptr != conn_)
        {
            if (0 != ErrorMsg(amqp_connection_close(conn_, AMQP_REPLY_SUCCESS), "Closing connection"))
                return -1;

            if (amqp_destroy_connection(conn_) < 0)
                return -2;

            conn_ = nullptr;
        }

        return 0;
    }

    int rabbitmq_client::ExchangeDeclare(
        const string& exchange_name,
        const string& type,
        const bool    passive,
        const bool    durable,
        const bool    auto_delete) const
    {
        amqp_channel_open(conn_, channel_settings_.channel);

        const amqp_bytes_t _exchange = amqp_cstring_bytes(exchange_name.c_str());
        const amqp_bytes_t _type     = amqp_cstring_bytes(type.c_str());

        amqp_exchange_declare(
            conn_, channel_settings_.channel,
            _exchange, _type, passive, durable, auto_delete, 0, amqp_empty_table);

        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "exchange_declare"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -1;
        }

        amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
        return 0;
    }

    int rabbitmq_client::QueueDeclare(
        const string& queue_name,
        const bool    passive,
        const bool    durable,
        const bool    exclusive,
        const bool    auto_delete) const
    {
        if (nullptr == conn_)
        {
            std::println(std::cerr, "QueueDeclare conn_ is nullptr");
            return -1;
        }

        amqp_channel_open(conn_, channel_settings_.channel);
        amqp_queue_declare(
            conn_, channel_settings_.channel,
            amqp_cstring_bytes(queue_name.c_str()),
            passive, durable, exclusive, auto_delete, amqp_empty_table);
        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "queue_declare"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -1;
        }

        amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
        return 0;
    }

    int rabbitmq_client::QueueBind(
        const string& queue_name,
        const string& exchange_name,
        const string& routing_key) const
    {
        if (nullptr == conn_)
        {
            std::println(std::cerr, "QueueBind conn_ is nullptr");
            return -1;
        }

        amqp_channel_open(conn_, channel_settings_.channel);
        const amqp_bytes_t _queue     = amqp_cstring_bytes(queue_name.c_str());
        const amqp_bytes_t _exchange  = amqp_cstring_bytes(exchange_name.c_str());
        const amqp_bytes_t _route_key = amqp_cstring_bytes(routing_key.c_str());
        amqp_queue_bind(conn_, channel_settings_.channel, _queue, _exchange, _route_key, amqp_empty_table);
        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "queue_bind"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -1;
        }

        amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
        return 0;
    }

    int rabbitmq_client::QueueUnbind(
        const string& queue_name,
        const string& exchange_name,
        const string& routing_key) const
    {
        if (nullptr == conn_)
        {
            std::println(std::cerr, "QueueUnbind conn_ is nullptr");
            return -1;
        }

        amqp_channel_open(conn_, channel_settings_.channel);
        const amqp_bytes_t _queue     = amqp_cstring_bytes(queue_name.c_str());
        const amqp_bytes_t _exchange  = amqp_cstring_bytes(exchange_name.c_str());
        const amqp_bytes_t _route_key = amqp_cstring_bytes(routing_key.c_str());
        amqp_queue_unbind(conn_, channel_settings_.channel, _queue, _exchange, _route_key, amqp_empty_table);
        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "queue_unbind"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -1;
        }

        amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
        return 0;
    }

    int rabbitmq_client::QueueDelete(const string& queue_name, const int is_unused) const
    {
        if (nullptr == conn_)
        {
            std::println(std::cerr, "QueueDelete conn_ is nullptr");
            return -1;
        }

        amqp_channel_open(conn_, channel_settings_.channel);
        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "open channel"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -2;
        }

        amqp_queue_delete(
            conn_,
            channel_settings_.channel,
            amqp_cstring_bytes(queue_name.c_str()),
            is_unused,
            0);
        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "delete queue"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -3;
        }

        amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
        return 0;
    }

    int rabbitmq_client::Publish(const string& message, const string& exchange_name, const string& routing_key) const
    {
        if (nullptr == conn_)
        {
            std::println(std::cerr, "publish conn_ is nullptr, publish failed");
            return -1;
        }

        amqp_channel_open(conn_, channel_settings_.channel);
        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "open channel"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -2;
        }

        amqp_bytes_t message_bytes;
        message_bytes.len = message.length();
        // ReSharper disable once CppCStyleCast
        message_bytes.bytes = (void*)message.c_str();
        //std::println(std::cerr, "publish message(%d): %.*s", (int)message_bytes.len, (int)message_bytes.len, (char *)message_bytes.bytes);

        /*
        amqp_basic_properties_t props;
        props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
        props.content_type = amqp_cstring_bytes(m_type.c_str());
        props.delivery_mode = m_durable;    // persistent delivery mode
        */

        //if (0 != amqp_basic_publish(conn_, channel_settings_.channel, exchange, routekey, 0, 0, &props, message_bytes)) {
        if (0 != amqp_basic_publish(
            conn_, channel_settings_.channel,
            amqp_cstring_bytes(exchange_name.c_str()),
            amqp_cstring_bytes(routing_key.c_str()),
            0,
            0,
            nullptr,
            message_bytes))
        {
            std::println(std::cerr, "publish amqp_basic_publish failed");
            if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "amqp_basic_publish"))
            {
                amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
                return -3;
            }
        }

        amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
        return 0;
    }

    int rabbitmq_client::Consumer(
        std::queue<string>& message_queue,
        const string&       queue_name,
        int                 msg_count,
        const timeval*      timeout) const
    {
        if (nullptr == conn_)
        {
            std::println(std::cerr, "Consumer conn_ is nullptr, Consumer failed");
            return -1;
        }

        amqp_channel_open(conn_, channel_settings_.channel);
        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "open channel"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -2;
        }

        amqp_basic_qos(conn_, channel_settings_.channel, 0, msg_count, 0);
        constexpr int ack = false; // no_ack    是否需要确认消息后再从队列中删除消息
        amqp_basic_consume(
            conn_, channel_settings_.channel,
            amqp_cstring_bytes(queue_name.c_str()),
            amqp_empty_bytes,
            0,
            ack,
            0,
            amqp_empty_table);

        if (0 != ErrorMsg(amqp_get_rpc_reply(conn_), "Consuming"))
        {
            amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
            return -3;
        }

        int is_get = 0;

        amqp_envelope_t envelope;
        while (msg_count > 0)
        {
            amqp_maybe_release_buffers(conn_);
            if (const amqp_rpc_reply_t res = amqp_consume_message(conn_, &envelope, timeout, 0);
                AMQP_RESPONSE_NORMAL != res.reply_type)
            {
                std::println(std::cerr, "Consumer amqp_channel_close failed");
                amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);

                if (0 == is_get)
                    return -res.reply_type;
                return 0;
            }

            string str(static_cast<char*>(envelope.message.body.bytes),
                       static_cast<char*>(envelope.message.body.bytes) + envelope.message.body.len);
            message_queue.push(str);
            const int rtn = amqp_basic_ack(conn_, channel_settings_.channel, envelope.delivery_tag, 1);
            amqp_destroy_envelope(&envelope);
            if (rtn != 0)
            {
                amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
                return -4;
            }

            msg_count--;
            is_get++;
            usleep(1);
        }

        amqp_channel_close(conn_, channel_settings_.channel, AMQP_REPLY_SUCCESS);
        return 0;
    }

    int rabbitmq_client::ErrorMsg(const amqp_rpc_reply_t& x, char const* context)
    {
        switch (x.reply_type)
        {
        case AMQP_RESPONSE_NORMAL:
            return 0;

        case AMQP_RESPONSE_NONE:
            std::println(std::cerr, "{}: missing RPC reply type!", context);
            break;

        case AMQP_RESPONSE_LIBRARY_EXCEPTION:
            std::println(std::cerr, "{}: {}", context, amqp_error_string2(x.library_error));
            break;

        case AMQP_RESPONSE_SERVER_EXCEPTION:
            switch (x.reply.id)
            {
            case AMQP_CONNECTION_CLOSE_METHOD: {
                const auto  m = static_cast<amqp_connection_close_t*>(x.reply.decoded);
                std::string msg{static_cast<char*>(m->reply_text.bytes), m->reply_text.len};
                std::println(std::cerr, "{}: server connection error {}, message: {}",
                             context, m->reply_code, msg);
                return m->reply_code;
            }
            case AMQP_CHANNEL_CLOSE_METHOD: {
                const auto  m = static_cast<amqp_channel_close_t*>(x.reply.decoded);
                std::string msg{static_cast<char*>(m->reply_text.bytes), m->reply_text.len};
                std::println(std::cerr, "{}: server channel error {}, message: {}",
                             context, m->reply_code, msg);
                return m->reply_code;
            }
            default:
                std::println(std::cerr, "{}: unknown server error, method id {}",
                             context, x.reply.id);
                break;
            }
            break;
        }

        return -1;
    }
}
