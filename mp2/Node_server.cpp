// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Node.h"
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#include <concurrency/ThreadManager.h>
#include <concurrency/PosixThreadFactory.h>
#include <server/TThreadedServer.h>
#include <iostream>
#include <assert.h>
#include <stdint.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace ::apache::thrift::concurrency;

using boost::shared_ptr;

using namespace  ::mp2;
using namespace std;

int ipow(int base, int exp) {
    int result = 1;
    while (exp)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}

class Node {
  public:
    int id;
    int port;
    finger_entry successor;
    finger_entry predecessor;
    vector<finger_entry> finger_table;
    int m;
    int introducerPort;
    int stabilizeInterval;
    int fixInterval;
    int seed;
    map<int, _FILE> keys_table;

    Node(int m, int id, int port) {
      int i;
      finger_entry null_finger;
      null_finger.id = -1;
      null_finger.port = -1;
      this->m = m;
      this->id = id;
      this->port = port;
      for (i = 0; i < m; i++)
        finger_table.push_back(null_finger);
      if (id == 0){
        introducerPort = port;
        predecessor.id = id;
        predecessor.port = port;
        successor.id = id;
        successor.port  = port;
        finger_table[0] = successor;
        cout << "node= <" << id << ">: updated predecessor= <" << predecessor.id << ">" << endl;
        cout << "node= <" << id << ">: updated finger entry: i= <" << 1 << ">, pointer= <" << finger_table[0].id << ">" << endl;
      }
      else
      {
          predecessor.id = -1;
          predecessor.port = -1;
          successor.id = -1;
          successor.port = -1;
      }

      stabilizeInterval = 1;
      fixInterval = 1;
      seed = 10;
      srand(seed);
    }

    void setIntroducerPort(int introducerPort) {
      this->introducerPort = introducerPort;
      predecessor.port = introducerPort;
      successor.port = introducerPort;
    }

    void setStabilizeInterval(int stabilizeInterval) {
      this->stabilizeInterval = stabilizeInterval;
    }

    void setFixInterval(int fixInterval) {
      this->fixInterval = fixInterval;
    }

    void setSeed(int seed) {
      this->seed = seed;
      srand(seed);
    }

    finger_entry find_successor_local(int id) {
      // printf("id = %d in find_successor_local\n", id);
      finger_entry n = find_predecessor(id);
      // printf("predecessor = %d %d \n", n.id, n.port);
      if (n.id == this->id)
      {
        finger_entry _return;
        _return.id = this->successor.id;
        _return.port = this->successor.port;
        return _return;
      }
      boost::shared_ptr<TSocket> socket(new TSocket("localhost", n.port));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      NodeClient client(protocol);
      transport->open();

      finger_entry n_successor;
      client.get_successor(n_successor);
      
      transport->close();
      return n_successor;
    }

    finger_entry find_predecessor(int id) {
      finger_entry n;
      n.id = this->id;
      n.port = this->port;
      int succ = successor.id;
      while ((succ != n.id) && !((succ > n.id && (id > n.id && id <= succ)) || 
             (n.id > succ && (id > n.id || id <= succ)))) {
        boost::shared_ptr<TSocket> socket1(new TSocket("localhost", n.port));
        boost::shared_ptr<TTransport> transport1(new TBufferedTransport(socket1));
        boost::shared_ptr<TProtocol> protocol1(new TBinaryProtocol(transport1));
        NodeClient client1(protocol1);
        transport1->open();
        client1.closest_preceding_finger(n, id);
        transport1->close();
      
        boost::shared_ptr<TSocket> socket2(new TSocket("localhost", n.port));
        boost::shared_ptr<TTransport> 
          transport2(new TBufferedTransport(socket2));
        boost::shared_ptr<TProtocol> protocol2(new TBinaryProtocol(transport2));
        NodeClient client2(protocol2);
        transport2->open();
        finger_entry n_successor;
        client2.get_successor(n_successor);
        succ = n_successor.id;
        transport2->close();
      }
      return n;
    }

