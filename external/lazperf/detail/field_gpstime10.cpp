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

#include <cmath>

#include "../las.hpp"

#define LASZIP_GPSTIME_MULTI 500
#define LASZIP_GPSTIME_MULTI_MINUS -10
#define LASZIP_GPSTIME_MULTI_UNCHANGED (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 1)
#define LASZIP_GPSTIME_MULTI_CODE_FULL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 2)
#define LASZIP_GPSTIME_MULTI_TOTAL (LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS + 6)

namespace lazperf
{
namespace detail
{

Gpstime10Base::Gpstime10Base() : have_last_(false), m_gpstime_multi(LASZIP_GPSTIME_MULTI_TOTAL),
    m_gpstime_0diff(6), last(0), next(0)
{
    last_gpstime.fill(las::gpstime());
    last_gpstime_diff.fill(0);
    multi_extreme_counter.fill(0);
}

Gpstime10Compressor::Gpstime10Compressor(encoders::arithmetic<OutCbStream>& encoder) :
    enc_(encoder), compressor_inited_(false), ic_gpstime(32, 9)
{}

void Gpstime10Compressor::init()
{
    ic_gpstime.init();
}

const char *Gpstime10Compressor::compress(const char *buf)
{
    las::gpstime this_val(buf);

    if (!compressor_inited_) {
        init();
        compressor_inited_ = true;
    }

    if (!have_last_) {
        // don't have the first data yet, just push it to our have last stuff and move on
        have_last_ = true;
        last_gpstime[0] = this_val;

        // write this out to the encoder as it is
        enc_.getOutStream().putBytes((const unsigned char*)buf, sizeof(las::gpstime));
        buf += sizeof(las::gpstime);
        return buf;
    }

    // if last integer different was 0
    if (last_gpstime_diff[last] == 0)
    {
        if (this_val.value == last_gpstime[last].value)
        {
            enc_.encodeSymbol(m_gpstime_0diff, 0);
        }
        else {
            // calculate the difference between the two doubles as an integer
            int64_t curr_gpstime_diff_64 = this_val.value - last_gpstime[last].value;
            int curr_gpstime_diff = static_cast<int>(curr_gpstime_diff_64);

            if (curr_gpstime_diff_64 == static_cast<int64_t>(curr_gpstime_diff))
            {
                // this difference is small enough to be represented with 32 bits
                enc_.encodeSymbol(m_gpstime_0diff, 1);
                ic_gpstime.compress(enc_, 0, curr_gpstime_diff, 0);
                last_gpstime_diff[last] = curr_gpstime_diff;
                multi_extreme_counter[last] = 0;
            }
            else { // the difference is huge
                uint32_t i;

                // maybe the double belongs to another time sequence
                //
                for (i = 1; i < 4; i++) {
                    int64_t other_gpstime_diff_64 = this_val.value -
                        last_gpstime[(last + i) & 3].value;
                    int other_gpstime_diff = static_cast<int>(other_gpstime_diff_64);

                    if (other_gpstime_diff_64 == static_cast<int64_t>(other_gpstime_diff))
                    {
                        // it belongs to another sequence
                        enc_.encodeSymbol(m_gpstime_0diff, i + 2);
                        last = (last + i) & 3;
                        return compress(buf);
                    }
                }

                // no other sequence found. start new sequence.
                enc_.encodeSymbol(m_gpstime_0diff, 2);
                ic_gpstime.compress(enc_, static_cast<int>(last_gpstime[last].value >> 32),
                    static_cast<int>(this_val.value >> 32), 8);
                enc_.writeInt(static_cast<unsigned int>(this_val.value));

                next = (next + 1) & 3;
                last = next;
                last_gpstime_diff[last] = 0;
                multi_extreme_counter[last] = 0;
            }
            last_gpstime[last] = this_val;
        }
    }
    else { // the last integer difference was *not* zero
        if (this_val.value == last_gpstime[last].value)
        {
            // if the doubles have not changed use a special symbol
            enc_.encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI_UNCHANGED);
        }
        else
        {
            // calculate the difference between the two doubles as an integer
            int64_t curr_gpstime_diff_64 = this_val.value - last_gpstime[last].value;
            int curr_gpstime_diff = static_cast<int>(curr_gpstime_diff_64);

            // if the current gpstime difference can be represented with 32 bits
            if (curr_gpstime_diff_64 == static_cast<int64_t>(curr_gpstime_diff)) {
                // compute multiplier between current and last integer difference
                float multi_f = (float)curr_gpstime_diff / (float)(last_gpstime_diff[last]);
                int multi = (int)std::round(multi_f);

                // compress the residual curr_gpstime_diff in dependance on the multiplier
                if (multi == 1) {
                    // this is the case we assume we get most often for regular spaced pulses
                    enc_.encodeSymbol(m_gpstime_multi, 1);
                    ic_gpstime.compress(enc_, last_gpstime_diff[last], curr_gpstime_diff, 1);
                    multi_extreme_counter[last] = 0;
                }
                else if (multi > 0) {
                    if (multi < LASZIP_GPSTIME_MULTI) {
                        // positive multipliers up to LASZIP_GPSTIME_MULTI are compressed directly
                        enc_.encodeSymbol(m_gpstime_multi, multi);
                        if (multi < 10)
                            ic_gpstime.compress(enc_, multi * last_gpstime_diff[last],
                                curr_gpstime_diff, 2);
                        else
                            ic_gpstime.compress(enc_, multi * last_gpstime_diff[last],
                                curr_gpstime_diff, 3);
                    }
                    else {
                        enc_.encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI);
                        ic_gpstime.compress(enc_, LASZIP_GPSTIME_MULTI * last_gpstime_diff[last],
                            curr_gpstime_diff, 4);
                        multi_extreme_counter[last]++;

                        if (multi_extreme_counter[last] > 3)
                        {
                            last_gpstime_diff[last] = curr_gpstime_diff;
                            multi_extreme_counter[last] = 0;
                        }
                    }
                }
                else if (multi < 0) {
                    if (multi > LASZIP_GPSTIME_MULTI_MINUS)
                    {
                        // negative multipliers larger than LASZIP_GPSTIME_MULTI_MINUS are
                        // compressed directly
                        enc_.encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI - multi);
                        ic_gpstime.compress(enc_, multi * last_gpstime_diff[last],
                            curr_gpstime_diff, 5);
                    }
                    else {
                        enc_.encodeSymbol(m_gpstime_multi,
                            LASZIP_GPSTIME_MULTI - LASZIP_GPSTIME_MULTI_MINUS);
                        ic_gpstime.compress(enc_,
                            LASZIP_GPSTIME_MULTI_MINUS * last_gpstime_diff[last],
                            curr_gpstime_diff, 6);

                        multi_extreme_counter[last]++;
                        if (multi_extreme_counter[last] > 3)
                        {
                            last_gpstime_diff[last] = curr_gpstime_diff;
                            multi_extreme_counter[last] = 0;
                        }
                    }
                }
                else {
                    enc_.encodeSymbol(m_gpstime_multi, 0);
                    ic_gpstime.compress(enc_, 0, curr_gpstime_diff, 7);
                    multi_extreme_counter[last]++;
                    if (multi_extreme_counter[last] > 3)
                    {
                        last_gpstime_diff[last] = curr_gpstime_diff;
                        multi_extreme_counter[last] = 0;
                    }
                }
            }
            else
            {
                // the difference is huge
                int i;
                // maybe the double belongs to another time sequence
                for (i = 1; i < 4; i++)
                {
                    int64_t other_gpstime_diff_64 = this_val.value -
                        last_gpstime[(last + i)  &3].value;
                    int other_gpstime_diff = static_cast<int>(other_gpstime_diff_64);

                    if (other_gpstime_diff_64 == static_cast<int64_t>(other_gpstime_diff))
                    {
                        // it belongs to this sequence
                        enc_.encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI_CODE_FULL+i);
                        last = (last + i) & 3;
                        return compress(buf);
                    }
                }

