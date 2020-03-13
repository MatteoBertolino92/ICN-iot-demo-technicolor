#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/util/io.hpp>
#include <boost/assert.hpp>
#include <boost/asio.hpp> 
#include <fstream>
#include <iostream>
#include "lp/nack-header.hpp"
#include "face.hpp"

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

class Producer : noncopyable
{
public:

  void
  initilize()
  {
    m_face = make_shared<Face>();
    m_validator = make_shared<ValidatorConfig>(*m_face);
    m_validator->load("/usr/local/etc/ndn/validator-config-interest.conf");
  }

  void
  run()
  {
    log.open("producerLog.txt", std::ios::out);
   m_face->setInterestFilter("/house/alice",
                             bind(&Producer::onInterest, this, _1, _2),
                             RegisterPrefixSuccessCallback(),
                             bind(&Producer::onRegisterFailed, this, _1, _2));
    m_face->processEvents();
  }

private:
  void
  onInterest(const InterestFilter& filter, const Interest& interest)
  {
    log << "<< I: " << interest << std::endl;
    log << "Need to verify it"<< std::endl; 	
    //VERIFY THE INTEREST
    //m_validator->validate(interest, bind(&Producer::sendData, this, _1),
                          //bind(&Producer::onInterestValidationFailed, this, _1, _2));
    log << "Interest (verified)"<< std::endl;


    // Create new name, based on Interest's name
    Name dataName(interest.getName());
    dataName
      .append("testApp") // add "testApp" component to Interest name
      .appendVersion();  // add "version" component (current UNIX timestamp in milliseconds)

    static const std::string content = "HELLO";

    // Create Data packet
    shared_ptr<Data> data = make_shared<Data>();
    data->setName(dataName);
    data->setFreshnessPeriod(time::seconds(10));
    data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());

    Name producerId("/house/alice"); ///test/site2/user2
    m_keyChain.signByIdentity(*data, producerId);
    m_face->put(*data);
  }


private:
  void
  onInterestValidationFailed(const shared_ptr<const Interest>& interest,
                         const std::string& failureInfo)
  {
    log << "failed " << failureInfo << std::endl;
    lp::Nack outNack(*interest);
    lp::NackReason nack_reason = lp::NackReason::BADREQUEST;
    outNack.setReason(nack_reason);
    m_face->put(outNack);
  }

  void
  sendData(const ndn::shared_ptr<const ndn::Interest>& interest)
  {
    log << "Interest verified"<< std::endl;


    // Create new name, based on Interest's name
    Name dataName(interest->getName());
    dataName
      .append("testApp") // add "testApp" component to Interest name
      .appendVersion();  // add "version" component (current UNIX timestamp in milliseconds)

    static const std::string content = "HELLO";

    // Create Data packet
    shared_ptr<Data> data = make_shared<Data>();
    data->setName(dataName);
    data->setFreshnessPeriod(time::seconds(10));
    data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());

    Name producerId("/house/alice"); ///test/site2/user2
    m_keyChain.signByIdentity(*data, producerId);
    m_face->put(*data);
  }

  void
  onRegisterFailed(const Name& prefix, const std::string& reason)
  {
    std::cerr << "ERROR: Failed to register prefix \""
              << prefix << "\" in local hub's daemon (" << reason << ")"
              << std::endl;
    m_face->shutdown();
  }

private:
  shared_ptr<Face> m_face;
  shared_ptr<ValidatorConfig> m_validator;
  KeyChain m_keyChain;
  std::ofstream log;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  ndn::examples::Producer producer;
  try {
    producer.initilize();
    producer.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
