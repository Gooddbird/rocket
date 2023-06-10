#include <tinyxml/tinyxml.h>
#include "rocket/common/config.h"



#define READ_XML_NODE(name, parent) \
TiXmlElement* name##_node = parent->FirstChildElement(#name); \
if (!name##_node) { \
  printf("Start rocket server error, failed to read node [%s]\n", #name); \
  exit(0); \
} \



#define READ_STR_FROM_XML_NODE(name, parent) \
  TiXmlElement* name##_node = parent->FirstChildElement(#name); \
  if (!name##_node|| !name##_node->GetText()) { \
    printf("Start rocket server error, failed to read config file %s\n", #name); \
    exit(0); \
  } \
  std::string name##_str = std::string(name##_node->GetText()); \



namespace rocket {


static Config* g_config = NULL;


Config* Config::GetGlobalConfig() {
  return g_config;
}

void Config::SetGlobalConfig(const char* xmlfile) {
  if (g_config == NULL) {
    if (xmlfile != NULL) {
      g_config = new Config(xmlfile);
    } else {
      g_config = new Config();
    }

  }
}

Config::~Config() {
  if (m_xml_document) {
    delete m_xml_document;
    m_xml_document = NULL;
  }
}

Config::Config() {
  m_log_level = "DEBUG";

}
  
Config::Config(const char* xmlfile) {
  m_xml_document = new TiXmlDocument();

  bool rt = m_xml_document->LoadFile(xmlfile);
  if (!rt) {
    printf("Start rocket server error, failed to read config file %s, error info[%s] \n", xmlfile, m_xml_document->ErrorDesc());
    exit(0);
  }

  READ_XML_NODE(root, m_xml_document);
  READ_XML_NODE(log, root_node);
  READ_XML_NODE(server, root_node);

  READ_STR_FROM_XML_NODE(log_level, log_node);
  READ_STR_FROM_XML_NODE(log_file_name, log_node);
  READ_STR_FROM_XML_NODE(log_file_path, log_node);
  READ_STR_FROM_XML_NODE(log_max_file_size, log_node);
  READ_STR_FROM_XML_NODE(log_sync_interval, log_node);

  m_log_level = log_level_str;
  m_log_file_name = log_file_name_str;
  m_log_file_path = log_file_path_str;
  m_log_max_file_size = std::atoi(log_max_file_size_str.c_str()) ;
  m_log_sync_inteval = std::atoi(log_sync_interval_str.c_str());

  printf("LOG -- CONFIG LEVEL[%s], FILE_NAME[%s],FILE_PATH[%s] MAX_FILE_SIZE[%d B], SYNC_INTEVAL[%d ms]\n", 
    m_log_level.c_str(), m_log_file_name.c_str(), m_log_file_path.c_str(), m_log_max_file_size, m_log_sync_inteval);

  READ_STR_FROM_XML_NODE(port, server_node);
  READ_STR_FROM_XML_NODE(io_threads, server_node);

  m_port = std::atoi(port_str.c_str());
  m_io_threads = std::atoi(io_threads_str.c_str());


  TiXmlElement* stubs_node = root_node->FirstChildElement("stubs");

  if (stubs_node) {
    for (TiXmlElement* node = stubs_node->FirstChildElement("rpc_server"); node; node = node->NextSiblingElement("rpc_server")) {
      RpcStub stub;
      stub.name = std::string(node->FirstChildElement("name")->GetText());
      stub.timeout = std::atoi(node->FirstChildElement("timeout")->GetText());

      std::string ip = std::string(node->FirstChildElement("ip")->GetText());
      uint16_t port = std::atoi(node->FirstChildElement("port")->GetText());
      stub.addr = std::make_shared<IPNetAddr>(ip, port);

      m_rpc_stubs.insert(std::make_pair(stub.name, stub));
    }
  }



  printf("Server -- PORT[%d], IO Threads[%d]\n", m_port, m_io_threads);

}


}