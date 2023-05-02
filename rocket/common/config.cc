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
    g_config = new Config(xmlfile);
  }
}
  
Config::Config(const char* xmlfile) {
  TiXmlDocument* xml_document = new TiXmlDocument();

  bool rt = xml_document->LoadFile(xmlfile);
  if (!rt) {
    printf("Start rocket server error, failed to read config file %s, error info[%s] \n", xmlfile, xml_document->ErrorDesc());
    exit(0);
  }

  READ_XML_NODE(root, xml_document);
  READ_XML_NODE(log, root_node);


  READ_STR_FROM_XML_NODE(log_level, log_node);

  m_log_level = log_level_str;

}


}