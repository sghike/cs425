#include "Node.h"
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <iomanip>
#include <string.h>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "listener.hpp"
#include <netdb.h>
#include <arpa/inet.h>
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#ifdef WIN32
#include <io.h>
#endif
#include "sha1.h"
#include <fcntl.h>

using namespace std;
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;
using namespace ::mp2;
using boost::shared_ptr;

int introducer_port;
int atn_port;

int main(int argc, char* argv[])
{
    string input;
    string command;
    int cmp = -1;
    int cmd = 0;
    int m = 0;
    int sp = 0;
    int atn = 0;
    int si = 0;
    int fi = 0;
    int lc = 0;
    int i;
    int m_pos = 0;
    int sp_pos = 0;
    int atn_pos = 0;
    int si_pos = 0;
    int fi_pos = 0;
    int lc_pos = 0;
    vector<int> ports; 
    string portnum;
    int number_of_ports = 0;
    int j = 0;
    stringstream ss;
    string argument;
    string check_int;
    string m_val;
    string si_val;
    string fi_val;
    string atn_val;  
    
    // initialize global variables
    introducer_port = 0;
    atn_port = 0;

    // parse out arguments and check if they are valid
    for(i = 1; i < argc; i++)
    {
        argument.assign(argv[i]);
        
        if(argument.compare("--m") == 0)
        {
            m++;
            m_pos = i;
        }
        else if(argument.compare("--startingPort")==0)
        {
            sp++;
            sp_pos = i;
        }
        else if(argument.compare("--attachToNode")==0)
        {
            atn++;
            atn_pos = i;
        }
        else if(argument.compare("--stabilizeInterval")==0)
        {
            si++;
            si_pos = i;
        }
        else if(argument.compare("--fixInterval")==0)
        {
            fi++;
            fi_pos = i;
        }
        else if(argument.compare("--logConf")==0)
        {
            lc++;
            lc_pos = i;
        }
	else if(argument.find("-") != -1)
        {
             cout << "Wrong usage of program\n";
             return -1;
        }
        argument.clear();
    }
    
    if( m > 1 || sp > 1 || atn > 1 || si > 1 || fi > 1 || lc > 1)
    {   
        cout << "Wrong usage of program\n";
        return -1;
    }   
    
    if(m == 1)
    {
        // check if the number is actually an integer        
        check_int.assign(argv[m_pos+1]);
        for(j = 0; j < check_int.size(); j++)
        {
            if((check_int[j] >= '0' && check_int[j] <= '9') == false)
            {
                cout << "specify valid integer for number of bits" << endl;
                return -1;
            }
        }

        if(atoi(argv[m_pos+1]) > 10 || atoi(argv[m_pos+1]) < 5)
        { 
            cout <<  "specify valid integer for number of bits (5 <= m <= 10)" << endl;
            return -1;
        }
        m_val.assign(argv[m_pos+1]);
        check_int.clear();
        cout << "the number of bits of the keys/nodeIDs : " << atoi(argv[m_pos+1]) << endl;
        
    }
    if(si == 1)
    {
        cout << "stabilize interval to " << atoi(argv[si_pos+1]) << endl;
        check_int.assign(argv[si_pos+1]);
        for(j = 0; j < check_int.size(); j++)
        {
            if((check_int[j] >= '0' && check_int[j] <= '9') == false)
            {
                cout << "specify valid integer for number of bits" << endl;
                return -1;
            }
        }
        si_val.assign(argv[si_pos+1]);
        check_int.clear();
    }

    if(atn == 1)
    {
        cout << "attaching to node " << atoi(argv[atn_pos+1]) << endl;    
        check_int.assign(argv[atn_pos+1]);
        for(j = 0; j < check_int.size(); j++)
        {
            if((check_int[j] >= '0' && check_int[j] <= '9') == false)
            {
                cout << "specify valid integer for number of bits" << endl;
                return -1;
            }
        }
        atn_val.assign(argv[atn_pos+1]);
        check_int.clear();
        atn_port = atoi(atn_val.c_str());
    }

    if(sp == 1)
    {
        i = 1;
        // check if there is a port specified
        if(sp_pos+i < argc)
                portnum.assign(argv[sp_pos+i]);
        else
        {   
            cout << "specify at least one port" << endl;
            return -1;
        }
        
        // get all the ports specified
        while(portnum.find("--") == -1)
        {
            // check if the port is actually an integer
            check_int.assign(argv[sp_pos+i]);
            for(j = 0; j < check_int.size(); j++)
            {
                if((check_int[j] >= '0' && check_int[j] <= '9') == false)
                {
                    cout << "specify valid integer for port" << endl;
                    return -1;
                }
                    
            }
            
            number_of_ports++;
            ports.push_back(atoi(argv[sp_pos+i]));
            i++;
            portnum.clear();
            check_int.clear();
            if(sp_pos+i < argc)
                portnum.assign(argv[sp_pos+i]);
            else
                break;
        }
        
        if (number_of_ports == 0)
        {   
            cout << "specify at least one port" << endl;
            return -1;
        }
               
        cout << "checking " << number_of_ports << " ports : " ;
        for(i = 0; i < number_of_ports; i++)
        {
            cout << atoi(argv[sp_pos+1+i]) << " ";
        }
        cout << endl;
    }
    if(fi == 1)
    {
        cout << "fix interval to : " << atoi(argv[fi_pos+1]) << endl;
        check_int.assign(argv[fi_pos+1]);
        for(j = 0; j < check_int.size(); j++)
        {
            if((check_int[j] >= '0' && check_int[j] <= '9') == false)
            {
                cout << "specify valid integer for number of bits" << endl;
                return -1;
            }
        }
        fi_val.assign(argv[fi_pos+1]);
        check_int.clear();
    }

    if(lc == 1)
        cout << "logging is enabled" << endl;    
    
    // take inputs from the terminal
    while(1)
    {
        cout << "INPUT : ";
        getline(cin, input);
         
        // obtain first word
        command = input;
                
        // decide command 
        if(command.compare(0, 8, "ADD_NODE") == 0)
            cmp = 0;
        else if(command.compare(0, 8, "ADD_FILE") == 0)
            cmp = 1;
        else if(command.compare(0, 8, "DEL_FILE") == 0)
            cmp = 2;
        else if(command.compare(0, 8, "GET_FILE") == 0)
            cmp = 3;
        else if(command.compare(0, 9, "GET_TABLE") == 0)
            cmp = 4;
        else
            cout << "wrong command" << command << endl;    
          
        switch(cmp)
        {
            case 0:
                if(atn == 1)
                {
                     cout << "You cannot use this function because" << 
                       " you are not attached to the introducer" << endl; 
                }
                else 
                    add_node_func(input, ports, m_val, si_val, fi_val, lc);
                break;
            case 1:
                add_file(input, m_val);
                break;
            case 2:
                del_file(input, m_val);
                break;
            case 3:
                get_file(input, m_val);
                break;
            case 4:
                get_table(input, m_val);
                break;
            case -1:
                break;
        }
        
        // clear containers
        input.clear();
        command.clear();
        cmp = -1;
    }
    
    return 0;
}

