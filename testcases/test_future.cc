#include <future>
#include <pthread.h>
#include <unistd.h>
#include "rocket/common/log.h"
#include "rocket/common/config.h"
#include "rocket/common/log.h"
#include "rocket/net/tcp/net_addr.h"
#include "rocket/net/tcp/tcp_server.h"
#include "rocket/net/tcp/tcp_client.h"


std::promise<int> prom;
std::function<void()> cb;

std::string re = "";

void test_client() {
  rocket::IPNetAddr::s_ptr addr = std::make_shared<rocket::IPNetAddr>("127.0.0.1", 12345);
  rocket::TcpClent clent(addr);
  auto cb = []() {
    DEBUGLOG("get response %s", re.c_str());
  };
  clent.connect([addr, &clent](){
    DEBUGLOG("connect [%s] success", addr->toString().c_str());

    clent.send("hello, rocket", [](std::string& req) {
      DEBUGLOG("send data success [%s]", req.c_str());
    });

    std::string msg;
    clent.read(msg, [](std::string& res) {
      DEBUGLOG("get response success [%s]", res.c_str());
    });
  });
}

int main() {

  rocket::Config::SetGlobalConfig("../conf/rocket.xml");

  rocket::Logger::InitGlobalLogger();

  test_client();
  
  return 0;
}