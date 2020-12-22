/*
===============================================================================

  FILE:  main.cpp

  CONTENTS:
    Run templatized LASzip

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

#include "../common/common.hpp"
#include "compressor.hpp"
#include "decompressor.hpp"

#include "encoder.hpp"
#include "decoder.hpp"
#include "formats.hpp"

#include <iostream>
#include <memory>

struct SuchStream {
	SuchStream() : buf(), idx(0) {}

	void putBytes(const unsigned char* b, size_t len) {
		while(len --) {
			buf.push_back(*b++);
		}
	}

	void putByte(const unsigned char b) {
		buf.push_back(b);
	}

	unsigned char getByte() {
		return buf[idx++];
	}

	void getBytes(unsigned char *b, int len) {
		for (int i = 0 ; i < len ; i ++) {
			b[i] = getByte();
		}
	}

	std::vector<unsigned char> buf;	// cuz I'm ze faste
	size_t idx;
};

void runEncoderDecoder() {
	using namespace laszip;
	using namespace laszip::formats;

	auto start = common::tick();
	std::cout << common::since(start) << std::endl;

	record_compressor<
		field<int>,
		field<short>,
		field<unsigned short>,
		field<unsigned int> > compressor;

	// Throw stuff down at the coder and capture the output
	SuchStream s;

	encoders::arithmetic<SuchStream> encoder(s);
	struct {
		int a;
		short b;
		unsigned short c;
		unsigned int d;
	} data;


	for (int i = 0 ; i < 10; i ++) {
		data.a = i;
		data.b = i + 10;
		data.c = i + 40000;
		data.d = i + (1 << 31);

		compressor.compressWith(encoder, (const char*)&data);
	}
	encoder.done();

	std::cout << "Points compressed to: " << s.buf.size() << " bytes" << std::endl;

	record_decompressor<
		field<int>,
		field<short>,
		field<unsigned short>,
		field<unsigned int> > decompressor;

	decoders::arithmetic<SuchStream> decoder(s);

	for (int i = 0 ; i < 10 ; i ++) {
		decompressor.decompressWith(decoder, (char *)&data);

		if (data.a != i ||
			data.b != i + 10 ||
			data.c != i + 40000 ||
			data.d != i + (1 << 31))
			throw std::runtime_error("Failure!");
	}
}

int main() {
	runEncoderDecoder();

	return 0;
}