    void join(int introducer) {
      predecessor.id = -1;
      predecessor.port =-1;

      assert(introducerPort != -1);
      boost::shared_ptr<TSocket> socket(new TSocket("localhost", 
            introducerPort));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      NodeClient client(protocol);
      transport->open();

      finger_entry succ;
      finger_entry my_entry;
      my_entry.id = id;
      my_entry.port = port;
      client.find_successor(succ, my_entry);

      transport->close(); 
      this->successor = succ;
      cout << "node = <" << id << ">: initial sucessor= <" << successor.id << ">" << endl;
      if (finger_table[0] != this->successor)
      {
          finger_table[0] = this->successor;    
          cout << "node= <" << id << ">: updated finger entry: i= <" << 1 << ">, pointer= <" << finger_table[0].id << ">" << endl;
      }
    }

    void stabilize() {
      // printf("in stabilize\n");
      boost::shared_ptr<TSocket> socket(new TSocket("localhost", 
            successor.port));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      NodeClient client(protocol);
      transport->open();
      finger_entry x;
      client.get_predecessor(x);
      transport->close();

      if ((id < successor.id && (x.id > id && x.id < successor.id)) ||
          (successor.id < id && (x.id > id || x.id < successor.id))) {
        successor = x;
        if (finger_table[0] != this->successor)
        {
            finger_table[0] = successor;
        cout << "node= <" << id << ">: updated finger entry: i= <" << 1 << ">, pointer= <" << finger_table[0].id << ">" << endl;
        }
      }

      transport->open();
      finger_entry my_entry;
      my_entry.id = id;
      my_entry.port = port;
      client.notify(my_entry);
      transport->close();
    }

    void fix_fingers() {
      // printf("inside fix fingers\n");
      int i = (rand() % (m-1)) + 1;
      int start = (id + ipow(2, i)) % ipow(2, m);
      // printf("i is %d and start is %d", i, start);
      finger_entry new_finger = find_successor_local(start);
      if (finger_table[i] != new_finger) {
        finger_table[i] = new_finger;
        cout << "node= <" << id << ">: updated finger entry: i= <" << i+1 << ">, pointer= <" << new_finger.id << ">" << endl;
      }
    }
  
};

Node *me;

class NodeHandler : virtual public NodeIf {
 public:
  NodeHandler() {
    // Your initialization goes here
  }
  
  void find_successor(finger_entry& _return, const finger_entry& caller) {
    // Your implementation goes here
    // printf("find_successor\n");
    
    finger_entry n = me->find_predecessor(caller.id);
    
    if (n.id == me->id) {
      _return.id = me->successor.id;
      _return.port = me->successor.port;
    }
    if (me->id == 0 && me->successor.id == 0) {
      me->successor.id = caller.id;
      me->successor.port = caller.port;
      me->predecessor.id = caller.id;
      me->predecessor.port = caller.port;
      me->finger_table[0] = me->successor;
      // print statements here
        cout << "node= <" << me->id << ">: updated predecessor= <" << me->predecessor.id << ">" << endl;
        cout << "node= <" << me->id << ">: updated finger entry: i= <" << 1 << ">, pointer= <" << me->finger_table[0].id << ">" << endl;
      return;
    }
    boost::shared_ptr<TSocket> socket(new TSocket("localhost", n.port));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    NodeClient client(protocol);
    transport->open();

   //  printf("n id is %d n port is %d\n", n.id, n.port);
    client.get_successor(_return);
    
    transport->close();
  }


  void closest_preceding_finger(finger_entry& _return, const int32_t id) {
    // Your implementation goes here
    // printf("closest_preceding_finger\n");
    int i;
    for (i = me->m - 1; i >= 0; i--) {
      int finger_id = me->finger_table[i].id;
      if (finger_id == -1)
           continue;
      if ((me->id < id && (finger_id > me->id && finger_id < id)) ||
          (me->id > id && (finger_id < id || finger_id > me->id))) {
        _return = me->finger_table[i];
        return;
      }
    }
    _return.id  = me->id;
    _return.port = me->port;
  }
  
  void get_successor(finger_entry& _return) {
    // Your implementation goes here
    // printf("get_successor\n");
    _return = me->successor;
  }

  void get_predecessor(finger_entry& _return) {
    // Your implementation goes here
    // printf("get_predecessor\n");
    _return = me->predecessor;
  }

