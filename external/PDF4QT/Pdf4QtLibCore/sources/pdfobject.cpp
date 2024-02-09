//    Copyright (C) 2018-2023 Jakub Melka
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

#include "pdfobject.h"
#include "pdfvisitor.h"
#include "pdfdbgheap.h"

namespace pdf
{

QByteArray PDFObject::getString() const
{
    PDFStringRef stringRef = getStringObject();
    Q_ASSERT(stringRef.inplaceString || stringRef.memoryString);

    return stringRef.inplaceString ? stringRef.inplaceString->getString() : stringRef.memoryString->getString();
}

const PDFDictionary* PDFObject::getDictionary() const
{
    const PDFObjectContentPointer& objectContent = std::get<PDFObjectContentPointer>(m_data);

    Q_ASSERT(dynamic_cast<const PDFDictionary*>(objectContent.get()));
    return static_cast<const PDFDictionary*>(objectContent.get());
}

PDFStringRef PDFObject::getStringObject() const
{
    if (std::holds_alternative<PDFInplaceString>(m_data))
    {
        return { &std::get<PDFInplaceString>(m_data) , nullptr };
    }
    else
    {
        const PDFObjectContentPointer& objectContent = std::get<PDFObjectContentPointer>(m_data);

        Q_ASSERT(dynamic_cast<const PDFString*>(objectContent.get()));
        return { nullptr, static_cast<const PDFString*>(objectContent.get()) };
    }
}

const PDFStream* PDFObject::getStream() const
{
    const PDFObjectContentPointer& objectContent = std::get<PDFObjectContentPointer>(m_data);

    Q_ASSERT(dynamic_cast<const PDFStream*>(objectContent.get()));
    return static_cast<const PDFStream*>(objectContent.get());
}

const PDFArray* PDFObject::getArray() const
{
    const PDFObjectContentPointer& objectContent = std::get<PDFObjectContentPointer>(m_data);

    Q_ASSERT(dynamic_cast<const PDFArray*>(objectContent.get()));
    return static_cast<const PDFArray*>(objectContent.get());
}

bool PDFObject::operator==(const PDFObject& other) const
{
    if (m_type == other.m_type)
    {
        if (m_type == Type::String || m_type == Type::Name)
        {
            PDFStringRef leftString = getStringObject();
            PDFStringRef rightString = other.getStringObject();

            if (leftString.inplaceString && rightString.inplaceString)
            {
                return *leftString.inplaceString == *rightString.inplaceString;
            }
            else if (leftString.memoryString && rightString.memoryString)
            {
                return leftString.memoryString->equals(rightString.memoryString);
            }

            // We have inplace string in one object and memory string in the other.
            // So they are not equal, because memory strings have always greater
            // size, than inplace strings.
            return false;
        }

        Q_ASSERT(std::holds_alternative<PDFObjectContentPointer>(m_data) == std::holds_alternative<PDFObjectContentPointer>(other.m_data));

        // If we have content object defined, then use its equal operator,
        // otherwise use default compare operator. The only problem with
        // default compare operator can occur, when we have a double
        // with NaN value. Then operator == can return false, even if
        // values are "equal" (NaN == NaN returns false)
        if (std::holds_alternative<PDFObjectContentPointer>(m_data))
        {
            Q_ASSERT(std::get<PDFObjectContentPointer>(m_data));
            return std::get<PDFObjectContentPointer>(m_data)->equals(std::get<PDFObjectContentPointer>(other.m_data).get());
        }

        return m_data == other.m_data;
    }

    return false;
}

void PDFObject::accept(PDFAbstractVisitor* visitor) const
{
    switch (m_type)
    {
        case Type::Null:
            visitor->visitNull();
            break;

        case Type::Bool:
            visitor->visitBool(getBool());
            break;

        case Type::Int:
            visitor->visitInt(getInteger());
            break;

        case Type::Real:
            visitor->visitReal(getReal());
            break;

        case Type::String:
            visitor->visitString(getStringObject());
            break;

        case Type::Name:
            visitor->visitName(getStringObject());
            break;

        case Type::Array:
            visitor->visitArray(getArray());
            break;

        case Type::Dictionary:
            visitor->visitDictionary(getDictionary());
            break;

        case Type::Stream:
            visitor->visitStream(getStream());
            break;

        case Type::Reference:
            visitor->visitReference(getReference());
            break;

        default:
            Q_ASSERT(false);
    }
}

PDFObject PDFObject::createName(QByteArray name)
{
    if (name.size() > PDFInplaceString::MAX_STRING_SIZE)
    {
        return PDFObject(Type::Name, std::make_shared<PDFString>(qMove(name)));
    }
    else
    {
        return PDFObject(Type::Name, PDFInplaceString(qMove(name)));
    }
}

PDFObject PDFObject::createString(QByteArray name)
{
    if (name.size() > PDFInplaceString::MAX_STRING_SIZE)
    {
        return PDFObject(Type::String, std::make_shared<PDFString>(qMove(name)));
    }
    else
    {
        return PDFObject(Type::String, PDFInplaceString(qMove(name)));
    }
}

PDFObject PDFObject::createName(PDFStringRef name)
{
    if (name.memoryString)
    {
        return PDFObject(Type::Name, std::make_shared<PDFString>(name.getString()));
    }
    else
    {
        return PDFObject(Type::Name, *name.inplaceString);
    }
}

PDFObject PDFObject::createString(PDFStringRef name)
{
    if (name.memoryString)
    {
        return PDFObject(Type::String, std::make_shared<PDFString>(name.getString()));
    }
    else
    {
        return PDFObject(Type::String, *name.inplaceString);
    }
}

bool PDFString::equals(const PDFObjectContent* other) const
{
    Q_ASSERT(dynamic_cast<const PDFString*>(other));
    const PDFString* otherString = static_cast<const PDFString*>(other);
    return m_string == otherString->m_string;
}

void PDFString::setString(const QByteArray& string)
{
    m_string = string;
}

void PDFString::optimize()
{
    m_string.shrink_to_fit();
}

bool PDFArray::equals(const PDFObjectContent* other) const
{
    Q_ASSERT(dynamic_cast<const PDFArray*>(other));
    const PDFArray* otherArray = static_cast<const PDFArray*>(other);
    return m_objects == otherArray->m_objects;
}

void PDFArray::appendItem(PDFObject object)
{
    m_objects.push_back(std::move(object));
}

void PDFArray::optimize()
{
    m_objects.shrink_to_fit();
}

bool PDFDictionary::equals(const PDFObjectContent* other) const
{
    Q_ASSERT(dynamic_cast<const PDFDictionary*>(other));
    const PDFDictionary* otherStream = static_cast<const PDFDictionary*>(other);
    return m_dictionary == otherStream->m_dictionary;
}

const PDFObject& PDFDictionary::get(const QByteArray& key) const
{
    auto it = find(key);
    if (it != m_dictionary.cend())
    {
        return it->second;
    }
    else
    {
        static PDFObject dummy;
        return dummy;
    }
}

const PDFObject& PDFDictionary::get(const char* key) const
{
    auto it = find(key);
    if (it != m_dictionary.cend())
    {
        return it->second;
    }
    else
    {
        static PDFObject dummy;
        return dummy;
    }
}

const PDFObject& PDFDictionary::get(const PDFInplaceOrMemoryString& key) const
{
    auto it = find(key);
    if (it != m_dictionary.cend())
    {
        return it->second;
    }
    else
    {
        static PDFObject dummy;
        return dummy;
    }
}

void PDFDictionary::removeEntry(const char* key)
{
    auto it = find(key);
    if (it != m_dictionary.end())
    {
        m_dictionary.erase(it);
    }
}

void PDFDictionary::setEntry(const PDFInplaceOrMemoryString& key, PDFObject&& value)
{
    auto it = find(key);
    if (it != m_dictionary.end())
    {
        it->second = qMove(value);
    }
    else
    {
        addEntry(key, qMove(value));
    }
}

void PDFDictionary::removeNullObjects()
{
    m_dictionary.erase(std::remove_if(m_dictionary.begin(), m_dictionary.end(), [](const DictionaryEntry& entry) { return entry.second.isNull(); }), m_dictionary.end());
    m_dictionary.shrink_to_fit();
}

void PDFDictionary::optimize()
{
    m_dictionary.shrink_to_fit();
}

std::vector<PDFDictionary::DictionaryEntry>::const_iterator PDFDictionary::find(const QByteArray& key) const
{
    return std::find_if(m_dictionary.cbegin(), m_dictionary.cend(), [&key](const DictionaryEntry& entry) { return entry.first == key; });
}

std::vector<PDFDictionary::DictionaryEntry>::iterator PDFDictionary::find(const QByteArray& key)
{
    return std::find_if(m_dictionary.begin(), m_dictionary.end(), [&key](const DictionaryEntry& entry) { return entry.first == key; });
}

std::vector<PDFDictionary::DictionaryEntry>::const_iterator PDFDictionary::find(const char* key) const
{
    return std::find_if(m_dictionary.cbegin(), m_dictionary.cend(), [key](const DictionaryEntry& entry) { return entry.first == key; });
}

std::vector<PDFDictionary::DictionaryEntry>::const_iterator PDFDictionary::find(const PDFInplaceOrMemoryString& key) const
{
    return std::find_if(m_dictionary.cbegin(), m_dictionary.cend(), [&key](const DictionaryEntry& entry) { return entry.first == key; });
}

std::vector<PDFDictionary::DictionaryEntry>::iterator PDFDictionary::find(const PDFInplaceOrMemoryString& key)
{
    return std::find_if(m_dictionary.begin(), m_dictionary.end(), [&key](const DictionaryEntry& entry) { return entry.first == key; });
}

std::vector<PDFDictionary::DictionaryEntry>::iterator PDFDictionary::find(const char* key)
{
    return std::find_if(m_dictionary.begin(), m_dictionary.end(), [key](const DictionaryEntry& entry) { return entry.first == key; });
}

bool PDFStream::equals(const PDFObjectContent* other) const
{
    Q_ASSERT(dynamic_cast<const PDFStream*>(other));
    const PDFStream* otherStream = static_cast<const PDFStream*>(other);
    return m_dictionary.equals(&otherStream->m_dictionary) && m_content == otherStream->m_content;
}

PDFObject PDFObjectManipulator::merge(PDFObject left, PDFObject right, MergeFlags flags)
{
    const bool leftHasDictionary = left.isDictionary() || left.isStream();

    if (left.getType() != right.getType() && (leftHasDictionary != right.isDictionary()))
    {
        return right;
    }

    if (left.isStream())
    {
        Q_ASSERT(right.isDictionary() || right.isStream());
        const PDFStream* leftStream = left.getStream();
        const PDFStream* rightStream = right.isStream() ? right.getStream() : nullptr;

        PDFDictionary targetDictionary = *leftStream->getDictionary();
        const PDFDictionary& sourceDictionary = rightStream ? *rightStream->getDictionary() : *right.getDictionary();

        for (size_t i = 0, count = sourceDictionary.getCount(); i < count; ++i)
        {
            const auto& key = sourceDictionary.getKey(i);
            PDFObject value = merge(targetDictionary.get(key), sourceDictionary.getValue(i), flags);
            targetDictionary.setEntry(key, qMove(value));
        }

        if (flags.testFlag(RemoveNullObjects))
        {
            targetDictionary.removeNullObjects();
        }

        return PDFObject::createStream(std::make_shared<PDFStream>(qMove(targetDictionary), QByteArray(rightStream ? *rightStream->getContent() : *leftStream->getContent())));
    }
    if (left.isDictionary())
    {
        Q_ASSERT(right.isDictionary());

        PDFDictionary targetDictionary = *left.getDictionary();
        const PDFDictionary& sourceDictionary = *right.getDictionary();

        for (size_t i = 0, count = sourceDictionary.getCount(); i < count; ++i)
        {
            const auto& key = sourceDictionary.getKey(i);
            PDFObject value = merge(targetDictionary.get(key), sourceDictionary.getValue(i), flags);
            targetDictionary.setEntry(key, qMove(value));
        }

        if (flags.testFlag(RemoveNullObjects))
        {
            targetDictionary.removeNullObjects();
        }

        return PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(targetDictionary)));
    }
    else if (left.isArray() && flags.testFlag(ConcatenateArrays))
    {
        // Concatenate arrays
        const PDFArray* leftArray = left.getArray();
        const PDFArray* rightArray = right.getArray();

        std::vector<PDFObject> objects;
        objects.reserve(leftArray->getCount() + rightArray->getCount());
        for (size_t i = 0, count = leftArray->getCount(); i < count; ++i)
        {
            objects.emplace_back(leftArray->getItem(i));
        }
        for (size_t i = 0, count = rightArray->getCount(); i < count; ++i)
        {
            objects.emplace_back(rightArray->getItem(i));
        }
        return PDFObject::createArray(std::make_shared<PDFArray>(qMove(objects)));
    }

    return right;
}

