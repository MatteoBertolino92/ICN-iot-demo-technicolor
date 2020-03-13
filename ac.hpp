#include <iostream>
#include <fstream>
#include <ndn-cxx/face.hpp>
#include <ndn-cxx/security/key-chain.hpp>
#include <ndn-cxx/security/validator.hpp>
#include <ndn-cxx/encoding/block.hpp>
#include <ndn-cxx/security/validator-config.hpp>
#include <ndn-cxx/util/io.hpp>
#include <boost/assert.hpp>
#include <boost/asio.hpp> 
#include <map>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h> 
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <sqlite3.h> 


using namespace std;
using namespace boost; 

typedef struct a {
   string house;
   string id;
   string room;
   string type;
   string sensor;
   bool allow;
}SqlData;

typedef struct b {
   std::size_t house;
   std::size_t room;
   std::size_t type;
   std::size_t sensor;
   std::size_t sensor_end;
   std::size_t sig_start;
   std::size_t sig_end; 
}PosData;

namespace nfd {
namespace fw {
	int access_control (Name interestName);
	string extract_signature(string tmp);
        bool take_decision(std::list<SqlData> lista);
 	int callback(void *lista, int argc, char **argv, char **azColName);
 }
} 
