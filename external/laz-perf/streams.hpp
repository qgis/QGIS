/*
===============================================================================

  FILE:  streams.hpp

  CONTENTS:
    Stream abstractions

  PROGRAMMERS:

    uday.karan@gmail.com - Hobu, Inc.

  COPYRIGHT:

    (c) 2014, Uday Verma, Hobu, Inc.

    This is free software; you can redistribute and/or modify it under the
    terms of the GNU Lesser General Licence as published by the Free Software
    Foundation. See the COPYING file for more information.

    This software is distributed WITHOUT ANY WARRANTY and without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  CHANGE HISTORY:

===============================================================================
*/

#ifndef __streams_hpp__
#define __streams_hpp__

#include <algorithm>

namespace laszip {
	namespace streams {
		struct memory_stream {
			memory_stream(const char *buf, std::streamsize len) :
				buf_(buf), len_(len), offset_(0),
				is_bad_(false), is_eof_(false), last_read_count_(0) {
			}

			void read(char *into, std::streamsize size) {
				if (is_eof_) {
					is_bad_ = true;
					return;
				}

				std::streamsize to_read = (std::min)(size, len_ - offset_);
				std::copy(buf_ + offset_, buf_ + offset_ + to_read, into);
				offset_ += to_read;
				last_read_count_ = to_read;

				if (offset_ >= len_)
					is_eof_ = true;
			}

			bool eof() {
				return is_eof_;
			}

			std::streamsize gcount() {
				return last_read_count_;
			}

			bool good() {
				bool b = is_bad_;
				is_bad_ = false;
				return !b;
			}

			void clear() {
				is_bad_ = false;
				is_eof_ = false;
			}

			std::streamsize tellg() {
				return offset_;
			}

			void seekg(std::ios::pos_type p) {
				if (p >= len_)
					is_bad_ = true;
				else
					offset_ = p;
			}

			void seekg(std::ios::off_type p, std::ios_base::seekdir dir) {
				std::streamoff new_offset_ = 0;
				switch(dir) {
					case std::ios::beg: new_offset_ = p; break;
					case std::ios::end: new_offset_ = len_ + p - 1; break;
					case std::ios::cur: new_offset_ = offset_ + p; break;
                    default: break;
				}

				if (new_offset_ >= len_ || new_offset_ < 0)
					is_bad_ = true;
				else {
					is_bad_ = false;
					offset_ = new_offset_;
				}
			}



			const char *buf_;
			std::streamsize len_, offset_;
			bool is_bad_, is_eof_;
			std::streamsize last_read_count_;
		};
	}
}

#endif // __streams_hpp__
