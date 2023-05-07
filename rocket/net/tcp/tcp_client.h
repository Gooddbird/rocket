#ifndef ROCKET_NET_TPC_TCP_CLIENT_H
#define ROCKET_NET_TPC_TCP_CLIENT_H

#include <sys/socket.h>
#include "rocket/net/tcp/tcp_connection.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/eventloop.h"
#include "rocket/net/fd_event_group.h"

namespace rocket {

class TcpClent{
 public:
  TcpClent(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);
    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, peer_addr);
    m_connection->setType(TcpClientType);

  }

  void connect(std::function<void()> done) {
    m_fd_event->listen(FdEvent::OUT_EVENT, [this, done](){
      int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
      m_connection->setState(Connected);
      if (done) {
        done();
      }
    });
    m_event_loop->addEpollEvent(m_fd_event);
    m_event_loop->loop();
  }

  void send(const std::string& req, std::function<void(std::string& req)> done) {
    m_connection->m_write_done = done;
    m_connection->m_out_buffer->writeToBuffer(req.c_str(), req.length());

    m_connection->listenWrite();
  }

  void read(std::string& response, std::function<void(std::string& req)> done) {
    m_connection->m_read_done = done;
    m_connection->listenRead();
  }

 private:
  NetAddr::s_ptr m_peer_addr;
  EventLoop* m_event_loop {NULL};
  int m_fd {-1};
  FdEvent* m_fd_event {NULL};
  TcpConnection::s_ptr m_connection;

}; 
}


#endif
