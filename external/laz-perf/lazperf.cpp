#include <vector>

#include "las.hpp"
#include "lazperf.hpp"
#include "portable_endian.hpp"

namespace lazperf
{

// COMPRESSOR

las_compressor::~las_compressor()
{}

// 1.2 COMPRESSOR BASE

struct point_compressor_base_1_2::Private
{
    Private(OutputCb cb, size_t ebCount) : stream_(cb), encoder_(stream_), point_(encoder_),
        gpstime_(encoder_), rgb_(encoder_), byte_(encoder_, ebCount)
    {}

    OutCbStream stream_;
    encoders::arithmetic<OutCbStream> encoder_;
    detail::Point10Compressor point_;
    detail::Gpstime10Compressor gpstime_;
    detail::Rgb10Compressor rgb_;
    detail::Byte10Compressor byte_;
};

point_compressor_base_1_2::point_compressor_base_1_2(OutputCb cb, size_t ebCount) :
    p_(new Private(cb, ebCount))
{}

point_compressor_base_1_2::~point_compressor_base_1_2()
{}

void point_compressor_base_1_2::done()
{
    p_->encoder_.done();
}

// COMPRESSOR 0

point_compressor_0::~point_compressor_0()
{}

point_compressor_0::point_compressor_0(OutputCb cb, size_t ebCount) :
    point_compressor_base_1_2(cb, ebCount)
{}

const char *point_compressor_0::compress(const char *in)
{
    in = p_->point_.compress(in);
    in = p_->byte_.compress(in);
    return in;
}

// COMPRESSOR 1

point_compressor_1::~point_compressor_1()
{}

point_compressor_1::point_compressor_1(OutputCb cb, size_t ebCount) :
    point_compressor_base_1_2(cb, ebCount)
{}

const char *point_compressor_1::compress(const char *in)
{
    in = p_->point_.compress(in);
    in = p_->gpstime_.compress(in);
    in = p_->byte_.compress(in);
    return in;
}

// COMPRESSOR 2

point_compressor_2::~point_compressor_2()
{}

point_compressor_2::point_compressor_2(OutputCb cb, size_t ebCount) :
    point_compressor_base_1_2(cb, ebCount)
{}

const char *point_compressor_2::compress(const char *in)
{
    in = p_->point_.compress(in);
    in = p_->rgb_.compress(in);
    in = p_->byte_.compress(in);
    return in;
}

// COMPRESSOR 3

point_compressor_3::~point_compressor_3()
{}

point_compressor_3::point_compressor_3(OutputCb cb, size_t ebCount) :
    point_compressor_base_1_2(cb, ebCount)
{}

const char *point_compressor_3::compress(const char *in)
{
    in = p_->point_.compress(in);
    in = p_->gpstime_.compress(in);
    in = p_->rgb_.compress(in);
    in = p_->byte_.compress(in);
    return in;
}

// 1.4 COMPRESSOR BASE

struct point_compressor_base_1_4::Private
{
    Private(OutputCb cb, size_t ebCount) : stream_(cb), chunk_count_(0), point_(stream_),
        rgb_(stream_), nir_(stream_), byte_(stream_, ebCount)
    {}

