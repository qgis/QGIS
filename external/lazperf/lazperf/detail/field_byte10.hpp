/*
===============================================================================

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

===============================================================================
*/

#include <deque>

namespace lazperf
{
namespace detail
{

class Byte10Base
{
protected:
    Byte10Base(size_t count);

    size_t count_;
    bool have_last_;
    std::vector<uint8_t> lasts_;
    std::vector<uint8_t> diffs_;
    std::deque<models::arithmetic> models_;
};

class Byte10Compressor : public Byte10Base
{
public:
    Byte10Compressor(encoders::arithmetic<OutCbStream>& encoder, size_t count);

    const char *compress(const char *buf);

private:
    encoders::arithmetic<OutCbStream>& enc_;
};

class Byte10Decompressor : public Byte10Base
{
public:
    Byte10Decompressor(decoders::arithmetic<InCbStream>& decoder, size_t count);

    char *decompress(char *buf);

private:
    decoders::arithmetic<InCbStream>& dec_;
};

} // namespace detail
} // namespace lazperf
