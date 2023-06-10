#ifndef ROCKET_NET_RPC_RPC_CHANNEL_H
#define ROCKET_NET_RPC_RPC_CHANNEL_H

#include <google/protobuf/service.h>
#include <memory>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/net/timer_event.h"

namespace rocket {


#define NEWMESSAGE(type, var_name) \
  std::shared_ptr<type> var_name = std::make_shared<type>(); \

#define NEWRPCCONTROLLER(var_name) \
  std::shared_ptr<rocket::RpcController> var_name = std::make_shared<rocket::RpcController>(); \

#define NEWRPCCHANNEL(addr, var_name) \
  std::shared_ptr<rocket::RpcChannel> var_name = std::make_shared<rocket::RpcChannel>(std::make_shared<rocket::IPNetAddr>(addr)); \

#define CALLRPRC(addr, stub_name, method_name, controller, request, response, closure) \
  { \
  NEWRPCCHANNEL(addr, channel); \
  channel->Init(controller, request, response, closure); \
  stub_name(channel.get()).method_name(controller.get(), request.get(), response.get(), closure.get()); \
  } \



class RpcChannel : public google::protobuf::RpcChannel, public std::enable_shared_from_this<RpcChannel> {
 
 public:
  typedef RpcChannel* s_ptr;
  typedef google::protobuf::RpcController* controller_s_ptr;
  typedef google::protobuf::Message* message_s_ptr;
  typedef google::protobuf::Closure* closure_s_ptr;

 public:
  // 创建实例
  static RpcChannel* Create(NetAddr::s_ptr peer_addr);

  // 销毁实例
  static void Destroy(RpcChannel* rpc_channel);

 public:

  // void Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done);

  void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                          google::protobuf::Message* response, google::protobuf::Closure* done);


  google::protobuf::RpcController* getController(); 

  google::protobuf::Message* getRequest();

  google::protobuf::Message* getResponse();

  google::protobuf::Closure* getClosure();

  TcpClient* getTcpClient();

  TimerEvent::s_ptr getTimerEvent();

  void callBack();

 private:

  // 析构函数设为私有，这样就无法在栈上创建对象，只能用 new
  RpcChannel(NetAddr::s_ptr peer_addr);

  // 析构函数设为私有，这样就无法在栈上创建对象，只能用 new
  ~RpcChannel();

 private:
  NetAddr::s_ptr m_peer_addr {nullptr};
  NetAddr::s_ptr m_local_addr {nullptr};

  google::protobuf::RpcController* m_controller {NULL};
  google::protobuf::Message* m_request {NULL};
  google::protobuf::Message* m_response {NULL};
  google::protobuf::Closure* m_closure {NULL};

  // bool m_is_init {false};

  TcpClient::s_ptr m_client {nullptr};

  TimerEvent ::s_ptr m_timer_event{nullptr};

};

}


#endif