#include <algorithm>

#include "filestream.hpp"
#include "excepts.hpp"

namespace lazperf
{

OutFileStream::OutFileStream(std::ostream& out) : f_(out)
{}

// Should probably write to memory to avoid all these calls to a file that can't
// be seen by the compiler.
void OutFileStream::putBytes(const unsigned char *c, size_t len)
{
    f_.write(reinterpret_cast<const char *>(c), len);
}

OutputCb OutFileStream::cb()
{
    using namespace std::placeholders;

    return std::bind(&OutFileStream::putBytes, this, _1, _2);
}

// InFileStream

struct InFileStream::Private
{
    // Setting the offset_ to the buffer size will force a fill on the first read.
    Private(std::istream& in) : f_(in)
    { reset(); }

    void getBytes(unsigned char *buf, size_t request);
    size_t fillit();
    void reset()
    {
        buf_.resize(1 << 20);
        offset_ = buf_.size();
    }

    std::istream& f_;
    std::vector<unsigned char> buf_;
    size_t offset_;
};

InFileStream::InFileStream(std::istream& in) : p_(new Private(in))
{}

InFileStream::~InFileStream()
{}

// This will force a fill on the next fetch.
void InFileStream::reset()
{
    p_->reset();
}

InputCb InFileStream::cb()
{
    using namespace std::placeholders;

    return std::bind(&InFileStream::Private::getBytes, p_.get(), _1, _2);
}

void InFileStream::Private::getBytes(unsigned char *buf, size_t request)
{
    // Almost all requests are size 1.
    if (request == 1)
    {
        if (offset_ >= buf_.size())
            fillit();
        *buf = buf_[offset_++];
        return;
    }

    size_t available = buf_.size() - offset_;
    if (request <= available)
    {
        unsigned char *begin = buf_.data() + offset_;
        unsigned char *end = begin + request;
        std::copy(begin, end, buf);
        offset_ += request;
    }
    else
    {
        do
        {
            size_t bytes = (std::min)(request, available);
            unsigned char *begin = buf_.data() + offset_;
            unsigned char *end = begin + bytes;
            std::copy(begin, end, buf);
            offset_ += bytes;
            request -= bytes;
            if (request == 0)
                break;
            buf += bytes;
            available = fillit();
        } while (true);
    }
}

size_t InFileStream::Private::fillit()
{
    offset_ = 0;
    f_.read(reinterpret_cast<char *>(buf_.data()), buf_.size());
    size_t filled = f_.gcount();

    if (filled == 0)
        throw error("Unexpected end of file.");
    buf_.resize(filled);
    return filled;
}

} // namespace lazperf
