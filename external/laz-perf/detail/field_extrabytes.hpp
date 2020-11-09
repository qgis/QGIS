/*
===============================================================================

  FILE:  field_extrabytes.hpp
  
  CONTENTS:
    

  PROGRAMMERS:

    martin.isenburg@rapidlasso.com  -  http://rapidlasso.com
    uday.karan@gmail.com - Hobu, Inc.
    andrew.bell.ia@gmail.com - Hobu Inc.
  
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
#error Cannot directly include this file, this is a part of las.hpp
#endif

#include <deque>

namespace laszip {
	namespace formats {

		template<>
		struct field<las::extrabytes> {
			typedef las::extrabytes type;

            size_t count_;
            bool have_last_;
            std::vector<uint8_t> lasts_;
            std::vector<uint8_t> diffs_;
            std::deque<models::arithmetic> models_;

			field(size_t count) :
                count_(count), have_last_(false), lasts_(count), diffs_(count),
                models_(count, models::arithmetic(256))
            {}

			template<typename TEncoder>
			inline const char *compressWith(TEncoder& enc, const char *buf)
            {
                auto li = lasts_.begin();
                auto di = diffs_.begin();
                while (di != diffs_.end())
                {
                    *di = *buf - *li;
                    *li = *buf;
                    di++; buf++; li++;
                }

                if (!have_last_)
                {
                    enc.getOutStream().putBytes(lasts_.data(), count_);
                    have_last_ = true;
                }
                else
                {
                    di = diffs_.begin();
                    auto mi = models_.begin();
                    while (di != diffs_.end())
                        enc.encodeSymbol(*mi++, *di++);
                }
                return buf;
            }

            template<typename TDecoder>
            inline char *decompressWith(TDecoder& dec, char *buf)
            {
                if (!have_last_)
                {
                    dec.getInStream().getBytes((unsigned char *)buf, count_);
                    std::copy(buf, buf + count_, lasts_.data());
                    have_last_ = true;
                    return buf + count_;
                }
                // Use the diff vector for our current values.
                auto& curs = diffs_;
                auto ci = curs.begin();
                auto li = lasts_.begin();
                auto mi = models_.begin();
                while (li != lasts_.end())
                {
                    *ci = u8_fold(*li + dec.decodeSymbol(*mi));
                    *li = *buf = *ci;
                    li++; buf++; ci++; mi++;
                }
                return buf;
            }
        };
	}
}
