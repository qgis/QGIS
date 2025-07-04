/*
===============================================================================

  FILE:  field_byte14.hpp
  
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

class Byte14Base
{
protected:
    struct ChannelCtx
    {
        int have_last_;
        las::byte14 last_;
        std::vector<models::arithmetic> byte_model_;

        ChannelCtx(size_t count) : have_last_(false), last_(count),
            byte_model_(count, models::arithmetic(256))
        {}
    };

public:
    size_t count() const;

protected:
    Byte14Base(size_t count);

    size_t count_;
    int last_channel_;
    std::array<ChannelCtx, 4> chan_ctxs_;
    std::vector<decoders::arithmetic<MemoryStream>> byte_dec_;
};

class Byte14Compressor : public Byte14Base
{
public:
    Byte14Compressor(OutCbStream& stream, size_t count);

    void writeSizes();
    void writeData();
    const char *compress(const char *buf, int& sc);

private:
    OutCbStream& stream_;
    std::vector<bool> valid_;
    std::vector<encoders::arithmetic<MemoryStream>> byte_enc_;
};

class Byte14Decompressor : public Byte14Base
{
public:
    Byte14Decompressor(InCbStream& stream, size_t count);

    void dumpSums();
    void readSizes();
    void readData();
    char *decompress(char *buf, int& sc);

private:
    InCbStream& stream_;
    std::vector<uint32_t> byte_cnt_;
    std::vector<decoders::arithmetic<MemoryStream>> byte_dec_;
    utils::Summer sumByte;
};

} // namespace detail
} // namespace lazperf
