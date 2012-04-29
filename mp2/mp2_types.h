/**
 * Autogenerated by Thrift Compiler (0.8.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef mp2_TYPES_H
#define mp2_TYPES_H

#include <Thrift.h>
#include <TApplicationException.h>
#include <protocol/TProtocol.h>
#include <transport/TTransport.h>



namespace mp2 {

typedef struct _finger_entry__isset {
  _finger_entry__isset() : id(false), port(false) {}
  bool id;
  bool port;
} _finger_entry__isset;

class finger_entry {
 public:

  static const char* ascii_fingerprint; // = "989D1F1AE8D148D5E2119FFEC4BBBEE3";
  static const uint8_t binary_fingerprint[16]; // = {0x98,0x9D,0x1F,0x1A,0xE8,0xD1,0x48,0xD5,0xE2,0x11,0x9F,0xFE,0xC4,0xBB,0xBE,0xE3};

  finger_entry() : id(0), port(0) {
  }

  virtual ~finger_entry() throw() {}

  int32_t id;
  int32_t port;

  _finger_entry__isset __isset;

  void __set_id(const int32_t val) {
    id = val;
  }

  void __set_port(const int32_t val) {
    port = val;
  }

  bool operator == (const finger_entry & rhs) const
  {
    if (!(id == rhs.id))
      return false;
    if (!(port == rhs.port))
      return false;
    return true;
  }
  bool operator != (const finger_entry &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const finger_entry & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct __FILE__isset {
  __FILE__isset() : name(false), data(false) {}
  bool name;
  bool data;
} __FILE__isset;

class _FILE {
 public:

  static const char* ascii_fingerprint; // = "07A9615F837F7D0A952B595DD3020972";
  static const uint8_t binary_fingerprint[16]; // = {0x07,0xA9,0x61,0x5F,0x83,0x7F,0x7D,0x0A,0x95,0x2B,0x59,0x5D,0xD3,0x02,0x09,0x72};

  _FILE() : name(""), data("") {
  }

  virtual ~_FILE() throw() {}

  std::string name;
  std::string data;

  __FILE__isset __isset;

  void __set_name(const std::string& val) {
    name = val;
  }

  void __set_data(const std::string& val) {
    data = val;
  }

  bool operator == (const _FILE & rhs) const
  {
    if (!(name == rhs.name))
      return false;
    if (!(data == rhs.data))
      return false;
    return true;
  }
  bool operator != (const _FILE &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const _FILE & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _node_table__isset {
  _node_table__isset() : finger_table(false), predecessor(false), keys_table(false) {}
  bool finger_table;
  bool predecessor;
  bool keys_table;
} _node_table__isset;

class node_table {
 public:

  static const char* ascii_fingerprint; // = "036EEE9BFD6C9942B0949C5E4EFEE818";
  static const uint8_t binary_fingerprint[16]; // = {0x03,0x6E,0xEE,0x9B,0xFD,0x6C,0x99,0x42,0xB0,0x94,0x9C,0x5E,0x4E,0xFE,0xE8,0x18};

  node_table() {
  }

  virtual ~node_table() throw() {}

  std::vector<finger_entry>  finger_table;
  finger_entry predecessor;
  std::map<int32_t, _FILE>  keys_table;

  _node_table__isset __isset;

  void __set_finger_table(const std::vector<finger_entry> & val) {
    finger_table = val;
  }

  void __set_predecessor(const finger_entry& val) {
    predecessor = val;
  }

  void __set_keys_table(const std::map<int32_t, _FILE> & val) {
    keys_table = val;
  }

  bool operator == (const node_table & rhs) const
  {
    if (!(finger_table == rhs.finger_table))
      return false;
    if (!(predecessor == rhs.predecessor))
      return false;
    if (!(keys_table == rhs.keys_table))
      return false;
    return true;
  }
  bool operator != (const node_table &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const node_table & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

typedef struct _file_data__isset {
  _file_data__isset() : node(false), file(false) {}
  bool node;
  bool file;
} _file_data__isset;

class file_data {
 public:

  static const char* ascii_fingerprint; // = "B653DC64DEACE7BC45A1DCBA5EC3CA53";
  static const uint8_t binary_fingerprint[16]; // = {0xB6,0x53,0xDC,0x64,0xDE,0xAC,0xE7,0xBC,0x45,0xA1,0xDC,0xBA,0x5E,0xC3,0xCA,0x53};

  file_data() : node(0) {
  }

  virtual ~file_data() throw() {}

  int32_t node;
  _FILE file;

  _file_data__isset __isset;

  void __set_node(const int32_t val) {
    node = val;
  }

  void __set_file(const _FILE& val) {
    file = val;
  }

  bool operator == (const file_data & rhs) const
  {
    if (!(node == rhs.node))
      return false;
    if (!(file == rhs.file))
      return false;
    return true;
  }
  bool operator != (const file_data &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const file_data & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

} // namespace

#endif