PDFObject PDFObjectManipulator::removeNullObjects(PDFObject object)
{
    return merge(object, object, RemoveNullObjects);
}

PDFObject PDFObjectManipulator::removeDuplicitReferencesInArrays(PDFObject object)
{
    switch (object.getType())
    {
        case PDFObject::Type::Stream:
        {
            const PDFStream* stream = object.getStream();
            PDFDictionary dictionary = *stream->getDictionary();

            for (size_t i = 0, count = dictionary.getCount(); i < count; ++i)
            {
                dictionary.setEntry(dictionary.getKey(i), removeDuplicitReferencesInArrays(dictionary.getValue(i)));
            }

            return PDFObject::createStream(std::make_shared<PDFStream>(qMove(dictionary), QByteArray(*stream->getContent())));
        }

        case PDFObject::Type::Dictionary:
        {
            PDFDictionary dictionary = *object.getDictionary();

            for (size_t i = 0, count = dictionary.getCount(); i < count; ++i)
            {
                dictionary.setEntry(dictionary.getKey(i), removeDuplicitReferencesInArrays(dictionary.getValue(i)));
            }

            return PDFObject::createDictionary(std::make_shared<PDFDictionary>(qMove(dictionary)));
        }

        case PDFObject::Type::Array:
        {
            PDFArray array;
            std::set<PDFObjectReference> usedReferences;

            for (const PDFObject& arrayObject : *object.getArray())
            {
                if (arrayObject.isReference())
                {
                    PDFObjectReference reference = arrayObject.getReference();
                    if (!usedReferences.count(reference))
                    {
                        usedReferences.insert(reference);
                        array.appendItem(PDFObject::createReference(reference));
                    }
                }
                else
                {
                    array.appendItem(removeDuplicitReferencesInArrays(arrayObject));
                }
            }

            return PDFObject::createArray(std::make_shared<PDFArray>(qMove(array)));
        }

        default:
            return object;
    }
}

