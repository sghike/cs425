#ifndef __LISTENER_HPP_
#define __LISTENER_HPP_

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
#include "ListenerService.h"
#include "Node.h"


void add_node_func(std::string input, std::vector<int> ports, std::string m_val, 
                   std::string si_val, std::string fi_val, int lc);
int add_node(int ID, std::vector<int> ports, std::string m_val, std::string si_val,
             std::string fi_val, int lc);
int add_file(std::string input, std::string m_val);
int del_file(std::string input);
int get_file(std::string input);
int get_table(std::string input, std::string m_val);
int scan_port(int port_num);
int make_syscall(std::string m_val, int ID, int port_num, std::string si_val, 
                 std::string fi_val, int lc, int seed);
std::string get_DEL_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const bool deleted,
                                         const int32_t nodeId);
std::string get_ADD_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const int32_t nodeId);
std::string get_keys_table_as_string(const std::map<int32_t, std::string>& table);
std::string get_GET_TABLE_result_as_string(
        const vector<finger_entry>& finger_table,
        const uint32_t m,
        const uint32_t myid,
        const uint32_t idx_of_entry1,
        const std::map<int32_t, std::string>& keys_table);
std::string get_finger_table_as_string(const std::vector<finger_entry>& table,
                           const uint32_t m,
                           const uint32_t myid,
                           const uint32_t idx_of_entry1);
string get_GET_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const bool found,
                                         const int32_t nodeId,
                                         const char *fdata);
#endif



