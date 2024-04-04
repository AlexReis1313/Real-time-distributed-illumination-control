#include "serial_port.h"
#include <boost/asio.hpp>
#include <boost/system/detail/error_category.hpp>
#include <iostream>
#include <string>

using std::stoi;

using namespace boost::asio;
constexpr size_t MAX_SZ{128};

class session {
  ip::tcp::socket sock;
  char rbuf[MAX_SZ] = {0}, tbuf[MAX_SZ] = {0};
  SerialCommunicator &communicator; // Reference to SerialCommunicator instance

public:
  session(io_context &io, SerialCommunicator &comm)
      : sock(io), communicator(comm) {
  } // Constructor to initialize communicator
  ip::tcp::socket &socket() { return sock; }
  void start() {
    sock.async_read_some(buffer(rbuf, MAX_SZ),
                         [this](boost::system::error_code ec, size_t sz) {
                           if (!ec) {
                             rbuf[sz] = 0;
                             std::cout << "Received " << rbuf << std::endl;
                             strncpy(tbuf, rbuf, sz);
                             // Clear the buffers before reuse
                             memset(rbuf, 0, MAX_SZ);

                             if (tbuf[0] == 's') {
                               // Start streaming from the communicator
                               auto rp = communicator.sendSerialnStream(tbuf);

                               async_write(sock, buffer(rp.first, rp.second),
                                           [this](boost::system::error_code ec,
                                                  std::size_t sz) {});
                             } else {
                               auto rp = communicator.sendSerialnRecv(tbuf);

                               async_write(sock, buffer(rp.first, sz),
                                           [this](boost::system::error_code ec,
                                                  std::size_t sz) {});
                             }
                             memset(tbuf, 0, MAX_SZ);
                             // receives message from arduino and prints to
                             // client this might take some time. I don't want
                             // to get stuck here.

                             start();
                           } else {

                             std::cout << "start error: " << ec.message()
                                       << std::endl;
                             delete this; // destroy this session object
                           }
                         } // end async_read_some lambda arg
    );                     // end async_read call
  }                        // end start()
};
class server {
  io_context &io;
  ip::tcp::acceptor acc;
  SerialCommunicator &communicator; // Reference to SerialCommunicator instance

  void start_accept() {
    std::cout << "start_accept" << std::endl;
    session *sess =
        new session{io, communicator}; // Pass communicator instance to session
    acc.async_accept(sess->socket(),
                     [this, sess](boost::system::error_code ec) {
                       if (!ec) {
                         sess->start();
                         std::cout << "Created new session" << std::endl;
                       } else {
                         std::cerr << "Accept error: " << ec.message();
                         delete sess;
                       }
                       start_accept();
                     } // end async_accept lambda arg
    );                 // end async_accept cal
  }                    // end start_accept();

public:
  server(io_service &io, unsigned short port, SerialCommunicator &comm)
      : io{io}, acc{io, ip::tcp::endpoint{ip::tcp::v4(), port}},
        communicator(comm) {
    std::cout << "Receiving at: " << acc.local_endpoint() << std::endl;
    start_accept();
  }
};

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <tcp_port> <serial_port>"
              << std::endl;
    return 1;
  }
  io_context io;
  std::string portName = argv[2];
  SerialCommunicator communicator(io, portName);
  std::cout << "Serial Port: " << portName << std::endl;
  server s{io, static_cast<unsigned short>(stoi(argv[1])), communicator};
  io.run();
  std::cout << "finished" << std::endl;
}
