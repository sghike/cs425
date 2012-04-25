#include <assert.h>
#include <stdint.h>
// #include <iomanip>
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

using namespace std;

int introducer_port;

int main(int argc, char* argv[])
{
    std::string input;
    std::string command;
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
    std::string portnum;
    int number_of_ports = 0;
    
    std::stringstream ss;
    std::string argument;
    
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
        else if(argument.compare("--stablizeInterval")==0)
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
        argument.clear();
    }
    
    if( m > 1 || sp > 1 || atn > 1 || si > 1 || fi > 1 || lc > 1)
    {   
        cout << "Wrong usage of program\n";
        return -1;
    }   
    
    if(m == 1)
        cout << "the number of bits of the keys/nodeIDs : " << atoi(argv[m_pos+1]) << endl;
    if(si == 1)
        cout << "stabilize interval to " << atoi(argv[si_pos+1]) << endl;
    if(atn == 1)
        cout << "attaching to node " << atoi(argv[atn_pos+1]) << endl;    
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
            number_of_ports++;
            ports.push_back(atoi(argv[sp_pos+i]));
            i++;
            portnum.clear();
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
        cout << "fix interval to : " << atoi(argv[sp_pos+1]) << endl;
    if(lc == 1)
        cout << "logging is enabled" << endl;    
    
    // take inputs from the terminal
    while(1)
    {
        cout << "INPUT : ";
        getline(cin, input);
        
        // obtain first word
        ss << input;
        ss >> command;
        cout << command << "\n";        
                
        // decide command 
        if(command.compare("ADD_NODE") == 0)
            cmp = 0;
        else if(command.compare("ADD_FILE") == 0)
            cmp = 1;
        else if(command.compare("DEL_FILE") == 0)
            cmp = 2;
        else if(command.compare("GET_FILE") == 0)
            cmp = 3;
        else if(command.compare("GET_TABLE") == 0)
            cmp = 4;
        else
            cout << "wrong command\n";    
          
        switch(cmp)
        {
            case 0:
                add_node_func(input, ports);
                break;
            case 1:
                add_file(input);
                break;
            case 2:
                del_file(input);
                break;
            case 3:
                get_file(input);
                break;
            case 4:
                get_table(input);
                break;
            case -1:
                break;
        }
        
        // clear containers
        ss.str(std::string());
        input.clear();
        command.clear();
    }
    
    return 0;
}

void add_node_func(std::string input, vector<int> ports)
{
    std::string buf;
    std::stringstream ss;
    char * id_num;
    vector<std::string> tokens;
    vector<std::string>::iterator it;
    ss << input;
    while(ss >> buf) 
         tokens.push_back(buf);

    // parse out node numbers
    for(it = tokens.begin()+1; it < tokens.end(); it++)
    {
        id_num = new char [it->size()+1];
        strcpy(id_num, it->c_str());
        cout << "adding node : " << atoi(id_num) << "\n";
        add_node(atoi(id_num), ports);
    }
    
    return;
}

// 
int add_node(int ID, vector<int> ports)
{
    int port_num;
    int s;
    int i = 1;
    
    // pick a port number 49152 and 65535
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
        introducer_port = port_num;
    
    // fork a process listening on port rand_port
    
    // contact the introducer
    
    
    return 0;
}

// pass to introducer
int add_file(std::string input)
{
    std::string buf;
    std::stringstream ss;
    std::string filename, data;
    vector<std::string> tokens;
    vector<std::string>::iterator it;
    
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
    
    // pass tokens[it] to introducer
        
    return 0;
}

// pass to introducer
int del_file(std::string input)
{
    std::string buf;
    std::stringstream ss;
    std::string filename;
    vector<std::string> tokens;
    vector<std::string>::iterator it;
    
    ss << input;
    while(ss >> buf) 
         tokens.push_back(buf);
   
    // parse out file name
    it = tokens.begin()+1;
    filename.assign(*it);
    
    cout << "filename is : " << filename << endl;

    // pass tokens[it] to introducer
    
    return 0;
}

// pass to introducer
int get_file(std::string input)
{
    std::string buf;
    std::stringstream ss;
    std::string filename;
    vector<std::string> tokens;
    vector<std::string>::iterator it;
    
    ss << input;
    while(ss >> buf) 
         tokens.push_back(buf);
   
    // parse out file name
    it = tokens.begin()+1;
    filename.assign(*it);
    
    cout << "filename is : " << filename << endl;

    // pass tokens[it] to introducer
    
    return 0;
}

// pass to introducer
int get_table(std::string input)
{
    std::string buf;
    std::stringstream ss;
    char * id_num;
    vector<std::string> tokens;
    vector<std::string>::iterator it;
    
    ss << input;
    
    while(ss >> buf) 
         tokens.push_back(buf);
         
    it = tokens.begin()+1;
    id_num = new char [it->size()+1];
    strcpy(id_num, it->c_str());
    cout << "getting finger table and key table for node : " << atoi(id_num) << "\n";
           
    // pass tokens[it] to introducer
    
    return 0;
}

int scan_port(int port_num)
{
    cout << "Using port : " << port_num << "\n";
    return 0;
    
}

