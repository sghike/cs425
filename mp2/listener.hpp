#ifndef __LISTENER_HPP_
#define __LISTENER_HPP_

#include <vector>

void attach_to_node(std::string port);
void add_node_func(std::string input, std::vector<int> ports, std::string m_val, 
                   std::string si_val, std::string fi_val, int lc);
int add_node(int ID, std::vector<int> ports, std::string m_val, std::string si_val,
             std::string fi_val, int lc);
int add_file(std::string input, std::string m_val);
int del_file(std::string input);
int get_file(std::string input);
int get_table(std::string input);
int scan_port(int port_num);
int make_syscall(std::string m_val, int ID, int port_num, std::string si_val, 
                 std::string fi_val, int lc, int seed);

#endif



