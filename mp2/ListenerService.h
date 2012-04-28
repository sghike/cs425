/**
 * Autogenerated by Thrift Compiler (0.8.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef ListenerService_H
#define ListenerService_H

#include <TProcessor.h>
#include "mp2_types.h"

namespace mp2 {

class ListenerServiceIf {
 public:
  virtual ~ListenerServiceIf() {}
};

class ListenerServiceIfFactory {
 public:
  typedef ListenerServiceIf Handler;

  virtual ~ListenerServiceIfFactory() {}

  virtual ListenerServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(ListenerServiceIf* /* handler */) = 0;
};

class ListenerServiceIfSingletonFactory : virtual public ListenerServiceIfFactory {
 public:
  ListenerServiceIfSingletonFactory(const boost::shared_ptr<ListenerServiceIf>& iface) : iface_(iface) {}
  virtual ~ListenerServiceIfSingletonFactory() {}

  virtual ListenerServiceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(ListenerServiceIf* /* handler */) {}

 protected:
  boost::shared_ptr<ListenerServiceIf> iface_;
};

class ListenerServiceNull : virtual public ListenerServiceIf {
 public:
  virtual ~ListenerServiceNull() {}
};

class ListenerServiceClient : virtual public ListenerServiceIf {
 public:
  ListenerServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  ListenerServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class ListenerServiceProcessor : public ::apache::thrift::TProcessor {
 protected:
  boost::shared_ptr<ListenerServiceIf> iface_;
  virtual bool process_fn(apache::thrift::protocol::TProtocol* iprot, apache::thrift::protocol::TProtocol* oprot, std::string& fname, int32_t seqid, void* callContext);
 private:
  std::map<std::string, void (ListenerServiceProcessor::*)(int32_t, apache::thrift::protocol::TProtocol*, apache::thrift::protocol::TProtocol*, void*)> processMap_;
 public:
  ListenerServiceProcessor(boost::shared_ptr<ListenerServiceIf> iface) :
    iface_(iface) {
  }

  virtual bool process(boost::shared_ptr<apache::thrift::protocol::TProtocol> piprot, boost::shared_ptr<apache::thrift::protocol::TProtocol> poprot, void* callContext);
  virtual ~ListenerServiceProcessor() {}
};

class ListenerServiceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  ListenerServiceProcessorFactory(const ::boost::shared_ptr< ListenerServiceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< ListenerServiceIfFactory > handlerFactory_;
};

class ListenerServiceMultiface : virtual public ListenerServiceIf {
 public:
  ListenerServiceMultiface(std::vector<boost::shared_ptr<ListenerServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~ListenerServiceMultiface() {}
 protected:
  std::vector<boost::shared_ptr<ListenerServiceIf> > ifaces_;
  ListenerServiceMultiface() {}
  void add(boost::shared_ptr<ListenerServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
};

} // namespace

#endif