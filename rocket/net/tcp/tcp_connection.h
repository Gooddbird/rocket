#ifndef ROCKET_NET_TCP_TCP_CONNECTION_H
#define ROCKET_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_buffer.h"
#include "rocket/net/io_thread.h"

namespace rocket {

enum TcpState {
  NotConnected = 1,
  Connected = 2,
  HalfClosing = 3,
  Closed = 4,
};

enum TcpType {
  TcpServerType = 1,
  TcpClientType = 2,
};

class TcpConnection {
 public:

  typedef std::shared_ptr<TcpConnection> s_ptr;


 public:
  TcpConnection(IOThread* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);
  TcpConnection(EventLoop* io_thread, int fd, int buffer_size, NetAddr::s_ptr peer_addr);

  ~TcpConnection();

  void onRead();

  void excute();

  void onWrite();

  void setState(const TcpState state);

  TcpState getState();

  void clear();

  // 服务器主动关闭连接
  void shutdown();

  void listenWrite();

  void listenRead();
  
  void setType(TcpType type) { m_type = type;}


 private:

  IOThread* m_io_thread {NULL};   // 代表持有该连接的 IO 线程

  EventLoop* m_event_loop {NULL};   // 代表持有该连接的 IO 线程

  NetAddr::s_ptr m_local_addr;
  NetAddr::s_ptr m_peer_addr;
 public:
  TcpBuffer::s_ptr m_in_buffer;   // 接收缓冲区
  TcpBuffer::s_ptr m_out_buffer;  // 发送缓冲区


  FdEvent* m_fd_event {NULL};

  TcpState m_state;

  int m_fd {0};

  TcpType m_type {TcpServerType};
  std::string* m_res {NULL};
  std::function<void(std::string&)> m_write_done {nullptr};
  std::function<void(std::string&)> m_read_done {nullptr};
  
};

}

#endif
