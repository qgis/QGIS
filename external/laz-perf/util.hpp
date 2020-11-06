/*
===============================================================================

  FILE:  util.hpp
  
  CONTENTS:
    Utility classes

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

#ifndef __util_hpp__
#define __util_hpp__

#include <array>
#include <cstdint>
#include <cstdlib>

namespace laszip {
	namespace utils {
#define ALIGN 64

		static inline void *aligned_malloc(int size) {
			void *mem = malloc(size+ALIGN+sizeof(void*));
			void **ptr = (void**)(( ((uintptr_t)mem)+ALIGN+sizeof(void*) ) & ~(uintptr_t)(ALIGN-1) );
			ptr[-1] = mem;
			return ptr;
		}

		static inline void aligned_free(void *ptr) {
			free(((void**)ptr)[-1]);
		}

		template<
			typename T
		>
		class streaming_median {
		public:
			std::array<T, 5> values;
			BOOL high;

			void init() {
				values.fill(T(0));
				high = true;
			}

			inline void add(const T& v) {
				if (high) {
					if (v < values[2]) {
						values[4] = values[3];
						values[3] = values[2];
						if (v < values[0]) {
							values[2] = values[1];
							values[1] = values[0];
							values[0] = v;
						}
						else if (v < values[1]) {
							values[2] = values[1];
							values[1] = v;
						}
						else {
							values[2] = v;
						}
					}
					else {
						if (v < values[3]) {
							values[4] = values[3];
							values[3] = v;
						}
						else {
							values[4] = v;
						}
						high = false;
					}
				}
				else {
					if (values[2] < v) {
						values[0] = values[1];
						values[1] = values[2];
						if (values[4] < v) {
							values[2] = values[3];
							values[3] = values[4];
							values[4] = v;
						}
						else if (values[3] < v) {
							values[2] = values[3];
							values[3] = v;
						}
						else {
							values[2] = v;
						}
					}
					else {
						if (values[1] < v) {
							values[0] = values[1];
							values[1] = v;
						}
						else {
							values[0] = v;
						}
						high = true;
					}
				}
			}

			inline T get() const {
				return values[2];
			}

			streaming_median() {
				init();
			}
		};

		// for LAS files with the return (r) and the number (n) of
		// returns field correctly populated the mapping should really
		// be only the following.
		//  { 15, 15, 15, 15, 15, 15, 15, 15 },
		//  { 15,  0, 15, 15, 15, 15, 15, 15 },
		//  { 15,  1,  2, 15, 15, 15, 15, 15 },
		//  { 15,  3,  4,  5, 15, 15, 15, 15 },
		//  { 15,  6,  7,  8,  9, 15, 15, 15 },
		//  { 15, 10, 11, 12, 13, 14, 15, 15 },
		//  { 15, 15, 15, 15, 15, 15, 15, 15 },
		//  { 15, 15, 15, 15, 15, 15, 15, 15 }
		// however, some files start the numbering of r and n with 0,
		// only have return counts r, or only have number of return
		// counts n, or mix up the position of r and n. we therefore
		// "complete" the table to also map those "undesired" r & n
		// combinations to different contexts
		const unsigned char number_return_map[8][8] = 
		{
		  { 15, 14, 13, 12, 11, 10,  9,  8 },
		  { 14,  0,  1,  3,  6, 10, 10,  9 },
		  { 13,  1,  2,  4,  7, 11, 11, 10 },
		  { 12,  3,  4,  5,  8, 12, 12, 11 },
		  { 11,  6,  7,  8,  9, 13, 13, 12 },
		  { 10, 10, 11, 12, 13, 14, 14, 13 },
		  {  9, 10, 11, 12, 13, 14, 15, 14 },
		  {  8,  9, 10, 11, 12, 13, 14, 15 }
		};

		// for LAS files with the return (r) and the number (n) of
		// returns field correctly populated the mapping should really
		// be only the following.
		//  {  0,  7,  7,  7,  7,  7,  7,  7 },
		//  {  7,  0,  7,  7,  7,  7,  7,  7 },
		//  {  7,  1,  0,  7,  7,  7,  7,  7 },
		//  {  7,  2,  1,  0,  7,  7,  7,  7 },
		//  {  7,  3,  2,  1,  0,  7,  7,  7 },
		//  {  7,  4,  3,  2,  1,  0,  7,  7 },
		//  {  7,  5,  4,  3,  2,  1,  0,  7 },
		//  {  7,  6,  5,  4,  3,  2,  1,  0 }
		// however, some files start the numbering of r and n with 0,
		// only have return counts r, or only have number of return
		// counts n, or mix up the position of r and n. we therefore
		// "complete" the table to also map those "undesired" r & n
		// combinations to different contexts
		const unsigned char number_return_level[8][8] = 
		{
		  {  0,  1,  2,  3,  4,  5,  6,  7 },
		  {  1,  0,  1,  2,  3,  4,  5,  6 },
		  {  2,  1,  0,  1,  2,  3,  4,  5 },
		  {  3,  2,  1,  0,  1,  2,  3,  4 },
		  {  4,  3,  2,  1,  0,  1,  2,  3 },
		  {  5,  4,  3,  2,  1,  0,  1,  2 },
		  {  6,  5,  4,  3,  2,  1,  0,  1 },
		  {  7,  6,  5,  4,  3,  2,  1,  0 }
		};
	}
}

#endif // __util_hpp__
