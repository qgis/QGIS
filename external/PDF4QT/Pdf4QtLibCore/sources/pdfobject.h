//    Copyright (C) 2018-2021 Jakub Melka
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


#ifndef PDFOBJECT_H
#define PDFOBJECT_H

#include "pdfglobal.h"

#include <QByteArray>

#include <memory>
#include <vector>
#include <variant>
#include <array>
#include <initializer_list>
#include <cstring>

namespace pdf
{
class PDFArray;
class PDFString;
class PDFStream;
class PDFDictionary;
class PDFAbstractVisitor;

/// This class represents a content of the PDF object. It can be
/// array of objects, dictionary, content stream data, or string data.
class PDFObjectContent
{
public:
    constexpr PDFObjectContent() = default;
    virtual ~PDFObjectContent() = default;

    /// Equals operator. Returns true, if content of this object is
    /// equal to the content of the other object.
    virtual bool equals(const PDFObjectContent* other) const = 0;

    /// Optimizes memory consumption of this object
    virtual void optimize() = 0;
};

/// This class represents inplace string in the PDF object. To avoid too much
/// memory allocation, we store small strings inplace as small objects, so
/// we do not use memory allocator, so this doesn't cause performance downgrade.
/// Very often, PDF document consists of large number of names and strings
/// objects, which will fit into this category.
struct PDFInplaceString
{
    static constexpr const int MAX_STRING_SIZE = sizeof(PDFObjectReference) - 1;

    constexpr PDFInplaceString() = default;

    inline PDFInplaceString(const char* data, int size)
    {
        Q_ASSERT(size <= MAX_STRING_SIZE);
        this->size = static_cast<uint8_t>(size);
        std::copy(data, data + size, string.data());
    }

    inline PDFInplaceString(const QByteArray& data)
    {
        Q_ASSERT(data.size() <= MAX_STRING_SIZE);
        size = static_cast<uint8_t>(data.size());
        std::copy(data.cbegin(), data.cend(), string.data());
    }

    inline bool operator==(const PDFInplaceString& other) const
    {
        if (size != other.size)
        {
            return false;
        }

        for (uint8_t i = 0; i < size; ++i)
        {
            if (string[i] != other.string[i])
            {
                return false;
            }
        }

        return true;
    }

    inline bool operator !=(const PDFInplaceString& other) const
    {
        return !(*this == other);
    }

    QByteArray getString() const
    {
        return (size > 0) ? QByteArray(string.data(), size) : QByteArray();
    }

    uint8_t size = 0;
    std::array<char, MAX_STRING_SIZE> string = { };
};

/// Reference to the string implementations
struct PDF4QTLIBCORESHARED_EXPORT PDFStringRef
{
    const PDFInplaceString* inplaceString = nullptr;
    const PDFString* memoryString = nullptr;

    QByteArray getString() const;
};

/// This class represents string, which can be inplace string (no memory allocation),
/// or classic byte array string, if not enough space for embedded string.
class PDF4QTLIBCORESHARED_EXPORT PDFInplaceOrMemoryString
{
public:
    constexpr PDFInplaceOrMemoryString() = default;
    explicit PDFInplaceOrMemoryString(const char* string);
    explicit PDFInplaceOrMemoryString(QByteArray string);

    // Default destructor should be OK
    inline ~PDFInplaceOrMemoryString() = default;

    // Enforce default copy constructor and default move constructor
    inline PDFInplaceOrMemoryString(const PDFInplaceOrMemoryString&) = default;
    inline PDFInplaceOrMemoryString(PDFInplaceOrMemoryString&&) = default;

    // Enforce default copy assignment operator and move assignment operator
    inline PDFInplaceOrMemoryString& operator=(const PDFInplaceOrMemoryString&) = default;
    inline PDFInplaceOrMemoryString& operator=(PDFInplaceOrMemoryString&&) = default;

    bool equals(const char* value, size_t length) const;

    inline bool operator==(const PDFInplaceOrMemoryString&) const = default;
    inline bool operator!=(const PDFInplaceOrMemoryString&) const = default;

    inline bool operator==(const QByteArray& value) const { return equals(value.constData(), value.size()); }
    inline bool operator==(const char* value) const { return equals(value, std::strlen(value)); }

    /// Returns true, if string is inplace (i.e. doesn't allocate memory)
    bool isInplace() const;

