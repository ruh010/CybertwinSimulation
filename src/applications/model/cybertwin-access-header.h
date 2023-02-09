#ifndef CT_ACCESS_HEADER_H
#define CT_ACCESS_HEADER_H

#include "ns3/header.h"

namespace ns3
{

class CybertwinAccessHeader : public Header
{
  public:
    CybertwinAccessHeader();

    void SetSrcGUID(uint64_t src);
    uint64_t GetSrcGUID() const;

    void SetDstGUID(uint64_t dst);
    uint64_t GetDstGUID() const;

    void SetCommand(uint16_t cmd);
    uint16_t GetCommand() const;

    static TypeId GetTypeId();

    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;

  private:
    uint64_t m_src;
    uint64_t m_dst;
    uint16_t m_cmd;
};

} // namespace ns3

#endif