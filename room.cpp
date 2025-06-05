#include "chatroom.hpp"

void Room::join(ParticipantPointer participant) {
    this->participants.insert(participant);
}

void Room::leave(ParticipantPointer participant) {
    this->participants.erase(participant);
}

void Room::deliver(ParticipantPointer sender, Message &message) {
    messageQueue.push_back(message);
    while (!messageQueue.empty()) {
        Message msg = messageQueue.front();
        messageQueue.pop_front(); 
        
        for (ParticipantPointer participant : participants) {
            if (sender != participant) {
                participant->write(msg);
            }
        }
    }
}

Session::Session(tcp::socket s, Room& r) : clientSocket(std::move(s)), room(r) {}

void Session::start() {
    room.join(shared_from_this());
    read_header();
}

void Session::read_header() {
    auto self(shared_from_this());
    boost::asio::async_read(clientSocket,
        boost::asio::buffer(readMessage_.data, Message::header),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec && readMessage_.decodeHeader()) {
                read_body();
            } else {
                room.leave(shared_from_this());
                if (ec == boost::asio::error::eof) {
                    std::cout << "Connection closed by peer" << std::endl;
                } else {
                    std::cout << "Header read error: " << ec.message() << std::endl;
                }
            }
        });
}

void Session::read_body() {
    auto self(shared_from_this());
    boost::asio::async_read(clientSocket,
        boost::asio::buffer(readMessage_.data + Message::header, readMessage_.getBodyLength()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                std::cout << "Received: " << readMessage_.getBody() << std::endl;
                deliver(readMessage_);
                read_header(); // Continue reading next message
            } else {
                room.leave(shared_from_this());
                std::cout << "Body read error: " << ec.message() << std::endl;
            }
        });
}

void Session::deliver(Message& message) {
    room.deliver(shared_from_this(), message);
}

void Session::write(Message &message) {
    bool do_write = writeQueue_.empty();
    writeQueue_.push_back(message);
    if (do_write) {
        do_write_();
    }
}

void Session::do_write_() {
    auto self(shared_from_this());
    boost::asio::async_write(clientSocket,
        boost::asio::buffer(writeQueue_.front().data, 
                           Message::header + writeQueue_.front().getBodyLength()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                writeQueue_.pop_front();
                if (!writeQueue_.empty()) {
                    do_write_();
                }
            } else {
                room.leave(shared_from_this());
                std::cout << "Write error: " << ec.message() << std::endl;
            }
        });
}

void accept_connection(boost::asio::io_context &io, tcp::acceptor &acceptor, Room &room) {
    auto new_session = std::make_shared<Session>(tcp::socket(io), room);
    acceptor.async_accept(new_session->socket(),
        [&io, &acceptor, &room, new_session](boost::system::error_code ec) {
            if (!ec) {
                new_session->start();
            }
            accept_connection(io, acceptor, room);
        });
}

int main(int argc, char *argv[]) {
    try {
        if (argc < 2) {
            std::cerr << "Usage: server <port>\n";
            return 1;
        }
        
        Room room;
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), std::atoi(argv[1])));
        
        accept_connection(io_context, acceptor, room);
        io_context.run();
        
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
