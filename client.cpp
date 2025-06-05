#include "message.hpp"
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
/*
Connects to the server over TCP using Boost.Asio
Can send messages typed by the user to the server
Can receive messages from the server asynchronously
Uses a separate thread to handle user input and sending
Now uses Message header protocol instead of newline protocol
*/
using boost::asio::ip::tcp;

tcp::socket* g_socket;
boost::asio::io_context* g_io_context;
Message g_readMessage; 

// Forward declarations
void read_header();
void read_body();
void on_header_read(boost::system::error_code ec, std::size_t length);
void on_body_read(boost::system::error_code ec, std::size_t length);
void on_message_written(boost::system::error_code ec, std::size_t length);

// This handles reading the 4-byte header first
void read_header() {
    boost::asio::async_read(*g_socket,
        boost::asio::buffer(g_readMessage.data, Message::header),
        on_header_read);
}

// Callback function for when header is read
void on_header_read(boost::system::error_code ec, std::size_t length) {
    if (!ec) {
        // Try to decode the header to get body length
        if (g_readMessage.decodeHeader()) {
            read_body();  // Header decoded successfully, now read body
        } else {
            std::cout << "Failed to decode message header" << std::endl;
        }
    } else {
        std::cout << "Header read error: " << ec.message() << std::endl;
    }
}

// This reads the message body based on the decoded header length
void read_body() {
    boost::asio::async_read(*g_socket,
        boost::asio::buffer(g_readMessage.data + Message::header, g_readMessage.getBodyLength()),
        on_body_read);
}

// Callback function for when body is read
void on_body_read(boost::system::error_code ec, std::size_t length) {
    if (!ec) {
        // Extract and display the message body
        std::string received_message = g_readMessage.getBody();
        std::cout << "Server: " << received_message << std::endl;
        
        // Keep listening for more messages recursively
        read_header();
    } else {
        std::cout << "Body read error: " << ec.message() << std::endl;
    }
}

// Function to send a message using the Message protocol
void send_message(const std::string& user_input) {
    // Create a Message object from user input (this encodes header automatically)
    Message* writeMessage = new Message(user_input);
    
    // Send the complete message (header + body)
    boost::asio::async_write(*g_socket,
        boost::asio::buffer(writeMessage->data, Message::header + writeMessage->getBodyLength()),
        [writeMessage](boost::system::error_code ec, std::size_t length) {
            on_message_written(ec, length);
            delete writeMessage;  // Clean up the dynamically allocated message
        });
}

// Callback function for when message is written
void on_message_written(boost::system::error_code ec, std::size_t length) {
    if (ec) {
        std::cout << "Write error: " << ec.message() << std::endl;
    }
}

// Function to handle user input in separate thread
void handle_user_input() {
    while (true) {
        std::string data;
        std::cout << "Enter message: ";
        std::getline(std::cin, data);
        
        // Post the send operation to the io_context thread
        boost::asio::post(*g_io_context, [data]() {
            send_message(data);
        });
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Provide port as second argument" << std::endl;
        return 1;
    }
    
    boost::asio::io_context io_context;
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);
    
    g_socket = &socket;
    g_io_context = &io_context;
    
    try {
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", argv[1]));
        std::cout << "Connected to server on port " << argv[1] << std::endl;
        
        read_header();
        
        /*
        We still need two threads for the same reason:
        THREAD 1 (Main thread):
        - Runs io_context.run()
        - Handles read_header(), read_body() callbacks
        - Processes incoming messages from server using Message protocol
        
        THREAD 2 (Input thread):  
        - Blocks on std::getline() waiting for user input
        - Posts send_message() operations to io_context
        - Allows user to type while still receiving messages
        
        The difference now is we use Message protocol instead of newline protocol:
        - Messages have 4-byte headers indicating body length
        - We read exact number of bytes instead of reading until newline
        - More robust and can handle binary data or messages with newlines
        */
        std::thread input_thread(handle_user_input);
        
        // Run the io_context to handle async operations
        io_context.run();
        input_thread.join();
        
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    return 0;
}
