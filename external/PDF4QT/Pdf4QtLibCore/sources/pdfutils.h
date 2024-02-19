//    Copyright (C) 2019-2021 Jakub Melka
//
//    This file is part of PDF4QT.
//
//    PDF4QT is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Lesser General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    with the written consent of the copyright owner, any later version.
//
//    PDF4QT is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public License
//    along with PDF4QT.  If not, see <https://www.gnu.org/licenses/>.

#ifndef PDFUTILS_H
#define PDFUTILS_H

#include "pdfglobal.h"

#include <QRectF>
#include <QColor>
#include <QByteArray>
#include <QDataStream>

#include <set>
#include <vector>
#include <iterator>
#include <functional>
#include <type_traits>

namespace pdf
{

/// Class for easy storing of cached item. This class is not thread safe,
/// and for this reason, access function are not constant (they can modify the
/// object).
template<typename T>
class PDFCachedItem
{
public:
    explicit inline PDFCachedItem() :
        m_dirty(true),
        m_object()
    {

    }

    /// Returns the cached object. If object is dirty, then cached object is refreshed.
    /// \param holder Holder object, which owns the cached item
    /// \param function Refresh function
    template<typename H>
    inline const T& get(const H* holder, T(H::* function)(void) const)
    {
        if (m_dirty)
        {
            m_object = (holder->*function)();
            m_dirty = false;
        }

        return m_object;
    }

    /// Returns the cached object. If object is dirty, then cached object is refreshed.
    /// \param holder Holder object, which owns the cached item
    /// \param function Refresh function
    template<typename H>
    inline const T& get(H* holder, T(H::* function)(void))
    {
        if (m_dirty)
        {
            m_object = (holder->*function)();
            m_dirty = false;
        }

        return m_object;
    }

    /// Returns the cached object. If object is dirty, then cached object is refreshed.
    /// \param function Refresh function
    inline const T& get(const std::function<T(void)>& function)
    {
        if (m_dirty)
        {
            m_object = function();
            m_dirty = false;
        }

        return m_object;
    }

    /// Invalidates the cached item, so it must be refreshed from the cache next time,
    /// if it is accessed.
    inline void dirty()
    {
        m_dirty = true;
        m_object = T();
    }

    /// Returns true, if cache is dirty
    inline bool isDirty() const { return m_dirty; }

private:
    bool m_dirty;
    T m_object;
};

/// Bit-reader, which can read n-bit unsigned integers from the stream.
/// Number of bits can be set in the constructor and is constant.
class PDF4QTLIBCORESHARED_EXPORT PDFBitReader
{
public:
    using Value = uint64_t;

    explicit PDFBitReader(const QByteArray* stream, Value bitsPerComponent);

    PDFBitReader(const PDFBitReader&) = default;
    PDFBitReader(PDFBitReader&&) = default;

    PDFBitReader& operator=(const PDFBitReader&) = default;
    PDFBitReader& operator=(PDFBitReader&&) = default;

    /// Returns maximal value of n-bit unsigned integer.
    Value max() const { return m_maximalValue; }

    /// Reads single n-bit value from the stream. If stream hasn't enough data,
    /// then exception is thrown.
    Value read() { return read(m_bitsPerComponent); }

    /// Reads single n-bit value from the stream. If stream hasn't enough data,
    /// then exception is thrown.
    Value read(Value bits);

    /// Reads single n-bit value from the stream. If stream hasn't enough data,
    /// then exception is thrown. State of the stream is not changed, i.e., read
    /// bits are reverted back.
    Value look(Value bits) const;

    /// Seeks the desired position in the data stream. If position can't be seeked,
    /// then exception is thrown.
    void seek(qint64 position);

    /// Skips desired number of bytes
    void skipBytes(Value bytes);

    /// Seeks data to the byte boundary (number of processed bits is divisible by 8)
    void alignToBytes();

    /// Returns true, if we are at the end of the data stream (no more data can be read)
    bool isAtEnd() const;

    /// Returns position in the data stream (byte position, not bit position, so
    /// result of this function is sometimes inaccurate)
    int getPosition() const { return m_position; }

    /// Reads signed 32-bit integer from the stream
    int32_t readSignedInt();

    /// Reads signed 8-bit integer from the stream
    int8_t readSignedByte();

