/*
===============================================================================

  FILE:  formats.hpp

  CONTENTS:
    Format support

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
#ifndef __formats_hpp__
#define __formats_hpp__

#include <cstdint>
#include <iostream>
#include "compressor.hpp"
#include "decompressor.hpp"

namespace laszip {
	namespace formats {
		template<typename T>
		struct packers {
			static_assert(sizeof(T) == 0,
					"Only specialized instances of packers should be used");
		};

		template<>
		struct packers<uint32_t> {
			static unsigned int unpack(const char *in) {
				uint32_t b1 = in[0],
					b2 = in[1],
					b3 = in[2],
					b4 = in[3];

				return (b4 << 24) |
					((b3 & 0xFF) << 16) |
					((b2 & 0xFF) << 8) |
					(b1 & 0xFF);
			}

			static void pack(uint32_t v, char *out) {
				out[3] = (v >> 24) & 0xFF;
				out[2] = (v >> 16) & 0xFF;
				out[1] = (v >> 8) & 0xFF;
				out[0] = v & 0xFF;
			}
		};

		template<>
		struct packers<uint16_t> {
			static unsigned short unpack(const char *in) {
				uint16_t b1 = in[0],
							 b2 = in[1];

				return (((b2 & 0xFF) << 8) | (b1 & 0xFF));
			}

			static void pack(uint16_t v, char *out) {
				out[1] = (v >> 8) & 0xFF;
				out[0] = v & 0xFF;
			}
		};

		template<>
		struct packers<uint8_t> {
			static unsigned char unpack(const char *in) {
				return static_cast<uint8_t>(in[0]);
			}

			static void pack(uint8_t c, char *out) {
				out[0] = static_cast<char>(c);
			}
		};

		template<>
		struct packers<int32_t> {
			static int unpack(const char *in) {
				return static_cast<int32_t>(packers<uint32_t>::unpack(in));
			}

			static void pack(int32_t t, char *out) {
				packers<uint32_t>::pack(static_cast<uint32_t>(t), out);
			}
		};

		template<>
		struct packers<int16_t> {
			static short unpack(const char *in) {
				return static_cast<int16_t>(packers<uint16_t>::unpack(in));
			}

			static void pack(int16_t t, char *out) {
				packers<uint16_t>::pack(static_cast<uint16_t>(t), out);
			}
		};

		template<>
		struct packers<int8_t> {
			static int8_t unpack(const char *in) {
				return in[0];
			}

			static void pack(int8_t t, char *out) {
				out[0] = t;
			}
		};

        // Char is neither signed char nor unsigned char.
		template<>
		struct packers<char> {
			static char unpack(const char *in) {
				return in[0];
			}

			static void pack(char t, char *out) {
				out[0] = t;
			}
		};

		/** A simple strategy which returns simple diffs */
		template<typename T>
		struct standard_diff_method {
			standard_diff_method<T>()
				: have_value_(false) {}

			inline void push(const T& v) {
				if (!have_value_)
					have_value_ = true;

				value = v;
			}

			inline bool have_value() const {
				return have_value_;
			}

			T value;
			bool have_value_;
		};

		struct base_field {
			typedef std::shared_ptr<base_field> ptr;

			virtual ~base_field() {
				// hello 1996
			}

			virtual const char *compressRaw(const char *buf)
            { return buf; }
			virtual char *decompressRaw(char *buf)
            { return buf; }
		};

		template<typename T, typename TDiffMethod = standard_diff_method<T> >
		struct field {
			static_assert(std::is_integral<T>::value,
					"Default implementation for field only handles integral types");

			typedef T type;

			field() :
				compressor_(sizeof(T) * 8),
				decompressor_(sizeof(T) * 8),
				compressor_inited_(false),
				decompressor_inited_(false) { }

			template<
				typename TEncoder
			>
			inline const char *compressWith(TEncoder& encoder,
                const char *buf)
            {
                T this_val = packers<type>::unpack(buf);
				if (!compressor_inited_)
					compressor_.init();

				// Let the differ decide what values we're going to push
				//
				if (differ_.have_value()) {
					compressor_.compress(encoder, differ_.value, this_val, 0);
				}
				else {
					// differ is not ready for us to start encoding values
                    // for us, so we need to write raw into
					// the outputstream
					//
					encoder.getOutStream().putBytes((const unsigned char*)buf,
                        sizeof(T));
				}
				differ_.push(this_val);
                return buf + sizeof(T);
			}

			template<
				typename TDecoder
			>
			inline char *decompressWith(TDecoder& decoder, char *buffer)
            {
				if (!decompressor_inited_)
					decompressor_.init();

				T r;
				if (differ_.have_value()) {
					r = static_cast<T>(decompressor_.decompress(decoder, differ_.value, 0));
                    packers<T>::pack(r, buffer);
				}
				else {
					// this is probably the first time we're reading stuff, read the record as is
					decoder.getInStream().getBytes((unsigned char*)buffer, sizeof(T));

					r = packers<T>::unpack(buffer);
				}
                buffer += sizeof(T);

				differ_.push(r);
				return buffer;
			}

			laszip::compressors::integer compressor_;
			laszip::decompressors::integer decompressor_;

			bool compressor_inited_, decompressor_inited_;

			TDiffMethod differ_;
		};

		template<typename... TS>
		struct record_compressor;

		template<typename... TS>
		struct record_decompressor;

		template<>
		struct record_compressor<> {
			record_compressor() {}
			template<
				typename T
			>
			inline const char *compressWith(T&, const char *buf)
            { return buf; }
		};

		template<typename T, typename... TS>
		struct record_compressor<T, TS...> {
			record_compressor() {}

			template<
				typename TEncoder
			>
			inline const char *compressWith(TEncoder& encoder,
                const char *buffer)
            {
                buffer = field_.compressWith(encoder, buffer);

				// Move on to the next field
				return next_.compressWith(encoder, buffer);
			}

            // The field that we handle
            T field_;

			// Our default strategy right now is to just encode diffs,
            // but we would employ more advanced techniques soon
			record_compressor<TS...> next_;
		};

		template<>
		struct record_decompressor<> {
			record_decompressor() : firstDecompress(true) {}
			template<
				typename TDecoder
			>
			inline char *decompressWith(TDecoder& decoder, char *buf) {
				if (firstDecompress) {
					decoder.readInitBytes();
					firstDecompress = false;
				}
                return buf;
			}

			bool firstDecompress;
		};

		template<typename T, typename... TS>
		struct record_decompressor<T, TS...> {
			record_decompressor() {}

			template<
				typename TDecoder
			>
			inline char *decompressWith(TDecoder& decoder, char *buf) {
                buf = field_.decompressWith(decoder, buf);

				// Move on to the next field
				return next_.decompressWith(decoder, buf);
			}

            // The field that we handle
            T field_;

			// Our default strategy right now is to just encode diffs, but we would employ more advanced techniques soon
			record_decompressor<TS...> next_;
		};

		struct dynamic_compressor {
			typedef std::shared_ptr<dynamic_compressor> ptr;

			virtual const char *compress(const char *in) = 0;
			virtual ~dynamic_compressor() {}
		};

		struct dynamic_decompressor {
			typedef std::shared_ptr<dynamic_decompressor> ptr;

			virtual char *decompress(char *in) = 0;
			virtual ~dynamic_decompressor() {}
		};

		template<
			typename TEncoder,
			typename TRecordCompressor
		>
		struct dynamic_compressor1 : public dynamic_compressor {
			dynamic_compressor1(TEncoder& enc, TRecordCompressor* compressor) :
				enc_(enc), compressor_(compressor) {}

			virtual const char *compress(const char *in) {
				return compressor_->compressWith(enc_, in);
			}

			dynamic_compressor1(const dynamic_compressor1<TEncoder , TRecordCompressor>&) = delete;
			dynamic_compressor1<TEncoder, TRecordCompressor>& operator=(dynamic_compressor1<TEncoder, TRecordCompressor>&) = delete;

			TEncoder& enc_;
			std::unique_ptr<TRecordCompressor> compressor_;
		};

		template<
			typename TEncoder,
			typename TRecordCompressor
		>
		static dynamic_compressor::ptr make_dynamic_compressor(TEncoder& encoder, TRecordCompressor* compressor) {
			return dynamic_compressor::ptr(
					new dynamic_compressor1<TEncoder, TRecordCompressor>(encoder, compressor));
		}

		template<
			typename TDecoder,
			typename TRecordDecompressor
		>
		struct dynamic_decompressor1 : public dynamic_decompressor {
			dynamic_decompressor1(TDecoder& dec, TRecordDecompressor* decompressor) :
				dec_(dec), decompressor_(decompressor) {}

			virtual char *decompress(char *in)
            {
				return decompressor_->decompressWith(dec_, in);
			}

			dynamic_decompressor1(const dynamic_decompressor1<TDecoder, TRecordDecompressor>&) = delete;
			dynamic_decompressor1<TDecoder, TRecordDecompressor>& operator=(dynamic_decompressor1<TDecoder, TRecordDecompressor>&) = delete;

			TDecoder& dec_;
			std::unique_ptr<TRecordDecompressor> decompressor_;
		};

		template<
			typename TDecoder,
			typename TRecordDecompressor
		>
		static dynamic_decompressor::ptr make_dynamic_decompressor(TDecoder& decoder, TRecordDecompressor* decompressor) {
			return dynamic_decompressor::ptr(
					new dynamic_decompressor1<TDecoder, TRecordDecompressor>(decoder, decompressor));
		}

		// type-erasure stuff for fields
		template<
			typename TEncoderDecoder,
			typename TField
		>
		struct dynamic_compressor_field : base_field {
			dynamic_compressor_field(TEncoderDecoder& encdec) :
                encdec_(encdec), field_()
            {}

			dynamic_compressor_field(TEncoderDecoder& encdec, const TField& f) :
                encdec_(encdec), field_(f)
            {}

			virtual const char *compressRaw(const char *in) {
				return field_.compressWith(encdec_, in);
			}

			TEncoderDecoder& encdec_;
			TField field_;
		};

		template<
			typename TEncoderDecoder,
			typename TField
		>
		struct dynamic_decompressor_field : base_field {
			dynamic_decompressor_field(TEncoderDecoder& encdec) :
                encdec_(encdec), field_()
            {}

			dynamic_decompressor_field(TEncoderDecoder& encdec,
                const TField& f) : encdec_(encdec), field_(f)
            {}

			virtual char *decompressRaw(char *buf) {
                return field_.decompressWith(encdec_, buf);
			}

			TEncoderDecoder& encdec_;
			TField field_;
		};

		template<
			typename TEncoder
		>
		struct dynamic_field_compressor: dynamic_compressor {
            typedef dynamic_field_compressor<TEncoder>  this_type;
            typedef std::shared_ptr<this_type>          ptr;

			dynamic_field_compressor(TEncoder& encoder) :
                enc_(encoder), fields_()
            {}

			template<typename TFieldType>
			void add_field()
            {
                using TField = field<TFieldType>;

				fields_.push_back(base_field::ptr(new
                    dynamic_compressor_field<TEncoder, TField>(enc_)));
			}

			template<typename TField>
			void add_field(const TField& f)
            {
				fields_.push_back(base_field::ptr(new
                    dynamic_compressor_field<TEncoder, TField>(enc_, f)));
			}

			virtual const char *compress(const char *in)
            {
                for (auto f: fields_)
                    in = f->compressRaw(in);
                return in;
			}

			TEncoder& enc_;
			std::vector<base_field::ptr> fields_;
		};


        template<
            typename TDecoder
        >
        struct dynamic_field_decompressor : dynamic_decompressor {
            typedef dynamic_field_decompressor<TDecoder> this_type;
            typedef std::shared_ptr<this_type>           ptr;

			dynamic_field_decompressor(TDecoder& decoder) :
                dec_(decoder), fields_(), first_decomp_(true)
            {}

			template<typename TFieldType>
			void add_field()
            {
                using TField = field<TFieldType>;

				fields_.push_back(base_field::ptr(new
                    dynamic_decompressor_field<TDecoder, TField>(dec_)));
			}

            template<typename TField>
            void add_field(const TField& f)
            {
				fields_.push_back(base_field::ptr(new
                    dynamic_decompressor_field<TDecoder, TField>(dec_, f)));
			}


			virtual char *decompress(char *out)
            {
                for (auto f: fields_)
                    out = f->decompressRaw(out);

                // the decoder needs to be told that it should read the
                // init bytes after the first record has been read
                //
                if (first_decomp_) {
                    first_decomp_ = false;
                    dec_.readInitBytes();
                }
                return out;
			}

			TDecoder& dec_;
			std::vector<base_field::ptr> fields_;
            bool first_decomp_;
        };

        template<
            typename TEncoder
        >
        static typename dynamic_field_compressor<TEncoder>::ptr make_dynamic_compressor(TEncoder& encoder) {
            typedef typename dynamic_field_compressor<TEncoder>::ptr ptr;
            return ptr(new dynamic_field_compressor<TEncoder>(encoder));
        }


		template<
			typename TDecoder
		>
		static typename dynamic_field_decompressor<TDecoder>::ptr make_dynamic_decompressor(TDecoder& decoder) {
            typedef typename dynamic_field_decompressor<TDecoder>::ptr ptr;
            return ptr(new dynamic_field_decompressor<TDecoder>(decoder));
		}


	}
}

#endif // __formats_hpp__
