/*
===============================================================================

  FILE:  las.hpp

  CONTENTS:
    Point formats for LAS

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.

  COPYRIGHT:

    (c) 2007-2014, martin isenburg, rapidlasso - tools to catch reality
    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

#ifndef __las_hpp__
#define __las_hpp__

#include "formats.hpp"
#include "model.hpp"
#include "compressor.hpp"
#include "util.hpp"

namespace laszip {
	namespace formats {
		namespace las {
#pragma pack(push, 1)
			struct point10 {
				int x;
				int y;
				int z;
				unsigned short intensity;
				unsigned char return_number : 3;
				unsigned char number_of_returns_of_given_pulse : 3;
				unsigned char scan_direction_flag : 1;
				unsigned char edge_of_flight_line : 1;
				unsigned char classification;
				char scan_angle_rank;
				unsigned char user_data;
				unsigned short point_source_ID;

                point10() : x(0), y(0), intensity(0), return_number(0),
                    number_of_returns_of_given_pulse(0), scan_direction_flag(0),
                    edge_of_flight_line(0), classification(0),
                    scan_angle_rank(0), user_data(0), point_source_ID(0)
                {}
			};

			struct gpstime {
				int64_t value;

				gpstime() : value(0) {}
				gpstime(int64_t v) : value(v) {}
			};

			struct rgb {
				unsigned short r, g, b;

				rgb(): r(0), g(0), b(0) {}
				rgb(unsigned short _r, unsigned short _g, unsigned short _b) :
					r(_r), g(_g), b(_b) {}
			};

            // just the XYZ fields out of the POINT10 struct
			struct xyz {
				int x, y, z;

                xyz() : x(0), y(0), z(0)
                {}
            };

            struct extrabytes : public std::vector<uint8_t>
            {};
#pragma pack(pop)
		}
	}
}

#include "detail/field_extrabytes.hpp"
#include "detail/field_point10.hpp"
#include "detail/field_gpstime.hpp"
#include "detail/field_rgb.hpp"
#include "detail/field_xyz.hpp"

#endif // __las_hpp__
