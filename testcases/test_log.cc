
#include <pthread.h>
#include "rocket/common/log.h"
#include "rocket/common/config.h"


void* fun(void*) {
  
  int i = 20;
  while (i--) {
    DEBUGLOG("debug this is thread in %s", "fun");
    INFOLOG("info this is thread in %s", "fun");
  }

  return NULL;
}

int main() {

  rocket::Config::SetGlobalConfig(NULL);

  rocket::Logger::InitGlobalLogger(0);


  pthread_t thread;
  pthread_create(&thread, NULL, &fun, NULL);

  int i = 20;
  while (i--) {
    DEBUGLOG("test debug log %s", "11");
    INFOLOG("test info log %s", "11");
  }

  pthread_join(thread, NULL);
  return 0;
}