                // no other sequence found. start new sequence.
                enc_.encodeSymbol(m_gpstime_multi, LASZIP_GPSTIME_MULTI_CODE_FULL);
                ic_gpstime.compress(enc_, static_cast<int>(last_gpstime[last].value >> 32),
                    static_cast<int>(this_val.value >> 32), 8);
                enc_.writeInt(static_cast<unsigned int>(this_val.value));
                next = (next + 1) & 3;
                last = next;
                last_gpstime_diff[last] = 0;
                multi_extreme_counter[last] = 0;
            }

            last_gpstime[last] = this_val;
        }
    }
    return buf + sizeof(las::gpstime);
}

// DECOMPRESSOR

Gpstime10Decompressor::Gpstime10Decompressor(decoders::arithmetic<InCbStream>& decoder) :
    dec_(decoder), decompressor_inited_(false), ic_gpstime(32, 9)
{}

void Gpstime10Decompressor::init()
{
    ic_gpstime.init();
}

char *Gpstime10Decompressor::decompress(char *buf)
{
    if (!decompressor_inited_)
    {
        init();
        decompressor_inited_ = true;
    }

    if (!have_last_)
    {
        // don't have the first data yet, read the whole point out of the stream
        have_last_ = true;

        dec_.getInStream().getBytes((unsigned char*)buf, sizeof(las::gpstime));
        // decode this value
        last_gpstime[0].unpack(buf);

        // we are done here
        return buf + sizeof(las::gpstime);
    }

    int multi;
    if (last_gpstime_diff[last] == 0)
    {
        // if the last integer difference was zero
        multi = dec_.decodeSymbol(m_gpstime_0diff);

        if (multi == 1)
        {
            // the difference can be represented with 32 bits
            last_gpstime_diff[last] = ic_gpstime.decompress(dec_, 0, 0);
            last_gpstime[last].value += last_gpstime_diff[last];
            multi_extreme_counter[last] = 0;
        }
        else if (multi == 2)
        {
            // the difference is huge
            next = (next + 1) & 3;
            last_gpstime[next].value = ic_gpstime.decompress(dec_,
                (last_gpstime[last].value >> 32), 8);
            last_gpstime[next].value = last_gpstime[next].value << 32;
            last_gpstime[next].value |= dec_.readInt();
            last = next;
            last_gpstime_diff[last] = 0;
            multi_extreme_counter[last] = 0;
        }
        else if (multi > 2)
        {
            // we switch to another sequence
            last = (last + multi -2) & 3;
            decompress(buf);
        }
    }
    else
    {
        multi = dec_.decodeSymbol(m_gpstime_multi);
        if (multi == 1)
        {
            last_gpstime[last].value += ic_gpstime.decompress(dec_, last_gpstime_diff[last], 1);
            multi_extreme_counter[last] = 0;
        }
        else if (multi < LASZIP_GPSTIME_MULTI_UNCHANGED)
        {
            int gpstime_diff;
            if (multi == 0)
            {
                gpstime_diff = ic_gpstime.decompress(dec_, 0, 7);
                multi_extreme_counter[last]++;
                if (multi_extreme_counter[last] > 3)
                {
                    last_gpstime_diff[last] = gpstime_diff;
                    multi_extreme_counter[last] = 0;
                }
            }
            else if (multi < LASZIP_GPSTIME_MULTI)
            {
                if (multi < 10)
                    gpstime_diff = ic_gpstime.decompress(dec_,
                        multi * last_gpstime_diff[last], 2);
                else
                    gpstime_diff = ic_gpstime.decompress(dec_, multi * last_gpstime_diff[last], 3);
            }
            else if (multi == LASZIP_GPSTIME_MULTI)
            {
                gpstime_diff = ic_gpstime.decompress(dec_,
                    LASZIP_GPSTIME_MULTI * last_gpstime_diff[last], 4);
                multi_extreme_counter[last]++;
                if (multi_extreme_counter[last] > 3)
                {
                    last_gpstime_diff[last] = gpstime_diff;
                    multi_extreme_counter[last] = 0;
                }
            }
            else {
                multi = LASZIP_GPSTIME_MULTI - multi;
                if (multi > LASZIP_GPSTIME_MULTI_MINUS)
                {
                    gpstime_diff = ic_gpstime.decompress(dec_, multi * last_gpstime_diff[last], 5);
                }
                else
                {
                    gpstime_diff = ic_gpstime.decompress(dec_,
                        LASZIP_GPSTIME_MULTI_MINUS * last_gpstime_diff[last], 6);
                    multi_extreme_counter[last]++;
                    if (multi_extreme_counter[last] > 3)
                    {
                        last_gpstime_diff[last] = gpstime_diff;
                        multi_extreme_counter[last] = 0;
                    }
                }
            }
            last_gpstime[last].value += gpstime_diff;
        }
        else if (multi ==  LASZIP_GPSTIME_MULTI_CODE_FULL)
        {
            next = (next + 1) & 3;
            last_gpstime[next].value = ic_gpstime.decompress(dec_,
                static_cast<int>(last_gpstime[last].value >> 32), 8);
            last_gpstime[next].value = last_gpstime[next].value << 32;
            last_gpstime[next].value |= dec_.readInt();
            last = next;
            last_gpstime_diff[last] = 0;
            multi_extreme_counter[last] = 0;
        }
        else if (multi >=  LASZIP_GPSTIME_MULTI_CODE_FULL)
        {
            last = (last + multi - LASZIP_GPSTIME_MULTI_CODE_FULL) & 3;
            decompress(buf);
        }
    }

    last_gpstime[last].pack(buf);
    return buf + sizeof(las::gpstime);
}

} // namespace detail
} // namespace lazperf
