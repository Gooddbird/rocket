#ifndef ROCKET_NET_TCP_TCP_ACCEPTOR_H
#define ROCKET_NET_TCP_TCP_ACCEPTOR_H

#include <memory>
#include "rocket/net/tcp/net_addr.h"

namespace rocket {

class TcpAcceptor {
 public:
  typedef std::shared_ptr<TcpAcceptor> s_ptr;

  TcpAcceptor(NetAddr::s_ptr local_addr);

  ~TcpAcceptor();

  std::pair<int, NetAddr::s_ptr> accept();

  int getListenFd();

 private:
  NetAddr::s_ptr m_local_addr; // 服务端监听的地址，addr -> ip:port 

  int m_family {-1};

  int m_listenfd {-1}; // 监听套接字

};

}

#endif