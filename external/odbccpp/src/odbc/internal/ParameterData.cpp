#include <new>
#include <odbc/internal/Odbc.h>
#include <odbc/internal/ParameterData.h>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <utility>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
ParameterData::ParameterData()
    : state_(UNINITIALIZED)
    , valueType_(0)
    , columnSize_(0)
    , decimalDigits_(0)
{
}
//------------------------------------------------------------------------------
ParameterData::ParameterData(ParameterData&& other)
    : state_(other.state_)
    , valueType_(other.valueType_)
    , columnSize_(other.columnSize_)
    , decimalDigits_(other.decimalDigits_)
    , size_(other.size_)
{
    switch (state_)
    {
    case UNINITIALIZED:
    case IS_NULL:
        break;
    case NORMAL_INPLACE:
        memcpy(inplaceData_, other.inplaceData_, size_);
        break;
    case NORMAL_HEAP_OWNING:
    case NORMAL_HEAP_NOT_OWNING:
        capacity_ = other.capacity_;
        heapData_ = other.heapData_;
        break;
    }
    other.state_ = UNINITIALIZED;
}
//------------------------------------------------------------------------------
ParameterData::~ParameterData()
{
    if (state_ == NORMAL_HEAP_OWNING)
        free(heapData_);
}
//------------------------------------------------------------------------------
ParameterData& ParameterData::operator=(ParameterData&& other)
{
    if (this == &other)
        return *this;

    if (state_ == NORMAL_HEAP_OWNING)
        free(heapData_);

    state_ = other.state_;
    valueType_ = other.valueType_;
    columnSize_ = other.columnSize_;
    decimalDigits_ = other.decimalDigits_;
    size_ = other.size_;
    switch (state_)
    {
    case UNINITIALIZED:
        break;
    case IS_NULL:
        break;
    case NORMAL_INPLACE:
        memcpy(inplaceData_, other.inplaceData_, size_);
        break;
    case NORMAL_HEAP_OWNING:
    case NORMAL_HEAP_NOT_OWNING:
        capacity_ = other.capacity_;
        heapData_ = other.heapData_;
        break;
    }
    other.state_ = UNINITIALIZED;
    return *this;
}
//------------------------------------------------------------------------------
void ParameterData::setValue(int16_t type, const void* value, size_t size)
{
    // TODO: Maybe we'd like to set the value on the heap even if it fitted into
    // the inplace buffer. This would avoid re-allocations if a value slightly
    // larger then the inplace buffer is set.
    if (size <= INPLACE_BYTES)
        setValueInplace(value, size);
    else
        setValueOnHeap(value, size);
    valueType_ = type;
    columnSize_ = 0;
    decimalDigits_ = 0;
}
//------------------------------------------------------------------------------
void ParameterData::setNull(int16_t type)
{
    if (state_ == NORMAL_HEAP_OWNING)
        free(heapData_);
    valueType_ = type;
    state_ = IS_NULL;
    size_ = SQL_NULL_DATA;
}
//------------------------------------------------------------------------------
void ParameterData::clear()
{
    if (state_ == NORMAL_HEAP_OWNING)
        free(heapData_);
    state_ = UNINITIALIZED;
}
//------------------------------------------------------------------------------
const void* ParameterData::getData() const
{
    switch (state_)
    {
    case UNINITIALIZED:
    case IS_NULL:
        assert(false);
        return nullptr;
    case NORMAL_INPLACE:
        return inplaceData_;
    case NORMAL_HEAP_OWNING:
    case NORMAL_HEAP_NOT_OWNING:
        return heapData_;
    }
    return nullptr;
}
//------------------------------------------------------------------------------
bool ParameterData::usesHeapBuffer() const
{
    return (state_ == NORMAL_HEAP_OWNING) || (state_ == NORMAL_HEAP_NOT_OWNING);
}
//------------------------------------------------------------------------------
void ParameterData::releaseHeapBufferOwnership()
{
    assert(state_ == NORMAL_HEAP_OWNING);
    state_ = NORMAL_HEAP_NOT_OWNING;
}
//------------------------------------------------------------------------------
void ParameterData::restoreHeapBufferOwnership()
{
    assert(state_ == NORMAL_HEAP_NOT_OWNING);
    state_ = NORMAL_HEAP_OWNING;
}
//------------------------------------------------------------------------------
void ParameterData::setValueInplace(const void* value, size_t size)
{
    if (state_ == NORMAL_HEAP_OWNING)
        free(heapData_);
    state_ = NORMAL_INPLACE;
    size_ = size;
    memcpy(inplaceData_, value, size);
}
//------------------------------------------------------------------------------
void ParameterData::setValueOnHeap(const void* value, size_t size)
{
    if (state_ != NORMAL_HEAP_OWNING)
    {
        void* data = malloc(size);
        if (data == nullptr)
            throw bad_alloc();
        memcpy(data, value, size);
        capacity_ = size;
        heapData_ = data;
        state_ = NORMAL_HEAP_OWNING;
        size_ = size;
        return;
    }

    size_t reallocIfLess = (size_t)(LOAD_FACTOR * (double)capacity_);
    if ((size > capacity_) || (size < reallocIfLess))
    {
        void* data = malloc(size);
        if (data == nullptr)
            throw bad_alloc();
        memcpy(data, value, size);
        free(heapData_);
        capacity_ = size;
        heapData_ = data;
        size_ = size;
        return;
    }

    memcpy(heapData_, value, size);
    size_ = size;
}
//------------------------------------------------------------------------------
NS_ODBC_END
