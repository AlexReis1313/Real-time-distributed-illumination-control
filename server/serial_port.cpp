#include "serial_port.h"

using namespace boost::asio;
using namespace boost::system;

constexpr size_t MAX_SZ{128};

SerialCommunicator::SerialCommunicator(io_context &io,
                                       const std::string &portName)
    : serialPort(io, portName) {
  initSerialPort(portName);
}

void SerialCommunicator::initSerialPort(const std::string &portName) {
  try {
    // gets errors for no reason
    serialPort.open(portName);
    serialPort.set_option(serial_port_base::baud_rate(9600));
    serialPort.set_option(serial_port_base::character_size(8));
    serialPort.set_option(
        serial_port_base::stop_bits(serial_port_base::stop_bits::one));
    serialPort.set_option(
        serial_port_base::parity(serial_port_base::parity::none));
    std::cout << "Serial port opened successfully: " << portName << std::endl;
  } catch (const system_error &e) {
    std::cerr << "Failed to open serial port " << portName << ": " << e.what()
              << std::endl;
  }
}

std::pair<std::string, size_t>
SerialCommunicator::sendSerialnRecv(const std::string &message) {
  try {
    std::cout << "Sending to Arduino " << message << std::endl;
    write(serialPort, buffer(message));

    std::string response;
    char c;
    while (read(serialPort, buffer(&c, 1)) > 0) {
      response += c;
      if (c == '\n')
        break;
    }
    std::cout << "Received " << response << std::endl;
    return {response, response.size()};
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return {"", 0};
  }
}

std::pair<std::string, size_t>
SerialCommunicator::sendSerialnStream(const std::string &message) {
  try {
    std::cout << "Sending to Arduino " << message << std::endl;
    write(serialPort, buffer(message));

    std::string stop_stream = message;
    stop_stream[0] = 'S';
    /* std::this_thread::sleep_for(std::chrono::milliseconds(1500)); */

    std::string response;
    char c;
    //before with 4 lines worked,
    int lines = 10;
    while (read(serialPort, buffer(&c, 1)) > 0) {
      if (lines == 0) {
        std::cout << "Sending to Arduino " << stop_stream << std::endl;
        write(serialPort, buffer(stop_stream));
        lines--;
      }
      if (c == '\n')
        lines--;

      response += c;
      if (c == 'k'){
        while(c != '\n')
          read(serialPort, buffer(&c, 1));
        response += c;
        break;
      }
    }
    std::cout << "Received " << response << std::endl;
    return {response, response.size()};
  } catch (std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return {"", 0};
  }
}
