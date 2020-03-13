#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/validator-regex.hpp>
#include <ndn-cxx/util/io.hpp>
#include <boost/assert.hpp>
#include <stdlib.h>
#include <unistd.h>
#include <boost/asio.hpp>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

class IotIntern : noncopyable
{
public:
  void
  run()
{
while(true) {
   int p = rand() % 2;
   if (p==0)
   	privateSend(Name("/gw_alice/house/alice/kitchen/security/presence_detector/=/False"));
   else 
   	privateSend(Name("/gw_alice/house/alice/kitchen/security/presence_detector/=/True"));	
   sleep(1);	
   
   p = rand() % 2;
   if (p==0)
   	privateSend(Name("/gw_alice/house/alice/kitchen/security/smoke_detector/=/False"));
   else 
   	privateSend(Name("/gw_alice/house/alice/kitchen/security/smoke_detector/=/True"));	
   	
   sleep(30);
}
}


void privateSend(Name iname) {
    Interest interest(iname);
    interest.setInterestLifetime(time::milliseconds(200));
    m_face.expressInterest(interest,
                           bind(&IotIntern::onData, this,  _1, _2),
			   bind(&IotIntern::onNack, this,  _1, _2),
                           bind(&IotIntern::onTimeout, this, _1));
    m_face.processEvents();
}

private:
  void
  onData(const Interest& interest, const Data& data)
  {
    std::cout << data << std::endl;
  }

  void
  onTimeout(const Interest& interest)
  {
    std::cout << "Timeout " << interest << std::endl;
  }

  void
  onNack(const Interest& interest, const lp::Nack& nack)
  {
    	std::cout << "Nacked with reason: " << nack.getReason() << std::endl;
  }
private:
  Face m_face;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  ndn::examples::IotIntern iot;
  try {
    iot.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
