/*
===============================================================================

  FILE:  common.hpp

  CONTENTS:


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

#pragma once

#include <chrono>

namespace common {
	inline std::chrono::time_point<std::chrono::high_resolution_clock> tick() {
		return std::chrono::high_resolution_clock::now();
	}

	inline float since(const std::chrono::time_point<std::chrono::high_resolution_clock>& p) {
		using namespace std::chrono;

		auto now = high_resolution_clock::now();
		return duration_cast<duration<float> >(now - p).count();
	}
}

