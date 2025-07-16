/*
===============================================================================

  FILE:  field_nir14.hpp
  
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

class Nir14Base
{
protected:
    struct ChannelCtx
    {
        int have_last_;
        las::nir14 last_;
        models::arithmetic used_model_;
        std::array<models::arithmetic, 2> diff_model_;

        ChannelCtx() : have_last_{false}, used_model_(4),
            diff_model_{ models::arithmetic(256), models::arithmetic(256) }
        {}
    };

    std::array<ChannelCtx, 4> chan_ctxs_;
    int last_channel_ = -1;
};

class Nir14Compressor : public Nir14Base
{
public:
    Nir14Compressor(OutCbStream& stream) : stream_(stream), nir_enc_(false)
    {}

    void writeSizes();
    void writeData();
    const char *compress(const char *buf, int& sc);

private:
    OutCbStream& stream_;
    encoders::arithmetic<MemoryStream> nir_enc_;
};

class Nir14Decompressor : public Nir14Base
{
public:
    Nir14Decompressor(InCbStream& stream) : stream_(stream)
    {}

    void dumpSums();
    void readSizes();
    void readData();
    char *decompress(char *buf, int& sc);

private:
    InCbStream& stream_;
    uint32_t nir_cnt_;
    decoders::arithmetic<MemoryStream> nir_dec_;
    utils::Summer sumNir;
};

} // namespace detail
} // namespace lazperf
