# Chatroom Application

## Overview
This is a simple C++ chatroom application that allows multiple users to connect and communicate with each other in real-time over a TCP/IP network. The application consists of a server that manages the chatroom and clients that connect to it.

## What is Boost.Asio?

**Boost.Asio** is a cross-platform C++ library for network and low-level I/O programming. It provides a consistent asynchronous model using `io_context`, sockets, timers, and more. The core strength of Boost.Asio is its support for asynchronous operations using callbacks or coroutines, which enables highly scalable and responsive applications like chat servers, without blocking threads unnecessarily.

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

#### Key Classes:
- **`Participant`** (interface):
  - Abstract base for clients connected to the server.
  - Requires `deliver()` and `write()` methods.

- **`Room`**:
  - Manages a set of connected participants.
  - Broadcasts messages to all participants.
  - Maintains a message queue with a limited history.

- **`Session`**:
  - Represents a single connected client session.
  - Inherits from `Participant`.
  - Handles socket I/O using `async_read` and `async_write`.
  - Adds/removes itself from the room upon connect/disconnect.
  - Receives and sends `Message` objects.
### How It Works

1. **Server Side** :
   - Accepts incoming connections.
   - Creates a `Session` for each client.
   - Adds the `Session` to the `Room`.

2. **Client Side**:
   - Connects to the server using IP and port.
   - Continuously reads messages asynchronously.
   - Sends user input messages to the server in a separate thread.
   - Why Two Threads Are Necessary:
        The dual-thread architecture solves a fundamental blocking I/O problem:
        std::getline() is synchronous and blocking - there's no async version in standard C++
        If everything ran on one thread, the client would freeze while waiting for user input
        Incoming messages would pile up unread until the user finishes typing
        The separate threads ensure network operations continue regardless of user input state

3. **Communication Flow**:
   - Client types a message and sends it.
   - Server receives it in the client's session.
   - Server delivers the message to all other participants via the `Room`.

## Network Communication
- Built using Boost.Asio library for asynchronous network I/O
- Uses TCP/IP for reliable message delivery
- Implements a simple message protocol with fixed-size headers

## Usage
Compile the server and client applications separately and run them as follows:
1. Start the server with the desired port number
2. Connect multiple clients using the server's IP address and port

##
