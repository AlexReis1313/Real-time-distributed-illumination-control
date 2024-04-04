#include <boost/asio.hpp>
#include <iostream>
using namespace boost::asio;

void consoleInputThread(std::string& input_str) {
     std::getline(std::cin, input_str);
}

void streamThread(std::string& stream_msg, ip::tcp::socket& connection) {
  constexpr int MAX_SZ{1028};
  char buf[MAX_SZ]{0};
  write(connection, buffer(stream_msg));
  int n = connection.read_some(buffer(buf, MAX_SZ));
  if (n < MAX_SZ)
    buf[n] = 0;  // string terminator
  std::cout << buf << std::endl;
}
int main(int argc, char *argv[]) {
  io_context io;
  boost::system::error_code ec;
  constexpr int MAX_SZ{1028};
  char buf[MAX_SZ]{0};
  bool stream_routine = false;
  std::string stream_msg;
  std::string input_str;
  ip::tcp::resolver res{io};  // find endpoints from address
  ip::tcp::socket connection{io};
  connect(connection, res.resolve(argv[1], argv[2]), ec);
  if (ec) {
    std::cout << "Cannot connect" << std::endl;
    return 1;
  }
  while (true) {
    if (std::cin.peek() != std::char_traits<char>::eof()) {
        consoleInputThread(input_str);
    }
    /* std::thread input_thread(consoleInputThread, std::ref(input_str)); */
    /* input_thread.join(); */
    if (!input_str.empty()){
      if(input_str.substr(0, 4) == "s l " || input_str.substr(0, 4) == "s d "){
        stream_msg = input_str;
        std::cout << "Starting streaming "<< stream_msg << std::endl;
        stream_routine = true;
      }
      if(input_str[0] == 'S'){
        std::cout << "Ending streaming"<< std::endl;
        stream_routine = false;
      }
      if(!stream_routine){
        write(connection, buffer(input_str));
        int n = connection.read_some(buffer(buf, MAX_SZ));
        if (n < MAX_SZ)
          buf[n] = 0;  // string terminator
        //received buff
        std::cout << buf << std::endl;
      }
      input_str.clear();
    }

    if(stream_routine){
      /* std::cout << "Streaming "<< stream_msg << std::endl; */
      while(true){
      streamThread(stream_msg, connection);
      }
      std::cout << "streaming ended"<< stream_msg <<std::endl;
      stream_routine = false;
    }
  }
}
