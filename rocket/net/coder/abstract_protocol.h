#ifndef ROCKET_NET_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_ABSTRACT_PROTOCOL_H

#include <memory>


namespace rocket {

struct AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> {
 public:
  typedef std::shared_ptr<AbstractProtocol> s_ptr;

  virtual ~AbstractProtocol() {}

 public:
  std::string m_req_id;     // 请求号，唯一标识一个请求或者响应


};

}


#endif
