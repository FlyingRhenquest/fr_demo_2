/**
 * A very simple socket server class. This will listen on a port
 * and when a connection is received it will kick off a thread
 * to service the connection. I could have used boost::asio or
 * something, but it seems like overkill.
 *
 * Copyright 2011 Bruce Ide
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _H_SOCKETSERVER
#define _H_SOCKETSERVER


#include <arpa/inet.h>
#include <iostream>
#include <boost/thread/thread.hpp>
#include <ext/stdio_filebuf.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/select.h>

/**
 * Usage:
 * 1) Create a class that implements () and whose constructor takes a
 *    SocketServer *owner and an int file descriptor. This will be 
 *    the service class which is spawned after a connection is received.
 * 2) Create a SocketServer with this class as a template.
 * 3) Objects of this class will be run in a separate thread,
 *    so you may need to worry about locking resources, especially
 *    if you write to any.
 */

template <typename ServiceClass>
class SocketServer {
 protected:
  enum {DEFAULT_PORT = 12345};
  int port;
  bool shutdownFlag;
  bool isReady;

  class ListenerThread {
    
    SocketServer<ServiceClass> *owner;
    enum {BACKLOG = 20, BUFFER_SIZE = 80 };

  public:

    ListenerThread(SocketServer<ServiceClass> *owner) : owner(owner)
    {
    }

    void operator()()
    {
      int retval = 0;
      int sock = socket(AF_INET, SOCK_STREAM, 0);
      if (0 > sock) {
        perror("Error creating socket");
        exit(1);
      }
      sockaddr_in addr;

      addr.sin_family = AF_INET;
      addr.sin_port = htons(owner->port);
      addr.sin_addr.s_addr = INADDR_ANY;
      
      retval = bind(sock, (sockaddr *) &addr, sizeof(sockaddr_in));
      if (0 > retval) {
        perror("Error binding to address");
        exit(1);
      }
      
      retval = listen(sock, BACKLOG);
      if (0 > retval) {
        perror("Error setting up listen");
        exit(1);
      }

      owner->isReady = true;

      while(! owner->done()) {
        fd_set set;
        timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_SET(sock, &set);
        
        char buf[BUFFER_SIZE];
        sockaddr_in incoming_address;
        socklen_t size;
        select(sock + 1, &set, NULL, NULL, &tv);
        if (FD_ISSET(sock, &set)) {
          int fdes = accept(sock, (sockaddr *) &incoming_address, &size);
          if (0 > fdes) {
            perror("Error while accepting connections");
            // Going to treat this as non-fatal
          } else {
            ServiceClass serveit(owner, fdes);
            boost::thread *thrd = new boost::thread(serveit);
            thrd->detach(); // I don't really care what it does now
          }
        }
      }
      
    }

  };

 public:

 SocketServer(int port = DEFAULT_PORT) : port(port), shutdownFlag(false), isReady(false)
  {
  }

  boost::thread *start()
  {
    ListenerThread listener(this);
    boost::thread *thrd = new boost::thread(listener);
    return thrd;
  }

  bool ready() 
  {
    return isReady;
  }

  void shutdown()
  {
    shutdownFlag = true;
  }

  bool done()
  {
    return shutdownFlag;
  }

};

#endif
