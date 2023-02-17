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

#include "../compressor.hpp"
#include "../decompressor.hpp"

namespace lazperf
{
namespace detail
{

class Point10Base
{
protected:
    Point10Base();
    ~Point10Base();

    las::point10 last_;
    std::array<unsigned short, 16> last_intensity;

    std::array<utils::streaming_median<int>, 16> last_x_diff_median5;
    std::array<utils::streaming_median<int>, 16> last_y_diff_median5;

    std::array<int, 8> last_height;
    models::arithmetic m_changed_values;

    // Arithmetic model has no default constructor, so we store they here as raw pointers
    std::array<models::arithmetic*, 2> m_scan_angle_rank;
    std::array<models::arithmetic*, 256> m_bit_byte;
    std::array<models::arithmetic*, 256> m_classification;
    std::array<models::arithmetic*, 256> m_user_data;
    bool have_last_;
};

class Point10Compressor : public Point10Base
{
public:
    Point10Compressor(encoders::arithmetic<OutCbStream>&);

    const char *compress(const char *buf);

private:
    void init();

    encoders::arithmetic<OutCbStream>& enc_;
    compressors::integer ic_intensity;
    compressors::integer ic_point_source_ID;
    compressors::integer ic_dx;
    compressors::integer ic_dy;
    compressors::integer ic_z;
    bool compressors_inited_;
};

class Point10Decompressor : public Point10Base
{
public:
    Point10Decompressor(decoders::arithmetic<InCbStream>&);

    char *decompress(char *buf);

private:
    void init();

    decoders::arithmetic<InCbStream>& dec_;
    decompressors::integer ic_intensity;
    decompressors::integer ic_point_source_ID;
    decompressors::integer ic_dx;
    decompressors::integer ic_dy;
    decompressors::integer ic_z;
    bool decompressors_inited_;
};

} // namespace detail
} // namespace lazperf
