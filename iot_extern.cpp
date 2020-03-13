
// correct way to include ndn-cxx headers
// #include <ndn-cxx/face.hpp>
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

class Iot : noncopyable
{
public:
  void
  run()
{

while (true) {
   std::string location = "Ille et Vilaine, Rennes";
   std::string pm = "/gw_alice/house/alice/exterior/air_quality/particular_matter2.5/=/Location: ";
   pm.append(location);
   pm.append(" PM2.5 Value: ");
   pm.append(std::to_string(rand() % 75));
   privateSend(Name(pm.c_str()));
   sleep(1);	


   std::string nitro = "/gw_alice/house/alice/exterior/air_quality/nitrogen/=/Location: ";
   nitro.append(location);
   nitro.append(" Nitrogen Value: ");
   nitro.append(std::to_string(rand() % 601));
   privateSend(Name(nitro.c_str()));
   sleep(1);


   std::string oz = "/gw_alice/house/alice/exterior/air_quality/ozone/=/Location: ";
   oz.append(location);
   oz.append(" Ozone Value: ");
   oz.append(std::to_string(rand() % 241));
   privateSend(Name(oz.c_str()));
   sleep(1);  
   

   int x = rand() % 2;
   std::string nut = "";
   if (x==0) 
      nut.append("/gw_alice/house/alice/exterior/gardening/plant_nutrition/=/Level of nutrition is low");
   else if (x==1) 
      nut.append("/gw_alice/house/alice/exterior/gardening/plant_nutrition/=/Level of nutrition is good");
   else
      nut.append("/gw_alice/house/alice/exterior/gardening/plant_nutrition/=/Level of nutrition is too much");

   privateSend(Name(nut.c_str()));
   sleep(1);

   std::string hum = "/gw_alice/house/alice/exterior/gardening/plant_humidity/=/Level of plant's humidity is: ";
   hum.append(std::to_string(rand() % 100));
   hum.append(" %");
   privateSend(Name(hum.c_str()));
   sleep(1);  


   std::string sol = "/gw_alice/house/alice/exterior/energy/solar_panel_power/=/Solar panel's power:  ";
   sol.append(std::to_string(rand() % 25+25));
   sol.append(" Watt");
   privateSend(Name(sol.c_str()));
   sleep(30); 
}
}


void privateSend(Name iname) {
    Interest interest(iname);
    interest.setInterestLifetime(time::milliseconds(200));
    m_face.expressInterest(interest,
                           bind(&Iot::onData, this,  _1, _2),
			   bind(&Iot::onNack, this,  _1, _2),
                           bind(&Iot::onTimeout, this, _1));
    m_face.processEvents();
}

private:
  void
  onData(const Interest& interest, const Data& data)
  {
    //std::cout << data << std::endl;
  }

  void
  onTimeout(const Interest& interest)
  {
    //std::cout << "Timeout " << interest << std::endl;
  }

  void
  onNack(const Interest& interest, const lp::Nack& nack)
  {
    	//std::cout << "Nacked with reason: " << nack.getReason() << std::endl;
  }
private:
  Face m_face;
};

} // namespace examples
} // namespace ndn

int
main(int argc, char** argv)
{
  ndn::examples::Iot iot;
  try {
    iot.run();
  }
  catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
  }
  return 0;
}