    /// Reads unsigned 32-bit integer from the stream
    uint32_t readUnsignedInt() { return read(32); }

    /// Reads unsigned 16-bit integer from the stream
    uint16_t readUnsignedWord() { return read(16); }

    /// Reads unsigned 8-bit integer from the stream
    uint8_t readUnsignedByte() { return read(8); }

    /// Return underlying byte stream
    const QByteArray* getStream() const { return m_stream; }

    /// Reads substream from current stream. This function works only on byte boundary,
    /// otherwise exception is thrown.
    /// \param length Length of the substream. Can be -1, in this case, all remaining data is read.
    QByteArray readSubstream(int length);

private:
    const QByteArray* m_stream;
    int m_position;

    Value m_bitsPerComponent;
    Value m_maximalValue;

    Value m_buffer;
    Value m_bitsInBuffer;
};

/// Bit writer
class PDF4QTLIBCORESHARED_EXPORT PDFBitWriter
{
public:
    using Value = uint64_t;

    explicit PDFBitWriter(Value bitsPerComponent);

    /// Writes value to the output stream
    void write(Value value);

    /// Finish line - align to byte boundary
    void finishLine() { flush(true); }

    /// Returns the result byte array
    QByteArray takeByteArray() { return qMove(m_outputByteArray); }

    /// Reserve memory in buffer
    void reserve(int size) { m_outputByteArray.reserve(size); }

private:
    void flush(bool alignToByteBoundary);

    QByteArray m_outputByteArray;

    Value m_bitsPerComponent;
    Value m_mask;
    Value m_buffer;
    Value m_bitsInBuffer;
};

/// Simple class guard, for properly saving/restoring new/old value. In the constructor,
/// new value is stored in the pointer (old one is being saved), and in the destructor,
/// old value is restored. This object assumes, that value is not a null pointer.
template<typename Value>
class PDFTemporaryValueChange
{
public:
    /// Constructor
    /// \param value Value pointer (must not be a null pointer)
    /// \param newValue New value to be set to the pointer
    explicit inline PDFTemporaryValueChange(Value* valuePointer, Value newValue) :
        m_oldValue(qMove(*valuePointer)),
        m_value(valuePointer)
    {
        *valuePointer = qMove(newValue);
    }

    inline ~PDFTemporaryValueChange()
    {
        *m_value = qMove(m_oldValue);
    }

private:
    Value m_oldValue;
    Value* m_value;
};

/// Implements range for range based for cycles
template<typename T>
class PDFIntegerRange
{
public:
    explicit inline constexpr PDFIntegerRange(T begin, T end) : m_begin(begin), m_end(end) { }

    struct Iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = ptrdiff_t;
        using value_type        = T;
        using pointer           = T*;
        using reference         = T&;

        inline Iterator() : value(T(0)) { }
        inline Iterator(T value) : value(value) { }

        inline bool operator==(const Iterator& other) const { return value == other.value; }
        inline bool operator!=(const Iterator& other) const { return value != other.value; }

        inline T operator*() const { return value; }
        inline Iterator& operator+=(ptrdiff_t movement) { value += T(movement); return *this; }
        inline Iterator& operator-=(ptrdiff_t movement) { value -= T(movement); return *this; }
        inline Iterator operator+(ptrdiff_t movement) const { return Iterator(value + T(movement)); }
        inline ptrdiff_t operator-(const Iterator& other) const { return ptrdiff_t(value - other.value); }

        inline Iterator& operator++()
        {
            ++value;
            return *this;
        }

        inline Iterator operator++(int)
        {
            Iterator copy(*this);
            ++value;
            return copy;
        }

        inline Iterator& operator--()
        {
            --value;
            return *this;
        }

        inline Iterator operator--(int)
        {
            Iterator copy(*this);
            --value;
            return copy;
        }

        T value = 0;
    };

    Iterator begin() const { return Iterator(m_begin); }
    Iterator end() const { return Iterator(m_end); }

private:
    T m_begin;
    T m_end;
};

template<typename T>
bool contains(T value, std::initializer_list<T> list)
{
    return (std::find(list.begin(), list.end(), value) != list.end());
}

