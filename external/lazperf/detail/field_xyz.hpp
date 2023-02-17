/*
===============================================================================

  FILE:  field_xyz.hpp

  CONTENTS:
	XYZ fields encoder


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
#error Cannot directly include this file, this is a part of las.hpp
#endif

namespace laszip {
	namespace formats {
		// Teach packers how to pack unpack the xyz struct
		//
		template<>
		struct packers<las::xyz> {
			inline static las::xyz unpack(const char *in) {
				// blind casting will cause problems for ARM and Emscripten targets
				//
				las::xyz p;

				p.x = packers<int>::unpack(in);						in += sizeof(int);
				p.y = packers<int>::unpack(in);						in += sizeof(int);
				p.z = packers<int>::unpack(in);

				return p;
			}

			inline static void pack(const las::xyz& p, char *buffer) {
				packers<int>::pack(p.x, buffer);					buffer += sizeof(int);
				packers<int>::pack(p.y, buffer);					buffer += sizeof(int);
				packers<int>::pack(p.z, buffer);
			}
		};

		// specialize field to compress point 10
		//
		template<>
		struct field<las::xyz> {
			typedef las::xyz type;

			field() : compressor_inited_(false), decompressors_inited_(false) { }

			template<
				typename TEncoder
			>
            inline const char *compressWith(TEncoder& enc, const char *buf)
            {
				if (!compressor_inited_) {
					compressors_.init();
					compressor_inited_ = true;
				}

                las::xyz this_val = packers<las::xyz>::unpack(buf);
				if (!common_.have_last_) {
					// don't have the first data yet, just push it to our have last stuff and move on
					common_.have_last_ = true;
					common_.last_ = this_val;

					enc.getOutStream().putBytes((const unsigned char*)buf,
                        sizeof(las::xyz));

					// we are done here
					return buf + sizeof(las::xyz);
				}

				unsigned int k_bits;
				int median, diff;

				// compress x coordinate
				median = common_.last_x_diff_median5.get();
				diff = this_val.x - common_.last_.x;
				compressors_.ic_dx.compress(enc, median, diff, 0);
				common_.last_x_diff_median5.add(diff);

				// compress y coordinate
				k_bits = compressors_.ic_dx.getK();
				median = common_.last_y_diff_median5.get();
				diff = this_val.y - common_.last_.y;
				compressors_.ic_dy.compress(enc, median, diff, ( k_bits < 20 ? U32_ZERO_BIT_0(k_bits) : 20 ));
				common_.last_y_diff_median5.add(diff);

				// compress z coordinate
				k_bits = (compressors_.ic_dx.getK() + compressors_.ic_dy.getK()) / 2;
				compressors_.ic_z.compress(enc, common_.last_height, this_val.z, (k_bits < 18 ? U32_ZERO_BIT_0(k_bits) : 18));
				common_.last_height = this_val.z;

				common_.last_ = this_val;
                return buf + sizeof(las::xyz);
			}

			template<
				typename TDecoder
			>
			inline char *decompressWith(TDecoder& dec, char *buf)
            {
				if (!decompressors_inited_) {
					decompressors_.init();
					decompressors_inited_ = true;
				}

				if (!common_.have_last_) {
					// don't have the first data yet, read the whole point out of the stream
					common_.have_last_ = true;

					dec.getInStream().getBytes((unsigned char*)buf,
                        sizeof(las::xyz));

					// decode this value
					common_.last_ = packers<las::xyz>::unpack(buf);

					// we are done here
					return buf + sizeof(las::xyz);
				}

				unsigned int k_bits;
				int median, diff;

				// decompress x coordinate
				median = common_.last_x_diff_median5.get();

				diff = decompressors_.ic_dx.decompress(dec, median, 0);
				common_.last_.x += diff;
				common_.last_x_diff_median5.add(diff);

				// decompress y coordinate
				median = common_.last_y_diff_median5.get();
				k_bits = decompressors_.ic_dx.getK();
				diff = decompressors_.ic_dy.decompress(dec, median, ( k_bits < 20 ? U32_ZERO_BIT_0(k_bits) : 20 ));
				common_.last_.y += diff;
				common_.last_y_diff_median5.add(diff);

				// decompress z coordinate
				k_bits = (decompressors_.ic_dx.getK() + decompressors_.ic_dy.getK()) / 2;
				common_.last_.z = decompressors_.ic_z.decompress(dec, common_.last_height, (k_bits < 18 ? U32_ZERO_BIT_0(k_bits) : 18));
				common_.last_height = common_.last_.z;

                packers<las::xyz>::pack(common_.last_, buf);
				return buf + sizeof(las::xyz);
			}

			// All the things we need to compress a point, group them into structs
			// so we don't have too many names flying around

			// Common parts for both a compressor and decompressor go here
			struct __common {
				type last_;

				utils::streaming_median<int>    last_x_diff_median5;
				utils::streaming_median<int>    last_y_diff_median5;

				int last_height;
				bool have_last_;

				__common() :
                    last_height(0),
					have_last_(false) {
				}

				~__common() {
				}
			} common_;

			// These compressors are specific to a compressor usage, so we keep them separate here
			struct __compressors {
				compressors::integer ic_dx;
				compressors::integer ic_dy;
				compressors::integer ic_z;

				__compressors() :
					ic_dx(32, 2),
					ic_dy(32, 22),
					ic_z(32, 20) { }

				void init() {
					ic_dx.init();
					ic_dy.init();
					ic_z.init();
				}
			} compressors_;

			struct __decompressors {
				decompressors::integer ic_dx;
				decompressors::integer ic_dy;
				decompressors::integer ic_z;

				__decompressors() :
					ic_dx(32, 2),
					ic_dy(32, 22),
					ic_z(32, 20) { }

				void init() {
					ic_dx.init();
					ic_dy.init();
					ic_z.init();
				}
			} decompressors_;

			bool compressor_inited_;
			bool decompressors_inited_;
		};
	}
}
