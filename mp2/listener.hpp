#ifndef __LISTENER_HPP_
#define __LISTENER_HPP_
#include <vector>
#include <map>

using namespace std;
using namespace ::mp2;

void add_node_func(string input, vector<int> ports, string m_val, string si_val, string fi_val, string lc_val);

int add_node(int ID, vector<int> ports, string m_val, string si_val,
             string fi_val, string lc_val);

int add_file(string input, string m_val);

int del_file(string input, string m_val);

int get_file(string input, string m_val);

int get_table(string input, string m_val);

int scan_port(int port_num);

int make_syscall(string m_val, int ID, int port_num, string si_val, 
                 string fi_val, string lc_val, int seed);

string get_DEL_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const bool deleted,
                                         const int32_t nodeId);

string get_ADD_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const int32_t nodeId);

string get_keys_table_as_string(const map<int32_t, _FILE>& table);

string get_GET_TABLE_result_as_string(
        const vector<finger_entry>& finger_table,
        const uint32_t m,
        const uint32_t id,
        const uint32_t idx_of_entry1,
        const map<int32_t, _FILE>& keys_table);

string get_finger_table_as_string(const vector<finger_entry>& table,
                           const uint32_t m,
                           const uint32_t id,
                           const uint32_t idx_of_entry1);

string get_GET_FILE_result_as_string(const char *fname,
                                         const int32_t key,
                                         const bool found,
                                         const int32_t nodeId,
                                         const char *fdata);
#endif



