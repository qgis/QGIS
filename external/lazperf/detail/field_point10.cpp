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

#include "../las.hpp"

namespace lazperf
{
namespace detail
{

namespace
{
    // for LAS files with the return (r) and the number (n) of
    // returns field correctly populated the mapping should really
    // be only the following.
    //  { 15, 15, 15, 15, 15, 15, 15, 15 },
    //  { 15,  0, 15, 15, 15, 15, 15, 15 },
    //  { 15,  1,  2, 15, 15, 15, 15, 15 },
    //  { 15,  3,  4,  5, 15, 15, 15, 15 },
    //  { 15,  6,  7,  8,  9, 15, 15, 15 },
    //  { 15, 10, 11, 12, 13, 14, 15, 15 },
    //  { 15, 15, 15, 15, 15, 15, 15, 15 },
    //  { 15, 15, 15, 15, 15, 15, 15, 15 }
    // however, some files start the numbering of r and n with 0,
    // only have return counts r, or only have number of return
    // counts n, or mix up the position of r and n. we therefore
    // "complete" the table to also map those "undesired" r & n
    // combinations to different contexts
    const unsigned char number_return_map[8][8] =
    {
        { 15, 14, 13, 12, 11, 10,  9,  8 },
        { 14,  0,  1,  3,  6, 10, 10,  9 },
        { 13,  1,  2,  4,  7, 11, 11, 10 },
        { 12,  3,  4,  5,  8, 12, 12, 11 },
        { 11,  6,  7,  8,  9, 13, 13, 12 },
        { 10, 10, 11, 12, 13, 14, 14, 13 },
        {  9, 10, 11, 12, 13, 14, 15, 14 },
        {  8,  9, 10, 11, 12, 13, 14, 15 }
    };

    // for LAS files with the return (r) and the number (n) of
    // returns field correctly populated the mapping should really
    // be only the following.
    //  {  0,  7,  7,  7,  7,  7,  7,  7 },
    //  {  7,  0,  7,  7,  7,  7,  7,  7 },
    //  {  7,  1,  0,  7,  7,  7,  7,  7 },
    //  {  7,  2,  1,  0,  7,  7,  7,  7 },
    //  {  7,  3,  2,  1,  0,  7,  7,  7 },
    //  {  7,  4,  3,  2,  1,  0,  7,  7 },
    //  {  7,  5,  4,  3,  2,  1,  0,  7 },
    //  {  7,  6,  5,  4,  3,  2,  1,  0 }
    // however, some files start the numbering of r and n with 0,
    // only have return counts r, or only have number of return
    // counts n, or mix up the position of r and n. we therefore
    // "complete" the table to also map those "undesired" r & n
    // combinations to different contexts
    const unsigned char number_return_level[8][8] =
    {
        {  0,  1,  2,  3,  4,  5,  6,  7 },
        {  1,  0,  1,  2,  3,  4,  5,  6 },
        {  2,  1,  0,  1,  2,  3,  4,  5 },
        {  3,  2,  1,  0,  1,  2,  3,  4 },
        {  4,  3,  2,  1,  0,  1,  2,  3 },
        {  5,  4,  3,  2,  1,  0,  1,  2 },
        {  6,  5,  4,  3,  2,  1,  0,  1 },
        {  7,  6,  5,  4,  3,  2,  1,  0 }
    };


