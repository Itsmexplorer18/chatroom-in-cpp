#ifndef CHATROOM_HPP
#define CHATROOM_HPP

#include "message.hpp"
#include <deque>
#include <set>
#include <memory>
#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Participant {
public: 
    virtual void deliver(Message& message) = 0;
    virtual void write(Message &message) = 0;
    virtual ~Participant() = default;
};

typedef std::shared_ptr<Participant> ParticipantPointer;

class Room {
public:
    void join(ParticipantPointer participant);
    void leave(ParticipantPointer participant);
    void deliver(ParticipantPointer participantPointer, Message &message);

private:
    std::deque<Message> messageQueue;
    enum {maxParticipants = 100};
    std::set<ParticipantPointer> participants;
};

class Session : public Participant, public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket s, Room &room);
    void start();
    void deliver(Message& message) override;
    void write(Message &message) override;
    
    // Provide access to socket for acceptor
    tcp::socket& socket() { return clientSocket; }

private:
    void read_header();
    void read_body();
    void do_write_();
    
    tcp::socket clientSocket;
    Room& room;
    Message readMessage_;
    std::deque<Message> writeQueue_;
};

#endif // CHATROOM_HPP
