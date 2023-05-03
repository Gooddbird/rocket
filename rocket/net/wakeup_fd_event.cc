#include <unistd.h>
#include "rocket/net/wakeup_fd_event.h"
#include "rocket/common/log.h"


namespace rocket {

WakeUpFdEvent::WakeUpFdEvent(int fd) : FdEvent(fd) {
}

WakeUpFdEvent::~WakeUpFdEvent() {

}

void WakeUpFdEvent::wakeup() {
  char buf[8] = {'a'};
  int rt = write(m_fd, buf, 8);
  if (rt != 8) {
    ERRORLOG("write to wakeup fd less than 8 bytes, fd[%d]", m_fd);
  }
  DEBUGLOG("success read 8 bytes");
}


}