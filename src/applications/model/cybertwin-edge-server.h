#ifndef CYBERTWIN_EDGE_SERVER_H
#define CYBERTWIN_EDGE_SERVER_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ptr.h"
#include "ns3/seq-ts-size-header.h"
#include "ns3/traced-callback.h"

#endif

namespace ns3
{
class Address;
class Socket;
class Packet;

class CybertwinEdgeServer : public Application
{
  public:
    static TypeId GetTypeId();
    CybertwinEdgeServer();

    ~CybertwinEdgeServer() override;

  protected:
    void DoDispose() override;

  private:
    void StartApplication() override;
    void StopApplication() override;

    void HandleRead(Ptr<Socket> socket);
    void HandleAccept(Ptr<Socket> socket, const Address& from);
    void HandlePeerClose(Ptr<Socket> socket);
    void HandlePeerError(Ptr<Socket> socket);

    void PacketReceived(const Ptr<Packet>& p, const Address& from, const Address& localAddress);

    Ptr<Socket> m_socket;
    std::list<Ptr<Socket>> m_socketList;

    Address m_local;
    uint16_t m_localPort;
    TypeId m_tid;
};

} // namespace ns3