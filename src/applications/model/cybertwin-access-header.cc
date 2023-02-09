#include "cybertwin-access-header.h"

#include "ns3/log.h"

namespace ns3
{
NS_LOG_COMPONENT_DEFINE("CybertwinAccessHeader");
NS_OBJECT_ENSURE_REGISTERED(CybertwinAccessHeader);

CybertwinAccessHeader::CybertwinAccessHeader()
    : m_src(0),
      m_dst(0),
      m_cmd(0)
{
}

TypeId
CybertwinAccessHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::CybertwinAccessHeader")
                            .SetParent<Header>()
                            .SetGroupName("Applications")
                            .AddConstructor<CybertwinAccessHeader>();
    return tid;
}

TypeId
CybertwinAccessHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint64_t
CybertwinAccessHeader::GetSrcGUID() const
{
    return m_src;
}

void
CybertwinAccessHeader::SetSrcGUID(uint64_t src)
{
    m_src = src;
}

uint64_t
CybertwinAccessHeader::GetDstGUID() const
{
    return m_dst;
}

void
CybertwinAccessHeader::SetDstGUID(uint64_t dst)
{
    m_dst = dst;
}

uint16_t
CybertwinAccessHeader::GetCommand() const
{
    return m_cmd;
}

void
CybertwinAccessHeader::SetCommand(uint16_t cmd)
{
    m_cmd = cmd;
}

void
CybertwinAccessHeader::Print(std::ostream& os) const
{
    os << "(src=" << m_src << " dst=" << m_dst << " cmd=" << m_cmd << ")";
}

uint32_t
CybertwinAccessHeader::GetSerializedSize() const
{
    return 8 + 8 + 2;
}

void
CybertwinAccessHeader::Serialize(Buffer::Iterator start) const
{
    Buffer::Iterator i = start;
    i.WriteHtonU64(m_src);
    i.WriteHtonU64(m_dst);
    i.WriteHtonU16(m_cmd);
}

uint32_t
CybertwinAccessHeader::Deserialize(Buffer::Iterator start)
{
    Buffer::Iterator i = start;
    m_src = i.ReadNtohU64();
    m_dst = i.ReadNtohU64();
    m_cmd = i.ReadNtohU16();
    return GetSerializedSize();
}

} // namespace ns3