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

namespace lazperf
{
namespace detail
{

class Gpstime10Base
{
protected:
    Gpstime10Base();

    bool have_last_;
    models::arithmetic m_gpstime_multi, m_gpstime_0diff;
    unsigned int last;
    unsigned int next;
    std::array<las::gpstime, 4> last_gpstime;
    std::array<int, 4> last_gpstime_diff;
    std::array<int, 4> multi_extreme_counter;
};

class Gpstime10Compressor : public Gpstime10Base
{
public:
    Gpstime10Compressor(encoders::arithmetic<OutCbStream>&);

    const char *compress(const char *c);

private:
    void init();

    encoders::arithmetic<OutCbStream>& enc_;
    bool compressor_inited_;
    compressors::integer ic_gpstime;
};

class Gpstime10Decompressor : public Gpstime10Base
{
public:
    Gpstime10Decompressor(decoders::arithmetic<InCbStream>&);

    char *decompress(char *c);

private:
    void init();

    decoders::arithmetic<InCbStream>& dec_;
    bool decompressor_inited_;
    decompressors::integer ic_gpstime;
};

} // namespace detail
} // namespace lazperf
