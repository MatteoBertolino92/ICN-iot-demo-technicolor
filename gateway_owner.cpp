#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/util/io.hpp>
#include <boost/assert.hpp>
#include <boost/asio.hpp> 
#include <fstream>
#include <thread> 
#include <map>
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include "lp/nack-header.hpp"
#include "face.hpp"
#include <mutex>

// Enclosing code in ndn simplifies coding (can also use `using namespace ndn`)
namespace ndn {
// Additional nested namespace could be used to prevent/limit name contentions
namespace examples {

std::map<std::string, std::string> matching;
std::mutex msec;
/////////////////////////////////////////////////////////////////////////////////////////////   

class GWtoClients : noncopyable
{
public:

  void
  initilize()
  {
    m_face = make_shared<Face>();
    //m_validator = make_shared<ValidatorConfig>(*m_face);
    //m_validator->load("/usr/local/etc/ndn/validator-config-interest.conf");
  }

  void
  run()
  {
    log.open("GWLogtoUsers.txt", std::ios::out);
    m_face->setInterestFilter("/house/alice",
                             bind(&GWtoClients::onInterest, this, _1, _2),
                             RegisterPrefixSuccessCallback(),
                             bind(&GWtoClients::onRegisterFailed, this, _1, _2));
    m_face->processEvents();
  }

private:
  void
  onInterest(const InterestFilter& filter, const Interest& interest)
  {
    log << "<< I: " << interest << std::endl; 	
    //m_validator->validate(interest, bind(&GWtoClients::sendData, this, _1),
                         // bind(&GWtoClients::onInterestValidationFailed, this, _1, _2));
    log << "Interest verified"<< std::endl;
                    // /house/alice/room/type/sensor/ 
    int h = interest.getName().toUri().find("/", 1); //  /house/
    h = interest.getName().toUri().find("/",   h+1); // alice/
    h = interest.getName().toUri().find("/", h+1); 
    h = interest.getName().toUri().find("/", h+1); // type
    h = interest.getName().toUri().find("/", h+1); // sensor/
    //Extract the resource
    std::string resource = "/gw_alice";
    resource.append(interest.getName().toUri().substr(0, h));     
    log << resource << std::endl;
    //Take value in map 

    msec.lock() ; 
    std::string content = matching[resource];
    msec.unlock() ; 
    log << "CONTENT: " << content << std::endl;    

    //Prepare Data Packet
    Name dataName(interest.getName());
    shared_ptr<Data> data = make_shared<Data>();
    data->setName(dataName);
    data->setFreshnessPeriod(time::seconds(1));
    data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());
    Name producerId("/house/alice");
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

  shared_ptr<Face> m_face;
  shared_ptr<ValidatorConfig> m_validator;
  KeyChain m_keyChain;
  std::ofstream log;
};

/////////////////////////////////////////////////////////////////////////////////////////////    

/////////////////////////////////////////////////////////////////////////////////////////////

class GWtoIot : noncopyable
{
public:

  void
  initilize()
  {
    m_faceI = make_shared<Face>();
    std::thread t(&GWtoIot::run, this);
    t.detach();
  }

  void
  run()
  {
    //m_faceI = make_shared<Face>();
    ilog.open("GWLogtoIot.txt", std::ios::out);
    m_faceI->setInterestFilter("/gw_alice",
                             bind(&GWtoIot::onInterest, this, _1, _2),
                             RegisterPrefixSuccessCallback(),
                             bind(&GWtoIot::onRegisterFailed, this, _1, _2));
    m_faceI->processEvents();
  }

private:
  void
  onInterest(const InterestFilter& filter, const Interest& interest)
  {
    ilog << "<< NI: " << interest << std::endl;
    int x = interest.getName().toUri().find("%3D", 0);
    ilog << "<< = position: " << x << std::endl;  	

 
    // Take resource
    std::string resourceI = interest.getName().toUri().substr(0, x-1);
    ilog << "Resource: " << resourceI << std::endl;


    //Take content
    std::string contentI = interest.getName().toUri().substr(x+4, interest.getName().toUri().size());
    ilog << "Content: " << contentI << std::endl;

    //Publish
    msec.lock() ; 
    matching[resourceI] = contentI;
    msec.unlock() ; 

    ilog << "Put : " << matching[resourceI] << std::endl;
  }

  void
  onRegisterFailed(const Name& prefix, const std::string& reason)
  {
    std::cerr << "ERROR: Failed to register prefix \""
              << prefix << "\" in local hub's daemon (" << reason << ")"
              << std::endl;
    m_faceI->shutdown();
  }

  shared_ptr<Face> m_faceI;
  std::ofstream ilog;
};


} // namespace examples
} // namespace ndn
/////////////////////////////////////////////////////////////////////////////////////////////



int
main(int argc, char** argv)
{
  ndn::examples::GWtoClients gwUsers;
  ndn::examples::GWtoIot gwIot;
  try {        
    gwUsers.initilize();
    gwIot.initilize();
    gwUsers.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