    /// Returns string. If string is inplace, byte array is constructed.
    QByteArray getString() const;

private:
    std::variant<typename std::monostate, PDFInplaceString, QByteArray> m_value;
};

class PDF4QTLIBCORESHARED_EXPORT PDFObject
{
public:
    enum class Type : uint8_t
    {
        // Simple PDF objects
        Null,
        Bool,
        Int,
        Real,
        String,
        Name,

        // Complex PDF objects
        Array,
        Dictionary,
        Stream,
        Reference,

        // Last type mark
        LastType
    };

    static constexpr auto getTypes() { return std::array{ Type::Null, Type::Bool, Type::Int, Type::Real, Type::String, Type::Name, Type::Array, Type::Dictionary, Type::Stream, Type::Reference }; }

    typedef std::shared_ptr<PDFObjectContent> PDFObjectContentPointer;

    // Default constructor should be constexpr
    constexpr inline PDFObject() :
        m_data(),
        m_type(Type::Null)
    {

    }

    // Default destructor should be OK
    inline ~PDFObject() = default;

    // Enforce default copy constructor and default move constructor
    inline PDFObject(const PDFObject&) = default;
    inline PDFObject(PDFObject&&) = default;

    // Enforce default copy assignment operator and move assignment operator
    inline PDFObject& operator=(const PDFObject&) = default;
    inline PDFObject& operator=(PDFObject&&) = default;

    inline Type getType() const { return m_type; }

    // Test operators
    inline bool isNull() const { return m_type == Type::Null; }
    inline bool isBool() const { return m_type == Type::Bool; }
    inline bool isInt() const { return m_type == Type::Int; }
    inline bool isReal() const { return m_type == Type::Real; }
    inline bool isString() const { return m_type == Type::String; }
    inline bool isName() const { return m_type == Type::Name; }
    inline bool isArray() const { return m_type == Type::Array; }
    inline bool isDictionary() const { return m_type == Type::Dictionary; }
    inline bool isStream() const { return m_type == Type::Stream; }
    inline bool isReference() const { return m_type == Type::Reference; }

    inline bool getBool() const { return std::get<bool>(m_data); }
    inline PDFInteger getInteger() const { return std::get<PDFInteger>(m_data); }
    inline PDFReal getReal() const { return std::get<PDFReal>(m_data); }
    QByteArray getString() const;
    const PDFDictionary* getDictionary() const;
    PDFObjectReference getReference() const { return std::get<PDFObjectReference>(m_data); }
    PDFStringRef getStringObject() const;
    const PDFStream* getStream() const;
    const PDFArray* getArray() const;

    bool operator==(const PDFObject& other) const;
    bool operator!=(const PDFObject& other) const { return !(*this == other); }

    /// Accepts the visitor
    void accept(PDFAbstractVisitor* visitor) const;

    /// Creates a null object
    static inline PDFObject createNull() { return PDFObject(); }

    /// Creates a boolean object
    static inline PDFObject createBool(bool value) { return PDFObject(Type::Bool, value); }

    /// Creates an integer object
    static inline PDFObject createInteger(PDFInteger value) { return PDFObject(Type::Int, value); }

    /// Creates an object with real number
    static inline PDFObject createReal(PDFReal value) { return PDFObject(Type::Real, value); }

    /// Creates a reference object
    static inline PDFObject createReference(const PDFObjectReference& reference) { return PDFObject(Type::Reference, reference); }

    /// Creates an array object
    static inline PDFObject createArray(PDFObjectContentPointer&& value) { value->optimize(); return PDFObject(Type::Array, std::move(value)); }

    /// Creates a dictionary object
    static inline PDFObject createDictionary(PDFObjectContentPointer&& value) { value->optimize(); return PDFObject(Type::Dictionary, std::move(value)); }

    /// Creates a stream object
    static inline PDFObject createStream(PDFObjectContentPointer&& value) { value->optimize(); return PDFObject(Type::Stream, std::move(value)); }

    /// Creates a name object
    static PDFObject createName(QByteArray name);

    /// Creates a string object
    static PDFObject createString(QByteArray name);

    /// Creates a name object
    static PDFObject createName(PDFStringRef name);

    /// Creates a string object
    static PDFObject createString(PDFStringRef name);

private:
    template<typename T>
    inline PDFObject(Type type, T&& value) :
        m_data(std::forward<T>(value)),
        m_type(type)
    {

    }