/// Performs linear mapping of value x in interval [x_min, x_max] to the interval [y_min, y_max].
/// \param x Value to be linearly remapped from interval [x_min, x_max] to the interval [y_min, y_max].
/// \param x_min Start of the input interval
/// \param x_max End of the input interval
/// \param y_min Start of the output interval
/// \param y_max End of the output interval
static inline constexpr PDFReal interpolate(PDFReal x, PDFReal x_min, PDFReal x_max, PDFReal y_min, PDFReal y_max)
{
    return y_min + (x - x_min) * (y_max - y_min) / (x_max - x_min);
}

/// Performs linear mapping of value x in interval [x_min, x_max] to the interval [y_min, y_max].
/// \param x Value to be linearly remapped from interval [x_min, x_max] to the interval [y_min, y_max].
/// \param x_min Start of the input interval
/// \param x_max End of the input interval
/// \param y_min Start of the output interval
/// \param y_max End of the output interval
static inline constexpr PDFColorComponent interpolateColors(PDFColorComponent x, PDFColorComponent x_min, PDFColorComponent x_max, PDFColorComponent y_min, PDFColorComponent y_max)
{
    return y_min + (x - x_min) * (y_max - y_min) / (x_max - x_min);
}

inline
std::vector<uint8_t> convertByteArrayToVector(const QByteArray& data)
{
    return std::vector<uint8_t>(reinterpret_cast<const uint8_t*>(data.constData()), reinterpret_cast<const uint8_t*>(data.constData()) + data.size());
}

inline
const unsigned char* convertByteArrayToUcharPtr(const QByteArray& data)
{
    return reinterpret_cast<const unsigned char*>(data.constData());
}

inline
unsigned char* convertByteArrayToUcharPtr(QByteArray& data)
{
    return reinterpret_cast<unsigned char*>(data.data());
}

/// This function computes ceil of log base 2 of value. The algorithm is taken
/// from: http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn.
/// License for this function is public domain.
inline constexpr uint8_t log2ceil(uint32_t value)
{
    const uint32_t originalValue = value;
    constexpr uint8_t MULTIPLY_DE_BRUIJN_BIT_POSITION[32] =
    {
      0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
      8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
    };

    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;

    uint8_t logarithm = MULTIPLY_DE_BRUIJN_BIT_POSITION[static_cast<uint32_t>((value * 0x07C4ACDDU) >> 27)];

    // Ceil
    if ((1U << logarithm) < originalValue)
    {
        ++logarithm;
    }

    return logarithm;
}

struct PDF4QTLIBCORESHARED_EXPORT PDFDependentLibraryInfo
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFDependentLibraryInfo)

public:
    QString library;
    QString version;
    QString license;
    QString url;

    static std::vector<PDFDependentLibraryInfo> getLibraryInfo();
};

/// Union-find algorithm, which uses path compression optimization. It can run in time
/// O(n + f * (1 + log(n)/log(2 + f/n)), where n is number of unions (resp. size of the
/// array) and f is number of find operations.
template<typename T>
class PDFUnionFindAlgorithm
{
public:
    explicit PDFUnionFindAlgorithm(T size)
    {
        m_indices.resize(size, T(0));
        std::iota(m_indices.begin(), m_indices.end(), 0);
    }

    T find(T index)
    {
        // Use path compression optimization. We assume we will not
        // have long paths, so we will use simple recursion and
        // not while cycle.
        if (m_indices[index] != index)
        {
            m_indices[index] = find(m_indices[index]);
        }

        return m_indices[index];
    }

    void unify(T x, T y)
    {
        T xRoot = find(x);
        T yRoot = find(y);

        if (xRoot < yRoot)
        {
            m_indices[yRoot] = xRoot;
        }
        else if (xRoot > yRoot)
        {
            m_indices[xRoot] = yRoot;
        }
    }

private:
    std::vector<T> m_indices;
};

template<typename T>
constexpr bool isIntervalOverlap(T x1_min, T x1_max, T x2_min, T x2_max)
{
    // We have two situations, where intervals doesn't overlap:
    //    1)  |--------|        |---------|
    //       x1_min   x1_max   x2_min    x2_max
    //    2)  |--------|        |---------|
    //       x2_min   x2_max   x1_min    x1_max
    if (x1_max < x2_min || x2_max < x1_min)
    {
        return false;
    }

    return true;
}

