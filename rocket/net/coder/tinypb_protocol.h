#ifndef ROCKET_NET_CODER_TINYPB_PROTOCOL_H
#define ROCKET_NET_CODER_TINYPB_PROTOCOL_H 

#include <string>
#include "rocket/net/coder/abstract_protocol.h"

namespace rocket {

struct TinyPBProtocol : public AbstractProtocol {
 public:
  TinyPBProtocol(){}
  ~TinyPBProtocol() {}

 public:
  static char PB_START;
  static char PB_END;

 public:
  int32_t m_pk_len {0};
  int32_t m_req_id_len {0};
  // req_id 继承父类

  int32_t m_method_name_len {0};
  std::string m_method_name;
  int32_t m_err_code {0};
  int32_t m_err_info_len {0};
  std::string m_err_info;
  std::string m_pb_data;
  int32_t m_check_sum {0};

  bool parse_success {false};

};



}


#endif