    std::variant<typename std::monostate, bool, PDFInteger, PDFReal, PDFObjectReference, PDFObjectContentPointer, PDFInplaceString> m_data;
    Type m_type;
};

/// Represents raw string in the PDF file. No conversions are performed, this is
/// reason, that we do not use QString, but QByteArray instead.
class PDFString : public PDFObjectContent
{
public:
    inline explicit PDFString() = default;
    inline explicit PDFString(QByteArray&& value) :
        m_string(std::move(value))
    {

    }

    virtual ~PDFString() override = default;

    virtual bool equals(const PDFObjectContent* other) const override;

    const QByteArray& getString() const { return m_string; }
    void setString(const QByteArray &getString);

    /// Optimizes the string for memory consumption
    virtual void optimize() override;

private:
    QByteArray m_string;
};

/// Represents an array of objects in the PDF file.
class PDF4QTLIBCORESHARED_EXPORT PDFArray : public PDFObjectContent
{
public:
    inline PDFArray() = default;
    inline PDFArray(std::vector<PDFObject>&& objects) : m_objects(qMove(objects)) { }
    virtual ~PDFArray() override = default;

    virtual bool equals(const PDFObjectContent* other) const override;

    /// Returns item at the specified index. If index is invalid,
    /// then it throws an exception.
    const PDFObject& getItem(size_t index) const { return m_objects.at(index); }

    /// Sets item at the specified index. Index must be valid.
    void setItem(PDFObject value, size_t index) { m_objects[index] = qMove(value); }

    /// Returns size of the array (number of elements)
    size_t getCount() const { return m_objects.size(); }

    /// Returns capacity of the array (theoretical number of elements before reallocation)
    size_t getCapacity() const { return m_objects.capacity(); }

    /// Appends object to the end of object list
    void appendItem(PDFObject object);

    /// Optimizes the array for memory consumption
    virtual void optimize() override;

    auto begin() { return m_objects.begin(); }
    auto end() { return m_objects.end(); }

    auto begin() const { return m_objects.begin(); }
    auto end() const { return m_objects.end(); }

private:
    std::vector<PDFObject> m_objects;
};

/// Represents a dictionary of objects in the PDF file. Dictionary is
/// an array of pairs key-value, where key is name object and value is any
/// PDF object. For this reason, we use QByteArray for key. We do not use
/// map, because dictionaries are usually small.
class PDF4QTLIBCORESHARED_EXPORT PDFDictionary : public PDFObjectContent
{
public:
    using DictionaryEntry = std::pair<PDFInplaceOrMemoryString, PDFObject>;

    inline PDFDictionary() = default;
    inline PDFDictionary(std::vector<DictionaryEntry>&& dictionary) : m_dictionary(qMove(dictionary)) { }
    virtual ~PDFDictionary() override = default;

    virtual bool equals(const PDFObjectContent* other) const override;

    /// Returns object for the key. If key is not found in the dictionary,
    /// then valid reference to the null object is returned.
    /// \param key Key
    const PDFObject& get(const QByteArray& key) const;

    /// Returns object for the key. If key is not found in the dictionary,
    /// then valid reference to the null object is returned.
    /// \param key Key
    const PDFObject& get(const char* key) const;

    /// Returns object for the key. If key is not found in the dictionary,
    /// then valid reference to the null object is returned.
    /// \param key Key
    const PDFObject& get(const PDFInplaceOrMemoryString& key) const;

    /// Returns true, if dictionary contains a particular key
    /// \param key Key to be found in the dictionary
    bool hasKey(const QByteArray& key) const { return find(key) != m_dictionary.cend(); }

    /// Returns true, if dictionary contains a particular key
    /// \param key Key to be found in the dictionary
    bool hasKey(const char* key) const { return find(key) != m_dictionary.cend(); }

    /// Removes entry with given key. If entry with this key is not found,
    /// nothing happens.
    /// \param key Key to be removed
    void removeEntry(const char* key);

    /// Adds a new entry to the dictionary.
    /// \param key Key
    /// \param value Value
    void addEntry(PDFInplaceOrMemoryString&& key, PDFObject&& value) { m_dictionary.emplace_back(std::move(key), std::move(value)); }

    /// Adds a new entry to the dictionary.
    /// \param key Key
    /// \param value Value
    void addEntry(const PDFInplaceOrMemoryString& key, PDFObject&& value) { m_dictionary.emplace_back(key, std::move(value)); }

    /// Sets entry value. If entry with given key doesn't exist,
    /// then it is created.
    /// \param key Key
    /// \param value Value
    void setEntry(const PDFInplaceOrMemoryString& key, PDFObject&& value);

