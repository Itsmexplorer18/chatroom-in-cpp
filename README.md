# Chatroom Application

## Overview
This is a simple C++ chatroom application that allows multiple users to connect and communicate with each other in real-time over a TCP/IP network. The application consists of a server that manages the chatroom and clients that connect to it.

## Components

### Message (message.hpp)
- Defines the message format used for communication
- Handles message encoding/decoding with fixed header size
- Supports messages up to 512 bytes

### Room (room.hpp)
- Manages the chatroom participants
- Handles joining and leaving of participants
- Facilitates message delivery among connected participants
- Maintains a queue of recent messages

### Session (room.hpp)
- Represents a connection with a client
- Inherits from Participant interface
- Handles asynchronous reading and writing of messages
- Manages the client socket communication

### Client (client.cpp)
- Connects to the server using boost::asio
- Provides a user interface for sending messages
- Receives and displays messages from other participants

## How It Works
1. The server starts and listens for incoming connections
2. Clients connect to the server using the provided IP and port
3. When a client connects, a new Session is created to handle their communication
4. The Session joins the Room, making it part of the chatroom
5. Messages sent by clients are distributed to all connected participants
6. Asynchronous I/O operations ensure efficient handling of multiple connections

## Network Communication
- Built using Boost.Asio library for asynchronous network I/O
- Uses TCP/IP for reliable message delivery
- Implements a simple message protocol with fixed-size headers

## Usage
Compile the server and client applications separately and run them as follows:
1. Start the server with the desired port number
2. Connect multiple clients using the server's IP address and port
3. Start chatting across the network
