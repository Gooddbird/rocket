#ifndef ROCKET_NET_CODER_TINYPB_CODER_H
#define ROCKET_NET_CODER_TINYPB_CODER_H

#include "rocket/net/coder/abstract_coder.h"
#include "rocket/net/coder/tinypb_protocol.h"

namespace rocket {

class TinyPBCoder : public AbstractCoder {

 public:

  TinyPBCoder() {}
  ~TinyPBCoder() {}

  // 将 message 对象转化为字节流，写入到 buffer
  void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer);

  // 将 buffer 里面的字节流转换为 message 对象
  void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer);


 private:
  const char* encodeTinyPB(std::shared_ptr<TinyPBProtocol> message, int& len);

};


}


#endif