QByteArray PDFStringRef::getString() const
{
    if (inplaceString)
    {
        return inplaceString->getString();
    }
    if (memoryString)
    {
        return memoryString->getString();
    }
    return QByteArray();
}

PDFInplaceOrMemoryString::PDFInplaceOrMemoryString(const char* string)
{
    const int size = static_cast<int>(qMin(std::strlen(string), size_t(std::numeric_limits<int>::max())));
    if (size > PDFInplaceString::MAX_STRING_SIZE)
    {
        m_value = QByteArray(string, size);
    }
    else
    {
        m_value = PDFInplaceString(string, size);
    }
}

PDFInplaceOrMemoryString::PDFInplaceOrMemoryString(QByteArray string)
{
    const int size = string.size();
    if (size > PDFInplaceString::MAX_STRING_SIZE)
    {
        m_value = qMove(string);
    }
    else
    {
        m_value = PDFInplaceString(qMove(string));
    }
}

bool PDFInplaceOrMemoryString::equals(const char* value, size_t length) const
{
    if (std::holds_alternative<PDFInplaceString>(m_value))
    {
        const PDFInplaceString& string = std::get<PDFInplaceString>(m_value);
        return std::equal(string.string.data(), string.string.data() + string.size, value, value + length);
    }

    if (std::holds_alternative<QByteArray>(m_value))
    {
        const QByteArray& string = std::get<QByteArray>(m_value);
        return std::equal(string.constData(), string.constData() + string.size(), value, value + length);
    }

    return length == 0;
}

bool PDFInplaceOrMemoryString::isInplace() const
{
    return std::holds_alternative<PDFInplaceString>(m_value);
}

QByteArray PDFInplaceOrMemoryString::getString() const
{
    if (std::holds_alternative<PDFInplaceString>(m_value))
    {
        return std::get<PDFInplaceString>(m_value).getString();
    }

    if (std::holds_alternative<QByteArray>(m_value))
    {
        return std::get<QByteArray>(m_value);
    }

    return QByteArray();
}

}   // namespace pdf
