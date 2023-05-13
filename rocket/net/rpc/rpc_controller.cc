
#include "rocket/net/rpc/rpc_controller.h"

namespace rocket {

void RpcController::Reset() {
  m_error_code = 0;
  m_error_info = "";
  m_req_id = "";
  m_is_failed = false;
  m_is_cancled = false;
  m_local_addr = nullptr;
  m_peer_addr = nullptr;
  m_timeout = 1000;   // ms
}

bool RpcController::Failed() const {
  return m_is_failed;
}

std::string RpcController::ErrorText() const {
  return m_error_info;
}

void RpcController::StartCancel() {
  m_is_cancled = true;
}

void RpcController::SetFailed(const std::string& reason) {
  m_error_info = reason;
}

bool RpcController::IsCanceled() const {
  return m_is_cancled;
}

void RpcController::NotifyOnCancel(google::protobuf::Closure* callback) {

}


void RpcController::SetError(int32_t error_code, const std::string error_info) {
  m_error_code = error_code;
  m_error_info = error_info;
}

int32_t RpcController::GetErrorCode() {
  return m_error_code;
}

std::string RpcController::GetErrorInfo() {
  return m_error_info;
}

void RpcController::SetReqId(const std::string& req_id) {
  m_req_id = req_id;
}

std::string RpcController::GetReqId() {
  return m_req_id;
}

void RpcController::SetLocalAddr(NetAddr::s_ptr addr) {
  m_local_addr = addr;
}

void RpcController::SetPeerAddr(NetAddr::s_ptr addr) {
  m_peer_addr = addr;
}

NetAddr::s_ptr RpcController::GetLocalAddr() {
  return m_local_addr;
}

NetAddr::s_ptr RpcController::GetPeerAddr() {
  return m_peer_addr;
}

void RpcController::SetTimeout(int timeout) {
  m_timeout = timeout;
}

int RpcController::GetTimeout() {
  return m_timeout;
}

}