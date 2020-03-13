#include "ac.hpp"

namespace nfd {
namespace fw {

std::fstream log("/home/technicolor/Desktop/final_with_iot/NS/GW_Alice/logfile.txt", std::ios::app);

int access_control (Name interestName) { 
    //if (interestName.toUri().substr(0, 11).compare("/root/site1")==0 && interestName.toUri().substr(11, 7).compare("/public")!=0 /* && not * in the iot's fields */) {
    if (interestName.toUri().substr(0, 12).compare("/house/alice")==0 && interestName.toUri().substr(12, 7).compare("/public")!=0 /* && not * in the iot's fields */) {
	//1- Retrieve Identity, Resource, etc from Signed Interest
    char_separator<char> sep("/");
    tokenizer< char_separator<char> > tokens(interestName.toUri(), sep);	
    PosData x;
    SqlData request;
    std::string signatureTMP="";

    x.house = 0;
    x.room = interestName.toUri().find("/", x.house+1);
    x.room = interestName.toUri().find("/", x.room+1);
    x.type = interestName.toUri().find("/", x.room+1);
    x.sensor = interestName.toUri().find("/", x.type+1);
    
    std::size_t tmp3 = interestName.toUri().find("/", x.sensor+1);
    std::size_t tmp2 = interestName.toUri().find("/", tmp3+1);
    tmp3+=1;
    std::size_t tmp1 = interestName.toUri().find("%24", tmp3, tmp2-tmp3);
    log << std::string::npos << " " << tmp1 <<std::endl;

    if (tmp1 != std::string::npos) {                     //If dollar present, there is surely a condition. skip it, then take the signature
    	log << "$ found at: " << tmp1 << '\n';
	std::size_t condition_start = interestName.toUri().find("/", tmp1+1);
	x.sensor_end = tmp1-1;
	std::size_t condition_end = interestName.toUri().find("/", condition_start+1);
	x.sig_start = interestName.toUri().find("/", condition_end+1);
	x.sig_start = interestName.toUri().find("/", x.sig_start+1);
	x.sig_end = interestName.toUri().find("/", x.sig_start+1);
    } 
    else { //VERIFICA
	x.sig_start = interestName.toUri().find("/", x.sensor+1);
	x.sensor_end = x.sig_start;
	log << "Sensor end: " << x.sensor_end << std::endl;
	x.sig_start = interestName.toUri().find("/", x.sig_start+1);
	x.sig_start = interestName.toUri().find("/", x.sig_start+1);
	x.sig_end = interestName.toUri().find("/", x.sig_start+1);
	log << "Sig s: " << x.sig_start << std::endl;
	log << "Sig e: " << x.sig_end << std::endl;
    }

    request.house =  interestName.toUri().substr(x.house, x.room-x.house);
    log << "House: " << request.house << std::endl;
    request.room =  interestName.toUri().substr(x.room+1, x.type-x.room-1);
    log << "Room: " << request.room << std::endl;
    request.type =  interestName.toUri().substr(x.type+1, x.sensor-x.type-1);
    log << "Type: " << request.type << std::endl;	
    request.sensor =  interestName.toUri().substr(x.sensor+1, x.sensor_end-x.sensor-1);
    log << "Sensor: " << request.sensor << std::endl;

    request.id = extract_signature(interestName.toUri().substr(x.sig_start, x.sig_end - x.sig_start)); 
    log << "Signature:" << request.id << "test" << std::endl;	
     
    if (request.id.compare("") == 0)
	return 2; //BADREQUEST
 
    // Retrieve Access Information
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    std::string sql;
    std::list<SqlData> lista ={};

    //std::string dbstring = "/home/bertolino/Desktop/USECASE/site1.db";
    std::string dbstring = "";
    //dbstring.append("/home/technicolor/Desktop/final/AccessControlStuff/USECASE");
    dbstring.append(getenv("HOME"));
    dbstring.append(request.house.c_str());
    dbstring.append(".db");
    log << "DB: " << getenv("HOME") << std::endl;
    rc = sqlite3_open(dbstring.c_str(), &db);
 
    if(rc) {
       fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
       return(0);
    }
    else {
       log << "Opened database successfully "<< dbstring << std::endl;
   }


   sql = "";
   sql.append("Select id, room, type, sensor, allow from accesscontrol where (id='");
   sql.append(request.id.c_str());
   sql.append("' or id is NULL) and (room='");
   sql.append(request.room.c_str());
   sql.append("' or room is NULL) and (type='");
   sql.append(request.type.c_str());
   sql.append("' or type is NULL) and (sensor='");
   sql.append(request.sensor.c_str());
   sql.append("' or sensor is NULL)");
   log << "Req: " << sql << std::endl;
   const char *sql2=sql.c_str();


   rc = sqlite3_exec(db, sql2, callback, (void*)&lista, &zErrMsg);

   if( rc != SQLITE_OK ) {
      log << "SQL error: " << zErrMsg << std::endl;
      sqlite3_free(zErrMsg);
   }
   else {
       log << "Operation done successfully" << std::endl;
   }
   sqlite3_close(db);

   // Return a decision
    bool finalDec = take_decision(lista);
    if (finalDec == false)
    	return 0;	
    }
    return 1;
} //function

string extract_signature(string tmp) { //  /root/site2/ksk-1483008950879 from /%165%1B%01%01%1C0%07.%08%04root%08%03KEY%08%05site2%08%11ksk-1483008950879%08%07ID-CERT
     string sigTMP="";	
     bool flag=false;

     for (int i=0; i< (int)tmp.size(); i++) {
	if (tmp.substr(i, 3).compare("%08")==0) {
	    if (tmp.substr(i+6, 7).compare("ID-CERT")==0) {
	    	break;  
	    }
	    if (sigTMP[sigTMP.size()-1] != '/')
	    sigTMP+='/';
	    i+=5;
	    if (flag==false)
		 flag=true;		
	    continue;	
	}
	if (flag==true) {
	    if (tmp.substr(i, 3).compare("KEY")==0) {
	    	i+=9;  
	    }
	    sigTMP+=tmp[i];
	}
     }
     return sigTMP;	
  }



