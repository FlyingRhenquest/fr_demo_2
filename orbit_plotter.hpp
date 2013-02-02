/**
 * Orbit plotter takes a filename and a time. It reads the ephemeris
 * data from the file and constructs orbits consisting of 6 hours
 * prior to and later than the specified time for each satellite
 * in the file, and emits them.
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


#include <boost/bind.hpp>
#include "coordinates.hpp"
#include "ge_object.hpp"
#include <map>
#include "sp3_factory.hpp"
#include <vector>
#include <iomanip>

#ifndef _HPP_ORBIT_PLOTTER
#define _HPP_ORBIT_PLOTTER

namespace fr {

  namespace demo {

    class orbit_plotter {
      const double six_hours = 21600.0;
      const double time_step = 60;

      typedef std::vector<fr::data::sp3_factory::sp3_pair> pair_vector;
      typedef typename pair_vector::iterator pair_vector_iterator;
      typedef std::map<std::string, pair_vector> satellite_map;

      typedef typename satellite_map::iterator satellite_map_iterator;
      satellite_map all_satellites;
      
      double time_at;
      void populate_map(const std::string &satellite_name, const fr::data::sp3_factory::sp3_pair &line)
      {
	// Only take data from 6 hours prior to six hours after the orbit

	if ((line.first > time_at - six_hours) && (line.first < time_at + six_hours)) {
	  all_satellites[satellite_name].push_back(line);
	}
      }

      // First point and second point are separated by 5 minutes of time
      // in the GPS data files. Interpolate a point every minute for points
      // 1 - 3 (0 and 4 are the real ones.) Note we only write real point 0
      // since next iterator real point 4 will now be real point 0 (and
      // we write the last point in the orbit when we reach the end.)

      void populate_points(const fr::coordinates::ecef_vel &first, const double &time_first, const fr::coordinates::ecef_vel &second, const double &time_second, std::vector<fr::coordinates::lat_long> &points)
      {
	points.push_back(fr::coordinates::converter<fr::coordinates::lat_long>()(first));

	fr::coordinates::tod_eci_vel first_eci = fr::coordinates::converter<fr::coordinates::tod_eci_vel>()(first, time_first);
	fr::coordinates::tod_eci_vel second_eci = fr::coordinates::converter<fr::coordinates::tod_eci_vel>()(second,time_second);

	for(double i = time_first + time_step; i <= time_second - time_step; i += time_step) {
	  fr::coordinates::tod_eci_vel between = first_eci.interpolate(time_first, second_eci, time_second, i);
	  points.push_back(fr::coordinates::converter<fr::coordinates::lat_long>()(between, i));
	}
	// if I push back the second one, I'll have duplicate points in my
	// orbit.
      }
      
    public:
      orbit_plotter(const std::string &filename, double time_at) : time_at(time_at)
      {
	fr::data::sp3_factory factory(filename);
	factory.data_available.connect(boost::bind(&orbit_plotter::populate_map, this, _1, _2));
	factory.process();
      }

      // Returns a REALLY BIG STRING. This is atrocious, but not really
      // what I'm trying to demonstrate
      std::string plot()
      {
	std::string retval("");
	for(satellite_map_iterator i = all_satellites.begin(); i != all_satellites.end() ; ++i) {
	  std::vector<fr::coordinates::lat_long> orbit;
	  // i->first is satellite name. i->second is a vector of sp3 pairs
	  for (pair_vector_iterator j = i->second.begin(); j != i->second.end(); ++j) {
	    pair_vector_iterator peek_next = j + 1;
	    if (peek_next == i->second.end()) {
	      fr::coordinates::lat_long last = fr::coordinates::converter<fr::coordinates::lat_long>()(j->second);
	      orbit.push_back(last);
	    } else {
	      populate_points(j->second, j->first, peek_next->second, peek_next->first, orbit);
	    }  
	  }
	  // Now that we have all the points
	  fr::data::ge_object ge_orbit(orbit);
	  ge_orbit.set_altitude_mode(true);
	  ge_orbit.set_tessellate(true);
	  std::string satname("Satellite ");
	  satname.append(i->first);
	  ge_orbit.set_name(satname);
	  ge_orbit.set_description("12 hour orbit for a GPS satellite");
	  retval.append(ge_orbit.to_string());
	}
	return retval;
      }

    };

  }

}

#endif
