/*
===============================================================================

  FILE:  field_point14.hpp

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

#include <cmath>
#include <cstdint>
#include "../las.hpp"

namespace lazperf
{
namespace detail
{

namespace {
    const uint8_t number_return_map_6ctx[16][16] = {
        {  0,  1,  2,  3,  4,  5,  3,  4,  4,  5,  5,  5,  5,  5,  5,  5 },
        {  1,  0,  1,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3 },
        {  2,  1,  2,  4,  4,  4,  4,  4,  4,  4,  4,  3,  3,  3,  3,  3 },
        {  3,  3,  4,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },
        {  4,  3,  4,  4,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },
        {  5,  3,  4,  4,  4,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4 },
        {  3,  3,  4,  4,  4,  4,  5,  4,  4,  4,  4,  4,  4,  4,  4,  4 },
        {  4,  3,  4,  4,  4,  4,  4,  5,  4,  4,  4,  4,  4,  4,  4,  4 },
        {  4,  3,  4,  4,  4,  4,  4,  4,  5,  4,  4,  4,  4,  4,  4,  4 },
        {  5,  3,  4,  4,  4,  4,  4,  4,  4,  5,  4,  4,  4,  4,  4,  4 },
        {  5,  3,  4,  4,  4,  4,  4,  4,  4,  4,  5,  4,  4,  4,  4,  4 },
        {  5,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  4,  4,  4 },
        {  5,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  4,  4 },
        {  5,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5,  4 },
        {  5,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5,  5 },
        {  5,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  5,  5 }
    };
    const uint8_t number_return_level_8ctx[16][16] =
    {
        {  0,  1,  2,  3,  4,  5,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7 },
        {  1,  0,  1,  2,  3,  4,  5,  6,  7,  7,  7,  7,  7,  7,  7,  7 },
        {  2,  1,  0,  1,  2,  3,  4,  5,  6,  7,  7,  7,  7,  7,  7,  7 },
        {  3,  2,  1,  0,  1,  2,  3,  4,  5,  6,  7,  7,  7,  7,  7,  7 },
        {  4,  3,  2,  1,  0,  1,  2,  3,  4,  5,  6,  7,  7,  7,  7,  7 },
        {  5,  4,  3,  2,  1,  0,  1,  2,  3,  4,  5,  6,  7,  7,  7,  7 },
        {  6,  5,  4,  3,  2,  1,  0,  1,  2,  3,  4,  5,  6,  7,  7,  7 },
        {  7,  6,  5,  4,  3,  2,  1,  0,  1,  2,  3,  4,  5,  6,  7,  7 },
        {  7,  7,  6,  5,  4,  3,  2,  1,  0,  1,  2,  3,  4,  5,  6,  7 },
        {  7,  7,  7,  6,  5,  4,  3,  2,  1,  0,  1,  2,  3,  4,  5,  6 },
        {  7,  7,  7,  7,  6,  5,  4,  3,  2,  1,  0,  1,  2,  3,  4,  5 },
        {  7,  7,  7,  7,  7,  6,  5,  4,  3,  2,  1,  0,  1,  2,  3,  4 },
        {  7,  7,  7,  7,  7,  7,  6,  5,  4,  3,  2,  1,  0,  1,  2,  3 },
        {  7,  7,  7,  7,  7,  7,  7,  6,  5,  4,  3,  2,  1,  0,  1,  2 },
        {  7,  7,  7,  7,  7,  7,  7,  7,  6,  5,  4,  3,  2,  1,  0,  1 },
        {  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  5,  4,  3,  2,  1,  0 }
    };

    const int GpstimeMulti = 500;
    const int GpstimeMultiMinus = -10;
    const int GpstimeMultiCodeFull = 511;
} // unnamed namespace

Point14Base::Point14Base() : last_channel_(-1)
{
    //ABELL - For debug.
    chan_ctxs_[0].ctx_num_ = 0;
    chan_ctxs_[1].ctx_num_ = 1;
    chan_ctxs_[2].ctx_num_ = 2;
    chan_ctxs_[3].ctx_num_ = 3;
}

// COMPRESSOR

void Point14Compressor::writeSizes()
{
    xy_enc_.done();
    z_enc_.done();
    class_enc_.done();
    flags_enc_.done();
    intensity_enc_.done();
    scan_angle_enc_.done();
    user_data_enc_.done();
    point_source_id_enc_.done();
    gpstime_enc_.done();

    stream_ << xy_enc_.num_encoded();
    stream_ << z_enc_.num_encoded();
    stream_ << class_enc_.num_encoded();
    stream_ << flags_enc_.num_encoded();
    stream_ << intensity_enc_.num_encoded();
    stream_ << scan_angle_enc_.num_encoded();
    stream_ << user_data_enc_.num_encoded();
    stream_ << point_source_id_enc_.num_encoded();
    stream_ << gpstime_enc_.num_encoded();
}

void Point14Compressor::writeData()
{
#ifndef NDEBUG
    std::cerr << "XY        : " <<
        utils::sum(xy_enc_.encoded_bytes(), xy_enc_.num_encoded()) << "\n";
    std::cerr << "Z         : " <<
        utils::sum(z_enc_.encoded_bytes(), z_enc_.num_encoded()) << "\n";
    std::cerr << "Class     : " <<
        utils::sum(class_enc_.encoded_bytes(), class_enc_.num_encoded()) << "\n";
    std::cerr << "Flags     : " <<
        utils::sum(flags_enc_.encoded_bytes(), flags_enc_.num_encoded()) << "\n";
    std::cerr << "Intensity : " <<
        utils::sum(intensity_enc_.encoded_bytes(), intensity_enc_.num_encoded()) << "\n";
    std::cerr << "Scan angle: " <<
        utils::sum(scan_angle_enc_.encoded_bytes(), scan_angle_enc_.num_encoded()) << "\n";
    std::cerr << "User data : " <<
        utils::sum(user_data_enc_.encoded_bytes(), user_data_enc_.num_encoded()) << "\n";
    std::cerr << "Point src : " <<
        utils::sum(point_source_id_enc_.encoded_bytes(),
            point_source_id_enc_.num_encoded()) << "\n";
    std::cerr << "GPS time  : " <<
        utils::sum(gpstime_enc_.encoded_bytes(), gpstime_enc_.num_encoded()) << "\n";
#endif

    stream_.putBytes(xy_enc_.encoded_bytes(), xy_enc_.num_encoded());
    stream_.putBytes(z_enc_.encoded_bytes(), z_enc_.num_encoded());
    if (class_enc_.num_encoded())
        stream_.putBytes(class_enc_.encoded_bytes(), class_enc_.num_encoded());
    if (flags_enc_.num_encoded())
        stream_.putBytes(flags_enc_.encoded_bytes(), flags_enc_.num_encoded());
    if (intensity_enc_.num_encoded())
        stream_.putBytes(intensity_enc_.encoded_bytes(), intensity_enc_.num_encoded());
    if (scan_angle_enc_.num_encoded())
        stream_.putBytes(scan_angle_enc_.encoded_bytes(), scan_angle_enc_.num_encoded());
    if (user_data_enc_.num_encoded())
        stream_.putBytes(user_data_enc_.encoded_bytes(), user_data_enc_.num_encoded());
    if (point_source_id_enc_.num_encoded())
        stream_.putBytes(point_source_id_enc_.encoded_bytes(), point_source_id_enc_.num_encoded());
    if (gpstime_enc_.num_encoded())
        stream_.putBytes(gpstime_enc_.encoded_bytes(), gpstime_enc_.num_encoded());
}

const char *Point14Compressor::compress(const char *buf, int& scArg)
{
    const las::point14 point(buf);

    // We choose the model based on the scanner channel. In the laszip code, this
    // is called the context.
    int sc = point.scannerChannel();
    assert (sc >= 0 && sc <= 3);

    // don't have the first data for this channel yet, just push it to our have
    // last stuff and move on
    if (last_channel_ == -1)
    {
        ChannelCtx& c = chan_ctxs_[sc];
        stream_.putBytes((const unsigned char*)buf, sizeof(las::point14));
        c.last_ = point;
        c.have_last_ = true;
        c.last_gpstime_[0] = point.gpsTime();
        last_channel_ = sc;

        for (auto& z : c.last_z_)
            z = point.z();
        for (auto& last_intensity : c.last_intensity_)
            last_intensity = point.intensity();
        //ABELL - See note at end of function.
        scArg = sc;

        return buf + sizeof(las::point14);
    }

    // prev is the context for the previous point.
    ChannelCtx& prev = chan_ctxs_[last_channel_];

    // There are 8 contexts for the change bits based on the return number,
    // number of returns and a GPS time change. Calculate that context number.
    // Called 'lpr' in the laszip code.
    int change_stream =
        (prev.last_.returnNum() == 1) |                                 // bit 0
        ((prev.last_.returnNum() >= prev.last_.numReturns()) << 1) |    // bit 1
        (prev.gps_time_change_ << 2);                                   // bit 2

    // c is the context for this point.
    ChannelCtx& c = chan_ctxs_[sc];
    // old is the same as c unless we've switched channels and don't have a previous
    // for this channel, in which case we use the last channel's context.
    // In other words, we prefer the last point in the same channel as this point
    // if possible when computing the changes below.

    //ABELL - If we don't use the same context when actually storing the diffs (rather than
    //  the diff flags), it seems we could be mis-encoding data.
    ChannelCtx& old = c.have_last_ ? c : prev;

    //ABELL - The following comparison yields false if one of the values is NaN and
    //  true if both of the values are NaN. This probably wasn't what was intended but
    //  I don't think it really matters as long as the decoding is in sync. Care should
    //  be taken if a change is made.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
    bool gps_time_change = (point.gpsTime() != old.last_.gpsTime());
#pragma GCC diagnostic pop
    bool point_source_change = (point.pointSourceID() != old.last_.pointSourceID());
    bool scan_angle_change = (point.scanAngle() != old.last_.scanAngle());
    uint32_t last_n = old.last_.numReturns();
    uint32_t last_r = old.last_.returnNum();
    uint32_t n = point.numReturns();
    uint32_t r = point.returnNum();
    bool returnNumIncrements = (r == ((last_r + 1) % 16));
    bool returnNumDecrements = (r == ((last_r + 15) % 16));
    bool returnMiscChange = ((r != last_r) && !returnNumIncrements && !returnNumDecrements);

    int changed_values =
        ((returnNumIncrements || returnMiscChange) << 0) |
        ((returnNumDecrements || returnMiscChange) << 1) |
        ((n != last_n) << 2) |
        (scan_angle_change << 3) |
        (gps_time_change << 4) |
        (point_source_change << 5) |
        ((sc != last_channel_) << 6);

    xy_enc_.encodeSymbol(prev.changed_values_model_[change_stream], changed_values);

    if (sc > last_channel_)
        xy_enc_.encodeSymbol(prev.scanner_channel_model_, sc - last_channel_ - 1);
    else if (sc < last_channel_)
        xy_enc_.encodeSymbol(prev.scanner_channel_model_, sc - last_channel_ - 1 + 4);

    // If we haven't initialized the current context, do so.
    if (!c.have_last_)
    {
        c.have_last_ = true;
        c.last_ = prev.last_;
        for (auto& z : c.last_z_)
            z = prev.last_.z();
        for (auto& intensity : c.last_intensity_)
            intensity = prev.last_.intensity();
        c.last_gpstime_[0] = prev.last_.gpsTime();
    }

    if (n != last_n)
        xy_enc_.encodeSymbol(c.nr_model_[last_n], n);

    if (returnMiscChange)
    {
        if (gps_time_change)
            xy_enc_.encodeSymbol(c.rn_model_[last_r], r);
        else
        {
            //ABELL - I believe this is broken for the case when diff == -15, as
            //  it results in a symbol of -1, which is coerced into an unsigned value.
            int diff = r - last_r;
            if (diff > 1)
                xy_enc_.encodeSymbol(c.rn_gps_same_model_, diff - 2);
            else
                xy_enc_.encodeSymbol(c.rn_gps_same_model_, diff - 2 + 16);
        }
    }

    // X and Y
    {
        uint32_t ctx = (number_return_map_6ctx[n][r] << 1) | gps_time_change;
        int32_t median = c.last_x_diff_median5_[ctx].get();
        int32_t diff = point.x() - c.last_.x();
        c.dx_compr_.compress(xy_enc_, median, diff, point.numReturns() == 1);
        c.last_x_diff_median5_[ctx].add(diff);

        // Max of 20, low bit cleared to allow for numReturns change.
        uint32_t kbits = (std::min)(c.dx_compr_.getK(), 20U) & ~1;
        median = c.last_y_diff_median5_[ctx].get();
        diff = point.y() - c.last_.y();
        c.dy_compr_.compress(xy_enc_, median, diff, kbits | (n == 1));
        c.last_y_diff_median5_[ctx].add(diff);
    }

    // Z
    {
        uint32_t kbits = (c.dx_compr_.getK() + c.dy_compr_.getK()) / 2;
        kbits = (std::min)(kbits, 18U) & ~1;
        uint32_t ctx = number_return_level_8ctx[n][r];
        c.z_compr_.compress(z_enc_, c.last_z_[ctx], point.z(), kbits | (n == 1));
        c.last_z_[ctx] = point.z();
    }

    // Classification
    {
        int32_t ctx =
            // This bit is supposed to represent an only return.
            ((r == 1) && (r >= n)) |
            // Class 0 - 31, shifted.
            ((c.last_.classification() & 0x1F) << 1);

        if (point.classification() != c.last_.classification())
            class_enc_.makeValid();
        class_enc_.encodeSymbol(c.class_model_[ctx], point.classification());
    }

    // Flags
    {
        // This nonsense is to pack the flags, since we've already written the scanner
        // channel that's normally part of the flag byte.
        uint32_t flags =
            point.classFlags() |
            (point.scanDirFlag() << 4) |
            (point.eofFlag() << 5);
        uint32_t last_flags =
            c.last_.classFlags() |
            (c.last_.scanDirFlag() << 4) |
            (c.last_.eofFlag() << 5);

        if (flags != last_flags)
            flags_enc_.makeValid();;
        flags_enc_.encodeSymbol(c.flag_model_[last_flags], flags);
    }

    // Intensity
    {
        int32_t ctx =
            gps_time_change |
            ((r >= n) << 1) |
            ((r == 1) << 2);

        if (point.intensity() != c.last_.intensity())
            intensity_enc_.makeValid();
        c.intensity_compr_.compress(intensity_enc_,
                c.last_intensity_[ctx], point.intensity(), ctx >> 1);
        c.last_intensity_[ctx] = point.intensity();
    }

    // Scan angle
    {
        if (point.scanAngle() != c.last_.scanAngle())
        {
            scan_angle_enc_.makeValid();
            c.scan_angle_compr_.compress(scan_angle_enc_, c.last_.scanAngle(),
                    point.scanAngle(), gps_time_change);
        }
    }

    // User data
    {
        int32_t ctx = c.last_.userData() / 4;
        if (point.userData() != c.last_.userData())
            user_data_enc_.makeValid();
        user_data_enc_.encodeSymbol(c.user_data_model_[ctx], point.userData());
    }

    // Point Source ID
    {
        if (point_source_change)
        {
            point_source_id_enc_.makeValid();
            c.point_source_id_compr_.compress(point_source_id_enc_,
                    c.last_.pointSourceID(), point.pointSourceID(), 0);
        }
    }

    // GPS Time
    if (gps_time_change)
        encodeGpsTime(point, c);

//ABELL - There is a bug in laszip where the context does not get set unless there is a *change*
//  in context. This means that if the context is non-zero and never changes, it will
//  always be zero. This test maintains that behavior.
    if (sc != last_channel_)
        scArg = sc;
    last_channel_ = sc;
    c.gps_time_change_ = gps_time_change;
    c.last_ = point;
    return buf + sizeof(las::point14);
}

void Point14Compressor::encodeGpsTime(const las::point14& point, ChannelCtx& c)
{
    auto findSeq = [&c](double gpstime, int start, int32_t& diff)
    {
        for (int i = start; i < 4; ++i)
        {
            int testseq = (c.last_gps_seq_ + i) & 0x3;
            int64_t diff64 = utils::d2i(gpstime) - utils::d2i(c.last_gpstime_[testseq]);
            diff = (int32_t)diff64;
            if (diff64 == diff)
                return i;
        }
        return -1;
    };

    gpstime_enc_.makeValid();

    loop: 
    if (c.last_gpstime_diff_[c.last_gps_seq_] == 0)
    {
        int32_t diff;  // Difference between current time and last time in the sequence,
        // as 32 bit int.
        int idx = findSeq(point.gpsTime(), 0, diff);

        if (idx == 0)
        {
            gpstime_enc_.encodeSymbol(c.gpstime_0diff_model_, 0);
            c.gpstime_compr_.compress(gpstime_enc_, 0, diff, 0);
            c.last_gpstime_diff_[c.last_gps_seq_] = diff;
            c.multi_extreme_counter_[c.last_gps_seq_] = 0;
        }
        else if (idx > 0)
        {
            gpstime_enc_.encodeSymbol(c.gpstime_0diff_model_, idx + 1);
            c.last_gps_seq_ = (c.last_gps_seq_ + idx) & 3;
            goto loop;
        }
        else
        {
            gpstime_enc_.encodeSymbol(c.gpstime_0diff_model_, 1);
            c.gpstime_compr_.compress(gpstime_enc_,
                    utils::d2u(c.last_gpstime_[c.last_gps_seq_]) >> 32,
                    (int32_t)(utils::d2u(point.gpsTime()) >> 32), 8);
            gpstime_enc_.writeInt((uint32_t)(utils::d2u(point.gpsTime())));
            c.last_gps_seq_ = c.next_gps_seq_ = (c.next_gps_seq_ + 1) & 3;
            c.last_gpstime_diff_[c.last_gps_seq_] = 0;
            c.multi_extreme_counter_[c.last_gps_seq_] = 0;
        }
        c.last_gpstime_[c.last_gps_seq_] = point.gpsTime();
    }
    else  // Last diff nonzero.
    {
        int64_t diff64 = utils::d2i(point.gpsTime()) - utils::d2i(c.last_gpstime_[c.last_gps_seq_]);
        int32_t diff = (int32_t)diff64;

        // 32 bit difference.
        if (diff64 == diff)
        {
            // Compute multiplier between current and last int difference.
            int32_t multi =
                (int32_t)std::round(diff / (float)c.last_gpstime_diff_[c.last_gps_seq_]);
            if (multi > 0 && multi < GpstimeMulti)
            {
                int tag;
                if (multi == 1)
                    tag = 1;  // The case for regular spaced pulses.
                else if (multi > 1 && multi < 10)
                    tag = 2;
                else
                    tag = 3;
                gpstime_enc_.encodeSymbol(c.gpstime_multi_model_, multi);
                c.gpstime_compr_.compress(gpstime_enc_,
                        multi * c.last_gpstime_diff_[c.last_gps_seq_], diff, tag);
                if (tag == 1)
                    c.multi_extreme_counter_[c.last_gps_seq_] = 0;
            }
            else if (multi >= GpstimeMulti)
            {
                gpstime_enc_.encodeSymbol(c.gpstime_multi_model_, GpstimeMulti);
                c.gpstime_compr_.compress(gpstime_enc_,
                        GpstimeMulti * c.last_gpstime_diff_[c.last_gps_seq_], diff, 4);
                c.multi_extreme_counter_[c.last_gps_seq_]++;
                if (c.multi_extreme_counter_[c.last_gps_seq_] > 3)
                {
                    c.multi_extreme_counter_[c.last_gps_seq_] = 0;
                    c.last_gpstime_diff_[c.last_gps_seq_] = diff;
                }
            }
            else if (multi < 0 && multi > GpstimeMultiMinus)
            {
                gpstime_enc_.encodeSymbol(c.gpstime_multi_model_, GpstimeMulti - multi);
                c.gpstime_compr_.compress(gpstime_enc_,
                        multi * c.last_gpstime_diff_[c.last_gps_seq_], diff, 5);
            }
            else if (multi <= GpstimeMultiMinus)
            {
                gpstime_enc_.encodeSymbol(c.gpstime_multi_model_,
                        GpstimeMulti - GpstimeMultiMinus);
                c.gpstime_compr_.compress(gpstime_enc_,
                        GpstimeMultiMinus * c.last_gpstime_diff_[c.last_gps_seq_], diff, 6);
                c.multi_extreme_counter_[c.last_gps_seq_]++;
                if (c.multi_extreme_counter_[c.last_gps_seq_] > 3)
                {
                    c.multi_extreme_counter_[c.last_gps_seq_] = 0;
                    c.last_gpstime_diff_[c.last_gps_seq_] = diff;
                }
            }
            else if (multi == 0)
            {
                gpstime_enc_.encodeSymbol(c.gpstime_multi_model_, 0);
                c.gpstime_compr_.compress(gpstime_enc_, 0, diff, 7);
                c.multi_extreme_counter_[c.last_gps_seq_]++;
                if (c.multi_extreme_counter_[c.last_gps_seq_] > 3)
                {
                    c.multi_extreme_counter_[c.last_gps_seq_] = 0;
                    c.last_gpstime_diff_[c.last_gps_seq_] = diff;
                }
            }
        }
        // Large difference
        else
        {
            int32_t diff;
            int idx = findSeq(point.gpsTime(), 1, diff);
            if (idx > 0)
            {
                gpstime_enc_.encodeSymbol(c.gpstime_multi_model_,
                        GpstimeMultiCodeFull + idx);
                c.last_gps_seq_ = (c.last_gps_seq_ + idx) & 3;
                goto loop;
            }
            gpstime_enc_.encodeSymbol(c.gpstime_multi_model_, GpstimeMultiCodeFull);
            c.gpstime_compr_.compress(gpstime_enc_,
                    int32_t(utils::d2u(c.last_gpstime_[c.last_gps_seq_]) >> 32),
                    int32_t(utils::d2u(point.gpsTime()) >> 32), 8);
            gpstime_enc_.writeInt((uint32_t)(utils::d2u(point.gpsTime())));
            c.next_gps_seq_ = c.last_gps_seq_ = (c.next_gps_seq_ + 1) & 3;
            c.last_gpstime_diff_[c.last_gps_seq_] = 0;
            c.multi_extreme_counter_[c.last_gps_seq_] = 0;
        }
        c.last_gpstime_[c.last_gps_seq_] = point.gpsTime();
    }
}

// DECOMPRESSOR

void Point14Decompressor::dumpSums()
{
    if (sumChange.count() == 0)
        return;

    std::cout << "Change   : " << sumChange.value() << "\n";
    std::cout << "Return   : " << sumReturn.value() << "\n";
    std::cout << "X        : " << sumX.value() << "\n";
    std::cout << "Y        : " << sumY.value() << "\n";
    std::cout << "Z        : " << sumZ.value() << "\n";
    std::cout << "Class    : " << sumClass.value() << "\n";
    std::cout << "Flags    : " << sumFlags.value() << "\n";
    std::cout << "Intensity: " << sumIntensity.value() << "\n";
    std::cout << "Scan angl: " << sumScanAngle.value() << "\n";
    std::cout << "User data: " << sumUserData.value() << "\n";
    std::cout << "Point src: " << sumPointSourceId.value() << "\n";
    std::cout << "GPS time : " << sumGpsTime.value() << "\n";
}

void Point14Decompressor::readSizes()
{
    uint32_t xy_cnt;
    uint32_t z_cnt;
    uint32_t class_cnt;
    uint32_t flags_cnt;
    uint32_t intensity_cnt;
    uint32_t scan_angle_cnt;
    uint32_t user_data_cnt;
    uint32_t point_source_id_cnt;
    uint32_t gpstime_cnt;

    stream_ >> xy_cnt;
    stream_ >> z_cnt;
    stream_ >> class_cnt;
    stream_ >> flags_cnt;
    stream_ >> intensity_cnt;
    stream_ >> scan_angle_cnt;
    stream_ >> user_data_cnt;
    stream_ >> point_source_id_cnt;
    stream_ >> gpstime_cnt;

    sizes_.push_back(xy_cnt);
    sizes_.push_back(z_cnt);
    sizes_.push_back(class_cnt);
    sizes_.push_back(flags_cnt);
    sizes_.push_back(intensity_cnt);
    sizes_.push_back(scan_angle_cnt);
    sizes_.push_back(user_data_cnt);
    sizes_.push_back(point_source_id_cnt);
    sizes_.push_back(gpstime_cnt);
}

void Point14Decompressor::readData()
{
    auto cnt = sizes_.begin();

    // Copy data and read the init bytes.
    xy_dec_.initStream(stream_, *cnt++);
    z_dec_.initStream(stream_, *cnt++);
    class_dec_.initStream(stream_, *cnt++);
    flags_dec_.initStream(stream_, *cnt++);
    intensity_dec_.initStream(stream_, *cnt++);
    scan_angle_dec_.initStream(stream_, *cnt++);
    user_data_dec_.initStream(stream_, *cnt++);
    point_source_id_dec_.initStream(stream_, *cnt++);
    gpstime_dec_.initStream(stream_, *cnt++);
    sizes_.clear();
}

char *Point14Decompressor::decompress(char *buf, int& scArg)
{
    // This is weird, but the first point, stored raw, is written *before* the point
    // count.
    // First point.
    if (last_channel_ == -1)
    {
        stream_.getBytes((unsigned char *)buf, sizeof(las::point14));
        las::point14 point(buf);

        scArg = point.scannerChannel();
        ChannelCtx& c = chan_ctxs_[scArg];
        c.last_ = point;
        c.have_last_ = true;
        c.last_gpstime_[0] = point.gpsTime();
        last_channel_ = scArg;

        for (auto& z : c.last_z_)
            z = point.z();
        for (auto& last_intensity : c.last_intensity_)
            last_intensity = point.intensity();

        return buf + sizeof(las::point14);
    }
    ChannelCtx& prev = chan_ctxs_[last_channel_];

    // There are 8 streams for the change bits based on the return number,
    // number of returns and a GPS time change. Calculate that stream number.
    // Called 'lpr' in the laszip code.
    int change_stream =
        (prev.last_.returnNum() == 1) |                                 // bit 0
        ((prev.last_.returnNum() >= prev.last_.numReturns()) << 1) |    // bit 1
        (prev.gps_time_change_ << 2);                                   // bit 2

    int32_t changed_values = xy_dec_.decodeSymbol(prev.changed_values_model_[change_stream]);
    LAZDEBUG(sumChange.add(changed_values));

    bool scanner_channel_changed = (changed_values >> 6) & 1;
    bool point_source_changed = (changed_values >> 5) & 1;
    bool gps_time_changed = (changed_values >> 4) & 1;
    bool scan_angle_changed = (changed_values >> 3) & 1;
    bool nr_changes = (changed_values >> 2) & 1;
    bool rn_minus = (changed_values >> 1) & 1;
    bool rn_plus = (changed_values >> 0) & 1;
    bool rn_increments = rn_plus && !rn_minus;
    bool rn_decrements = rn_minus && !rn_plus;
    bool rn_misc_change = rn_plus && rn_minus;

    int sc = prev.last_.scannerChannel();
    if (scanner_channel_changed)
    {
        uint32_t diff = xy_dec_.decodeSymbol(prev.scanner_channel_model_);
        sc = (sc + diff + 1) % 4;
        last_channel_ = sc;
        scArg = sc;
    }

    ChannelCtx& c = chan_ctxs_[sc];
    if (!c.have_last_)
    {
        c.have_last_ = true;
        c.last_ = prev.last_;
        for (auto& z : c.last_z_)
            z = prev.last_.z();
        for (auto& intensity : c.last_intensity_)
            intensity = prev.last_.intensity();
        c.last_gpstime_[0] = prev.last_.gpsTime();
    }
    c.last_.setScannerChannel(sc);

    uint32_t n = c.last_.numReturns();
    uint32_t r = c.last_.returnNum();
    if (nr_changes)
        n = xy_dec_.decodeSymbol(c.nr_model_[c.last_.numReturns()]);
    c.last_.setNumReturns(n);

    if (rn_increments)
        r = (r + 1) % 16;
    else if (rn_decrements)
        r = ((r + 15) % 16);
    else if (rn_misc_change)
    {
        if (gps_time_changed)
            r = xy_dec_.decodeSymbol(c.rn_model_[r]);
        else
            r = (r + xy_dec_.decodeSymbol(c.rn_gps_same_model_) + 2) % 16;
    }
    c.last_.setReturnNum(r);
    LAZDEBUG(sumReturn.add(n));
    LAZDEBUG(sumReturn.add(r));

    uint32_t ctx = (number_return_map_6ctx[n][r] << 1) | gps_time_changed;
    // X
    {
        int32_t median = c.last_x_diff_median5_[ctx].get();
        int32_t diff = c.dx_decomp_.decompress(xy_dec_, median, n == 1);
        c.last_.setX(c.last_.x() + diff);
        c.last_x_diff_median5_[ctx].add(diff);
        LAZDEBUG(sumX.add(c.last_.x()));
    }

    // Y
    {
        uint32_t kbits = (std::min)(c.dx_decomp_.getK(), 20U) & ~1;
        int32_t median = c.last_y_diff_median5_[ctx].get();
        int32_t diff = c.dy_decomp_.decompress(xy_dec_, median, (n == 1) | kbits);
        c.last_.setY(c.last_.y() + diff);
        c.last_y_diff_median5_[ctx].add(diff);
        LAZDEBUG(sumY.add(c.last_.y()));
    }

    // Z
    {
        uint32_t kbits = (c.dx_decomp_.getK() + c.dy_decomp_.getK()) / 2;
        kbits = (std::min)(kbits, 18U) & ~1;
        uint32_t ctx = number_return_level_8ctx[n][r];
        int32_t z = c.z_decomp_.decompress(z_dec_, c.last_z_[ctx], (n == 1) | kbits);
        c.last_.setZ(z);
        c.last_z_[ctx] = z;
        LAZDEBUG(sumZ.add(c.last_.z()));
    }

    // Classification
    {
        int32_t ctx = ((r == 1 && r >= n) | ((c.last_.classification() & 0x1F) << 1));
        c.last_.setClassification(class_dec_.decodeSymbol(c.class_model_[ctx]));
        LAZDEBUG(sumClass.add(c.last_.classification()));
    }

    // Flags
    {
        uint32_t last_flags = c.last_.classFlags() |
            (c.last_.scanDirFlag() << 4) |
            (c.last_.eofFlag() << 5);

        uint32_t flags = flags_dec_.decodeSymbol(c.flag_model_[last_flags]);
        LAZDEBUG(sumFlags.add(flags));
        c.last_.setEofFlag((flags >> 5) & 1);
        c.last_.setScanDirFlag((flags >> 4) & 1);
        c.last_.setClassFlags(flags & 0x0F);
    }

    // Intensity
    {
        int32_t ctx = gps_time_changed | ((r >= n) << 1) | ((r == 1) << 2);

        uint16_t intensity = c.intensity_decomp_.decompress(intensity_dec_,
                c.last_intensity_[ctx], ctx >> 1);
        c.last_intensity_[ctx] = intensity;
        c.last_.setIntensity(intensity);
        LAZDEBUG(sumIntensity.add(c.last_.intensity()));
    }

    // Scan angle
    {
        if (scan_angle_changed)
        {
            c.last_.setScanAngle(c.scan_angle_decomp_.decompress(scan_angle_dec_,
                        c.last_.scanAngle(), gps_time_changed));
        }
        LAZDEBUG(sumScanAngle.add(c.last_.scanAngle()));
    }

    // User data
    {
        int32_t ctx = c.last_.userData() / 4;
        c.last_.setUserData(user_data_dec_.decodeSymbol(c.user_data_model_[ctx]));
        LAZDEBUG(sumUserData.add(c.last_.userData()));
    }

    // Point source ID
    {
        if (point_source_changed)
            c.last_.setPointSourceID(c.point_source_id_decomp_.decompress(
                        point_source_id_dec_, c.last_.pointSourceID(), 0));
        LAZDEBUG(sumPointSourceId.add(c.last_.pointSourceID()));
    }

    if (gps_time_changed)
        decodeGpsTime(c);
    LAZDEBUG(sumGpsTime.add(c.last_.gpsTime()));
    c.gps_time_change_ = gps_time_changed;
    las::point14 *point = reinterpret_cast<las::point14 *>(buf);
    *point = c.last_;
    return buf + sizeof(las::point14);
}

void Point14Decompressor::decodeGpsTime(ChannelCtx& c)
{
    loop:
    if (c.last_gpstime_diff_[c.last_gps_seq_] == 0)
    {
        int32_t multi = gpstime_dec_.decodeSymbol(c.gpstime_0diff_model_);
        if (multi == 0)
        {
            int32_t sym = c.gpstime_decomp_.decompress(gpstime_dec_, 0, 0);

            c.last_gpstime_diff_[c.last_gps_seq_] = sym;
            int64_t lasttime = utils::d2i(c.last_gpstime_[c.last_gps_seq_]) + sym;
            c.last_gpstime_[c.last_gps_seq_] = utils::i2d(lasttime);
            c.multi_extreme_counter_[c.last_gps_seq_] = 0;
        }
        else if (multi == 1)
        {
            c.next_gps_seq_ = (c.next_gps_seq_ + 1) & 3;
            double lasttime = c.last_gpstime_[c.last_gps_seq_];
            int32_t sym = c.gpstime_decomp_.decompress(gpstime_dec_,
                    (int32_t)(utils::d2u(lasttime) >> 32), 8);
            c.last_gpstime_[c.next_gps_seq_] =
                utils::u2d(((uint64_t)sym << 32) | gpstime_dec_.readInt());
            c.last_gps_seq_ = c.next_gps_seq_;
            c.last_gpstime_diff_[c.last_gps_seq_] = 0;
            c.multi_extreme_counter_[c.last_gps_seq_] = 0;
        }
        else
        {
            c.last_gps_seq_ = (c.last_gps_seq_ + multi - 1) & 3;
            goto loop;
        }
    }
    else
    {
        int32_t multi = gpstime_dec_.decodeSymbol(c.gpstime_multi_model_);
        int32_t gpstime_diff;
        if (multi == 1)
        {
            int32_t sym = c.gpstime_decomp_.decompress(gpstime_dec_,
                    c.last_gpstime_diff_[c.last_gps_seq_], 1);
            c.last_gpstime_[c.last_gps_seq_] =
                utils::i2d((int64_t)sym + utils::d2u(c.last_gpstime_[c.last_gps_seq_]));
            c.multi_extreme_counter_[c.last_gps_seq_] = 0;
        }
        else if (multi < GpstimeMultiCodeFull)
        {
            if (multi == 0)
            {
                gpstime_diff = c.gpstime_decomp_.decompress(gpstime_dec_, 0, 7);
                c.multi_extreme_counter_[c.last_gps_seq_]++;
                if (c.multi_extreme_counter_[c.last_gps_seq_] > 3)
                {
                    c.multi_extreme_counter_[c.last_gps_seq_] = 0;
                    c.last_gpstime_diff_[c.last_gps_seq_] = gpstime_diff;
                }
            }
            else if (multi < GpstimeMulti)
            {
                int tag;
                if (multi < 10)
                    tag = 2;
                else
                    tag = 3;
                gpstime_diff = c.gpstime_decomp_.decompress(gpstime_dec_,
                        multi * c.last_gpstime_diff_[c.last_gps_seq_], tag);
            }
            else if (multi == GpstimeMulti)
            {
                gpstime_diff = c.gpstime_decomp_.decompress(gpstime_dec_,
                        GpstimeMulti * c.last_gpstime_diff_[c.last_gps_seq_], 4);
                c.multi_extreme_counter_[c.last_gps_seq_]++;
                if (c.multi_extreme_counter_[c.last_gps_seq_] > 3)
                {
                    c.multi_extreme_counter_[c.last_gps_seq_] = 0;
                    c.last_gpstime_diff_[c.last_gps_seq_] = gpstime_diff;
                }
            }
            else
            {
                multi = GpstimeMulti - multi;
                if (multi > GpstimeMultiMinus)
                {
                    gpstime_diff = c.gpstime_decomp_.decompress(gpstime_dec_,
                            multi * c.last_gpstime_diff_[c.last_gps_seq_], 5);
                }
                else
                {
                    gpstime_diff = c.gpstime_decomp_.decompress(gpstime_dec_,
                            GpstimeMultiMinus * c.last_gpstime_diff_[c.last_gps_seq_], 6);
                    c.multi_extreme_counter_[c.last_gps_seq_]++;
                    if (c.multi_extreme_counter_[c.last_gps_seq_] > 3)
                    {
                        c.multi_extreme_counter_[c.last_gps_seq_] = 0;
                        c.last_gpstime_diff_[c.last_gps_seq_] = gpstime_diff;
                    }
                }
            }
            int64_t lasttime = utils::d2i(c.last_gpstime_[c.last_gps_seq_]);
            c.last_gpstime_[c.last_gps_seq_] = utils::i2d(lasttime + gpstime_diff);
        }
        else if (multi == GpstimeMultiCodeFull)
        {
            c.next_gps_seq_ = (c.next_gps_seq_ + 1) & 3;

            uint64_t lasttime = utils::d2u(c.last_gpstime_[c.last_gps_seq_]);
            int32_t sym = c.gpstime_decomp_.decompress(gpstime_dec_,
                    (int32_t)(lasttime >> 32), 8);
            c.last_gpstime_[c.next_gps_seq_] =
                utils::u2d(((uint64_t)(sym) << 32) | gpstime_dec_.readInt());
            c.last_gps_seq_ = c.next_gps_seq_;
            c.last_gpstime_diff_[c.last_gps_seq_] = 0;
            c.multi_extreme_counter_[c.last_gps_seq_] = 0;
        }
        else if (multi >= GpstimeMultiCodeFull)
        {
            c.last_gps_seq_ = (c.last_gps_seq_ + multi - GpstimeMultiCodeFull) & 3;
            goto loop;
        }
    }
    c.last_.setGpsTime(c.last_gpstime_[c.last_gps_seq_]);
}

} // namespace detail
} // namespace lazperf