constexpr bool isRectangleHorizontallyOverlapped(const QRectF& r1, const QRectF& r2)
{
    return isIntervalOverlap(r1.left(), r1.right(), r2.left(), r2.right());
}

constexpr bool isRectangleVerticallyOverlapped(const QRectF& r1, const QRectF& r2)
{
    return isIntervalOverlap(r1.top(), r1.bottom(), r2.top(), r2.bottom());
}

inline QColor invertColor(QColor color)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    float r = 0.0;
    float g = 0.0;
    float b = 0.0;
    float a = 0.0;
#else
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double a = 0.0;
#endif

    color.getRgbF(&r, &g, &b, &a);

    r = 1.0f - r;
    g = 1.0f - g;
    b = 1.0f - b;
    return QColor::fromRgbF(r, g, b, a);
}

/// Performs linear interpolation of interval [x1, x2] to interval [y1, y2],
/// using formula y = y1 + (x - x1) * (y2 - y1) / (x2 - x1), transformed
/// to formula y = k * x + q, where q = y1 - x1 * k and
/// k = (y2 - y1) / (x2 - x1).
template<typename T>
class PDFLinearInterpolation
{
public:
    constexpr inline PDFLinearInterpolation(T x1, T x2, T y1, T y2) :
        m_k((y2 - y1) / (x2 - x1)),
        m_q(y1 - x1 * m_k)
    {

    }

    /// Maps value from x interval to y interval
    constexpr inline T operator()(T x) const
    {
        return m_k * x + m_q;
    }

private:
    T m_k;
    T m_q;
};

/// Fuzzy compares two points, with given tolerance (so, if points are at lower distance
/// from each other than squared tolerance, they are considered as same and function returns true).
/// \param p1 First point
/// \param p2 Second point
/// \param squaredTolerance Squared tolerance
static inline bool isFuzzyComparedPointsSame(const QPointF& p1, const QPointF& p2, PDFReal squaredTolerance)
{
    QPointF dp = p2 - p1;
    const qreal squaredDistance = QPointF::dotProduct(dp, dp);
    return squaredDistance < squaredTolerance;
}

/// View on the array
template<typename T>
class PDFBuffer
{
public:
    using value_type = T;
    using value_ptr = value_type*;
    using const_value_type = typename std::add_const<value_type>::type;
    using const_value_ptr = const_value_type*;
    using value_ref = value_type&;
    using const_value_ref = typename std::add_const<value_ref>::type;

    explicit inline PDFBuffer() :
        m_begin(nullptr),
        m_end(nullptr)
    {

    }

    explicit inline PDFBuffer(value_ptr value, size_t size) :
        m_begin(value),
        m_end(value + size)
    {

    }

    inline value_ptr begin() { return m_begin; }
    inline value_ptr end() { return m_end; }

    inline const_value_ptr begin() const { return m_begin; }
    inline const_value_ptr end() const { return m_end; }

    inline const_value_ptr cbegin() const { return m_begin; }
    inline const_value_ptr cend() const { return m_end; }

    inline value_ref operator[](size_t index) { return *(m_begin + index); }
    inline const_value_ref operator[](size_t index) const { return *(m_begin + index); }

    size_t size() const { return m_end - m_begin; }

    PDFBuffer resized(size_t newSize) const
    {
        Q_ASSERT(newSize <= size());
        return PDFBuffer(m_begin, newSize);
    }

private:
    value_ptr m_begin;
    value_ptr m_end;
};

/// Storage for result of some operation. Stores, if operation was successful, or not and
/// also error message, why operation has failed. Can be converted explicitly to bool.
class PDFOperationResult
{
public:
    inline PDFOperationResult(bool success) :
        m_success(success)
    {

    }

    inline PDFOperationResult(QString message) :
        m_success(false),
        m_errorMessage(qMove(message))
    {

    }

    explicit operator bool() const { return m_success; }

    const QString& getErrorMessage() const { return m_errorMessage; }

private:
    bool m_success;
    QString m_errorMessage;
};

template<typename Enum>
class PDFFlags
{
public:
    using Integer = typename std::underlying_type<Enum>::type;

    constexpr inline PDFFlags() noexcept = default;
    constexpr inline PDFFlags(Integer flags) noexcept : m_flags(flags) { }
    constexpr inline PDFFlags(Enum flag) noexcept : m_flags(flag) { }

