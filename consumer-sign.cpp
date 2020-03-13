#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-regex.hpp>
#include <ndn-cxx/util/io.hpp>
#include <boost/assert.hpp>

#include <boost/asio.hpp>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

class Consumer : noncopyable
{
public:
  void
  initilize()
  {
    m_face = make_shared<Face>();
    m_validator = make_shared<ValidatorRegex>(*m_face);

    m_validator->addDataVerificationRule(ndn::make_shared<ndn::SecRuleRelative>("^(<>*)$",
                                                                               "^([^<KEY>]*)<KEY>(<>*)<ksk-.*><ID-CERT>$",
                                                                               ">", "\\1", "\\1\\2", true));

    ndn::shared_ptr<ndn::IdentityCertificate> anchor = ndn::io::load<ndn::IdentityCertificate>("/usr/local/etc/ndn/house.ndncert");

    if (static_cast<bool>(anchor))
      {
        BOOST_ASSERT(anchor->getName().size() >= 1);

        m_validator->addTrustAnchor(anchor);
      }
    else {
      throw "invalid certificate";
    }
  }

  void
  run()
  {
    Interest interest(Name("/house/alice"));
    interest.setInterestLifetime(time::milliseconds(4000));
    interest.setMustBeFresh(true);

    m_face->expressInterest(interest,
                            bind(&Consumer::onData, this,  _1, _2),
			    bind(&Consumer::onNack, this,  _1, _2),	
                            bind(&Consumer::onTimeout, this, _1));

    std::cout << "Sending " << interest << std::endl;

    // processEvents will block until the requested data received or timeout occurs
    m_face->processEvents();
  }

private:
  void
  onDataValidationFailed(const shared_ptr<const Data>& data,
                         const std::string& failureInfo)
  {
    std::cout << "failed" << failureInfo << std::endl;
  }

  void
  displayData(const ndn::shared_ptr<const ndn::Data>& data)
  {
    std::string message(reinterpret_cast<const char*>(data->getContent().value()),
                        data->getContent().value_size());
    std::cout << "msg: " << message << std::endl;
    m_face->getIoService().stop();
  }

  void
  onData(const Interest& interest, const Data& data)
  {
    m_validator->validate(data, bind(&Consumer::displayData, this, _1),
                          bind(&Consumer::onDataValidationFailed, this, _1, _2));
  }

  void
  onTimeout(const Interest& interest)
  {
    std::cout << interest << " timeout " << std::endl;
  }

void
  onNack(const Interest& interest, const lp::Nack& nack)
  {
    	std::cout << "Nacked with reason: " << nack.getReason() << std::endl;
  }

private:
  shared_ptr<Face> m_face;
  shared_ptr<ValidatorRegex> m_validator;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  ndn::examples::Consumer consumer;
  try {
    consumer.initilize();
    consumer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
