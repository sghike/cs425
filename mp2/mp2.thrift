namespace cpp mp2

struct finger_entry {
	1: i32 id;
	2: i32 port;
}

struct node_table {
  1: list<finger_entry> finger_table;
  2: map<i32, string> keys_table;
}

struct file_data {
  1: bool found;
  2: i32 node;
  3: string data;
}

service Node {
        finger_entry find_successor(1:finger_entry caller);
	finger_entry closest_preceding_finger(1:i32 id);
        finger_entry get_successor();
        finger_entry get_predecessor();
        void notify(1:finger_entry n);
        i32 add_file(1:i32 key_id, 2:string s);
        i32 del_file(1:i32 key_id);
        file_data get_file(1:i32 key_id);
        node_table get_table(1:i32 id);
}

service ListenerService {
	
}