    constexpr inline PDFFlags& operator|=(Integer flags) { m_flags |= flags; return *this; }
    constexpr inline PDFFlags& operator|=(PDFFlags flags) { m_flags |= flags.m_flags; return *this; }
    constexpr inline PDFFlags& operator|=(Enum flag) { m_flags |= flag; return *this; }
    constexpr inline PDFFlags& operator&=(Integer flags) { m_flags &= flags; return *this; }
    constexpr inline PDFFlags& operator&=(PDFFlags flags) { m_flags &= flags.m_flags; return *this; }
    constexpr inline PDFFlags& operator&=(Enum flag) { m_flags &= flag; return *this; }
    constexpr inline PDFFlags& operator^=(Integer flags) { m_flags ^= flags; return *this; }
    constexpr inline PDFFlags& operator^=(PDFFlags flags) { m_flags ^= flags.m_flags; return *this; }
    constexpr inline PDFFlags& operator^=(Enum flag) { m_flags ^= flag; return *this; }

    constexpr inline operator Integer() const { return m_flags; }

    constexpr inline PDFFlags operator|(Integer flags) const { return PDFFlags(m_flags | flags); }
    constexpr inline PDFFlags operator|(PDFFlags flags) const { return PDFFlags(m_flags | flags.m_flags); }
    constexpr inline PDFFlags operator|(Enum flag) const { return PDFFlags(m_flags | flag); }
    constexpr inline PDFFlags operator&(Integer flags) const { return PDFFlags(m_flags & flags); }
    constexpr inline PDFFlags operator&(PDFFlags flags) const { return PDFFlags(m_flags & flags.m_flags); }
    constexpr inline PDFFlags operator&(Enum flag) const { return PDFFlags(m_flags & flag); }
    constexpr inline PDFFlags operator^(Integer flags) const { return PDFFlags(m_flags ^ flags); }
    constexpr inline PDFFlags operator^(PDFFlags flags) const { return PDFFlags(m_flags ^ flags.m_flags); }
    constexpr inline PDFFlags operator^(Enum flag) const { return PDFFlags(m_flags ^ flag); }
    constexpr inline PDFFlags operator~() const { return PDFFlags(~m_flags); }

    // Explicit bool operator to disallow implicit conversion
    constexpr inline explicit operator bool() const { return m_flags != 0; }
    constexpr inline bool operator!() const { return m_flags == 0; }

    constexpr inline bool testFlag(Enum flag) const { return (m_flags & flag) == flag; }
    constexpr inline PDFFlags& setFlag(Enum flag, bool on = true)
    {
        if (on)
        {
            m_flags |= Integer(flag);
        }
        else
        {
            m_flags &= ~Integer(flag);
        }
        return *this;
    }

private:
    Integer m_flags = Integer();
};

/// Get system information
class PDF4QTLIBCORESHARED_EXPORT PDFSysUtils
{
public:

    static QString getUserName();
};

/// Set of closed intervals
class PDF4QTLIBCORESHARED_EXPORT PDFClosedIntervalSet
{
public:
    explicit inline PDFClosedIntervalSet() = default;

    bool operator ==(const PDFClosedIntervalSet&) const = default;

    using ClosedInterval = std::pair<PDFInteger, PDFInteger>;

    /// Adds closed interval, where \p low is lower bound
    /// of the closed interval, and high is upper bound
    /// of closed interval.
    /// \param low Lower bound of interval
    /// \param high Upper bound of interval
    void addInterval(PDFInteger low, PDFInteger high);

    /// Adds a single value to the interval (closed interval
    /// of single value)
    /// \param value Value
    void addValue(PDFInteger value) { addInterval(value, value); }

    /// Merge with other interval set
    void merge(const PDFClosedIntervalSet& other);

    /// Returns true, if given closed range is subset of
    /// this interval set.
    bool isCovered(PDFInteger low, PDFInteger high);

    /// Returns sum of interval lengths
    PDFInteger getTotalLength() const;

    /// Transforms interval set to readable text
    QString toText(bool withoutBrackets) const;

    /// Returns all integers from the range
    std::vector<PDFInteger> unfold() const;

    /// Returns true, if interval set is empty
    bool isEmpty() const { return m_intervals.empty(); }

