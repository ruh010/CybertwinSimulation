#include "cybertwin-edge-server-helper.h"

#include "ns3/inet-socket-address.h"
#include "ns3/names.h"
#include "ns3/string.h"

namespace ns3
{

CybertwinEdgeServerHelper::CybertwinEdgeServerHelper(std::string protocol, Address address)
{
    m_factory.SetTypeId("ns3::CybertwinEdgeServer");
    m_factory.Set("Protocol", StringValue(protocol));
    m_factory.Set("Local", AddressValue(address));
}

void
CybertwinEdgeServerHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
CybertwinEdgeServerHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
CybertwinEdgeServerHelper::Install(std::string nodeName) const
{
    Ptr<Node> node = Names::Find<Node>(nodeName);
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
CybertwinEdgeServerHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
CybertwinEdgeServerHelper::InstallPriv(Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<Application>();
    node->AddApplication(app);

    return app;
}

} // namespace ns3