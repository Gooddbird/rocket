#ifndef ROCKET_COMMON_CONFIG_H
#define ROCKET_COMMON_CONFIG_H

#include <map>
#include <tinyxml/tinyxml.h>


namespace rocket {

class Config {
 public:
  
  Config(const char* xmlfile);

  Config();

  ~Config();

 public:
  static Config* GetGlobalConfig();
  static void SetGlobalConfig(const char* xmlfile);

 public:
  std::string m_log_level;
  std::string m_log_file_name;
  std::string m_log_file_path;
  int m_log_max_file_size {0};
  int m_log_sync_inteval {0};   // 日志同步间隔，ms

  int m_port {0};
  int m_io_threads {0};

  TiXmlDocument* m_xml_document{NULL};

};


}


#endif