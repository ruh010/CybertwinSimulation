#include "cybertwin-edge-server.h"

#include "cybertwin-access-header.h"

#include "ns3/address.h"
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
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
}

CybertwinEdgeServer::~CybertwinEdgeServer()
{
    NS_LOG_FUNCTION(this);
}

void
CybertwinEdgeServer::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_socket = nullptr;
    m_socketList.clear();

    Application::DoDispose();
}

void
CybertwinEdgeServer::StartApplication()
{
    NS_LOG_FUNCTION(this);
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

    m_socket->SetRecvCallback(MakeCallback(&CybertwinEdgeServer::HandleRead, this));
    m_socket->SetRecvPktInfo(true);
    m_socket->SetAcceptCallback(MakeNullCallback<bool, Ptr<Socket>, const Address&>(),
                                MakeCallback(&CybertwinEdgeServer::HandleAccept, this));
    m_socket->SetCloseCallbacks(MakeCallback(&CybertwinEdgeServer::HandlePeerClose, this),
                                MakeCallback(&CybertwinEdgeServer::HandlePeerError, this));
}

void
CybertwinEdgeServer::StopApplication()
{
    while (!m_socketList.empty())
    {
        Ptr<Socket> acceptedSocket = m_socketList.front();
        m_socketList.pop_front();
        acceptedSocket->Close();
    }
    if (m_socket)
    {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
CybertwinEdgeServer::HandleRead(Ptr<Socket> socket)
{
    Ptr<Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from)))
    {
        if (packet->GetSize() == 0)
        {
            break;
        }
        if (InetSocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " edge server received "
                                   << packet->GetSize() << " bytes from "
                                   << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                                   << InetSocketAddress::ConvertFrom(from).GetPort());
        }
        else if (Inet6SocketAddress::IsMatchingType(from))
        {
            NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " edge server received "
                                   << packet->GetSize() << " bytes from "
                                   << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
                                   << Inet6SocketAddress::ConvertFrom(from).GetPort());
        }

        CybertwinAccessHeader header;
        packet->PeekHeader(header);

        NS_LOG_ERROR("Received Source GUID:" << header.GetSrcGUID()
                                             << " Destination GUID:" << header.GetDstGUID()
                                             << " Command:" << header.GetCommand());

        NS_LOG_ERROR(packet->ToString());

        // Ipv4PacketInfoTag interfaceInfo;
        // Ipv6PacketInfoTag interface6Info;
        // if (packet->RemovePacketTag(interfaceInfo))
        // {
        //     localAddress = InetSocketAddress(interfaceInfo.GetAddress(), m_localPort);
        // }
        // else if (packet->RemovePacketTag(interface6Info))
        // {
        //     localAddress = Inet6SocketAddress(interface6Info.GetAddress(), m_localPort);
        // }
        // else
        // {
        //     socket->GetSockName(localAddress);
        // }

        // PacketReceived(packet, from, localAddress);
    }
}

void
CybertwinEdgeServer::PacketReceived(const Ptr<Packet>& p,
                                    const Address& from,
                                    const Address& localAddress)
{
    CybertwinAccessHeader header;
    uint32_t headerSize = p->PeekHeader(header);

    NS_LOG_ERROR("Received " << headerSize << "bytes of header, Source GUID:" << header.GetSrcGUID()
                             << " Destination GUID:" << header.GetDstGUID()
                             << " Command:" << header.GetCommand());
}

void
CybertwinEdgeServer::HandlePeerClose(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
}

void
CybertwinEdgeServer::HandlePeerError(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
}

void
CybertwinEdgeServer::HandleAccept(Ptr<Socket> s, const Address& from)
{
    NS_LOG_FUNCTION(this << s << from);
    s->SetRecvCallback(MakeCallback(&CybertwinEdgeServer::HandleRead, this));
    m_socketList.push_back(s);
}

} // namespace ns3