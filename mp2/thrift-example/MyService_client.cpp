#include "MyService.h"

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>


#include <vector>
#include <vector>
#include <string>

using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

// the name space specified in the thrift file
using namespace mp2;

int main(int argc, char **argv) {
  /* server is listening on port 9090 */

  /* these next three lines are standard */
  boost::shared_ptr<TSocket> socket(new TSocket("localhost", 9090));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));

  /* prepare arguments for the rpc call */
  MyObject input;
  input.i = 123;
  input.d = 456.789;
  input.s = "hello";
  std::vector<std::string> stringvector;
  stringvector.push_back("one");
  stringvector.push_back("two");
  stringvector.push_back("three");

  /* I am a MyServiceClient */
  MyServiceClient client(protocol);
  transport->open();
  /* make the call and get the return value */
  MyObject result;
  client.an_rpc_func(result, input, 392, stringvector);
  transport->close();

  printf("result is: i=%d, s=%s, d=%f\n", 
	 result.i, result.s.c_str(), result.d);
  return 0;
}