  void notify(const finger_entry& n) {
    // Your implementation goes here
    // printf("notify\n");
    if ((me->predecessor.id == -1) || 
        ((me->predecessor.id < me->id && (n.id > me->predecessor.id && n.id < me->id)) ||
         (me->id < me->predecessor.id && (n.id > me->predecessor.id || n.id < me->id)))) {
      printf("me->predecessor is %d and n.id is %d\n", me->predecessor.id, n.id);
      if (me->predecessor != n) {
        me->predecessor = n;
        cout << "node= <" << me->id << ">: updated predecessor= <" << n.id << ">" << endl;
      }
    }
  }
  
  void get_table(node_table& _return, const int32_t id) {
    // Your implementation goes here
    printf("get_table\n");
    if (me->id == id) {
      _return.finger_table = me->finger_table;
      _return.keys_table = me->keys_table;
      return;
    } else {
      finger_entry pred;
      pred = me->find_predecessor(id);
      finger_entry pred_succ;
      boost::shared_ptr<TSocket> socket(new TSocket("localhost", pred.port));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      NodeClient client(protocol);
      transport->open();
      client.get_successor(pred_succ);
      transport->close();

      if (pred_succ.id != id) {
        return;
      } else {
        boost::shared_ptr<TSocket> socket1(new TSocket("localhost", 
              pred_succ.port));
        boost::shared_ptr<TTransport> 
          transport1(new TBufferedTransport(socket1));
        boost::shared_ptr<TProtocol> protocol1(new TBinaryProtocol(transport1));
        NodeClient client1(protocol1);
        transport1->open();
        client1.get_table(_return, id);
        transport1->close();
      }
    }
  }
  
  int add_file(const int32_t key_id, const _FILE& s) {
    // Your implementation goes here
    printf("add_file\n");
    int ret;
    finger_entry succ;
    succ = me->find_successor_local(key_id);
    if (succ.id == me->id) {
      pair<map<int, _FILE>::iterator, bool> check; 
      check = me->keys_table.insert(pair<int, _FILE>(key_id, s)); 
      if (check.second == false) {
        printf("file with same key exists already\n");
        ret = -1; //failure
       } else {
        ret = me->id; // success
        cout << "node= <" << me->id << ">: added file: k= <" << key_id << ">" << endl;
      }
    } else {
      boost::shared_ptr<TSocket> socket(new TSocket("localhost", succ.port));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      NodeClient client(protocol);
      transport->open();
      ret = client.add_file(key_id, s);
      transport->close();
    }
    return ret;
  }

  int del_file(const int32_t key_id) {
    // Your implementation goes here
    printf("del_file\n");
    int ret;
    finger_entry succ;
    succ = me->find_successor_local(key_id);
    if (succ.id == me->id) {
      int del = me->keys_table.erase(key_id); 
      if (del == 0) {
        ret = -1; //failure
        cout << "node= <" << me->id << ">: no such file k= <" << key_id << "> to delete" << endl;
       } else {
        ret = me->id; // success
        cout << "node= <" << me->id << ">: deleted file: k= <" << key_id << ">" << endl;
      }
    } else {
      boost::shared_ptr<TSocket> socket(new TSocket("localhost", succ.port));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      NodeClient client(protocol);
      transport->open();
      ret = client.del_file(key_id);
      transport->close();
    }
    return ret;
  }

  void get_file(file_data& _return, const int32_t key_id) {
    // Your implementation goes here
    printf("get_file\n");
    finger_entry succ;
    succ = me->find_successor_local(key_id);
    if (succ.id == me->id) {
      map<int, _FILE>::iterator it;
      it = me->keys_table.find(key_id); 
      if (it == me->keys_table.end()) { //failure
        _return.node = -1;
        cout << "node= <" << me->id << ">: no such file k= <" << key_id << "> to serve" << endl;
       } else {
         _return.node = me->id;
         _return.file = me->keys_table[key_id]; // success
         cout << "node= <" << me->id << ">: served file: k= <" << key_id << ">" << endl;
      }
    } else {
      boost::shared_ptr<TSocket> socket(new TSocket("localhost", succ.port));
      boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
      boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
      NodeClient client(protocol);
      transport->open();
      client.get_file(_return, key_id);
      transport->close();
    }
  }

};


