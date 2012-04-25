// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "Node.h"
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::mp2;

class NodeHandler : virtual public NodeIf {
 public:
  NodeHandler() {
    // Your initialization goes here
  }

  void closest_preceding_finger(finger_entry& _return, const int32_t id) {
    // Your implementation goes here
    printf("closest_preceding_finger\n");
  }

  void get_successor(finger_entry& _return) {
    // Your implementation goes here
    printf("get_successor\n");
  }

};

int main(int argc, char **argv) {
  int port = 9090;
  shared_ptr<NodeHandler> handler(new NodeHandler());
  shared_ptr<TProcessor> processor(new NodeProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

