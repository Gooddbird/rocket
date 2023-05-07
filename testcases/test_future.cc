#include <future>
#include <pthread.h>
#include <unistd.h>
#include "rocket/common/log.h"
#include "rocket/common/config.h"


std::promise<int> prom;

void* fun(void* arg) {
  
  DEBUGLOG("now sleep 3s");
  sleep(3);
  DEBUGLOG("end sleep 3s");
  DEBUGLOG("now set future");
  prom.set_value(1);

  return NULL;
}

int main() {

  rocket::Config::SetGlobalConfig("../conf/rocket.xml");

  rocket::Logger::InitGlobalLogger();

  std::future<int> fu = prom.get_future();

  pthread_t thread;
  pthread_create(&thread, NULL, &fun, NULL);

  DEBUGLOG("now wait future");
  fu.wait();
  DEBUGLOG("future back");

  pthread_join(thread, NULL);
  return 0;
}