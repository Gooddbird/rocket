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

RpcChannel::RpcChannel(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
  INFOLOG("RpcChannel");
}

RpcChannel::~RpcChannel() {
  INFOLOG("~RpcChannel");
}


void RpcChannel::callBack() {
  RpcController* my_controller = dynamic_cast<RpcController*>(getController());
  if (my_controller->Finished()) {
    return;
  }

  if (m_closure) {
    m_closure->Run();
    if (my_controller) {
      my_controller->SetFinished(true);
    }
  }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                        google::protobuf::RpcController* controller, const google::protobuf::Message* request,
                        google::protobuf::Message* response, google::protobuf::Closure* done) {


  std::shared_ptr<rocket::TinyPBProtocol> req_protocol = std::make_shared<rocket::TinyPBProtocol>();

  RpcController* my_controller = dynamic_cast<RpcController*>(controller);
  if (my_controller == NULL || request == NULL || response == NULL) {
    ERRORLOG("failed callmethod, RpcController convert error");
    my_controller->SetError(ERROR_RPC_CHANNEL_INIT, "controller or request or response NULL");
    callBack();
    return;
  }

  if (m_peer_addr == nullptr) {
    ERRORLOG("failed get peer addr");
    my_controller->SetError(ERROR_RPC_PEER_ADDR, "peer addr nullptr");
    callBack();
    return;
  }

  m_client = std::make_shared<TcpClient>(m_peer_addr);

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

  if (!m_is_init) {
    std::string err_info = "RpcChannel not call init()";
    my_controller->SetError(ERROR_RPC_CHANNEL_INIT, err_info);
    ERRORLOG("%s | %s, RpcChannel not init ", req_protocol->m_msg_id.c_str(), err_info.c_str());
    callBack();
    return;
  }

  // requeset 的序列化
  if (!request->SerializeToString(&(req_protocol->m_pb_data))) {
    std::string err_info = "failde to serialize";
    my_controller->SetError(ERROR_FAILED_SERIALIZE, err_info);
    ERRORLOG("%s | %s, origin requeset [%s] ", req_protocol->m_msg_id.c_str(), err_info.c_str(), request->ShortDebugString().c_str());
    callBack();
    return;
  }

  s_ptr channel = shared_from_this(); 

  TimerEvent::s_ptr timer_event = std::make_shared<TimerEvent>(my_controller->GetTimeout(), false, [my_controller, channel]() mutable {
    INFOLOG("%s | call rpc timeout arrive", my_controller->GetMsgId().c_str());
    if (my_controller->Finished()) {
      channel.reset();
      return;
    }

    my_controller->StartCancel();
    my_controller->SetError(ERROR_RPC_CALL_TIMEOUT, "rpc call timeout " + std::to_string(my_controller->GetTimeout()));

    channel->callBack();
    channel.reset();
  });

  m_client->addTimerEvent(timer_event);

  m_client->connect([req_protocol, this]() mutable {

    RpcController* my_controller = dynamic_cast<RpcController*>(getController());

    if (getTcpClient()->getConnectErrorCode() != 0) {
      my_controller->SetError(getTcpClient()->getConnectErrorCode(), getTcpClient()->getConnectErrorInfo());
      ERRORLOG("%s | connect error, error coode[%d], error info[%s], peer addr[%s]", 
        req_protocol->m_msg_id.c_str(), my_controller->GetErrorCode(), 
        my_controller->GetErrorInfo().c_str(), getTcpClient()->getPeerAddr()->toString().c_str());

      callBack();

      return;
    }

    INFOLOG("%s | connect success, peer addr[%s], local addr[%s]",
      req_protocol->m_msg_id.c_str(), 
      getTcpClient()->getPeerAddr()->toString().c_str(), 
      getTcpClient()->getLocalAddr()->toString().c_str()); 

    getTcpClient()->writeMessage(req_protocol, [req_protocol, this, my_controller](AbstractProtocol::s_ptr) mutable {
      INFOLOG("%s | send rpc request success. call method name[%s], peer addr[%s], local addr[%s]", 
        req_protocol->m_msg_id.c_str(), req_protocol->m_method_name.c_str(),
        getTcpClient()->getPeerAddr()->toString().c_str(), getTcpClient()->getLocalAddr()->toString().c_str());

        getTcpClient()->readMessage(req_protocol->m_msg_id, [this, my_controller](AbstractProtocol::s_ptr msg) mutable {
        std::shared_ptr<rocket::TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<rocket::TinyPBProtocol>(msg);
        INFOLOG("%s | success get rpc response, call method name[%s], peer addr[%s], local addr[%s]", 
          rsp_protocol->m_msg_id.c_str(), rsp_protocol->m_method_name.c_str(),
          getTcpClient()->getPeerAddr()->toString().c_str(), getTcpClient()->getLocalAddr()->toString().c_str());

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
          getTcpClient()->getPeerAddr()->toString().c_str(), getTcpClient()->getLocalAddr()->toString().c_str())

        callBack();
      
      });

    });

  });

}


void RpcChannel::Init(controller_s_ptr controller, message_s_ptr req, message_s_ptr res, closure_s_ptr done) {
  if (m_is_init) {
    return;
  }
  m_controller = controller;
  m_request = req; 
  m_response = res;
  m_closure = done;
  m_is_init = true;
}

google::protobuf::RpcController* RpcChannel::getController() {
  return m_controller.get();
}

google::protobuf::Message* RpcChannel::getRequest() {
  return m_request.get();
}

google::protobuf::Message* RpcChannel::getResponse() {
  return m_response.get();
}

google::protobuf::Closure* RpcChannel::getClosure() {
  return m_closure.get();
}


TcpClient* RpcChannel::getTcpClient() {
  return m_client.get();
}


NetAddr::s_ptr RpcChannel::FindAddr(const std::string& str) {
  if (IPNetAddr::CheckValid(str)) {
    return std::make_shared<IPNetAddr>(str);
  } else {
    auto it = Config::GetGlobalConfig()->m_rpc_stubs.find(str);
    if (it != Config::GetGlobalConfig()->m_rpc_stubs.end()) {
      INFOLOG("find addr [%s] in global config of str[%s]", (*it).second.addr->toString().c_str(), str.c_str());
      return (*it).second.addr;
    } else {
      INFOLOG("can not find addr in global config of str[%s]", str.c_str());
      return nullptr;
    }
  }
}


}