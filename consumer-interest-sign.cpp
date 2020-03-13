#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/util/io.hpp>
#include <boost/assert.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <boost/asio.hpp>
#include <ctime>
#include <string>
#include <string.h>


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
    m_validator = make_shared<ValidatorConfig>(*m_face);
    m_validator->load("/usr/local/etc/ndn/validator-config-base.conf");
  }

  void
  run(int argc, char** argv)
  {
    std::string resource="";
    if (argc < 2) {
	std::cout << "No arguments, resource requested default: /root/site1/kitchen/temperature/s_x" << std::endl;
	//resource.append("/root/site1/kitchen/temperature/s_x");
    }
    else {
	//std::cout << "Resource requested 1: " << argv[1] << std::endl;
	resource.append(argv[1]);
    }
    
    if (resource == "") {
  	std::cout << "error" << std::endl;
	exit (-2);
    }
    std::cout << "Resource requested: " << resource << std::endl;
    Interest interest(Name(resource.c_str()));
    interest.setInterestLifetime(time::milliseconds(4000));
    interest.setMustBeFresh(true);
    Name consumerId("/house/charlie");
    m_keyChain.signByIdentity(interest, consumerId);
    m_face->expressInterest(interest,
                            bind(&Consumer::onData, this,  _1, _2),
			    bind(&Consumer::onNack, this,  _1, _2),	
                            bind(&Consumer::onTimeout, this, _1));

    std::cout << "Sending " << interest << std::endl;

    // processEvents will block until the requested data received or timeout occurs
    m_face->processEvents();

    //duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
    //std::cout<<"printf: "<< duration <<'\n';
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
   // m_validator->validate(data, bind(&Consumer::displayData, this, _1),
                        //  bind(&Consumer::onDataValidationFailed, this, _1, _2));
       std::string message(reinterpret_cast<const char*>(data.getContent().value()),
                        data.getContent().value_size());
    std::cout << "msg: " << message << std::endl;
    //m_face->getIoService().stop();
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
  shared_ptr<ValidatorConfig> m_validator;
  KeyChain m_keyChain;
    std::clock_t start;
    double duration;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  ndn::examples::Consumer consumer;
  try {
    consumer.initilize();
    consumer.run(argc, argv);
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
