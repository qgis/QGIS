/*
===============================================================================

  FILE:  field_point10.hpp

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

#ifndef __las_hpp__
#error Cannot directly include this file, this is a part of las.hpp
#endif

namespace laszip {
	namespace formats {
		namespace detail {
			inline int changed_values(const las::point10& this_val, const las::point10& last, unsigned short last_intensity) {
				// This logic here constructs a 5-bit changed value which is basically a bit map of what has changed
				// since the last point, not considering the x, y and z values
				int bitfields_changed = (
					(last.return_number ^ this_val.return_number) |
					(last.number_of_returns_of_given_pulse ^ this_val.number_of_returns_of_given_pulse) |
					(last.scan_direction_flag ^ this_val.scan_direction_flag) |
					(last.edge_of_flight_line ^ this_val.edge_of_flight_line)) != 0;

				// last intensity is not checked with last point, but the passed in
				// last intensity value
				int intensity_changed =
					(last_intensity ^ this_val.intensity) != 0;

				int classification_changed =
					(last.classification ^ this_val.classification) != 0;

				int scan_angle_rank_changed =
					(last.scan_angle_rank ^ this_val.scan_angle_rank) != 0;

				int user_data_changed =
					(last.user_data ^ this_val.user_data) != 0;

				int point_source_changed =
					(last.point_source_ID ^ this_val.point_source_ID) != 0;

				return
					(bitfields_changed << 5) |
					(intensity_changed << 4) |
					(classification_changed << 3) |
					(scan_angle_rank_changed << 2) |
					(user_data_changed << 1) |
					(point_source_changed);
			}

			inline unsigned char bitfields_to_char(const las::point10& p) {
				unsigned char a = p.return_number,
							  b = p.number_of_returns_of_given_pulse,
							  c = p.scan_direction_flag,
							  d = p.edge_of_flight_line;

				return
					((d & 0x1) << 7) |
					((c & 0x1) << 6) |
					((b & 0x7) << 3) |
					(a & 0x7);
			}

			inline void char_to_bitfields(unsigned char d, las::point10& p) {
				p.return_number = d & 0x7;
				p.number_of_returns_of_given_pulse = (d >> 3) & 0x7;
				p.scan_direction_flag = (d >> 6) & 0x1;
				p.edge_of_flight_line = (d >> 7) & 0x1;
			}
		}

		// Teach packers how to pack unpack the point10 struct
		//
		template<>
		struct packers<las::point10> {
			inline static las::point10 unpack(const char *in) {
				// blind casting will cause problems for ARM and Emscripten targets
				//
				las::point10 p;

				p.x = packers<int>::unpack(in);						in += sizeof(int);
				p.y = packers<int>::unpack(in);						in += sizeof(int);
				p.z = packers<int>::unpack(in);						in += sizeof(int);
				p.intensity = packers<unsigned short>::unpack(in);  in += sizeof(unsigned short);

				unsigned char d =
					packers<unsigned char>::unpack(in);				in += sizeof(unsigned char);

				// unpack read bitfields into p
				detail::char_to_bitfields(d, p);

				p.classification =
                    packers<unsigned char>::unpack(in);             in += sizeof(unsigned char);

				p.scan_angle_rank = packers<char>::unpack(in);		in += sizeof(char);
				p.user_data = packers<char>::unpack(in);			in += sizeof(char);
				p.point_source_ID =
					packers<unsigned short>::unpack(in);

				return p;
			}

			inline static void pack(const las::point10& p, char *buffer) {
				packers<int>::pack(p.x, buffer);					buffer += sizeof(int);
				packers<int>::pack(p.y, buffer);					buffer += sizeof(int);
				packers<int>::pack(p.z, buffer);					buffer += sizeof(int);

				packers<unsigned short>::pack(p.intensity, buffer); buffer += sizeof(unsigned short);

				// pack bitfields into a char
				unsigned char e = detail::bitfields_to_char(p);

				packers<unsigned char>::pack(e, buffer);			buffer += sizeof(unsigned char);
				packers<unsigned char>::pack(
                    p.classification, buffer);                      buffer += sizeof(unsigned char);

				packers<char>::pack(p.scan_angle_rank, buffer);		buffer += sizeof(char);
				packers<char>::pack(p.user_data, buffer);			buffer += sizeof(char);
				packers<unsigned short>::pack(
						p.point_source_ID, buffer);

			}
		};

		// specialize field to compress point 10
		//
		template<>
		struct field<las::point10> {
			typedef las::point10 type;

			field() : compressor_inited_(false), decompressors_inited_(false) { }

			template<
				typename TEncoder
			>
			inline const char *compressWith(TEncoder& enc, const char *buf)
            {

                las::point10 this_val;
                this_val = packers<las::point10>::unpack(buf);

				if (!compressor_inited_) {
					compressors_.init();
					compressor_inited_ = true;
				}

				if (!common_.have_last_) {
					// don't have the first data yet, just push it to our have last stuff and move on
					common_.have_last_ = true;
					common_.last_ = this_val;

					// write this out to the encoder as it is
					enc.getOutStream().putBytes((const unsigned char*)buf,
                        sizeof(las::point10));
                    return buf + sizeof(las::point10);
				}

				// this is not the first point we're trying to compress, do crazy things
				//
				unsigned int r = this_val.return_number,
							 n = this_val.number_of_returns_of_given_pulse,
							 m = utils::number_return_map[n][r],
							 l = utils::number_return_level[n][r];

				unsigned int k_bits;
				int median, diff;

				// compress which other values have changed
				int changed_values = detail::changed_values(this_val, common_.last_, common_.last_intensity[m]);

				enc.encodeSymbol(common_.m_changed_values, changed_values);

				// if any of the bit fields changed, compress them
				if (changed_values & (1 << 5)) {
					unsigned char b = detail::bitfields_to_char(this_val),
								  last_b = detail::bitfields_to_char(common_.last_);
					enc.encodeSymbol(*common_.m_bit_byte[last_b], b);
				}

				// if the intensity changed, compress it
				if (changed_values & (1 << 4)) {
					compressors_.ic_intensity.compress(enc, common_.last_intensity[m], this_val.intensity, (m < 3 ? m : 3));
					common_.last_intensity[m] = this_val.intensity;
				}

				// if the classification has changed, compress it
				if (changed_values & (1 << 3)) {
					enc.encodeSymbol(*common_.m_classification[common_.last_.classification], this_val.classification);
				}

				// if the scan angle rank has changed, compress it
				if (changed_values & (1 << 2)) {
               	    enc.encodeSymbol(*common_.m_scan_angle_rank[this_val.scan_direction_flag],
							U8_FOLD(this_val.scan_angle_rank - common_.last_.scan_angle_rank));
				}

				// encode user data if changed
				if (changed_values & (1 << 1)) {
					enc.encodeSymbol(*common_.m_user_data[common_.last_.user_data], this_val.user_data);
				}

				// if the point source id was changed, compress it
				if (changed_values & 1) {
					compressors_.ic_point_source_ID.compress(enc, common_.last_.point_source_ID, this_val.point_source_ID, 0);
				}

				// compress x coordinate
				median = common_.last_x_diff_median5[m].get();
				diff = this_val.x - common_.last_.x;
				compressors_.ic_dx.compress(enc, median, diff, n == 1);
				common_.last_x_diff_median5[m].add(diff);

				// compress y coordinate
				k_bits = compressors_.ic_dx.getK();
				median = common_.last_y_diff_median5[m].get();
				diff = this_val.y - common_.last_.y;
				compressors_.ic_dy.compress(enc, median, diff, (n==1) + ( k_bits < 20 ? U32_ZERO_BIT_0(k_bits) : 20 ));
				common_.last_y_diff_median5[m].add(diff);

				// compress z coordinate
				k_bits = (compressors_.ic_dx.getK() + compressors_.ic_dy.getK()) / 2;
				compressors_.ic_z.compress(enc, common_.last_height[l], this_val.z, (n==1) + (k_bits < 18 ? U32_ZERO_BIT_0(k_bits) : 18));
				common_.last_height[l] = this_val.z;

				common_.last_ = this_val;
                return buf + sizeof(las::point10);
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
                        sizeof(las::point10));
                    // decode this value
                    common_.last_ = packers<las::point10>::unpack(buf);
					// we are done here

					common_.last_.intensity = 0;
					return buf + sizeof(las::point10);
				}

				unsigned int r, n, m, l, k_bits;
				int median, diff;

				// decompress which other values have changed
				int changed_values = dec.decodeSymbol(common_.m_changed_values);
				if (changed_values) {
					// there was some change in one of the fields (other than x, y and z)

					// decode bit fields if they have changed
					if (changed_values & (1 << 5)) {
						unsigned char b = detail::bitfields_to_char(common_.last_);
						b = (unsigned char)dec.decodeSymbol(*common_.m_bit_byte[b]);
						detail::char_to_bitfields(b, common_.last_);
					}

					r = common_.last_.return_number;
					n = common_.last_.number_of_returns_of_given_pulse;
					m = utils::number_return_map[n][r];
					l = utils::number_return_level[n][r];

					// decompress the intensity if it has changed
					if (changed_values & (1 << 4)) {
						common_.last_.intensity = static_cast<unsigned short>(decompressors_.ic_intensity.decompress(dec, common_.last_intensity[m], (m < 3 ? m : 3)));
						common_.last_intensity[m] = common_.last_.intensity;
					}
					else {
						common_.last_.intensity = common_.last_intensity[m];
					}

					// decompress the classification ... if it has changed
					if (changed_values & (1 << 3)) {
						common_.last_.classification =
							(unsigned char)dec.decodeSymbol(*common_.m_classification[common_.last_.classification]);
					}

					// decompress the scan angle rank if needed
					if (changed_values & (1 << 2)) {
						int val = dec.decodeSymbol(*common_.m_scan_angle_rank[common_.last_.scan_direction_flag]);
						common_.last_.scan_angle_rank = static_cast<unsigned char>(U8_FOLD(val + common_.last_.scan_angle_rank));
					}

					// decompress the user data
					if (changed_values & (1 << 1)) {
						common_.last_.user_data = (unsigned char)dec.decodeSymbol(*common_.m_user_data[common_.last_.user_data]);
					}

					// decompress the point source ID
					if (changed_values & 1) {
						common_.last_.point_source_ID = (unsigned short)decompressors_.ic_point_source_ID.decompress(dec,
								common_.last_.point_source_ID, 0);
					}
				}
				else {
					r = common_.last_.return_number;
					n = common_.last_.number_of_returns_of_given_pulse;
					m = utils::number_return_map[n][r];
					l = utils::number_return_level[n][r];
				}

				// decompress x coordinate
				median = common_.last_x_diff_median5[m].get();

				diff = decompressors_.ic_dx.decompress(dec, median, n==1);
				common_.last_.x += diff;
				common_.last_x_diff_median5[m].add(diff);

				// decompress y coordinate
				median = common_.last_y_diff_median5[m].get();
				k_bits = decompressors_.ic_dx.getK();
				diff = decompressors_.ic_dy.decompress(dec, median, (n==1) + ( k_bits < 20 ? U32_ZERO_BIT_0(k_bits) : 20 ));
				common_.last_.y += diff;
				common_.last_y_diff_median5[m].add(diff);

				// decompress z coordinate
				k_bits = (decompressors_.ic_dx.getK() + decompressors_.ic_dy.getK()) / 2;
				common_.last_.z = decompressors_.ic_z.decompress(dec, common_.last_height[l], (n==1) + (k_bits < 18 ? U32_ZERO_BIT_0(k_bits) : 18));
				common_.last_height[l] = common_.last_.z;

                packers<las::point10>::pack(common_.last_, buf);
                return buf + sizeof(las::point10);
			}

			// All the things we need to compress a point, group them into structs
			// so we don't have too many names flying around

			// Common parts for both a compressor and decompressor go here
			struct __common {
				las::point10 last_;

				std::array<unsigned short, 16> last_intensity;

				std::array<utils::streaming_median<int>, 16> last_x_diff_median5;
				std::array<utils::streaming_median<int>, 16> last_y_diff_median5;

				std::array<int, 8> last_height;

				models::arithmetic m_changed_values;

				// Arithmetic model has no default constructor, so we store they here as raw pointers
				//
				std::array<models::arithmetic*, 2> m_scan_angle_rank;
				std::array<models::arithmetic*, 256> m_bit_byte;
				std::array<models::arithmetic*, 256> m_classification;
				std::array<models::arithmetic*, 256> m_user_data;

				bool have_last_;

				__common() :
					m_changed_values(64),
					have_last_(false) {
					last_intensity.fill(0);

					m_scan_angle_rank[0] = new models::arithmetic(256);
					m_scan_angle_rank[1] = new models::arithmetic(256);

					last_height.fill(0);

					for (int i = 0 ; i < 256 ; i ++) {
						m_bit_byte[i] = new models::arithmetic(256);
						m_classification[i] = new models::arithmetic(256);
						m_user_data[i] = new models::arithmetic(256);
					}
				}


				~__common() {
					delete m_scan_angle_rank[0];
					delete m_scan_angle_rank[1];

					for (int i = 0 ; i < 256 ; i ++) {
						delete m_bit_byte[i];
						delete m_classification[i];
						delete m_user_data[i];
					}
				}
			} common_;

			// These compressors are specific to a compressor usage, so we keep them separate here
			struct __compressors {
				compressors::integer ic_intensity;
				compressors::integer ic_point_source_ID;
				compressors::integer ic_dx;
				compressors::integer ic_dy;
				compressors::integer ic_z;

				__compressors() :
					ic_intensity(16, 4),
					ic_point_source_ID(16),
					ic_dx(32, 2),
					ic_dy(32, 22),
					ic_z(32, 20) { }

				void init() {
					ic_intensity.init();
					ic_point_source_ID.init();
					ic_dx.init();
					ic_dy.init();
					ic_z.init();
				}
			} compressors_;

			struct __decompressors {
				decompressors::integer ic_intensity;
				decompressors::integer ic_point_source_ID;
				decompressors::integer ic_dx;
				decompressors::integer ic_dy;
				decompressors::integer ic_z;

				__decompressors() :
					ic_intensity(16, 4),
					ic_point_source_ID(16),
					ic_dx(32, 2),
					ic_dy(32, 22),
					ic_z(32, 20) { }

				void init() {
					ic_intensity.init();
					ic_point_source_ID.init();
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