    int changed_values(const las::point10& this_val, const las::point10& last,
        unsigned short last_intensity)
    {
        // This logic here constructs a 5-bit changed value which is basically a bit map of
        // what has changed since the last point, not considering the x, y and z values
        int bitfields_changed = (
            (last.return_number ^ this_val.return_number) |
            (last.number_of_returns_of_given_pulse ^ this_val.number_of_returns_of_given_pulse) |
            (last.scan_direction_flag ^ this_val.scan_direction_flag) |
            (last.edge_of_flight_line ^ this_val.edge_of_flight_line)) != 0;

        // last intensity is not checked with last point, but the passed in last intensity value
        int intensity_changed = (last_intensity ^ this_val.intensity) != 0;
        int classification_changed = (last.classification ^ this_val.classification) != 0;
        int scan_angle_rank_changed = (last.scan_angle_rank ^ this_val.scan_angle_rank) != 0;
        int user_data_changed = (last.user_data ^ this_val.user_data) != 0;
        int point_source_changed = (last.point_source_ID ^ this_val.point_source_ID) != 0;

        return (bitfields_changed << 5) |
               (intensity_changed << 4) |
               (classification_changed << 3) |
               (scan_angle_rank_changed << 2) |
               (user_data_changed << 1) |
               (point_source_changed);
    }
} // unnamed namespace

Point10Base::Point10Base() : m_changed_values(64), have_last_(false)
{
    last_intensity.fill(0);

    m_scan_angle_rank[0] = new models::arithmetic(256);
    m_scan_angle_rank[1] = new models::arithmetic(256);

    last_height.fill(0);

    for (int i = 0; i < 256; i ++) {
        m_bit_byte[i] = new models::arithmetic(256);
        m_classification[i] = new models::arithmetic(256);
        m_user_data[i] = new models::arithmetic(256);
    }
}

Point10Base::~Point10Base()
{
    delete m_scan_angle_rank[0];
    delete m_scan_angle_rank[1];

    for (int i = 0 ; i < 256 ; i ++)
    {
        delete m_bit_byte[i];
        delete m_classification[i];
        delete m_user_data[i];
    }
}

// COMPRESSOR

Point10Compressor::Point10Compressor(encoders::arithmetic<OutCbStream>& enc) : enc_(enc),
    ic_intensity(16, 4), ic_point_source_ID(16), ic_dx(32, 2), ic_dy(32, 22), ic_z(32, 20),
    compressors_inited_(false)
{}

void Point10Compressor::init()
{
    ic_intensity.init();
    ic_point_source_ID.init();
    ic_dx.init();
    ic_dy.init();
    ic_z.init();
}

const char *Point10Compressor::compress(const char *buf)
{
    las::point10 this_val(buf);

    if (!compressors_inited_)
    {
        init();
        compressors_inited_ = true;
    }

    // don't have the first data yet, just push it to our have last stuff and move on
    if (!have_last_)
    {
        have_last_ = true;
        last_ = this_val;

        // write this out to the encoder as it is
        enc_.getOutStream().putBytes((const unsigned char*)buf, sizeof(las::point10));
        return buf + sizeof(las::point10);
    }

    // this is not the first point we're trying to compress, do crazy things
    unsigned int r = this_val.return_number,
                 n = this_val.number_of_returns_of_given_pulse,
                 m = number_return_map[n][r],
                 l = number_return_level[n][r];

    unsigned int k_bits;
    int median, diff;

    // compress which other values have changed
    int changed_values = detail::changed_values(this_val, last_, last_intensity[m]);
    enc_.encodeSymbol(m_changed_values, changed_values);

    // if any of the bit fields changed, compress them
    if (changed_values & (1 << 5))
    {
        unsigned char b = this_val.from_bitfields();
        unsigned char last_b = last_.from_bitfields();
        enc_.encodeSymbol(*m_bit_byte[last_b], b);
    }

    // if the intensity changed, compress it
    if (changed_values & (1 << 4))
    {
        ic_intensity.compress(enc_, last_intensity[m], this_val.intensity, (m < 3 ? m : 3));
        last_intensity[m] = this_val.intensity;
    }

    // if the classification has changed, compress it
    if (changed_values & (1 << 3))
    {
        enc_.encodeSymbol(*m_classification[last_.classification],
            this_val.classification);
    }

    // if the scan angle rank has changed, compress it
    if (changed_values & (1 << 2))
    {
        enc_.encodeSymbol(*m_scan_angle_rank[this_val.scan_direction_flag],
            uint8_t(this_val.scan_angle_rank - last_.scan_angle_rank));
    }

    // encode user data if changed
    if (changed_values & (1 << 1))
    {
        enc_.encodeSymbol(*m_user_data[last_.user_data], this_val.user_data);
    }

    // if the point source id was changed, compress it
    if (changed_values & 1)
    {
        ic_point_source_ID.compress(enc_, last_.point_source_ID, this_val.point_source_ID, 0);
    }

    // compress x coordinate
    median = last_x_diff_median5[m].get();
    diff = this_val.x - last_.x;
    ic_dx.compress(enc_, median, diff, n == 1);
    last_x_diff_median5[m].add(diff);

    // compress y coordinate
    k_bits = ic_dx.getK();
    median = last_y_diff_median5[m].get();
    diff = this_val.y - last_.y;
    ic_dy.compress(enc_, median, diff, (n==1) + ( k_bits < 20 ? utils::clearBit<0>(k_bits) : 20));
    last_y_diff_median5[m].add(diff);

    // compress z coordinate
    k_bits = (ic_dx.getK() + ic_dy.getK()) / 2;
    ic_z.compress(enc_, last_height[l], this_val.z,
        (n==1) + (k_bits < 18 ? utils::clearBit<0>(k_bits) : 18));
    last_height[l] = this_val.z;
    last_ = this_val;
    return buf + sizeof(las::point10);
}

// DECOMPRESSOR

Point10Decompressor::Point10Decompressor(decoders::arithmetic<InCbStream>& decoder) : dec_(decoder),
    ic_intensity(16, 4), ic_point_source_ID(16), ic_dx(32, 2), ic_dy(32, 22), ic_z(32, 20),
    decompressors_inited_(false)
{}

void Point10Decompressor::init()
{
    ic_intensity.init();
    ic_point_source_ID.init();
    ic_dx.init();
    ic_dy.init();
    ic_z.init();
}

char *Point10Decompressor::decompress(char *buf)
{
    if (!decompressors_inited_)
    {
        init();
        decompressors_inited_ = true;
    }

    // don't have the first data yet, read the whole point out of the stream
    if (!have_last_)
    {
        have_last_ = true;
        dec_.getInStream().getBytes((unsigned char*)buf, sizeof(las::point10));
        // decode this value
        last_.unpack(buf);
        last_.intensity = 0;
        return buf + sizeof(las::point10);
    }

    unsigned int r, n, m, l, k_bits;
    int median, diff;

    // decompress which other values have changed
    int changed_values = dec_.decodeSymbol(m_changed_values);
    if (changed_values)
    {
        // there was some change in one of the fields (other than x, y and z)

        // decode bit fields if they have changed
        if (changed_values & (1 << 5))
        {
            unsigned char b = last_.from_bitfields();
            b = (unsigned char)dec_.decodeSymbol(*m_bit_byte[b]);
            last_.to_bitfields(b);
        }

        r = last_.return_number;
        n = last_.number_of_returns_of_given_pulse;
        m = number_return_map[n][r];
        l = number_return_level[n][r];

        // decompress the intensity if it has changed
        if (changed_values & (1 << 4))
        {
            last_.intensity = static_cast<unsigned short>(
                ic_intensity.decompress(dec_, last_intensity[m], (m < 3 ? m : 3)));
            last_intensity[m] = last_.intensity;
        }
        else
            last_.intensity = last_intensity[m];

        // decompress the classification ... if it has changed
        if (changed_values & (1 << 3)) {
            last_.classification =
                (unsigned char)dec_.decodeSymbol(*m_classification[last_.classification]);
        }

        // decompress the scan angle rank if needed
        if (changed_values & (1 << 2))
        {
            int val = dec_.decodeSymbol(*m_scan_angle_rank[last_.scan_direction_flag]);
            last_.scan_angle_rank = uint8_t(val + last_.scan_angle_rank);
        }

        // decompress the user data
        if (changed_values & (1 << 1))
        {
            last_.user_data = (unsigned char)dec_.decodeSymbol(*m_user_data[last_.user_data]);
        }

        // decompress the point source ID
        if (changed_values & 1)
        {
            last_.point_source_ID = (unsigned short)ic_point_source_ID.decompress(dec_,
                last_.point_source_ID, 0);
        }
    }
    else
    {
        r = last_.return_number;
        n = last_.number_of_returns_of_given_pulse;
        m = number_return_map[n][r];
        l = number_return_level[n][r];
    }

    // decompress x coordinate
    median = last_x_diff_median5[m].get();
    diff = ic_dx.decompress(dec_, median, n==1);
    last_.x += diff;
    last_x_diff_median5[m].add(diff);

    // decompress y coordinate
    median = last_y_diff_median5[m].get();
    k_bits = ic_dx.getK();
    diff = ic_dy.decompress(dec_, median, (n==1) + (k_bits < 20 ? utils::clearBit<0>(k_bits) : 20));
    last_.y += diff;
    last_y_diff_median5[m].add(diff);

    // decompress z coordinate
    k_bits = (ic_dx.getK() + ic_dy.getK()) / 2;
    last_.z = ic_z.decompress(dec_, last_height[l],
        (n==1) + (k_bits < 18 ? utils::clearBit<0>(k_bits) : 18));
    last_height[l] = last_.z;

    last_.pack(buf);
    return buf + sizeof(las::point10);
}

} // namespace detail
} // namespace lazperf