   bool take_decision(std::list<SqlData> lista) {  
   bool decision=false;
   bool unsolvable_conflict=false;
   int best = 0;
  
   if (lista.size() == 0) {
	log << "No matches" << endl;	
	return decision;
   }

   else
     for (std::list<SqlData>::iterator it=lista.begin(); it != lista.end(); it++) {
        int tmp=1;
	SqlData x = *it;
        log << "Entry: " << x.id << " " << x.room << " " << x.type << " " << x.sensor << endl;

	if (x.id != "") 
	   tmp*=100;

	if (x.room != "") 
	   tmp*=3;

	if (x.type != "") 
 	   tmp*=3;
		 
        if (x.sensor != "") 
	   tmp*=10;

           log << "New tmp: " << tmp << endl;	

	if (tmp == best) {
	   if (decision != x.allow) {
		unsolvable_conflict = true; 
            	log << "tmp unsolvable conflict" << std::endl; 
           }	
	}
	if (tmp > best) {
	   best = tmp;
	   decision = x.allow;
	   unsolvable_conflict = false;	
           log << "New best: " << best << endl;
	}	
     }

     if (unsolvable_conflict == true) {
	log << "Unsolvable conflict! Returning FALSE" << endl;
	return false; 
     }
     log << "AC Decision: " << decision << " with points: " << best <<  endl;
     return decision;
  }	

int callback(void *lista, int argc, char **argv, char **azColName){
   SqlData x;
   if (argv[0]!=NULL)
     x.id = (string) argv[0];
   if (argv[1]!=NULL)
     x.room = (string) argv[1];
   if (argv[2]!=NULL)
     x.type = (string) argv[2];
   if (argv[3]!=NULL)
     x.sensor = argv[3];
   if (argv[4]!=NULL)
     x.allow = boost::lexical_cast<bool>(argv[4]);

   //std::cout << x.room << " " << x.type << " " << x.sensor << " " << x.allow << std::endl;
   std::list<SqlData> *tmp =  (std::list<SqlData>*)lista; 
   std::list<SqlData>::iterator it;
   it = tmp->begin();
   tmp->insert(it, x);
   return 0;
}




 }
} 
