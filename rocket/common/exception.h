#ifndef ROCKET_COMMON_EXCEPTION_H
#define ROCKET_COMMON_EXCEPTION_H

#include <exception>

namespace rocket {

class RocketException : public std::exception {
 public:

  RocketException() = default;

  // 异常处理
  // 当 EventLoop 捕获到 RocketException 及其子类对象的异常时，会执行该函数
  virtual void handle() = 0;

  virtual ~RocketException() {};

};


}



#endif