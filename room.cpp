//understanding the server functions
void accept_connection(tcp::acceptor &acceptor, Room &room) {
    acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket) {
        if(!ec) {
            std::shared_ptr<Session> session = std::make_shared<Session>(std::move(socket), room);
            session->start();
        }
        // Recursively accept the next connection
        accept_connection(acceptor, room);
    });
}

int main(int argc, char *argv[]) {
    try {
        if(argc < 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }
        
        Room room;
        boost::asio::io_context io_context;
        
        // Create acceptor and bind to the specified port
        int port = std::atoi(argv[1]);
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
        
        // Start accepting connections
        accept_connection(acceptor, room);
        
        // Run the event loop
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
