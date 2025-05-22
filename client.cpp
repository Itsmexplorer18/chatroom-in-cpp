#include "message.hpp"
#include <iostream>
#include <boost/asio.hpp>
/*
Connects to the server over TCP using Boost.Asio
Can send messages typed by the user to the server
Can receive messages from the server asynchronously
Uses a separate thread to handle user input and sending
 */
using boost::asio::ip::tcp;

//this handles the incoming messages from the server
void async_read(tcp::socket &socket) {
    auto buffer = std::make_shared<boost::asio::streambuf>();
    boost::asio::async_read_until(socket, *buffer, "\n",
        [&socket, buffer](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::istream is(buffer.get());
                std::string received;
                std::getline(is, received);
                std::cout << "Server: " << received << std::endl;
                async_read(socket);  //keep listening for messages recursively "what other clients are sending"
            }
        }
    );
}

int main(int argc, char* argv[]){
    if(argc < 2){
        std::cerr << "Provide port too as second argument" << std::endl;
        return 1;
    }
    
    boost::asio::io_context io_context;
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);
    //This synchronously connects to the server running on localhost:port
    boost::asio::connect(socket, resolver.resolve("127.0.0.1", argv[1]));//-----> accpeted by the server

    async_read(socket);

    /*
    a new thread is needed for handling the input from the user 
    why do we need two threads??
    THREAD 1 (Main thread):
- Runs io_context.run()
- Handles async_read() callbacks
- Processes incoming messages from server
THREAD 2 (Input thread):  
- Blocks on std::getline() waiting for user input
- Posts write operations to io_context
- Allows user to type while still receiving messages
Without separate threads:
- If we used getline() in main thread → couldn't receive messages while typing
        */
    std::thread t([&io_context, &socket]() {
        while (true) {
            std::string data;
            std::cout << "Enter message: ";
            std::getline(std::cin, data);
            data += "\n";

            boost::asio::post(io_context, [&, data]() {
                boost::asio::write(socket, boost::asio::buffer(data));
            });
        }
    });

    io_context.run();
    t.join();

    return 0;
}