    OutCbStream stream_;
    uint32_t chunk_count_;
    detail::Point14Compressor point_;
    detail::Rgb14Compressor rgb_;
    detail::Nir14Compressor nir_;
    detail::Byte14Compressor byte_;
};

point_compressor_base_1_4::point_compressor_base_1_4(OutputCb cb, size_t ebCount) :
    p_(new Private(cb, ebCount))
{}

// COMPRESOR 6

point_compressor_6::~point_compressor_6()
{}

point_compressor_6::point_compressor_6(OutputCb cb, size_t ebCount) :
    point_compressor_base_1_4(cb, ebCount)
{}

const char *point_compressor_6::compress(const char *in)
{
    int channel = 0;
    p_->chunk_count_++;
    in = p_->point_.compress(in, channel);
    if (p_->byte_.count())
        in = p_->byte_.compress(in, channel);
    return in;
}

void point_compressor_6::done()
{
    p_->stream_ << p_->chunk_count_;

    p_->point_.writeSizes();
    if (p_->byte_.count())
        p_->byte_.writeSizes();

    p_->point_.writeData();
    if (p_->byte_.count())
        p_->byte_.writeData();
}

// COMPRESOR 7

point_compressor_7::~point_compressor_7()
{}

point_compressor_7::point_compressor_7(OutputCb cb, size_t ebCount) :
    point_compressor_base_1_4(cb, ebCount)
{}

const char *point_compressor_7::compress(const char *in)
{
    int channel = 0;
    p_->chunk_count_++;
    in = p_->point_.compress(in, channel);
    in = p_->rgb_.compress(in, channel);
    if (p_->byte_.count())
        in = p_->byte_.compress(in, channel);
    return in;
}

void point_compressor_7::done()
{
    p_->stream_ << p_->chunk_count_;

    p_->point_.writeSizes();
    p_->rgb_.writeSizes();
    if (p_->byte_.count())
        p_->byte_.writeSizes();

    p_->point_.writeData();
    p_->rgb_.writeData();
    if (p_->byte_.count())
        p_->byte_.writeData();
}

// COMPRESOR 8

point_compressor_8::~point_compressor_8()
{}

point_compressor_8::point_compressor_8(OutputCb cb, size_t ebCount) :
    point_compressor_base_1_4(cb, ebCount)
{}

const char *point_compressor_8::compress(const char *in)
{
    int channel = 0;
    p_->chunk_count_++;
    in = p_->point_.compress(in, channel);
    in = p_->rgb_.compress(in, channel);
    in = p_->nir_.compress(in, channel);
    if (p_->byte_.count())
        in = p_->byte_.compress(in, channel);
    return in;
}

void point_compressor_8::done()
{
    p_->stream_ << p_->chunk_count_;

    p_->point_.writeSizes();
    p_->rgb_.writeSizes();
    p_->nir_.writeSizes();
    if (p_->byte_.count())
        p_->byte_.writeSizes();

    p_->point_.writeData();
    p_->rgb_.writeData();
    p_->nir_.writeData();
    if (p_->byte_.count())
        p_->byte_.writeData();
}

// DECOMPRESSOR

las_decompressor::~las_decompressor()
{}

// 1.2 DECOMPRESSOR BASE

struct point_decompressor_base_1_2::Private
{
    Private(InputCb cb, size_t ebCount) : stream_(cb), decoder_(stream_), point_(decoder_),
        gpstime_(decoder_), rgb_(decoder_), byte_(decoder_, ebCount), first_(true)
    {}

    InCbStream stream_;
    decoders::arithmetic<InCbStream> decoder_;
    detail::Point10Decompressor point_;
    detail::Gpstime10Decompressor gpstime_;
    detail::Rgb10Decompressor rgb_;
    detail::Byte10Decompressor byte_;
    bool first_;
};

point_decompressor_base_1_2::point_decompressor_base_1_2(InputCb cb, size_t ebCount) :
    p_(new Private(cb, ebCount))
{}

point_decompressor_base_1_2::~point_decompressor_base_1_2()
{}

void point_decompressor_base_1_2::handleFirst()
{
    if (p_->first_)
    {
        p_->decoder_.readInitBytes();
        p_->first_ = false;
    }
}

// DECOMPRESSOR 0

point_decompressor_0::~point_decompressor_0()
{}

point_decompressor_0::point_decompressor_0(InputCb cb, size_t ebCount) :
    point_decompressor_base_1_2(cb, ebCount)
{}

char *point_decompressor_0::decompress(char *in)
{
    in = p_->point_.decompress(in);
    in = p_->byte_.decompress(in);
    handleFirst();
    return in;
}

// DECOMPRESSOR 1

point_decompressor_1::~point_decompressor_1()
{}

point_decompressor_1::point_decompressor_1(InputCb cb, size_t ebCount) :
    point_decompressor_base_1_2(cb, ebCount)
{}

char *point_decompressor_1::decompress(char *in)
{
    in = p_->point_.decompress(in);
    in = p_->gpstime_.decompress(in);
    in = p_->byte_.decompress(in);
    handleFirst();
    return in;
}

// DECOMPRESSOR 2

point_decompressor_2::~point_decompressor_2()
{}

point_decompressor_2::point_decompressor_2(InputCb cb, size_t ebCount) :
    point_decompressor_base_1_2(cb, ebCount)
{}

char *point_decompressor_2::decompress(char *in)
{
    in = p_->point_.decompress(in);
    in = p_->rgb_.decompress(in);
    in = p_->byte_.decompress(in);
    handleFirst();
    return in;
}

// DECOMPRESSOR 3

point_decompressor_3::~point_decompressor_3()
{}

point_decompressor_3::point_decompressor_3(InputCb cb, size_t ebCount) :
    point_decompressor_base_1_2(cb, ebCount)
{}

char *point_decompressor_3::decompress(char *in)
{
    in = p_->point_.decompress(in);
    in = p_->gpstime_.decompress(in);
    in = p_->rgb_.decompress(in);
    in = p_->byte_.decompress(in);
    handleFirst();
    return in;
}

// 1.4 BASE DECOMPRESSOR

struct point_decompressor_base_1_4::Private
{
public:
    Private(InputCb cb, size_t ebCount) : cbStream_(cb), point_(cbStream_), rgb_(cbStream_),
        nir_(cbStream_), byte_(cbStream_, ebCount), chunk_count_(0), first_(true)
    {}

