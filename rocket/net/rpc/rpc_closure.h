#ifndef ROCKET_NET_RPC_RPC_CLOSURE_H
#define ROCKET_NET_RPC_RPC_CLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
#include "rocket/common/run_time.h"
#include "rocket/common/log.h"
#include "rocket/common/exception.h"

namespace rocket {

class RpcInterface;

class RpcClosure : public google::protobuf::Closure {
 public:
  typedef std::shared_ptr<RpcInterface> it_s_ptr;

  RpcClosure(it_s_ptr interface, std::function<void()> cb) : m_rpc_interface(interface), m_cb(cb) {
    INFOLOG("RpcClosure");
  }

  ~RpcClosure() {
    INFOLOG("~RpcClosure");
  }

  void Run() override {

    // 更新 runtime 的 RpcInterFace, 这里在执行 cb 的时候，都会以 RpcInterface 找到对应的接口，实现打印 app 日志等
    if (!m_rpc_interface) {
      RunTime::GetRunTime()->m_rpc_interface = m_rpc_interface.get();
    }
    try {
      if (m_cb != nullptr) {
        m_cb();
      }
    } catch (RocketException& e) {
        ERRORLOG("RocketException exception[%s], deal handle", e.what());
        e.handle();
    } catch (std::exception& e) {
        ERRORLOG("std::exception[%s]", e.what());
    }
    
    // // referenct count -1
    // if (m_rpc_interface) {
    //   m_rpc_interface.reset();
    // }

  }

 private:
  it_s_ptr m_rpc_interface {nullptr};
  std::function<void()> m_cb {nullptr};

};

}
#endif