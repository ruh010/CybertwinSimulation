#ifndef CYBERTWIN_EDGE_SERVER_HELPER_H
#define CYBERTWIN_EDGE_SERVER_HELPER_H

#include "ns3/application-container.h"
#include "ns3/ipv4-address.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

namespace ns3
{

class CybertwinEdgeServerHelper
{
  public:
    CybertwinEdgeServerHelper(std::string protocol, Address address);

    void SetAttribute(std::string name, const AttributeValue& value);

    ApplicationContainer Install(NodeContainer c) const;
    ApplicationContainer Install(Ptr<Node> node) const;
    ApplicationContainer Install(std::string nodeName) const;

  private:
    Ptr<Application> InstallPriv(Ptr<Node> node) const;
    ObjectFactory m_factory;
};

} // namespace ns3

#endif