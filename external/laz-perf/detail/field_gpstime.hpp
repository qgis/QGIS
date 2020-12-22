/*
===============================================================================

  FILE:  field_gpstime.hpp

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

#define LASZIP_GPSTIME_MULTI 500
#define LASZIP_GPSTIME_MULTI_MINUS -10
#define LASZIP_GPSTIME_MULTI_UNCHANGED (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 1)
#define LASZIP_GPSTIME_MULTI_CODE_FULL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 2)

#define LASZIP_GPSTIME_MULTI_TOTAL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 6)

namespace laszip {
	namespace formats {
		// Teach packers how to pack and unpack gps time
		//
		template<>
		struct packers<las::gpstime> {
			inline static las::gpstime unpack(const char *in) {
				uint64_t lower = packers<unsigned int>::unpack(in),
						 upper = packers<unsigned int>::unpack(in + 4);

				return las::gpstime((upper << 32) | lower);
			}

			inline static void pack(const las::gpstime& t, char *buffer) {
				packers<unsigned int>::pack(t.value & 0xFFFFFFFF, buffer);
				packers<unsigned int>::pack(t.value >> 32, buffer + 4);
			}
		};

		// Figure how to compress and decompress GPS time fields
		//
		template<>
		struct field<las::gpstime> {
			typedef las::gpstime type;

			field() : compressor_inited_(false), decompressor_inited_(false) {}

			template<
				typename TEncoder
			>
			inline const char *compressWith(TEncoder& enc, const char *buf)
            {
                las::gpstime this_val = packers<las::gpstime>::unpack(buf);

				if (!compressor_inited_) {
					compressors_.init();
					compressor_inited_ = true;
				}

				if (!common_.have_last_) {
					// don't have the first data yet, just push it to our have last stuff and move on
					common_.have_last_ = true;
					common_.last_gpstime[0] = this_val;

					// write this out to the encoder as it is
					enc.getOutStream().putBytes((const unsigned char*)buf,
                        sizeof(las::gpstime));
                    buf += sizeof(las::gpstime);

					// we are done here
					return buf;
				}

				if (common_.last_gpstime_diff[common_.last] == 0) { // if last integer different was 0
					if (this_val.value == common_.last_gpstime[common_.last].value) {
						enc.encodeSymbol(common_.m_gpstime_0diff, 0);
					}
					else {
						// calculate the difference between the two doubles as an integer
						//
						int64_t curr_gpstime_diff_64 = this_val.value - common_.last_gpstime[common_.last].value;
						int curr_gpstime_diff = static_cast<int>(curr_gpstime_diff_64);

						if (curr_gpstime_diff_64 == static_cast<int64_t>(curr_gpstime_diff)) {
							// this difference is small enough to be represented with 32 bits
							enc.encodeSymbol(common_.m_gpstime_0diff, 1);
							compressors_.ic_gpstime.compress(enc, 0, curr_gpstime_diff, 0);
							common_.last_gpstime_diff[common_.last] = curr_gpstime_diff;
							common_.multi_extreme_counter[common_.last] = 0;
						}
						else { // the difference is huge
							U32 i;

							// maybe the double belongs to another time sequence
							//
							for (i = 1; i < 4; i++) {
								int64_t other_gpstime_diff_64 = this_val.value -
									common_.last_gpstime[(common_.last+i)&3].value;
								int other_gpstime_diff = static_cast<int>(other_gpstime_diff_64);

								if (other_gpstime_diff_64 == static_cast<int64_t>(other_gpstime_diff)) {
									enc.encodeSymbol(common_.m_gpstime_0diff, i+2); // it belongs to another sequence
									common_.last = (common_.last+i)&3;
                                    return compressWith(enc, buf);
								}
							}

							// no other sequence found. start new sequence.
							enc.encodeSymbol(common_.m_gpstime_0diff, 2);
							compressors_.ic_gpstime.compress(enc,
									static_cast<int>(common_.last_gpstime[common_.last].value >> 32),
									static_cast<int>(this_val.value >> 32), 8);

							enc.writeInt(static_cast<unsigned int>(this_val.value));

							common_.next = (common_.next+1)&3;
							common_.last = common_.next;
							common_.last_gpstime_diff[common_.last] = 0;
							common_.multi_extreme_counter[common_.last] = 0;
						}
						common_.last_gpstime[common_.last] = this_val;
					}
				}
				else { // the last integer difference was *not* zero
					if (this_val.value == common_.last_gpstime[common_.last].value) {
						// if the doubles have not changed use a special symbol
						enc.encodeSymbol(common_.m_gpstime_multi, LASZIP_GPSTIME_MULTI_UNCHANGED);
					}
					else
					{
						// calculate the difference between the two doubles as an integer
						int64_t curr_gpstime_diff_64 = this_val.value -
							common_.last_gpstime[common_.last].value;
						int curr_gpstime_diff = static_cast<int>(curr_gpstime_diff_64);

						// if the current gpstime difference can be represented with 32 bits
						if (curr_gpstime_diff_64 == static_cast<int64_t>(curr_gpstime_diff)) {
							// compute multiplier between current and last integer difference
							float multi_f = (float)curr_gpstime_diff /
								(float)(common_.last_gpstime_diff[common_.last]);
							int multi = I32_QUANTIZE(multi_f);

							// compress the residual curr_gpstime_diff in dependance on the multiplier
							if (multi == 1) {
								// this is the case we assume we get most often for regular spaced pulses
								enc.encodeSymbol(common_.m_gpstime_multi, 1);
								compressors_.ic_gpstime.compress(enc,
										common_.last_gpstime_diff[common_.last], curr_gpstime_diff, 1);
								common_.multi_extreme_counter[common_.last] = 0;
							}
							else if (multi > 0) {
								if (multi < LASZIP_GPSTIME_MULTI) {
									// positive multipliers up to LASZIP_GPSTIME_MULTI are compressed directly
									enc.encodeSymbol(common_.m_gpstime_multi, multi);
									if (multi < 10)
										compressors_.ic_gpstime.compress(enc,
												multi*common_.last_gpstime_diff[common_.last],
												curr_gpstime_diff, 2);
									else
										compressors_.ic_gpstime.compress(enc,
												multi*common_.last_gpstime_diff[common_.last],
												curr_gpstime_diff, 3);
								}
								else {
									enc.encodeSymbol(common_.m_gpstime_multi, LASZIP_GPSTIME_MULTI);
									compressors_.ic_gpstime.compress(enc,
											LASZIP_GPSTIME_MULTI*common_.last_gpstime_diff[common_.last],
											curr_gpstime_diff, 4);
									common_.multi_extreme_counter[common_.last]++;

									if (common_.multi_extreme_counter[common_.last] > 3) {
										common_.last_gpstime_diff[common_.last] = curr_gpstime_diff;
										common_.multi_extreme_counter[common_.last] = 0;
									}
								}
							}
							else if (multi < 0) {
								if (multi > LASZIP_GPSTIME_MULTI_MINUS) {
									// negative multipliers larger than LASZIP_GPSTIME_MULTI_MINUS are compressed directly
									enc.encodeSymbol(common_.m_gpstime_multi, LASZIP_GPSTIME_MULTI - multi);
									compressors_.ic_gpstime.compress(enc,
											multi*common_.last_gpstime_diff[common_.last],
											curr_gpstime_diff, 5);
								}
								else {
									enc.encodeSymbol(common_.m_gpstime_multi, LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS);
									compressors_.ic_gpstime.compress(enc,
											LASZIP_GPSTIME_MULTI_MINUS*common_.last_gpstime_diff[common_.last],
											curr_gpstime_diff, 6);

									common_.multi_extreme_counter[common_.last]++;
									if (common_.multi_extreme_counter[common_.last] > 3)
									{
										common_.last_gpstime_diff[common_.last] = curr_gpstime_diff;
										common_.multi_extreme_counter[common_.last] = 0;
									}
								}
							}
							else {
								enc.encodeSymbol(common_.m_gpstime_multi, 0);
								compressors_.ic_gpstime.compress(enc, 0, curr_gpstime_diff, 7);
								common_.multi_extreme_counter[common_.last]++;
								if (common_.multi_extreme_counter[common_.last] > 3)
								{
									common_.last_gpstime_diff[common_.last] = curr_gpstime_diff;
									common_.multi_extreme_counter[common_.last] = 0;
								}
							}
						}
						else { // the difference is huge
							int i;
							// maybe the double belongs to another time sequence
							for (i = 1; i < 4; i++)
							{
								int64_t other_gpstime_diff_64 = this_val.value - common_.last_gpstime[(common_.last+i)&3].value;
								int other_gpstime_diff = static_cast<int>(other_gpstime_diff_64);

								if (other_gpstime_diff_64 == static_cast<int64_t>(other_gpstime_diff)) {
									// it belongs to this sequence
									enc.encodeSymbol(common_.m_gpstime_multi, LASZIP_GPSTIME_MULTI_CODE_FULL+i);
									common_.last = (common_.last+i)&3;
                                    return compressWith(enc, buf);
								}
							}

							// no other sequence found. start new sequence.
							enc.encodeSymbol(common_.m_gpstime_multi, LASZIP_GPSTIME_MULTI_CODE_FULL);
							compressors_.ic_gpstime.compress(
									enc,
									static_cast<int>(common_.last_gpstime[common_.last].value >> 32),
									static_cast<int>(this_val.value >> 32), 8);
							enc.writeInt(static_cast<unsigned int>(this_val.value));
							common_.next = (common_.next+1)&3;
							common_.last = common_.next;
							common_.last_gpstime_diff[common_.last] = 0;
							common_.multi_extreme_counter[common_.last] = 0;
						}

						common_.last_gpstime[common_.last] = this_val;
					}
				}
                return buf + sizeof(las::gpstime);
			}

			template<
				typename TDecoder
			>
			inline char *decompressWith(TDecoder& dec, char *buf) {
				if (!decompressor_inited_) {
					decompressors_.init();
					decompressor_inited_ = true;
				}

				if (!common_.have_last_) {
					// don't have the first data yet, read the whole point out of the stream
					common_.have_last_ = true;

					dec.getInStream().getBytes((unsigned char*)buf,
                        sizeof(las::gpstime));
                    // decode this value
                    common_.last_gpstime[0] = packers<las::gpstime>::unpack(buf);

					// we are done here
					return buf + sizeof(las::gpstime);
				}

				int multi;
				if (common_.last_gpstime_diff[common_.last] == 0) { // if the last integer difference was zero
					multi = dec.decodeSymbol(common_.m_gpstime_0diff);

					if (multi == 1) { // the difference can be represented with 32 bits
						common_.last_gpstime_diff[common_.last] = decompressors_.ic_gpstime.decompress(dec, 0, 0);
						common_.last_gpstime[common_.last].value += common_.last_gpstime_diff[common_.last];
						common_.multi_extreme_counter[common_.last] = 0;
					}
					else if (multi == 2) { // the difference is huge
						common_.next = (common_.next+1)&3;
						common_.last_gpstime[common_.next].value = decompressors_.ic_gpstime.decompress(
								dec,
								(common_.last_gpstime[common_.last].value >> 32), 8);
						common_.last_gpstime[common_.next].value = common_.last_gpstime[common_.next].value << 32;
						common_.last_gpstime[common_.next].value |= dec.readInt();
						common_.last = common_.next;
						common_.last_gpstime_diff[common_.last] = 0;
						common_.multi_extreme_counter[common_.last] = 0;
					}
					else if (multi > 2) { // we switch to another sequence
						common_.last = (common_.last+multi-2)&3;

						decompressWith(dec, buf);
					}
				}
				else {
					multi = dec.decodeSymbol(common_.m_gpstime_multi);
					if (multi == 1) {
						common_.last_gpstime[common_.last].value += decompressors_.ic_gpstime.decompress(
								dec,
								common_.last_gpstime_diff[common_.last], 1);
						common_.multi_extreme_counter[common_.last] = 0;
					}
					else if (multi < LASZIP_GPSTIME_MULTI_UNCHANGED) {
						int gpstime_diff;
						if (multi == 0) {
							gpstime_diff = decompressors_.ic_gpstime.decompress(dec, 0, 7);
							common_.multi_extreme_counter[common_.last]++;
							if (common_.multi_extreme_counter[common_.last] > 3) { common_.last_gpstime_diff[common_.last] = gpstime_diff;
								common_.multi_extreme_counter[common_.last] = 0;
							}
						}
						else if (multi < LASZIP_GPSTIME_MULTI) {
							if (multi < 10)
								gpstime_diff = decompressors_.ic_gpstime.decompress(dec,
										multi*common_.last_gpstime_diff[common_.last], 2);
							else
								gpstime_diff = decompressors_.ic_gpstime.decompress(dec,
										multi*common_.last_gpstime_diff[common_.last], 3);
						}
						else if (multi == LASZIP_GPSTIME_MULTI) {
							gpstime_diff = decompressors_.ic_gpstime.decompress(
									dec,
									LASZIP_GPSTIME_MULTI*common_.last_gpstime_diff[common_.last], 4);
							common_.multi_extreme_counter[common_.last]++;
							if (common_.multi_extreme_counter[common_.last] > 3) {
								common_.last_gpstime_diff[common_.last] = gpstime_diff;
								common_.multi_extreme_counter[common_.last] = 0;
							}
						}
						else {
							multi = LASZIP_GPSTIME_MULTI - multi;
							if (multi > LASZIP_GPSTIME_MULTI_MINUS) {
								gpstime_diff = decompressors_.ic_gpstime.decompress(
										dec,
										multi*common_.last_gpstime_diff[common_.last], 5);
							}
							else
							{
								gpstime_diff = decompressors_.ic_gpstime.decompress(
										dec,
										LASZIP_GPSTIME_MULTI_MINUS*common_.last_gpstime_diff[common_.last], 6);
								common_.multi_extreme_counter[common_.last]++;
								if (common_.multi_extreme_counter[common_.last] > 3) {
									common_.last_gpstime_diff[common_.last] = gpstime_diff;
									common_.multi_extreme_counter[common_.last] = 0;
								}
							}
						}
						common_.last_gpstime[common_.last].value += gpstime_diff;
					}
					else if (multi ==  LASZIP_GPSTIME_MULTI_CODE_FULL) {
						common_.next = (common_.next+1)&3;
						common_.last_gpstime[common_.next].value = decompressors_.ic_gpstime.decompress(
								dec, static_cast<int>(common_.last_gpstime[common_.last].value >> 32), 8);
						common_.last_gpstime[common_.next].value = common_.last_gpstime[common_.next].value << 32;
						common_.last_gpstime[common_.next].value |= dec.readInt();
						common_.last = common_.next;
						common_.last_gpstime_diff[common_.last] = 0;
						common_.multi_extreme_counter[common_.last] = 0;
					}
					else if (multi >=  LASZIP_GPSTIME_MULTI_CODE_FULL) {
						common_.last = (common_.last+multi-LASZIP_GPSTIME_MULTI_CODE_FULL)&3;

						decompressWith(dec, buf);
					}
				}
                packers<las::gpstime>::pack(common_.last_gpstime[common_.last],
                    buf);
                return buf + sizeof(las::gpstime);
			}

			// All the things we need to compress a point, group them into structs
			// so we don't have too many names flying around

			// Common parts for both a compressor and decompressor go here
			struct __common {
				bool have_last_;
				models::arithmetic m_gpstime_multi, m_gpstime_0diff;
				unsigned int last, next;
				std::array<las::gpstime, 4> last_gpstime;
				std::array<int, 4> last_gpstime_diff;
				std::array<int, 4> multi_extreme_counter;

				__common() :
					have_last_(false),
					m_gpstime_multi(LASZIP_GPSTIME_MULTI_TOTAL),
					m_gpstime_0diff(6),
					last(0), next(0) {

					last_gpstime.fill(las::gpstime());
					last_gpstime_diff.fill(0);
					multi_extreme_counter.fill(0);
				}


				~__common() {
				}
			} common_;

			// These compressors are specific to a compressor usage, so we keep them separate here
			struct __compressors {
				compressors::integer ic_gpstime;

				__compressors() :
					ic_gpstime(32, 9) {}

				void init() {
					ic_gpstime.init();
				}
			} compressors_;

			struct __decompressors {
				decompressors::integer ic_gpstime;

				__decompressors() :
					ic_gpstime(32, 9) {}

				void init() {
					ic_gpstime.init();
				}
			} decompressors_;

			bool compressor_inited_;
			bool decompressor_inited_;
		};
	}
}
