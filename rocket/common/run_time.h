#ifndef ROCKET_COMMON_RUN_TIME_H
#define ROCKET_COMMON_RUN_TIME_H


#include <string>

namespace rocket {

class RunTime {
 public:

 public:
  static RunTime* GetRunTime();

 public:
  std::string m_msgid;
  std::string m_method_name;

};

}


#endif