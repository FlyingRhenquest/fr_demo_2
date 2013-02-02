/**
 * The actual application. Grab the latest NGA GPS ephemeris file from
 * http://earth-info.nga.mil/GandG/sathtml/PEexe.html, un-lharc it
 * sith lha e file (It's an EXE but that doesn't matter,) and
 * run this program with orbit_server filename.eph. Then open up
 * google earth and point it at the demo kml.
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

#include <iostream>
#include "orbit_plotter.hpp"
#include "orbit_server.hpp"
#include "socket_server.hpp"
#include "timezone_manager.hpp"


// Fix time to GMT, which is the One True Timezone. Your old timezone
// will be restored upon program exit.
fr::time::timezone_manager manager;
std::string fr::demo::orbit_server::filename("");

int main(int argc, char *argv[])
{
  if (argc != 2) {
    std::cout << "Usage: orbit_server filename" << std::endl;
    std::cout << "Where filename is a file in SP3 ephemeris format." << std::endl;
  } else {
    fr::demo::orbit_server::filename = argv[1];
    SocketServer<fr::demo::orbit_server> server(12345);
    boost::thread *server_thread = server.start();
    std::cout << "Listening for connections on port 12345" << std::endl;
    server_thread->join();
  }
}
