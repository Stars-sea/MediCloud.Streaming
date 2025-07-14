#ifndef CRABBITMQCLIENT_H
#define CRABBITMQCLIENT_H

#include <string>
#include <queue>
#include <rabbitmq-c/amqp.h>
#include "settings.h"

namespace medi_cloud::messaging::rabbitmq
{
    using std::string;

    class rabbitmq_client
    {
    public:
        rabbitmq_client(
            settings::RabbitMQConnectionSettings connection_settings,
            settings::RabbitMQChannelSettings    channel_settings);
        ~rabbitmq_client();


        int Connect();
        int Disconnect();

        /**
        *   @brief 声明exchange
        *	@param [in] exchange_name
        *   @param [in] type
        *   @param [in] passive
        *   @param [in] durable
        *   @param [in] auto_delete
        *   @return 等于0值代表成功创建exchange，小于0代表错误
        */
        int ExchangeDeclare(
            const string& exchange_name,
            const string& type,
            bool          passive     = false,
            bool          durable     = true,
            bool          auto_delete = false) const;

        /**
        *   @brief 声明消息队列
        *   @return 等于0值代表成功创建queue，小于0代表错误
        */
        int QueueDeclare(
            const std::string& queue_name,
            bool               passive     = false,
            bool               durable     = true,
            bool               exclusive   = false,
            bool               auto_delete = false) const;

        /**
        *   @brief 将队列，交换机和绑定规则绑定起来形成一个路由表
        *	@param [in] queue_name      队列名称
        *	@param [in] exchange_name   交换机名称
        *	@param [in] routing_key     路由名称  “msg.#” “msg.weather.**”
        *   @return 等于0值代表成功绑定，小于0代表错误
        */
        int QueueBind(const string& queue_name, const string& exchange_name, const string& routing_key) const;

        /**
        *   @brief 将队列，交换机和绑定规则绑定解除
        *	@param [in] queue_name      队列名称
        *	@param [in] exchange_name   交换机名称
        *	@param [in] routing_key     路由名称  “msg.#” “msg.weather.**”
        *   @return 等于0值代表成功绑定，小于0代表错误
        */
        int QueueUnbind(const string& queue_name, const string& exchange_name, const string& routing_key) const;

        /**
        *   @brief 删除消息队列。
        *	@param [in] queue_name      队列名称
        *	@param [in] is_unused       消息队列是否在用，1 则论是否在用都删除
        *   @return 等于0值代表成功删除queue，小于0代表错误
        */
        int QueueDelete(const std::string& queue_name, int is_unused) const;

        /**
        * @brief Publish  发布消息
        * @param [in] message           消息实体
        * @param [in] exchange_name     交换器
        * @param [in] routing_key       路由规则
        *   1.Direct Exchange – 处理路由键。需要将一个队列绑定到交换机上，要求该消息与一个特定的路由键完全匹配。
        *   2.Fanout Exchange – 不处理路由键。将队列绑定到交换机上。一个发送到交换机的消息都会被转发到与该交换机绑定的所有队列上。
        *   3.Topic Exchange – 将路由键和某模式进行匹配。此时队列需要绑定要一个模式上。符号“#”匹配一个或多个词，符号“*”匹配不多不少一个词。
        *      因此“audit.#”能够匹配到“audit.irs.corporate”，但是“audit.*” 只会匹配到“audit.irs”
        * @return 等于0值代表成功发送消息实体，小于0代表发送错误
        */
        int Publish(const string& message, const string& exchange_name, const string& routing_key) const;

        /**
        * @brief consumer  消费消息
        * @param [out] message_queue    获取的消息实体
        * @param [in]  queue_name       队列名称
        * @param [in]  msg_count        需要取得的消息个数
        * @param [in]  timeout          取得的消息是延迟，若为NULL，表示持续取，无延迟，阻塞状态
        * @return 等于0值代表成功，小于0代表错误，错误信息从ErrorReturn返回
        */
        int Consumer(
            std::queue<string>& message_queue,
            const std::string&  queue_name,
            int                 msg_count = 1,
            const timeval*      timeout   = nullptr) const;

    private:
        rabbitmq_client(const rabbitmq_client& rh) = default;
        // void operator=(const rabbitmq_client& rh) = default;

        static int ErrorMsg(const amqp_rpc_reply_t& x, char const* context);


        settings::RabbitMQConnectionSettings connection_settings_;
        settings::RabbitMQChannelSettings    channel_settings_;

        amqp_socket_t*          sock_;
        amqp_connection_state_t conn_;
    };
}

#endif //CRABBITMQCLIENT_H