    /// Translates interval set by a given offset
    /// \param offset Offset
    void translate(PDFInteger offset);

    /// Parses text into closed interval set, text should be in form "1,3,4,7,-11,12-,52-53,-",
    /// where 1,3,4,7 means single pages, -11 means range from \p first to 11, 12- means range
    /// from 12 to \p last, and 52-53 means closed interval [52, 53]. If text is not in this form,
    /// then empty interval set is returned and if \p errorMessage is specified, then error message
    /// is stored here. Parsed numbers must be equal or greater than \p first and lower or equal
    /// to \p last, if overflow occurs, then error message is returned.
    /// \param[in] first Lower bound of work range
    /// \param[in] last Upper bound of work range
    /// \param[in] text Text
    /// \param[out] errorMessage Error message
    static PDFClosedIntervalSet parse(PDFInteger first, PDFInteger last, const QString& text, QString* errorMessage);

private:
    /// Normalizes interval ranges - merges adjacent intervals
    void normalize();

    /// Returns true, if interval overlaps, or is adjacent to the other one
    /// \param a First interval
    /// \param b Second interval
    static bool overlapsOrAdjacent(ClosedInterval a, ClosedInterval b);

    std::vector<ClosedInterval> m_intervals;
};

QDataStream& operator>>(QDataStream& stream, long unsigned int &i);

template<typename T>
QDataStream& operator>>(QDataStream& stream, std::vector<T>& vector)
{
    typename std::vector<T>::size_type size = 0;
    stream >> size;
    vector.resize(size);
    for (T& item : vector)
    {
        stream >> item;
    }
    return stream;
}

QDataStream& operator<<(QDataStream& stream, long unsigned int i);

template<typename T>
QDataStream& operator<<(QDataStream& stream, const std::vector<T>& vector)
{
    stream << vector.size();
    for (const T& item : vector)
    {
        stream << item;
    }
    return stream;
}

template<typename T, size_t Size>
QDataStream& operator>>(QDataStream& stream, std::array<T, Size>& array)
{
    typename std::array<T, Size>::size_type size = 0;
    stream >> size;

    for (size_t i = 0; i < size; ++i)
    {
        if (i < array.size())
        {
            stream >> array[i];
        }
        else
        {
            T item;
            stream >> item;
        }
    }

    // If array size was changed, then fill in empty objects
    for (size_t i = size; i < array.size(); ++i)
    {
        array[i] = T();
    }

    return stream;
}

template<typename T, size_t Size>
QDataStream& operator<<(QDataStream& stream, const std::array<T, Size>& array)
{
    stream << array.size();
    for (const T& item : array)
    {
        stream << item;
    }
    return stream;
}

template<typename T>
QDataStream& operator>>(QDataStream& stream, std::set<T>& set)
{
    typename std::set<T>::size_type size = 0;
    stream >> size;
    for (size_t i = 0; i < size; ++i)
    {
        T item;
        stream >> item;
        set.insert(set.end(), qMove(item));
    }
    return stream;
}

template<typename T>
QDataStream& operator<<(QDataStream& stream, const std::set<T>& set)
{
    stream << set.size();
    for (const T& item : set)
    {
        stream << item;
    }
    return stream;
}

/// Color scale represents hot-to-cold color scale. It maps value
/// to the color from blue trough green to red.
class PDF4QTLIBCORESHARED_EXPORT PDFColorScale
{
public:
    explicit PDFColorScale();

    /// Creates a new color scale for defined range
    /// \param min Lower bound of a scale range
    /// \param max Upper bound of a scale range
    explicit PDFColorScale(PDFReal min, PDFReal max);

    /// Map value to the color. If value is outside of the range, it
    /// is clamped to fit in the range.
    /// \param value Value
    QColor map(PDFReal value) const;

    /// Returns color values of the scale
    const std::vector<QColor> getColorScales() const { return m_colorScales; }

    PDFReal getMin() const { return m_min; }
    PDFReal getMax() const { return m_max; }

    /// Returns true, if color scale is valid
    bool isValid() const { return m_min < m_max && !m_colorScales.empty(); }

private:
    std::vector<QColor> m_colorScales;
    PDFReal m_min;
    PDFReal m_max;
};

}   // namespace pdf

#endif // PDFUTILS_H
