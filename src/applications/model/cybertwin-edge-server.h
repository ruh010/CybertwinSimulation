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

#include <string>
#include <unordered_map>

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

    bool ConnectionRequestCallback(Ptr<Socket> socket, const Address& address);
    void NewConnectionCreatedCallback(Ptr<Socket> socket, const Address& address);

    void NormalCloseCallback(Ptr<Socket> socket);
    void ErrorCloseCallback(Ptr<Socket> socket);

    void ReceivedDataCallback(Ptr<Socket> socket);

    Ptr<Socket> m_socket;
    Ptr<CybertwinControlTable> m_controlTable;

    Address m_local;
    uint16_t m_localPort;
    TypeId m_tid;
};

class CybertwinControlTable
{
  public:
    CybertwinControlTable();

    Ptr<CybertwinItem> Get(Ptr<Socket>);

    void Connect(Ptr<Socket>);
    void Disconnect(Ptr<Socket>);

    bool IsSocketConnected(Ptr<Socket>);

    void DoDispose();

  private:
    std::unordered_map<Ptr<Socket>, Ptr<CybertwinItem>> m_cybertwinTable;
    std::unordered_map<uint64_t, Ptr<Socket>> m_guidTable;
};

class CybertwinItem
{
  public:
    CybertwinItem();

    void PacketReceived(Ptr<Socket>);
    void AddSocket(Ptr<Socket>, uint64_t);
    uint32_t RemoveSocket(Ptr<Socket>);

    uint64_t GetInitialGuid() const;

  private:
    struct StreamState
    {
        uint32_t m_bytesToBeReceived;
        uint64_t m_dst;

        StreamState(uint64_t dst)
            : m_dst(dst),
              m_bytesToBeReceived(0)
        {
        }
    };

    std::unordered_map<Ptr<Socket>, StreamState> m_receiveStream;
    uint64_t m_initialGuid;
};

} // namespace ns3

#endif