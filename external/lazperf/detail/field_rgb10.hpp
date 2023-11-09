/*
===============================================================================

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

class Rgb10Base
{
protected:
    Rgb10Base();

    bool have_last_;
    las::rgb last;

    models::arithmetic m_byte_used;
    models::arithmetic m_rgb_diff_0;
    models::arithmetic m_rgb_diff_1;
    models::arithmetic m_rgb_diff_2;
    models::arithmetic m_rgb_diff_3;
    models::arithmetic m_rgb_diff_4;
    models::arithmetic m_rgb_diff_5;
};

class Rgb10Compressor : public Rgb10Base
{
public:
    Rgb10Compressor(encoders::arithmetic<OutCbStream>&);

    const char *compress(const char *buf);

private:
    encoders::arithmetic<OutCbStream>& enc_;
};

class Rgb10Decompressor : public Rgb10Base
{
public:
    Rgb10Decompressor(decoders::arithmetic<InCbStream>&);

    char *decompress(char *buf);

private:
    decoders::arithmetic<InCbStream>& dec_;
};

} // namespace detail
} // namespace lazperf
