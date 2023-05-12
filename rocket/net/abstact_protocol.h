#ifndef ROCKET_NET_ABSTRACT_PROTOCOL_H
#define ROCKET_NET_ABSTRACT_PROTOCOL_H

#include <memory>


namespace rocket {

class AbstractProtocol {
 public:
  typedef std::shared_ptr<AbstractProtocol> s_ptr;

};

}


#endif
