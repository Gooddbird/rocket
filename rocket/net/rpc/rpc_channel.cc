#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>
#include "rocket/net/rpc/rpc_channel.h"
#include "rocket/net/rpc/rpc_controller.h"
#include "rocket/net/coder/tinypb_protocol.h"
#include "rocket/net/tcp/tcp_client.h"
#include "rocket/common/log.h"
#include "rocket/common/msg_id_util.h"
#include "rocket/common/error_code.h"
#include "rocket/common/run_time.h"
#include "rocket/net/timer_event.h"

namespace rocket {

// 创建实例
RpcChannel* RpcChannel::Create(NetAddr::s_ptr peer_addr) {
  return new RpcChannel(peer_addr);
}

void RpcChannel::Destroy(RpcChannel* rpc_channel) {
  if (rpc_channel) {
    delete rpc_channel;
    rpc_channel = NULL;
  }
}

RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
  INFOLOG("RpcChannel");
  m_client = std::make_shared<TcpClient>(m_peer_addr);
}

RpcChannel::~RpcChannel() {
  INFOLOG("~RpcChannel");
}


void RpcChannel::callBack() {
  INFOLOG("now begin rpc callback");
  if (m_closure) {
    m_closure->Run();
    delete m_closure;
    m_closure = NULL;
  }

  if (m_request) {
    delete m_request;
    m_request = NULL;
  }

  if (m_response) {
    delete m_response;
    m_response = NULL;
  }
  if (m_controller) {
    delete m_controller;
    m_controller = NULL;
  }

}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                        google::protobuf::Message* response, google::protobuf::Closure* done) {

  
  m_controller = controller;
  m_request = const_cast<google::protobuf::Message*>(request); 
  m_response = response;
  m_closure = done;

  std::shared_ptr<rocket::TinyPBProtocol> req_protocol = std::make_shared<rocket::TinyPBProtocol>();

  RpcController* my_controller = dynamic_cast<RpcController*>(controller);

  if (!m_request || !m_response || !my_controller) {
    ERRORLOG("failed callmethod, RpcController or Request or Response NULL");
    my_controller->SetError(ERROR_RPC_CHANNEL_INIT, "failed callmethod, RpcController or Request or Response NULL");
    callBack();
    return;
  }

  if (my_controller->GetMsgId().empty()) {
    // 先从 runtime 里面取, 取不到再生成一个
    // 这样的目的是为了实现 msg_id 的透传，假设服务 A 调用了 B，那么同一个 msgid 可以在服务 A 和 B 之间串起来，方便日志追踪
    std::string msg_id = RunTime::GetRunTime()->m_msgid;
    if (!msg_id.empty()) {
      req_protocol->m_msg_id = msg_id;
      my_controller->SetMsgId(msg_id);
    } else {
      req_protocol->m_msg_id = MsgIDUtil::GenMsgID();
      my_controller->SetMsgId(req_protocol->m_msg_id);
    }

  } else {
    // 如果 controller 指定了 msgno, 直接使用
    req_protocol->m_msg_id = my_controller->GetMsgId();
  }

  req_protocol->m_method_name = method->full_name();
  INFOLOG("%s | call method name [%s]", req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str());

  // requeset 的序列化
  if (!request->SerializeToString(&(req_protocol->m_pb_data))) {
    std::string err_info = "failde to serialize";
    my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
    ERRORLOG("%s | %s, origin requeset [%s] ", req_protocol->m_msg_id.c_str(), err_info.c_str(), request->ShortDebugString().c_str());
    callBack();
    return;
  }

  m_timer_event = std::make_shared<TimerEvent>(my_controller->GetTimeout(), false, [my_controller, this]() mutable {
    if (my_controller->Finished()) {
      return;
    }

    my_controller->StartCancel();
    my_controller->SetError(ERROR_RPC_CALL_TIMEOUT, "rpc call timeout " + std::to_string(my_controller->GetTimeout()));
    this->callBack();

  });

  m_client->addTimerEvent(m_timer_event);

  m_client->connect([req_protocol, this]() mutable {

    RpcController* my_controller = dynamic_cast<RpcController*>(getController());
    TcpClient* client = getTcpClient();

    if (client->getConnectErrorCode() != 0) {
      my_controller->SetError(client->getConnectErrorCode(), client->getConnectErrorInfo());
      ERRORLOG("%s | connect error, error coode[%d], error info[%s], peer addr[%s]", 
        req_protocol->m_msg_id.c_str(), my_controller->GetErrorCode(), 
        my_controller->GetErrorInfo().c_str(), client->getPeerAddr()->toString().c_str());

      // 取消定时任务
      getTimerEvent()->setCancled(true);
      // 执行回调
      callBack();

      return;
    }

    INFOLOG("%s | connect success, peer addr[%s], local addr[%s]",
      req_protocol->m_msg_id.c_str(), 
      client->getPeerAddr()->toString().c_str(), 
      client->getLocalAddr()->toString().c_str()); 

    client->writeMessage(req_protocol, [req_protocol, this, my_controller, client](AbstractProtocol::s_ptr) mutable {
      INFOLOG("%s | send rpc request success. call method name[%s], peer addr[%s], local addr[%s]", 
        req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str(),
        client->getPeerAddr()->toString().c_str(), client->getLocalAddr()->toString().c_str());

        client->readMessage(req_protocol->m_msg_id, [this, my_controller, client](AbstractProtocol::s_ptr msg) mutable {
        std::shared_ptr<rocket::TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(msg);
        INFOLOG("%s | success get rpc response, call method name[%s], peer addr[%s], local addr[%s]", 
          rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
          client->getPeerAddr()->toString().c_str(), client->getLocalAddr()->toString().c_str());

        // 当成功读取到回包后， 取消定时任务
        getTimerEvent()->setCancled(true);

        if (!(getResponse()->ParseFromString(rsp_protocol->m_pb_data))){
          ERRORLOG("%s | serialize error", rsp_protocol->m_msg_id.c_str());
          my_controller->SetError(ERROR_FAILED_SERIALIZE, "serialize error");
          callBack();
          return;
        }

        if (rsp_protocol->m_err_code != 0) {
          ERRORLOG("%s | call rpc methood[%s] failed, error code[%d], error info[%s]", 
            rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
            rsp_protocol->m_err_code, rsp_protocol->m_err_info.c_str());

          my_controller->SetError(rsp_protocol->m_err_code, rsp_protocol->m_err_info);
          callBack();
          return;
        }

        INFOLOG("%s | call rpc success, call method name[%s], peer addr[%s], local addr[%s]",
          rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
          client->getPeerAddr()->toString().c_str(), client->getLocalAddr()->toString().c_str())

        if (!my_controller->IsCanceled()) {
          callBack();
        }
      
      });
    });
  });

}


google::protobuf::RpcController* RpcChannel::getController() {
  return m_controller;
}

google::protobuf::Message* RpcChannel::getRequest() {
  return m_request;
}

google::protobuf::Message* RpcChannel::getResponse() {
  return m_response;
}

google::protobuf::Closure* RpcChannel::getClosure() {
  return m_closure;
}

TcpClient* RpcChannel::getTcpClient() {
  return m_client.get();
}

TimerEvent::s_ptr RpcChannel::getTimerEvent() {
  return m_timer_event;
}

}