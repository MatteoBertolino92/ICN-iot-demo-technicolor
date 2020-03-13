#include "nack-header.hpp"

namespace ndn {
namespace lp {

std::ostream&
operator<<(std::ostream& os, NackReason reason)
{
  switch (reason) {
  case NackReason::CONGESTION:
    os << "Congestion";
    break;
  case NackReason::DUPLICATE:
    os << "Duplicate";
    break;
  case NackReason::NO_ROUTE:
    os << "NoRoute";
    break;
  case NackReason::NO_CONTENT:
    os << "NoContent";
    break;
  case NackReason::DENY:
    os << "PermissionDenied";
    break;
  case NackReason::BADREQUEST:
    os << "BadRequest";
    break;
  default:
    os << "None";
    break;
  }
  return os;
}

NackHeader::NackHeader()
  : m_reason(NackReason::NONE)
{
}

NackHeader::NackHeader(const Block& block)
{
  wireDecode(block);
}

template<encoding::Tag TAG>
size_t
NackHeader::wireEncode(EncodingImpl<TAG>& encoder) const
{
  size_t length = 0;
  length += prependNonNegativeIntegerBlock(encoder, tlv::NackReason,
                                           static_cast<uint32_t>(m_reason));
  length += encoder.prependVarNumber(length);
  length += encoder.prependVarNumber(tlv::Nack);
  return length;
}

template size_t
NackHeader::wireEncode<encoding::EncoderTag>(EncodingImpl<encoding::EncoderTag>& encoder) const;

template size_t
NackHeader::wireEncode<encoding::EstimatorTag>(EncodingImpl<encoding::EstimatorTag>& encoder) const;

const Block&
NackHeader::wireEncode() const
{
  if (m_wire.hasWire()) {
    return m_wire;
  }

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void
NackHeader::wireDecode(const Block& wire)
{
  if (wire.type() != tlv::Nack) {
    BOOST_THROW_EXCEPTION(ndn::tlv::Error("expecting Nack block"));
  }

  m_wire = wire;
  m_wire.parse();
  m_reason = NackReason::NONE;

  if (m_wire.elements_size() > 0) {
    Block::element_const_iterator it = m_wire.elements_begin();

    if (it->type() == tlv::NackReason) {
      m_reason = static_cast<NackReason>(readNonNegativeInteger(*it));
    }
    else {
      BOOST_THROW_EXCEPTION(ndn::tlv::Error("expecting NackReason block"));
    }
  }
}

NackReason
NackHeader::getReason() const
{
  switch (m_reason) {
  case NackReason::CONGESTION:
  case NackReason::DUPLICATE:
  case NackReason::NO_ROUTE:
  case NackReason::NO_CONTENT:
  case NackReason::DENY:
  case NackReason::BADREQUEST:
    return m_reason;
  default:
    return NackReason::NONE;
  }
}

NackHeader&
NackHeader::setReason(NackReason reason)
{
  m_reason = reason;
  m_wire.reset();
  return *this;
}

} // namespace lp
} // namespace ndn
