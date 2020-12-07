/*
===============================================================================

  FILE:  io.hpp

  CONTENTS:
    LAZ io

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

#ifndef __io_hpp__
#define __io_hpp__

#include <fstream>
#include <functional>
#include <limits>
#include <string.h>
#include <mutex>

#include "formats.hpp"
#include "excepts.hpp"
#include "factory.hpp"
#include "decoder.hpp"
#include "encoder.hpp"
#include "util.hpp"
#include "portable_endian.hpp"

namespace laszip {
	// A simple datastructure to get input from the user
	template<
		typename T
	>
	struct vector3 {
		T x, y, z;

		vector3() : x(0), y(0), z(0) {}
		vector3(const T& _x, const T& _y, const T& _z) :
			x(_x), y(_y), z(_z) {
		}

	};

#define DefaultChunkSize 50000

	namespace io {
		// LAZ file header
#pragma pack(push, 1)
		struct header {
			char magic[4];
			unsigned short file_source_id;
			unsigned short global_encoding;
			char guid[16];

			struct {
				unsigned char major;
				unsigned char minor;
			} version;

			char system_identifier[32];
			char generating_software[32];

			struct {
				unsigned short day;
				unsigned short year;
			} creation;

			unsigned short header_size;
			unsigned int point_offset;
			unsigned int vlr_count;

			unsigned char point_format_id;
			unsigned short point_record_length;

			unsigned int point_count;
			unsigned int points_by_return[5];

			struct {
				double x, y, z;
			} scale;

			struct {
				double x, y, z;
			} offset;

			struct {
				double x, y, z;
			} minimum;

			struct {
				double x, y, z;
			} maximum;
		};

		// A Single LAZ Item representation
		struct laz_item {
			unsigned short type,
						   size,
						   version;
		};

		struct laz_vlr {
			uint16_t compressor;
			uint16_t coder;

			struct {
				unsigned char major;
				unsigned char minor;
				uint16_t revision;
			} version;

			uint32_t options;
			uint32_t chunk_size;

			int64_t num_points,
					num_bytes;

			uint16_t num_items;
			laz_item *items;
			laz_vlr() : num_items(0), items(NULL) {}
			~laz_vlr() {
                delete [] items;
			}

            laz_vlr(const char *data) {
                items = NULL;
                fill(data);
            }

            size_t size() const {
                return sizeof(laz_vlr) - sizeof(laz_item *) +
                    (num_items * sizeof(laz_item));
            }

			laz_vlr(const laz_vlr& rhs) {
				compressor = rhs.compressor;
				coder = rhs.coder;

				// the version we're compatible with
				version.major = rhs.version.major;
				version.minor = rhs.version.minor;
				version.revision = rhs.version.revision;

				options = rhs.options;
				chunk_size = rhs.chunk_size;

				num_points = rhs.num_points;
				num_bytes = rhs.num_bytes;

				num_items = rhs.num_items;
				if (rhs.items) {
					items = new laz_item[num_items];
					for (int i = 0 ; i < num_items ; i ++) {
						items[i] = rhs.items[i];
					}
				}
			}

			laz_vlr& operator = (const laz_vlr& rhs) {
				if (this == &rhs)
					return *this;

				compressor = rhs.compressor;
				coder = rhs.coder;

				// the version we're compatible with
				version.major = rhs.version.major;
				version.minor = rhs.version.minor;
				version.revision = rhs.version.revision;

				options = rhs.options;
				chunk_size = rhs.chunk_size;

				num_points = rhs.num_points;
				num_bytes = rhs.num_bytes;

				num_items = rhs.num_items;
				if (rhs.items) {
					items = new laz_item[num_items];
					for (int i = 0 ; i < num_items ; i ++) {
						items[i] = rhs.items[i];
					}
				}

				return *this;
			}

            void fill(const char *data) {
                std::copy(data, data + sizeof(compressor), (char *)&compressor);
                compressor = le16toh(compressor);
                data += sizeof(compressor);

                std::copy(data, data + sizeof(coder), (char *)&coder);
                coder = le16toh(coder);
                data += sizeof(coder);

                version.major = *(const unsigned char *)data++;
                version.minor = *(const unsigned char *)data++;

                std::copy(data, data + sizeof(version.revision), (char *)&version.revision);
                version.revision = le16toh(version.revision);
                data += sizeof(version.revision);

                std::copy(data, data + sizeof(options), (char *)&options);
                options = le32toh(options);
                data += sizeof(options);

                std::copy(data, data + sizeof(chunk_size), (char *)&chunk_size);
                chunk_size = le32toh(chunk_size);
                data += sizeof(chunk_size);

                std::copy(data, data + sizeof(num_points), (char *)&num_points);
                num_points = le64toh(num_points);
                data += sizeof(num_points);

                std::copy(data, data + sizeof(num_bytes), (char *)&num_bytes);
                num_bytes = le64toh(num_bytes);
                data += sizeof(num_bytes);

                std::copy(data, data + sizeof(num_items), (char *)&num_items);
                num_items = le16toh(num_items);
                data += sizeof(num_items);

                delete [] items;
		        items = new laz_item[num_items];
                for (int i = 0 ; i < num_items ; i ++) {
                    laz_item& item = items[i];

                    std::copy(data, data + sizeof(item.type), (char *)&item.type);
                    item.type = le16toh(item.type);
                    data += sizeof(item.type);

                    std::copy(data, data + sizeof(item.size), (char *)&item.size);
                    item.size = le16toh(item.size);
                    data += sizeof(item.size);

                    std::copy(data, data + sizeof(item.version), (char *)&item.version);
                    item.version = le16toh(item.version);
                    data += sizeof(item.version);
                }
            }

            void extract(char *data) {
                uint16_t s;
                uint32_t i;
                uint64_t ll;
                char *src;

                s = htole16(compressor);
                src = (char *)&s;
                std::copy(src, src + sizeof(compressor), data);
                data += sizeof(compressor);

                s = htole16(coder);
                src = (char *)&s;
                std::copy(src, src + sizeof(coder), data);
                data += sizeof(coder);

                *data++ = version.major;
                *data++ = version.minor;

                s = htole16(version.revision);
                src = (char *)&s;
                std::copy(src, src + sizeof(version.revision), data);
                data += sizeof(version.revision);

                i = htole32(options);
                src = (char *)&i;
                std::copy(src, src + sizeof(options), data);
                data += sizeof(options);

                i = htole32(chunk_size);
                src = (char *)&i;
                std::copy(src, src + sizeof(chunk_size), data);
                data += sizeof(chunk_size);

                ll = htole64(num_points);
                src = (char *)&ll;
                std::copy(src, src + sizeof(num_points), data);
                data += sizeof(num_points);

                ll = htole64(num_bytes);
                src = (char *)&ll;
                std::copy(src, src + sizeof(num_bytes), data);
                data += sizeof(num_bytes);

                s = htole16(num_items);
                src = (char *)&s;
                std::copy(src, src + sizeof(num_items), data);
                data += sizeof(num_items);

                for (int k = 0 ; k < num_items ; k ++) {
                    laz_item& item = items[k];

                    s = htole16(item.type);
                    src = (char *)&s;
                    std::copy(src, src + sizeof(item.type), data);
                    data += sizeof(item.type);

                    s = htole16(item.size);
                    src = (char *)&s;
                    std::copy(src, src + sizeof(item.size), data);
                    data += sizeof(item.size);

                    s = htole16(item.version);
                    src = (char *)&s;
                    std::copy(src, src + sizeof(item.version), data);
                    data += sizeof(item.version);
                }
            }

			static laz_vlr from_schema(const factory::record_schema& s, uint32_t chunksize = DefaultChunkSize) {
				laz_vlr r;

                // We only do pointwise chunking.
				r.compressor = 2;
				r.coder = 0;

				// the version we're compatible with
				r.version.major = 2;
				r.version.minor = 2;
				r.version.revision = 0;

				r.options = 0;
				r.chunk_size = chunksize;

				r.num_points = -1;
				r.num_bytes = -1;

				r.num_items = static_cast<unsigned short>(s.records.size());
				r.items = new laz_item[s.records.size()];
				for (size_t i = 0 ; i < s.records.size() ; i ++) {
					laz_item& item = r.items[i];
					const factory::record_item& rec = s.records.at(i);

					item.type = static_cast<unsigned short>(rec.type);
					item.size = static_cast<unsigned short>(rec.size);
					item.version = static_cast<unsigned short>(rec.version);
				}

				return r;
			}

            static factory::record_schema to_schema(const laz_vlr& vlr, int point_len) {
                // convert the laszip items into record schema to be used by
                // compressor/decompressor

                using namespace factory;
                factory::record_schema schema;

                for(auto i = 0 ; i < vlr.num_items ; i++) {
                    laz_item& item = vlr.items[i];
                    schema.push(factory::record_item(item.type, item.size,
                        item.version));
                    point_len -= item.size;
                }
                if (point_len < 0)
                    throw laszip_format_unsupported();
                // Add extra bytes information
                if (point_len)
                    schema.push(factory::record_item(record_item::BYTE,
                        point_len, 2));
                return schema;
            }

#ifdef _WIN32
            __declspec(deprecated) static factory::record_schema to_schema(const laz_vlr& vlr)
#else
            static factory::record_schema to_schema(const laz_vlr& vlr) __attribute__ ((deprecated))
#endif
            {
                // convert the laszip items into record schema to be used by
                // compressor/decompressor

                using namespace factory;
                factory::record_schema schema;

                for(auto i = 0 ; i < vlr.num_items ; i++) {
                    laz_item& item = vlr.items[i];
                    schema.push(factory::record_item(item.type, item.size,
                        item.version));
                }
                return schema;
            }
		};
#pragma pack(pop)

		// cache line
#define BUF_SIZE (1 << 20)

		template<typename StreamType>
		struct __ifstream_wrapper {
			__ifstream_wrapper(StreamType& f) : f_(f), offset(0), have(0),
				buf_((char*)utils::aligned_malloc(BUF_SIZE)) {
			}

			~__ifstream_wrapper() {
				utils::aligned_free(buf_);
			}

			__ifstream_wrapper(const __ifstream_wrapper<StreamType>&) = delete;
			__ifstream_wrapper& operator = (const __ifstream_wrapper<StreamType>&) = delete;

			inline void fillit_() {
				offset = 0;
				f_.read(buf_, BUF_SIZE);
				have = f_.gcount();
				if (have == 0)
					throw end_of_file(); // this is an exception since we shouldn't be hitting eof
			}

			inline void reset() {
				offset = have = 0; // when a file is seeked, reset this
			}

			inline unsigned char getByte() {
                if (offset >= have)
                    fillit_();
				return static_cast<unsigned char>(buf_[offset++]);
			}

			inline void getBytes(unsigned char *buf, size_t request) {
                // Use what's left in the buffer, if anything.
                size_t fetchable = (std::min)((size_t)(have - offset), request);
				std::copy(buf_ + offset, buf_ + offset + fetchable, buf);
                offset += fetchable;
                request -= fetchable;

                // If we couldn't fetch everything requested, fill buffer
                // and go again.  We assume fillit_() satisfies any request.
                if (request)
                {
                    fillit_();
				    std::copy(buf_ + offset, buf_ + offset + request, buf + fetchable);
				    offset += request;
                }
			}

			StreamType& f_;
			std::streamsize offset, have;
			char *buf_;
		};

        template<typename StreamType>
		struct __ofstream_wrapper {
			__ofstream_wrapper(StreamType& f) : f_(f) {}

			void putBytes(const unsigned char *b, size_t len) {
				f_.write(reinterpret_cast<const char*>(b), len);
			}

			void putByte(unsigned char b) {
				f_.put((char)b);
			}

			__ofstream_wrapper(const __ofstream_wrapper&) = delete;
			__ofstream_wrapper& operator = (const __ofstream_wrapper&) = delete;

			StreamType& f_;
		};

		namespace reader {
			template <typename StreamType>
			class basic_file {
				typedef std::function<void (header&)> validator_type;

				public:
				basic_file(StreamType& st) : f_(st), wrapper_(f_) {
					_open();
				}

				~basic_file() {
				}

				const header& get_header() const {
					return header_;
				}

				const laz_vlr& get_laz_vlr() const {
					return laz_;
				}

				const factory::record_schema& get_schema() const {
					return schema_;
				}

				void readPoint(char *out) {
					// read the next point in
					if (chunk_state_.points_read == laz_.chunk_size ||
							!pdecomperssor_ || !pdecoder_) {
						// Its time to (re)init the decoder
						//
						pdecomperssor_.reset();
						pdecoder_.reset();

						pdecoder_.reset(new decoders::arithmetic<__ifstream_wrapper<StreamType> >(wrapper_));
						pdecomperssor_ = factory::build_decompressor(*pdecoder_, schema_);

						// reset chunk state
						chunk_state_.current++;
						chunk_state_.points_read = 0;
					}

					pdecomperssor_->decompress(out);
					chunk_state_.points_read ++;
				}

				private:
				void _open() {
					// Make sure our header is correct
					//
					char magic[4];
					f_.read(magic, sizeof(magic));

					if (std::string(magic, magic+4) != "LASF")
						throw invalid_magic();

					// Read the header in
					f_.seekg(0);
					f_.read((char*)&header_, sizeof(header_));

					// The mins and maxes are in a weird order, fix them
					_fixMinMax(header_);

					// make sure everything is valid with the header, note that validators are allowed
					// to manipulate the header, since certain validators depend on a header's orignial state
					// to determine what its final stage is going to be
					for (auto f : _validators())
						f(header_);

					// things look fine, move on with VLR extraction
					_parseLASZIP();

					// parse the chunk table offset
					_parseChunkTable();

					// set the file pointer to the beginning of data to start reading
					f_.clear(); // may have treaded past the EOL, so reset everything before we start reading:w
					f_.seekg(header_.point_offset + sizeof(int64_t));

					wrapper_.reset();
				}

				void _fixMinMax(header& h) {
					double mx, my, mz, nx, ny, nz;

					mx = h.minimum.x; nx = h.minimum.y;
					my = h.minimum.z; ny = h.maximum.x;
					mz = h.maximum.y; nz = h.maximum.z;

					h.minimum.x = nx; h.maximum.x = mx;
					h.minimum.y = ny; h.maximum.y = my;
					h.minimum.z = nz; h.maximum.z = mz;
				}

				void _parseLASZIP() {
					// move the pointer to the begining of the VLRs
					f_.seekg(header_.header_size);

#pragma pack(push, 1)
					struct {
						unsigned short reserved;
						char user_id[16];
						unsigned short record_id;
						unsigned short record_length;
						char desc[32];
					} vlr_header;
#pragma pack(pop)

					size_t count = 0;
					bool laszipFound = false;
					while(count < header_.vlr_count && f_.good() && !f_.eof()) {
						f_.read((char*)&vlr_header, sizeof(vlr_header));

						const char *user_id = "laszip encoded";

						if (std::equal(vlr_header.user_id, vlr_header.user_id + 14, user_id) &&
								vlr_header.record_id == 22204) {
							// this is the laszip VLR
							//
							laszipFound = true;

							std::unique_ptr<char> buffer(
                                new char[vlr_header.record_length]);

							f_.read(buffer.get(), vlr_header.record_length);
							_parseLASZIPVLR(buffer.get());

							break; // no need to keep iterating
						}

						f_.seekg(vlr_header.record_length, std::ios::cur); // jump foward
						count++;
					}

					if (!laszipFound)
						throw no_laszip_vlr();

                    schema_ = laz_vlr::to_schema(laz_, header_.point_record_length);
				}

				void binPrint(const char *buf, int len) {
					for (int i = 0 ; i < len ; i ++) {
						char b[256];
						sprintf(b, "%02X", buf[i] & 0xFF);
						std::cout << b << " ";
					}

					std::cout << std::endl;
				}

				void _parseLASZIPVLR(const char *buf) {
                    laz_.fill(buf);

					if (laz_.compressor != 2)
						throw laszip_format_unsupported();
				}

				void _parseChunkTable() {
					// Move to the begining of the data
					//
					f_.seekg(header_.point_offset);

					int64_t chunkoffset = 0;
					f_.read((char*)&chunkoffset, sizeof(chunkoffset));
					if (!f_.good())
						throw chunk_table_read_error();

					if (chunkoffset == -1)
						throw not_supported("Chunk table offset == -1 is not supported at this time");

					// Go to the chunk offset and read in the table
					//
					f_.seekg(chunkoffset);
					if (!f_.good())
						throw chunk_table_read_error();

					// Now read in the chunk table
					struct {
						unsigned int version,
									 chunk_count;
					} chunk_table_header;

					f_.read((char *)&chunk_table_header, sizeof(chunk_table_header));
					if (!f_.good())
						throw chunk_table_read_error();

					if (chunk_table_header.version != 0)
						throw unknown_chunk_table_format();

					// start pushing in chunk table offsets
					chunk_table_offsets_.clear();

					if (laz_.chunk_size == (std::numeric_limits<unsigned int>::max)())
						throw not_supported("chunk_size == uint.max is not supported at this time.");

					// Allocate enough room for our chunk
					chunk_table_offsets_.resize(chunk_table_header.chunk_count + 1);

					// Add The first one
					chunk_table_offsets_[0] = header_.point_offset + sizeof(uint64_t);

					if (chunk_table_header.chunk_count > 1) {
						// decode the index out
						//
						__ifstream_wrapper<StreamType> w(f_);

						decoders::arithmetic<__ifstream_wrapper<StreamType> > decoder(w);
						decompressors::integer decomp(32, 2);

						// start decoder
						decoder.readInitBytes();
						decomp.init();

						for (size_t i = 1 ; i <= chunk_table_header.chunk_count ; i ++) {
							chunk_table_offsets_[i] = static_cast<uint64_t>(decomp.decompress(decoder, (i > 1) ? static_cast<I32>(chunk_table_offsets_[i - 1]) : 0, 1));
						}

						for (size_t i = 1 ; i < chunk_table_offsets_.size() ; i ++) {
							chunk_table_offsets_[i] += chunk_table_offsets_[i-1];
						}
					}
				}


				static const std::vector<validator_type>& _validators() {
					static std::vector<validator_type> v; // static collection of validators
					static std::mutex lock;

					// To remain thread safe we need to make sure we have appropriate guards here
					//
					if (v.empty()) {
						lock.lock();
						// Double check here if we're still empty, the first empty just makes sure
						// we have a quick way out where validators are already filled up (for all calls
						// except the first one), for two threads competing to fill out the validators
						// only one of the will get here first, and the second one will bail if the v
						// is not empty, and hence the double check
						//
						if (v.empty()) {
							// TODO: Fill all validators here
							//
							v.push_back(
									// Make sure that the header indicates that file is compressed
									//
									[](header& h) {
										int bit_7 = (h.point_format_id >> 7) & 1,
											bit_6 = (h.point_format_id >> 6) & 1;

										if (bit_7 == 1 && bit_6 == 1)
											throw old_style_compression();

										if ((bit_7 ^ bit_6) == 0)
											throw not_compressed();

										h.point_format_id &= 0x3f;
									}
							);
						}

						lock.unlock();
					}

					return v;
				}

				// The file object is not copyable or copy constructible
				basic_file(const basic_file<StreamType>&) = delete;
				basic_file<StreamType>& operator = (const basic_file<StreamType>&) = delete;

				StreamType& f_;
				__ifstream_wrapper<StreamType> wrapper_;

				header header_;
				laz_vlr laz_;
				std::vector<uint64_t> chunk_table_offsets_;

				factory::record_schema schema_;	// the schema of this file, the LAZ items converted into factory recognizable description,

				// Our decompressor
				std::shared_ptr<decoders::arithmetic<__ifstream_wrapper<StreamType> > > pdecoder_;
				formats::dynamic_decompressor::ptr pdecomperssor_;

				// Establish our current state as we iterate through the file
				struct __chunk_state{
					int64_t current;
					int64_t points_read;
					int64_t current_index;

					__chunk_state() : current(0u), points_read(0u), current_index(-1) {}
				} chunk_state_;
			};

			typedef basic_file<std::ifstream> file;
		}

		namespace writer {

			// An object to encapsulate what gets passed to
			struct config {
				vector3<double> scale, offset;
				unsigned int chunk_size;

				explicit config() : scale(1.0, 1.0, 1.0), offset(0.0, 0.0, 0.0), chunk_size(DefaultChunkSize) {}
				config(const vector3<double>& s, const vector3<double>& o, unsigned int cs = DefaultChunkSize) :
					scale(s), offset(o), chunk_size(cs) {}
                config(const header& h) : scale(h.scale.x, h.scale.y, h.scale.z), offset(h.offset.x, h.offset.y, h.offset.z),
                    chunk_size(DefaultChunkSize) {}

				header to_header() const {
					header h; memset(&h, 0, sizeof(h)); // clear out header
					h.minimum = { (std::numeric_limits<double>::max)(), (std::numeric_limits<double>::max)(),
                        (std::numeric_limits<double>::max)() };
					h.maximum = { std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(),
                        std::numeric_limits<double>::lowest()};

					h.offset.x = offset.x;
					h.offset.y = offset.y;
					h.offset.z = offset.z;

					h.scale.x = scale.x;
					h.scale.y = scale.y;
					h.scale.z = scale.z;

					return h;
				}
			};

			class file {
			public:
				file() :
					wrapper_(f_) {}

				file(const std::string& filename,
						const factory::record_schema& s,
						const config& config) :
					wrapper_(f_),
					schema_(s),
					header_(config.to_header()),
					chunk_size_(config.chunk_size) {
						open(filename, s, config);
				}

				void open(const std::string& filename, const factory::record_schema& s, const config& c) {
					// open the file and move to offset of data, we'll write
					// headers and all other things on file close
					f_.open(filename, std::ios::binary | std::ios::trunc);
					if (!f_.good())
						throw write_open_failed();

					schema_ = s;
					header_ = c.to_header();
					chunk_size_ = c.chunk_size;

					// write junk to our prelude, we'll overwrite this with
					// awesome data later
					//
					size_t preludeSize =
						sizeof(header) +	// the LAS header
						54 + // size of one vlr header
						(34 + s.records.size() * 6) + // the LAZ vlr size
						sizeof(int64_t); // chunk table offset

					char *junk = new char[preludeSize];
					std::fill(junk, junk + preludeSize, 0);
					f_.write(junk, preludeSize);
					delete [] junk;

					// the first chunk begins at the end of prelude
				}

				void writePoint(const char *p) {
					if (chunk_state_.points_in_chunk == chunk_size_ ||
							!pcompressor_ || !pencoder_) {
						// Time to (re)init the encoder
						//
						pcompressor_.reset();
						if (pencoder_) {
							pencoder_->done(); // make sure we flush it out
							pencoder_.reset();
						}

						// reset chunk state
						//
						chunk_state_.current_chunk_index ++;
						chunk_state_.points_in_chunk = 0;

						// take note of the current offset
						std::streamsize offset = f_.tellp();
						if (chunk_state_.current_chunk_index > 0) {
							// When we hit this point the first time around, we don't do anything since we are just
							// starting to write out our first chunk.
							chunk_sizes_.push_back(offset - chunk_state_.last_chunk_write_offset);
						}

						chunk_state_.last_chunk_write_offset = offset;

						// reinit stuff
						pencoder_.reset(new encoders::arithmetic<__ofstream_wrapper<std::ofstream> >(wrapper_));
						pcompressor_ = factory::build_compressor(*pencoder_, schema_);
					}

					// now write the point
					pcompressor_->compress(p);
					chunk_state_.total_written ++;
					chunk_state_.points_in_chunk ++;


					_update_min_max(*(reinterpret_cast<const formats::las::point10*>(p)));
				}

				void close() {
					_flush();

					if (f_.is_open())
						f_.close();
				}

			private:
				void _update_min_max(const formats::las::point10& p) {
					double x = p.x * header_.scale.x + header_.offset.x,
						   y = p.y * header_.scale.y + header_.offset.y,
						   z = p.z * header_.scale.z + header_.offset.z;

					header_.minimum.x = (std::min)(x, header_.minimum.x);
					header_.minimum.y = (std::min)(y, header_.minimum.y);
					header_.minimum.z = (std::min)(z, header_.minimum.z);

					header_.maximum.x = (std::max)(x, header_.maximum.x);
					header_.maximum.y = (std::max)(y, header_.maximum.y);
					header_.maximum.z = (std::max)(z, header_.maximum.z);
				}

				void _flush() {
					// flush out the encoder
					pencoder_->done();

					// Note down the size of the offset of this last chunk
					chunk_sizes_.push_back((std::streamsize)f_.tellp() - chunk_state_.last_chunk_write_offset);

					// Time to write our header
					// Fill up things not filled up by our header
					//
					header_.magic[0] = 'L'; header_.magic[1] = 'A';
					header_.magic[2] = 'S'; header_.magic[3] = 'F';

					header_.version.major = 1;
					header_.version.minor = 2;

					header_.header_size = sizeof(header_);
					header_.point_offset = sizeof(header) + 54 + (34 + static_cast<unsigned int>(schema_.records.size()) * 6); // 54 is the size of one vlr header
					header_.vlr_count = 1;

					header_.point_format_id = schema_.format();
					header_.point_format_id |= (1 << 7);
					header_.point_record_length = static_cast<unsigned short>(schema_.size_in_bytes());
					header_.point_count = static_cast<unsigned int>(chunk_state_.total_written);

					// make sure we re-arrange mins and maxs for writing
					//
					double mx, my, mz, nx, ny, nz;
					nx = header_.minimum.x; mx = header_.maximum.x;
					ny = header_.minimum.y; my = header_.maximum.y;
					nz = header_.minimum.z; mz = header_.maximum.z;

					header_.minimum.x = mx; header_.minimum.y = nx;
					header_.minimum.z = my; header_.maximum.x = ny;
					header_.maximum.y = mz; header_.maximum.z = nz;

					f_.seekp(0);
					f_.write(reinterpret_cast<char*>(&header_), sizeof(header_));

					// before we can write the VLR, we need to write the LAS VLR definition
					// for it
					//
#pragma pack(push, 1)
					struct {
						unsigned short reserved;
						char user_id[16];
						unsigned short record_id;
						unsigned short record_length_after_header;
						char description[32];
					} las_vlr_header;
#pragma pack(pop)

					las_vlr_header.reserved = 0;
					las_vlr_header.record_id = 22204;
					las_vlr_header.record_length_after_header = static_cast<unsigned short>(34 + (schema_.records.size() * 6));

					strcpy(las_vlr_header.user_id, "laszip encoded");
					strcpy(las_vlr_header.description, "laz-perf variant");

					// write the las vlr header
					f_.write(reinterpret_cast<char*>(&las_vlr_header), sizeof(las_vlr_header));


					// prep our VLR so we can write it
					//
					laz_vlr vlr = laz_vlr::from_schema(schema_, chunk_size_);

                    std::unique_ptr<char> vlrbuf(new char[vlr.size()]);
                    vlr.extract(vlrbuf.get());
                    f_.write(vlrbuf.get(), vlr.size());

					// TODO: Write chunk table
					//
					_writeChunks();
				}

				void _writeChunks() {
					// move to the end of the file to start emitting our compresed table
					f_.seekp(0, std::ios::end);

					// take note of where we're writing the chunk table, we need this later
					int64_t chunk_table_offset = static_cast<int64_t>(f_.tellp());

					// write out the chunk table header (version and total chunks)
#pragma pack(push, 1)
					struct {
						unsigned int version,
									 chunks_count;
					} chunk_table_header = { 0, static_cast<unsigned int>(chunk_sizes_.size()) };
#pragma pack(pop)

					f_.write(reinterpret_cast<char*>(&chunk_table_header),
							 sizeof(chunk_table_header));


					// Now compress and write the chunk table
					//
					__ofstream_wrapper<std::ofstream> w(f_);

					encoders::arithmetic<__ofstream_wrapper<std::ofstream> > encoder(w);
					compressors::integer comp(32, 2);

					comp.init();

					for (size_t i = 0 ; i < chunk_sizes_.size() ; i ++) {
						comp.compress(encoder,
								i ? static_cast<int>(chunk_sizes_[i-1]) : 0,
								static_cast<int>(chunk_sizes_[i]), 1);
					}

					encoder.done();

					// go back to where we're supposed to write chunk table offset
					f_.seekp(header_.point_offset);
					f_.write(reinterpret_cast<char*>(&chunk_table_offset), sizeof(chunk_table_offset));
				}

				std::ofstream f_;
				__ofstream_wrapper<std::ofstream> wrapper_;

				formats::dynamic_compressor::ptr pcompressor_;
				std::shared_ptr<encoders::arithmetic<__ofstream_wrapper<std::ofstream> > > pencoder_;

				factory::record_schema schema_;

				header header_;
				unsigned int chunk_size_;

				struct __chunk_state {
					int64_t total_written; // total points written
					int64_t current_chunk_index; //  the current chunk index we're compressing
					unsigned int points_in_chunk;
					std::streamsize last_chunk_write_offset;
					__chunk_state() : total_written(0), current_chunk_index(-1), points_in_chunk(0), last_chunk_write_offset(0) {}
				} chunk_state_;

				std::vector<int64_t> chunk_sizes_; // all the places where chunks begin
			};
		}
	}
}

#endif // __io_hpp__