void *stabilize(void *stabilizeInterval) {
  while (1) {
    sleep(*((int *)stabilizeInterval));
    me->stabilize();
  }
  return NULL;
}

void *fixFingers(void *fixInterval) {
  while (1) {
    sleep(*((int *)fixInterval));
    me->fix_fingers();
  }
  return NULL;
}


int main(int argc, char **argv) {
//  INIT_LOCAL_LOGGER();
  int opt;
  int long_index;

  int m = -1;
  int id = -1;
  int port = -1;
  int introducerPort = -1;
  int stabilizeInterval = -1;
  int fixInterval = -1;
  int seed = -1;
  const char *logconffile = NULL;
  
  struct option long_options[] = {
    /* mandatory args */
    {"m", required_argument, 0, 1000},
    /* id of this node: 0 for introducer */
    {"id", required_argument, 0, 1001},
    /* port THIS node will listen on, at least for the
     * Chord-related API/service
     */
    {"port", required_argument, 0, 1002},
    /* optional args */
    /* if not introducer (id != 0), then this is required: port
     * the introducer is listening on.
     */
    {"introducerPort", required_argument, 0, 1003},
    /* path to the log configuration file */
    {"logConf", required_argument, 0, 1004},
    /* intervals (seconds) for runs of the stabilization and
     * fixfinger algorithms */
    {"stabilizeInterval", required_argument, 0, 1005},
    {"fixInterval", required_argument, 0, 1006},
    {"seed", required_argument, 0, 1007},
    {0, 0, 0, 0},
  };
  
  while ((opt = getopt_long(argc, argv, "", long_options, &long_index)) != -1) {
      switch (opt) {
      case 0:
          if (long_options[long_index].flag != 0) {
              break;
          }
          printf("option %s ", long_options[long_index].name);
          if (optarg) {
              printf("with arg %s\n", optarg);
          }
          printf("\n");
          break;
      case 1000:
          m = strtol(optarg, NULL, 10);
          assert((m >= 5) && (m <= 10));
          break;
      case 1001:
          id = strtol(optarg, NULL, 10);
          assert(id >= 0);
          break;
      case 1002:
          port = strtol(optarg, NULL, 10);
          assert(port > 0);
          break;
      case 1003:
          introducerPort = strtol(optarg, NULL, 10);
          assert(introducerPort > 0);
          break;
      case 1004:
          logconffile = optarg;
          break;
      case 1005:
          stabilizeInterval = strtol(optarg, NULL, 10);
          assert(stabilizeInterval > 0);
          break;
      case 1006:
          fixInterval = strtol(optarg, NULL, 10);
          assert(fixInterval > 0);
          break;
      case 1007:
          seed = strtol(optarg, NULL, 10);
          break;
      default:
          exit(1);
      }
  }

  // configureLogging(logconffile);
  
  me = new Node(m, id, port);
  if (introducerPort != -1) {
    me->setIntroducerPort(introducerPort);
  }
  if (stabilizeInterval != -1) {
    me->setStabilizeInterval(stabilizeInterval);
  }
  if (fixInterval != -1) {
    me->setFixInterval(fixInterval);
  }
  if (seed != -1) {
    me->setSeed(seed);
  }

  shared_ptr<NodeHandler> handler(new NodeHandler());
  shared_ptr<TProcessor> processor(new NodeProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());
  shared_ptr<ThreadManager> threadManager = ThreadManager::newSimpleThreadManager(4);
  shared_ptr<PosixThreadFactory> threadFactory = shared_ptr<PosixThreadFactory>(new PosixThreadFactory());
  threadManager->threadFactory(threadFactory);
  threadManager->start();
  TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
  


  if (me->id > 0) {
    me->join(0);
  }

  pthread_t stabilizer_thread, finger_thread;
  if (pthread_create(&stabilizer_thread, NULL, stabilize, 
        (void *)&(me->stabilizeInterval))) {
    cout << "Error in stabilizer thread creation." << endl;
    exit(1);
  }
  if (pthread_create(&finger_thread, NULL, fixFingers, 
        (void *)&(me->fixInterval))) {
    cout << "Error in Finger thread creation." << endl;
    exit(1);
  }
  
  server.serve();

  return 0;
}

