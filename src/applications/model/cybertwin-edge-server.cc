#include "cybertwin-edge-server.h"

#include "cybertwin-packet-header.h"

#include "ns3/address.h"
#include "ns3/boolean.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/ipv4-packet-info-tag.h"
#include "ns3/ipv6-packet-info-tag.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/udp-socket.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CybertwinEdgeServer");

NS_OBJECT_ENSURE_REGISTERED(CybertwinEdgeServer);

TypeId
CybertwinEdgeServer::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinEdgeServer")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<CybertwinEdgeServer>()
                            .AddAttribute("Local",
                                          "The Address on which to Bind the rx socket.",
                                          AddressValue(),
                                          MakeAddressAccessor(&CybertwinEdgeServer::m_local),
                                          MakeAddressChecker())
                            .AddAttribute("Protocol",
                                          "The type id of the protocol to use for the rx socket.",
                                          TypeIdValue(UdpSocketFactory::GetTypeId()),
                                          MakeTypeIdAccessor(&CybertwinEdgeServer::m_tid),
                                          MakeTypeIdChecker());
    return tid;
}

CybertwinEdgeServer::CybertwinEdgeServer()
    : m_socket(nullptr)
{
}

CybertwinEdgeServer::~CybertwinEdgeServer()
{
}

void
CybertwinEdgeServer::StartApplication()
{
    if (!m_socket)
    {
        m_socket = Socket::CreateSocket(GetNode(), m_tid);
        if (m_socket->Bind(m_local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        m_socket->Listen();
        m_socket->ShutdownSend();
    }

    if (InetSocketAddress::IsMatchingType(m_local))
    {
        m_localPort = InetSocketAddress::ConvertFrom(m_local).GetPort();
    }
    else if (Inet6SocketAddress::IsMatchingType(m_local))
    {
        m_localPort = Inet6SocketAddress::ConvertFrom(m_local).GetPort();
    }
    else
    {
        m_localPort = 0;
    }

    m_socket->SetAcceptCallback(
        MakeCallback(&CybertwinEdgeServer::ConnectionRequestCallback, this),
        MakeCallback(&CybertwinEdgeServer::NewConnectionCreatedCallback, this));
    m_socket->SetCloseCallbacks(MakeCallback(&CybertwinEdgeServer::NormalCloseCallback, this),
                                MakeCallback(&CybertwinEdgeServer::ErrorCloseCallback, this));
    m_socket->SetRecvCallback(MakeCallback(&CybertwinEdgeServer::ReceivedDataCallback, this));
}

void
CybertwinEdgeServer::StopApplication()
{
    m_controlTable->DoDispose();
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
                                    MakeNullCallback<void, Ptr<Socket>, const Address&>());
        m_socket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                                    MakeNullCallback<void, Ptr<Socket>>());
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

bool
CybertwinEdgeServer::ConnectionRequestCallback(Ptr<Socket> socket, const Address& address)
{
    return true;
}

void
CybertwinEdgeServer::NewConnectionCreatedCallback(Ptr<Socket> socket, const Address& address)
{
    socket->SetCloseCallbacks(MakeCallback(&CybertwinEdgeServer::NormalCloseCallback, this),
                              MakeCallback(&CybertwinEdgeServer::ErrorCloseCallback, this));
    socket->SetRecvCallback(MakeCallback(&CybertwinEdgeServer::ReceivedDataCallback, this));
    ReceivedDataCallback(socket);
}

void
CybertwinEdgeServer::NormalCloseCallback(Ptr<Socket> socket)
{
}

void
CybertwinEdgeServer::ReceivedDataCallback(Ptr<Socket> socket)
{
    if (!m_controlTable->IsSocketConnected(socket))
    {
        m_controlTable->Connect(socket);
    }
    Ptr<CybertwinItem> cybertwin = m_controlTable->Get(socket);
    cybertwin->PacketReceived(socket);
}

CybertwinControlTable::CybertwinControlTable(){};

bool
CybertwinControlTable::IsSocketConnected(Ptr<Socket> socket)
{
    return m_cybertwinTable.find(socket) != m_cybertwinTable.end();
}

Ptr<CybertwinItem>
CybertwinControlTable::Get(Ptr<Socket> socket)
{
    return m_cybertwinTable.find(socket)->second;
}

void
CybertwinControlTable::Connect(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    CybertwinPacketHeader header;

    packet = socket->Recv();
    packet->PeekHeader(header);

    if (packet->GetSize() == header.GetSerializedSize() && header.GetCmd() == 0)
    {
        uint64_t guid = header.GetSrc();
        Ptr<CybertwinItem> cybertwin;
        if (m_guidTable.find(guid) == m_guidTable.end())
        {
            cybertwin = Create<CybertwinItem>();
            m_guidTable.insert(std::make_pair(guid, socket));
        }
        else
        {
            Ptr<Socket> existedSocket = m_guidTable.find(guid)->second;
            cybertwin = m_cybertwinTable.find(existedSocket)->second;
        }
        cybertwin->AddSocket(socket, guid);
        m_cybertwinTable.insert(std::make_pair(socket, cybertwin));
    }
    else
    {
        NS_FATAL_ERROR("Failed to connect: invalid packet header");
    }
}

void
CybertwinControlTable::Disconnect(Ptr<Socket> socket)
{
    if (IsSocketConnected(socket))
    {
        Ptr<CybertwinItem> cybertwin = Get(socket);
        if (cybertwin->RemoveSocket(socket) == 0)
        {
            uint64_t guidToRemove = cybertwin->GetInitialGuid();
            m_guidTable.erase(guidToRemove);
        }
        m_cybertwinTable.erase(socket);
        socket->ShutdownSend();
        socket->ShutdownRecv();
    }
    else
    {
        NS_FATAL_ERROR("Failed to disconnect: socket not connected");
    }
}

CybertwinItem::CybertwinItem(){};

void
CybertwinItem::AddSocket(Ptr<Socket> socket, uint64_t guid)
{
    if (m_receiveStream.begin() == m_receiveStream.end())
    {
        m_initialGuid = guid;
    }
    m_receiveStream.insert(std::make_pair(socket, StreamState(guid)));
}

uint32_t
CybertwinItem::RemoveSocket(Ptr<Socket> socket)
{
    m_receiveStream.erase(socket);
    return m_receiveStream.size();
}

void
CybertwinItem::PacketReceived(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;

    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0)
        {
            break;
        }
    }
}

uint64_t
CybertwinItem::GetInitialGuid() const
{
    return m_initialGuid;
}

} // namespace ns3