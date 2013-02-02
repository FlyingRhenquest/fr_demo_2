/**
 * Uses orbit plotter to serve up a KML document through a socket.
 *
 * Copyright 2013 Bruce Ide
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <ext/stdio_filebuf.h>
#include <iostream>
#include "socket_server.hpp"
#include "orbit_plotter.hpp"
#include <string>
#include <time.h>

#ifndef _HPP_ORBIT_SERVER
#define _HPP_ORBIT_SERVER

// GCC 4.7.2 really didn't want to call the time function
// at all, for some reason.
extern "C" {
  time_t get_time()
  {
    return time((time_t *)NULL);
  }
}

namespace fr {

  namespace demo {

    class orbit_server {
      
      SocketServer<orbit_server> *owner;
      int fdes;
      
    public:
      
      static std::string filename;
      
      orbit_server(SocketServer<orbit_server> *owner, int fdes) : owner(owner), fdes(fdes)
      {	
      }

      void operator()()
      {
	std::cout << "Received new connection" << std::endl;
	std::string buffer;
	__gnu_cxx::stdio_filebuf<char> buf_in(fdes, std::ios_base::in);
	__gnu_cxx::stdio_filebuf<char> buf_out(fdes, std::ios_base::out);
	std::istream stream_in(&buf_in);
	std::ostream stream_out(&buf_out);
	getline(stream_in, buffer); // http get line from google earth
	// Plot for NOW minus three days
	time_t time_now = get_time();
	time_now -= 2*86400; // 3 days
	orbit_plotter all_orbits(filename, time_now);
	
	stream_out << "HTTP/1.0 200 OK" << std::endl;
	stream_out << "Content-Type: application/vnd.google-earth.kml+xml" << std::endl << std::endl;

	stream_out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
	stream_out << "<kml xmlns=\"http://www.opengis.net/kml/2.2\">" << std::endl;
	stream_out << "<Document>" << std::endl;
	stream_out << all_orbits.plot();
	stream_out << "</Document>" << std::endl;
	stream_out << "</kml>" << std::endl;
	stream_out.flush();
	close(fdes);
	std::cout << "Completed service for connection" << std::endl;
      }

    };

  }
}

#endif