// process the nodes that needs to be added to the chord system 
void add_node_func(string input, vector<int> ports, string m_val,
                   string si_val, string fi_val, int lc)
{
    string buf;
    stringstream ss;
    char * id_num;
    vector<string> tokens;
    vector<string>::iterator it;
    ss << input;
    while(ss >> buf) 
         tokens.push_back(buf);

    if(tokens.size() < 2)
    {
        cout << "specify at least one node" << endl;
    }
    // parse out node numbers
    for(it = tokens.begin()+1; it < tokens.end(); it++)
    {
        id_num = new char [it->size()+1];
        strcpy(id_num, it->c_str());
        cout << "adding node : " << atoi(id_num) << "\n";
        add_node(atoi(id_num), ports, m_val, si_val, fi_val, lc);
    }
    
    return;
}

// add a node to the chord system  
int add_node(int ID, vector<int> ports, string  m_val, string si_val,
             string fi_val, int lc)
{
    int port_num;
    int s;
    int i = 1;
    int seed = 0;
    
    // pick a port number 49152 and 65535
    // registered and well-known ports are not used
    // check if the port is available
    srand (time(NULL));
    
    while (i != 0)
    {
        if(ports.empty() == true)
            port_num = rand() % 16383 + 49152;
        else
        {
            port_num = ports[0];
            ports.erase(ports.begin());
        }
        i = scan_port(port_num);
    }
    
    // check if it is the introducer port
    if (ID == 0)
    {
        introducer_port = port_num;
    }
    
    // create a process listening on port rand_port
    if(make_syscall(m_val, ID, port_num, si_val, fi_val, lc, seed) == 1)
    {
        cout << "syscall failed"; 
        return 1;
    }
    
    return 0;
}

