#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "rocket/common/msg_id_util.h"
#include "rocket/common/log.h"

namespace rocket {


static int g_msg_id_length = 20;
static int g_random_fd = -1;

static thread_local std::string t_msg_id_no;
static thread_local std::string t_max_msg_id_no;

std::string MsgIDUtil::GenMsgID() {
  if (t_msg_id_no.empty() || t_msg_id_no == t_max_msg_id_no) {
    if (g_random_fd == -1) {
      g_random_fd = open("/dev/urandom", O_RDONLY);
    }
    std::string res(g_msg_id_length, 0);
    if ((read(g_random_fd, &res[0], g_msg_id_length)) != g_msg_id_length) {
      ERRORLOG("read form /dev/urandom error");
      return "";
    }
    for (int i = 0; i < g_msg_id_length; ++i) {
      uint8_t x = ((uint8_t)(res[i])) % 10;
      res[i] = x + '0';
      t_max_msg_id_no += "9";
    }
    t_msg_id_no = res;
  } else {
    size_t i = t_msg_id_no.length() - 1;
    while (t_msg_id_no[i] == '9' && i >= 0) {
      i--;
    }
    if (i >= 0) {
      t_msg_id_no[i] += 1;
      for (size_t j = i + 1; j < t_msg_id_no.length(); ++j) {
        t_msg_id_no[j] = '0';
      }
    }
  }

  return t_msg_id_no;

}

}
