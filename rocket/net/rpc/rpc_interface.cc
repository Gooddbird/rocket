#include "rocket/common/log.h"
#include "rocket/net/rpc/rpc_closure.h"
#include "rocket/net/rpc/rpc_closure.h"
#include "rocket/net/rpc/rpc_controller.h"
#include "rocket/net/rpc/rpc_interface.h"

namespace rocket {

RpcInterface::RpcInterface(RpcClosure* done, RpcController* controller)
  : m_done(done) , m_controller(controller) {

}

RpcInterface::~RpcInterface() {

}

void RpcInterface::reply() {
  // reply to client
  // you should call is when you wan to set response back
  // it means this rpc method done 
  if (m_done) {
    m_done->Run();
  }

}


}