    InCbStream cbStream_;
    detail::Point14Decompressor point_;
    detail::Rgb14Decompressor rgb_;
    detail::Nir14Decompressor nir_;
    detail::Byte14Decompressor byte_;
    uint32_t chunk_count_;
    bool first_;
};

point_decompressor_base_1_4::point_decompressor_base_1_4(InputCb cb, size_t ebCount) :
    p_(new Private(cb, ebCount))
{}
    
// DECOMPRESSOR 6

point_decompressor_6::point_decompressor_6(InputCb cb, size_t ebCount) :
    point_decompressor_base_1_4(cb, ebCount)
{}

point_decompressor_6::~point_decompressor_6()
{
#ifndef NDEBUG
    p_->point_.dumpSums();
    if (p_->byte_.count())
        p_->byte_.dumpSums();
    std::cerr << "\n";
#endif
}

char *point_decompressor_6::decompress(char *out)
{
    int channel = 0;

    out = p_->point_.decompress(out, channel);
    if (p_->byte_.count())
        out = p_->byte_.decompress(out, channel);

    if (p_->first_)
    {
        // Read the point count the streams for each data member.
        p_->cbStream_ >> p_->chunk_count_;
        p_->point_.readSizes();
        if (p_->byte_.count())
            p_->byte_.readSizes();

        p_->point_.readData();
        if (p_->byte_.count())
            p_->byte_.readData();
        p_->first_ = false;
    }
    return out;
}

// DECOMPRESSOR 7

point_decompressor_7::point_decompressor_7(InputCb cb, size_t ebCount) :
    point_decompressor_base_1_4(cb, ebCount)
{}

point_decompressor_7::~point_decompressor_7()
{
#ifndef NDEBUG
    p_->point_.dumpSums();
    p_->rgb_.dumpSums();
    if (p_->byte_.count())
        p_->byte_.dumpSums();
    std::cerr << "\n";
#endif
}

char *point_decompressor_7::decompress(char *out)
{
    int channel = 0;

    out = p_->point_.decompress(out, channel);
    out = p_->rgb_.decompress(out, channel);
    if (p_->byte_.count())
        out = p_->byte_.decompress(out, channel);

    if (p_->first_)
    {
        // Read the point count the streams for each data member.
        p_->cbStream_ >> p_->chunk_count_;
        p_->point_.readSizes();
        p_->rgb_.readSizes();
        if (p_->byte_.count())
            p_->byte_.readSizes();

        p_->point_.readData();
        p_->rgb_.readData();
        if (p_->byte_.count())
            p_->byte_.readData();
        p_->first_ = false;
    }
    return out;
}

// DECOMPRESSOR 8

point_decompressor_8::point_decompressor_8(InputCb cb, size_t ebCount) :
    point_decompressor_base_1_4(cb, ebCount)
{}

point_decompressor_8::~point_decompressor_8()
{
#ifndef NDEBUG
    p_->point_.dumpSums();
    p_->rgb_.dumpSums();
    p_->nir_.dumpSums();
    if (p_->byte_.count())
        p_->byte_.dumpSums();
    std::cerr << "\n";
#endif
}

char *point_decompressor_8::decompress(char *out)
{
    int channel = 0;

    out = p_->point_.decompress(out, channel);
    out = p_->rgb_.decompress(out, channel);
    out = p_->nir_.decompress(out, channel);
    if (p_->byte_.count())
        out = p_->byte_.decompress(out, channel);

    if (p_->first_)
    {
        // Read the point count the streams for each data member.
        p_->cbStream_ >> p_->chunk_count_;
        p_->point_.readSizes();
        p_->rgb_.readSizes();
        p_->nir_.readSizes();
        if (p_->byte_.count())
            p_->byte_.readSizes();

        p_->point_.readData();
        p_->rgb_.readData();
        p_->nir_.readData();
        if (p_->byte_.count())
            p_->byte_.readData();
        p_->first_ = false;
    }
    return out;
}

// FACTORY

las_compressor::ptr build_las_compressor(OutputCb cb, int format, size_t ebCount)
{
    las_compressor::ptr compressor;

    switch (format)
    {
    case 0:
        compressor.reset(new point_compressor_0(cb, ebCount));
        break;
    case 1:
        compressor.reset(new point_compressor_1(cb, ebCount));
        break;
    case 2:
        compressor.reset(new point_compressor_2(cb, ebCount));
        break;
    case 3:
        compressor.reset(new point_compressor_3(cb, ebCount));
        break;
    case 6:
        compressor.reset(new point_compressor_6(cb, ebCount));
        break;
    case 7:
        compressor.reset(new point_compressor_7(cb, ebCount));
        break;
    case 8:
        compressor.reset(new point_compressor_8(cb, ebCount));
    }
    return compressor;
}

las_decompressor::ptr build_las_decompressor(InputCb cb, int format, size_t ebCount)
{
    las_decompressor::ptr decompressor;

    switch (format)
    {
    case 0:
        decompressor.reset(new point_decompressor_0(cb, ebCount));
        break;
    case 1:
        decompressor.reset(new point_decompressor_1(cb, ebCount));
        break;
    case 2:
        decompressor.reset(new point_decompressor_2(cb, ebCount));
        break;
    case 3:
        decompressor.reset(new point_decompressor_3(cb, ebCount));
        break;
    case 6:
        decompressor.reset(new point_decompressor_6(cb, ebCount));
        break;
    case 7:
        decompressor.reset(new point_decompressor_7(cb, ebCount));
        break;
    case 8:
        decompressor.reset(new point_decompressor_8(cb, ebCount));
        break;
    }
    return decompressor;
}

// CHUNK TABLE

void compress_chunk_table(OutputCb cb, const std::vector<uint32_t>& chunks)
{
    OutCbStream stream(cb);
    encoders::arithmetic<OutCbStream> encoder(stream);
    compressors::integer compressor(32, 2);
    uint32_t predictor = 0;

    compressor.init();
    for (uint32_t chunk : chunks)
    {
        chunk = htole32(chunk);
        compressor.compress(encoder, predictor, chunk, 1);
        predictor = chunk;
    }
    encoder.done();
}

std::vector<uint32_t> decompress_chunk_table(InputCb cb, size_t numChunks)
{
    std::vector<uint32_t> chunks;

    InCbStream stream(cb);
    decoders::arithmetic<InCbStream> decoder(stream);
    decompressors::integer decomp(32, 2);

    decoder.readInitBytes();
    decomp.init();

    uint32_t predictor = 0;
    for (size_t i = 0; i < numChunks; ++i)
    {
        predictor = decomp.decompress(decoder, predictor, 1);
        chunks.push_back(le32toh(predictor));
    }
    return chunks;
}

} // namespace lazperf
