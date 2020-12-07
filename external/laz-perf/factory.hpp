/*
===============================================================================

  FILE:  factory.hpp
  
  CONTENTS:
    Factory to create dynamic compressors and decompressors

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


#ifndef __factory_hpp__
#define __factory_hpp__

#include "formats.hpp"
#include "excepts.hpp"
#include "las.hpp"

#include <sstream>

namespace laszip {
	namespace factory {
		struct record_item {
			enum {
                BYTE = 0,
				POINT10 = 6,
				GPSTIME = 7,
				RGB12 = 8
			};

			int type, size, version;
			record_item(int t, int s, int v) :
				type(t), size(s), version(v) {}

            bool operator == (const record_item& other) const
            {
                return (type == other.type &&
                    version == other.version &&
                    size == other.size);
            }

            bool operator != (const record_item& other) const
            {
                return !(*this == other);
            }

            static const record_item& point()
            {
                static record_item item(POINT10, 20, 2);
                return item;
            }

            static const record_item& gpstime()
            {
                static record_item item(GPSTIME, 8, 2);
                return item;
            }

            static const record_item& rgb()
            {
                static record_item item(RGB12, 6, 2);
                return item;
            }

            static const record_item eb(size_t count)
            {
                return record_item(BYTE, count, 2);
            }
		};

		struct record_schema {
			record_schema() : records() { }

			void push(const record_item& item) {
				records.push_back(item);
			}

            // This is backward compatible support. Remove.
#ifdef _WIN32
            __declspec(deprecated) void push(int t)
#else
            void push(int t) __attribute__ ((deprecated))
#endif
            {
                if (t == record_item::POINT10)
                    push(record_item::point());
                else if (t == record_item::GPSTIME)
                    push(record_item::gpstime());
                else if (t == record_item::RGB12)
                    push(record_item::rgb());
                else
                    throw unknown_schema_type();
            }

			record_schema& operator () (const record_item& i) {
				push(i);
				return *this;
			}

			int size_in_bytes() const {
				int sum = 0;
				for (auto i : records)
					sum += i.size;

				return sum;
			}

            int format() const
            {
                size_t count = records.size();
                if (count == 0)
                    return -1;

                // Ignore extrabytes record that should be at the end.
                if (extrabytes())
                    count--;

                if (count == 0 || records[0] != record_item::point())
                    return -1;

                if (count == 1)
                    return 0;
                if (count == 2)
                {
                    if (records[1] == record_item::gpstime())
                        return 1;
                    else if (records[1] == record_item::rgb())
                        return 2;
                }
                if (count == 3 && records[1] == record_item::gpstime() &&
                    records[2] == record_item::rgb())
                    return 3;
                return -1;
            }

            size_t extrabytes() const
            {
                if (records.size())
                {
                    auto ri = records.rbegin();
                    if (ri->type == record_item::BYTE && ri->version == 2)
                        return ri->size;
                }
                return 0;
            }

			std::vector<record_item> records;
		};


		template<typename TDecoder>
		formats::dynamic_decompressor::ptr build_decompressor(TDecoder& decoder,
            const record_schema& schema)
        {
			using namespace formats;

            int format = schema.format();
			if (format == -1)
                throw unknown_schema_type();
            size_t ebCount = schema.extrabytes();
            if (ebCount)
            {
                auto decompressor = make_dynamic_decompressor(decoder);
                decompressor->template add_field<las::point10>();
                if (format == 1 || format == 3)
                    decompressor->template add_field<las::gpstime>();
                if (format == 2 || format == 3)
                    decompressor->template add_field<las::rgb>();
                decompressor->add_field(field<las::extrabytes>(ebCount));
                return decompressor;
            }
            else
            {
                switch (format)
                {
                case 0:
				    return make_dynamic_decompressor(decoder,
						new formats::record_decompressor<
                            field<las::point10>>());
                case 1:
				    return make_dynamic_decompressor(decoder,
						new formats::record_decompressor<
							field<las::point10>,
							field<las::gpstime>>());
                case 2:
				    return make_dynamic_decompressor(decoder,
						new formats::record_decompressor<
							field<las::point10>,
							field<las::rgb>>());
                case 3:
				    return make_dynamic_decompressor(decoder,
						new formats::record_decompressor<
							field<las::point10>,
							field<las::gpstime>,
							field<las::rgb>>());
                }
            }
            return dynamic_decompressor::ptr();
		}

        template<typename TEncoder>
        formats::dynamic_compressor::ptr build_compressor(TEncoder& encoder,
            const record_schema& schema)
        {
            using namespace formats;

            int format = schema.format();
            if (format == -1)
                throw unknown_schema_type();
            size_t ebCount = schema.extrabytes();
            if (ebCount)
            {
                auto compressor = make_dynamic_compressor(encoder);
                compressor->template add_field<las::point10>();
                if (format == 1 || format == 3)
                    compressor->template add_field<las::gpstime>();
                if (format == 2 || format == 3)
                    compressor->template add_field<las::rgb>();
                compressor->add_field(field<las::extrabytes>(ebCount));
                return compressor;
            }
            else
            {
                switch (format)
                {
                case 0:
                    return make_dynamic_compressor(encoder,
                        new formats::record_compressor<
                            field<las::point10>>());
                case 1:
                    return make_dynamic_compressor(encoder,
                        new formats::record_compressor<
                            field<las::point10>,
                            field<las::gpstime>>());
                case 2:
                    return make_dynamic_compressor(encoder,
                        new formats::record_compressor<
                            field<las::point10>,
                            field<las::rgb>>());
                case 3:
                    return make_dynamic_compressor(encoder,
                        new formats::record_compressor<
                            field<las::point10>,
                            field<las::gpstime>,
                            field<las::rgb>>());
                }
            }
            return dynamic_compressor::ptr();
        }

    } // namespace factory
} // namespace laszip

#endif // __factory_hpp__
