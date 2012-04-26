#ifndef __LISTENER_HPP_
#define __LISTENER_HPP_

#include <vector>

void add_node_func(std::string input, std::vector<int> ports);
int add_node(int ID, std::vector<int> ports);
int add_file(std::string input);
int del_file(std::string input);
int get_file(std::string input);
int get_table(std::string input);
int scan_port(int port_num);
int make_syscall();

#endif



