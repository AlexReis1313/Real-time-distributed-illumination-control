#ifndef SERIAL_COMMUNICATOR_H
#define SERIAL_COMMUNICATOR_H

#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <thread>
class SerialCommunicator {
public:
  SerialCommunicator(boost::asio::io_context &io, const std::string &portName);
  std::pair<std::string, size_t> sendSerialnRecv(const std::string &message);
  std::pair<std::string, size_t> sendSerialnStream(const std::string &message);
private:
  void initSerialPort(const std::string &portName);

  boost::asio::serial_port serialPort;
};

#endif  // SERIAL_COMMUNICATOR_H