// add file to the chord system
int add_file(string input, string m_val)
{
    string buf;
    stringstream ss;
    string filename, data;
    vector<string> tokens;
    vector<string>::iterator it;
    int m;
    SHA1Context sha;
    FILE *fp;
    int32_t key_id;
    _FILE file;

    // change m_val into an integer
    m = atoi(m_val.c_str()); 

    ss << input;
    while(ss >> buf) 
         tokens.push_back(buf);
   
    // parse out file name and data
    it = tokens.begin()+1;
    filename.assign(*it);
    it++;
    data.assign(*it);
        
    cout << "filename is : " << filename << endl;
    cout << "file data is : " << data << endl;
    
    file.name = filename;
    file.data = data;
    // create sha-1 key
    SHA1Reset(&sha);
    SHA1Input(&sha, (unsigned char*)filename.c_str(), filename.size());
    if (!SHA1Result(&sha))
    {
        cout << "key_gen_test: could not compute key ID for" << filename << endl;
    }
    else
    {
        key_id = sha.Message_Digest[4]%((int)pow(2,m)) ;
        cout << "Key ID for " << filename << " : " << key_id << endl;
    }
    int port;
    // sending information to introducer
    if(introducer_port != 0)
    {
        port = introducer_port;
    }
    else if(atn_port != 0)
    {
        port = atn_port;
    }
    else 
    {
       cout << "listener is not connected to any node" << endl;
       return 1;
    }
    boost::shared_ptr<TSocket> socket(new TSocket("localhost", port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    NodeClient client(protocol);
    transport->open();
    // call rpc function
    int32_t id  = client.add_file(key_id, file);
    if(id > -1)
    {
        get_ADD_FILE_result_as_string(filename.c_str(), key_id, id);
    } 
    else 
       cout << "error in adding file" << endl;
    transport->close(); 
      
    return 0;
}

// delete the file specified
int del_file(string input, string m_val)
{
    string buf;
    stringstream ss;
    string filename;
    vector<string> tokens;
    vector<string>::iterator it;
    int key_id;
    SHA1Context sha;
    FILE *fp;
    int m = atoi(m_val.c_str());
    ss << input;
    while(ss >> buf) 
         tokens.push_back(buf);
   
    // parse out file name
    it = tokens.begin()+1;
    filename.assign(*it);
    
    cout << "filename is : " << filename << endl;
    
    // create sha-1 key
    SHA1Reset(&sha);
    SHA1Input(&sha, (unsigned char*)filename.c_str(), filename.size());
    if (!SHA1Result(&sha))
    {
        cout << "key_gen_test: could not compute key ID for" << filename << endl;
    }
    else
    {
        key_id = sha.Message_Digest[4]%((int)pow(2,m)) ;
        cout << "Key ID for " << filename << " : " << key_id << endl;
    }
    
    int port;
    // sending information to introducer
    if(introducer_port != 0)
    {
        port = introducer_port;
    }
    else if(atn_port != 0)
    {
        port = atn_port;
    }
    else 
    {
       cout << "listener is not connected to any node" << endl;
       return 1;
    }

    boost::shared_ptr<TSocket> socket(new TSocket("localhost", port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    NodeClient client(protocol);
    transport->open();
    // call rpc function
    int id  = client.del_file(key_id);
    if(id > -1)
    {
        get_ADD_FILE_result_as_string(filename.c_str(), key_id, id);
    } 
    else 
       cout << "error in adding file" << endl;
    transport->close(); 
     
    return 0;
}

// get the file contents of the file specified
int get_file(string input, string m_val)
{
    string buf;
    stringstream ss;
    string filename;
    vector<string> tokens;
    vector<string>::iterator it;
    int key_id;
    SHA1Context sha;
    FILE *fp;
    int m;

    m = atoi(m_val.c_str());

    ss << input;
    while(ss >> buf) 
         tokens.push_back(buf);
   
    // parse out file name
    it = tokens.begin()+1;
    filename.assign(*it);
    
    cout << "filename is : " << filename << endl;
     // create sha-1 key
    SHA1Reset(&sha);
    SHA1Input(&sha, (unsigned char*)filename.c_str(), filename.size());
    if (!SHA1Result(&sha))
    {
        cout << "key_gen_test: could not compute key ID for" << filename << endl;
    }
    else
    {
        key_id = sha.Message_Digest[4]%((int)pow(2,m)) ;
        cout << "Key ID for " << filename << " : " << key_id << endl;
    }

    // pass tokens[it] to introducer
    int port;
    // sending information to introducer
    if(introducer_port != 0)
    {
        port = introducer_port;
    }
    else if(atn_port != 0)
    {
        port = atn_port;
    }
    else 
    {
       cout << "listener is not connected to any node" << endl;
       return 1;
    }
    boost::shared_ptr<TSocket> socket(new TSocket("localhost", port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    NodeClient client(protocol);
    transport->open();
    //call rpc function
    bool check;
    int id;
    char* fdata;
    get_GET_FILE_result_as_string(filename.c_str(), key_id, check, id, fdata);
    transport->close(); 
     
    return 0;
}

// get and print finger table of the node specified
int get_table(string input, string m_val)
{
    string buf;
    stringstream ss;
    char * id_num;
    vector<string> tokens;
    vector<string>::iterator it;
    unsigned int i;
    int m;
    m = atoi(m_val.c_str()); 
    ss << input;
    
    while(ss >> buf) 
         tokens.push_back(buf);
         
    it = tokens.begin()+1;
    id_num = new char [it->size()+1];
    strcpy(id_num, it->c_str());
    cout << "getting finger table and key table for node : " << atoi(id_num) << "\n";
           
    // ask the node for the finger table
    int port;
    // sending information to introducer
    if(introducer_port != 0)
    {
        port = introducer_port;
    }
    else if(atn_port != 0)
    {
        port = atn_port;
    }
    else 
    {
       cout << "listener is not connected to any node" << endl;
       return 1;
    }
    boost::shared_ptr<TSocket> socket(new TSocket("localhost", port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    NodeClient client(protocol);
    transport->open();
    node_table table;
    client.get_table(table, atoi(id_num));
      transport->close(); 
     
    if(table.finger_table.size() == 0)
       cout << "requested node" << id_num << "does not exist" << endl;
    else
    {
       cout << get_GET_TABLE_result_as_string(table.finger_table, m, atoi(id_num), 0, table.keys_table);
    }   
    return 0;
}

// scan available ports on local host
int scan_port(int port_num)
{
    struct addrinfo hints, *res;
    int sockfd, ver;
    string s;
    stringstream ss;
    int yes = 1;
    
    // transform port_num into a string
    ss << port_num;
    s = ss.str();    

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, s.c_str(), &hints, &res);

    // make a socket
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    cout << "Checking port : " << port_num << endl;
    // see if port is usuable by trying to bind to socket
    ver = bind(sockfd, res->ai_addr, res->ai_addrlen);
   
    if(ver == -1)
    {
        cout << "port " << port_num << " is not available" << endl;
        return -1;
    }
    else
        cout << "port " << port_num << " is available" << endl;

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }
     
    cout << "Using port : " << port_num << "\n";
    
    return 0;
}

// do a syscall to create new nodes
int make_syscall(string m_val, int ID, int port_num, string si_val,
                 string fi_val, int lc, int seed)
{
    FILE *in;
    char buff[512];
   // pid_t pID;
    string command = "./node";
    cout << "creating a new node"<< endl;
   
    // number of bits of the keys/node IDs
    if(m_val.empty() == false)
    {
        command.append(" --m ");
        command.append(m_val);
    }
    
    // the node ID 
    command.append(" --id ");
    sprintf(buff, "%d", ID);    
    command.append(buff);

    // port number
    command.append(" --port ");
    sprintf(buff, "%d", port_num);
    command.append(buff);

    // introducer port if id != 0
    if(ID != 0)
    {
        command.append(" --introducerPort ");
        sprintf(buff, "%d", introducer_port);
        command.append(buff);
    }

    // stabilizeInterval
    if(si_val.empty() == false)
    {
        command.append(" --stabilizeInterval ");
        command.append(si_val);
    }
    
    // fixInterval
    if(fi_val.empty() == false)
    {
        command.append(" --fixInterval ");
        command.append(fi_val);
    }
 
    // to seed the randomm number generator
    if(seed != 0)
    {
        command.append(" --seed ");
        sprintf(buff, "%d", seed);
        command.append(buff);   
    }

    // logConf
    if(lc == 1)
    {
        command.append(" --logConf");
    }

    cout << command << endl;
    in = popen(command.c_str(), "r");
    return 0;

 /*   pID = fork();
    
    if(pID == 0)
    {
        in = popen("./my_server", "r");
    }
    
    else if (pID < 0)
    {
        cout << "fail to fork" << endl;
    }
    
   
    
    if(!(in = popen("./my_server", "w")))
    {
        return 1;
    }   
    
    while(fgets(buff, sizeof(buff), in) != NULL)
    {
        cout << buff;
    }
    
    pclose(in);  */
}

string get_GET_TABLE_result_as_string(
        const vector<finger_entry>& finger_table,
        const uint32_t m,
        const uint32_t id,
        const uint32_t idx_of_entry1,
        const map<int32_t, _FILE>& keys_table)
    {
        return get_finger_table_as_string(
            finger_table, m, id, idx_of_entry1) \
            + \
            get_keys_table_as_string(keys_table);
    }


string
get_finger_table_as_string(const vector<finger_entry>& table,
                           const uint32_t m,
                           const uint32_t id,
                           const uint32_t idx_of_entry1)
{
    stringstream s;
    assert(table.size() == (idx_of_entry1 + m));
    s << "finger table:\n";
    for (size_t i = 1; (i - 1 + idx_of_entry1) < table.size(); ++i) {
        using std::setw;
        s << "entry: i= " << setw(2) << i << ", interval=["
          << setw(4) << (id + (int)pow(2, i-1)) % ((int)pow(2, m))
          << ",   "
          << setw(4) << (id + (int)pow(2, i)) % ((int)pow(2, m))
          << "),   node= "
          << setw(4) << table.at(i - 1 + idx_of_entry1).id
          << "\n";
    }
    return s.str();
}

string
get_keys_table_as_string(const map<int32_t, _FILE>& table)
{
    stringstream s;
    map<int32_t, _FILE>::const_iterator it = table.begin();
    /* map keeps the keys sorted, so our iteration will be in
 *      * ascending order of the keys
 *           */
    s << "keys table:\n";
    for (; it != table.end(); ++it) {
        using std::setw;
        /* assuming file names are <= 10 chars long */
        s << "entry: k= " << setw(4) << it->first
          << ",  fname= " << setw(10) << it->second.name
          << ",  fdata= " << it->second.data
          << "\n";
    }
    return s.str();
}

string get_ADD_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const int32_t nodeId)
{
     stringstream s;
     s << "fname= " << fname << "\n";
     s << "key= " << key << "\n";
     s << "added to node= " << nodeId << "\n";
     return s.str();
}

string get_DEL_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const bool deleted,
                                         const int32_t nodeId)
{
     stringstream s;
     s << "fname= " << fname << "\n";
     s << "key= " << key << "\n";
     if (deleted) {
         // then nodeId is meaningful
         s << "was stored at node= " << nodeId << "\n";
         s << "deleted\n";
     }
     else {
         // assume that this means file was not found
         s << "file not found\n";
     }
     return s.str();
}


string get_GET_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const bool found,
                                         const int32_t nodeId,
                                         const char *fdata)
{
     stringstream s;
     s << "fname= " << fname << "\n";
     s << "key= " << key << "\n";
     if (found) {
         // then nodeId is meaningful
          s << "stored at node= " << nodeId << "\n";
          s << "fdata= " << fdata << "\n";
     }
     else {
        // assume that this means file was not found
          s << "file not found\n";
     }
     return s.str();
}
            