    /// Returns count of items in the dictionary
    size_t getCount() const { return m_dictionary.size(); }

    /// Returns capacity of items in the dictionary
    size_t getCapacity() const { return m_dictionary.capacity(); }

    /// Returns n-th key of the dictionary
    /// \param index Zero-based index of key in the dictionary
    const PDFInplaceOrMemoryString& getKey(size_t index) const { return m_dictionary[index].first; }

    /// Returns n-th value of the dictionary
    /// \param index Zero-based index of value in the dictionary
    const PDFObject& getValue(size_t index) const { return m_dictionary[index].second; }

    /// Removes null objects from dictionary
    void removeNullObjects();

    /// Optimizes the dictionary for memory consumption
    virtual void optimize() override;

private:
    /// Finds an item in the dictionary array, if the item is not in the dictionary,
    /// then end iterator is returned.
    /// \param key Key to be found
    std::vector<DictionaryEntry>::const_iterator find(const QByteArray& key) const;

    /// Finds an item in the dictionary array, if the item is not in the dictionary,
    /// then end iterator is returned.
    /// \param key Key to be found
    std::vector<DictionaryEntry>::iterator find(const QByteArray& key);

    /// Finds an item in the dictionary array, if the item is not in the dictionary,
    /// then end iterator is returned.
    /// \param key Key to be found
    std::vector<DictionaryEntry>::const_iterator find(const char* key) const;

    /// Finds an item in the dictionary array, if the item is not in the dictionary,
    /// then end iterator is returned.
    /// \param key Key to be found
    std::vector<DictionaryEntry>::iterator find(const char* key);

    /// Finds an item in the dictionary array, if the item is not in the dictionary,
    /// then end iterator is returned.
    /// \param key Key to be found
    std::vector<DictionaryEntry>::const_iterator find(const PDFInplaceOrMemoryString& key) const;

    /// Finds an item in the dictionary array, if the item is not in the dictionary,
    /// then end iterator is returned.
    /// \param key Key to be found
    std::vector<DictionaryEntry>::iterator find(const PDFInplaceOrMemoryString& key);

    std::vector<DictionaryEntry> m_dictionary;
};

/// Represents a stream object in the PDF file. Stream consists of dictionary
/// and stream content - byte array.
class PDF4QTLIBCORESHARED_EXPORT PDFStream : public PDFObjectContent
{
public:
    inline explicit PDFStream() = default;
    inline explicit PDFStream(PDFDictionary&& dictionary, QByteArray&& content) :
        m_dictionary(std::move(dictionary)),
        m_content(std::move(content))
    {

    }

    virtual ~PDFStream() override = default;

    virtual bool equals(const PDFObjectContent* other) const override;

    /// Returns dictionary for this content stream
    const PDFDictionary* getDictionary() const { return &m_dictionary; }

    /// Optimizes the stream for memory consumption
    virtual void optimize() override { m_dictionary.optimize(); m_content.shrink_to_fit(); }

    /// Returns content of the stream
    const QByteArray* getContent() const { return &m_content; }

private:
    PDFDictionary m_dictionary;
    QByteArray m_content;
};

class PDF4QTLIBCORESHARED_EXPORT PDFObjectManipulator
{
public:
    explicit PDFObjectManipulator() = delete;

    enum MergeFlag
    {
        NoFlag            = 0x0000,
        RemoveNullObjects = 0x0001, ///< Remove null object from dictionaries
        ConcatenateArrays = 0x0002, ///< Concatenate arrays instead of replace
    };
    Q_DECLARE_FLAGS(MergeFlags, MergeFlag)

    /// Merges two objects. If object type is different, then object from right is used.
    /// If both objects are dictionaries, then their content is merged, object \p right
    /// has precedence over object \p left. If both objects are arrays, and concatenating
    /// flag is turned on, then they are concatenated instead of replacing left array
    /// by right array. If remove null objects flag is turend on, then null objects
    /// are removed from dictionaries.
    /// \param left Left, 'slave' object
    /// \param right Right 'master' object, has priority over left
    /// \param flags Merge flags
    static PDFObject merge(PDFObject left, PDFObject right, MergeFlags flags);

    /// Remove null objects from all dictionaries
    /// \param object Object
    static PDFObject removeNullObjects(PDFObject object);

    /// Remove duplicit references from arrays
    /// \param object Object
    static PDFObject removeDuplicitReferencesInArrays(PDFObject object);
};

}   // namespace pdf

#endif // PDFOBJECT_H
