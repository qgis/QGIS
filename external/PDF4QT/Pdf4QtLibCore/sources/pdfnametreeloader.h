// MIT License
//
// Copyright (c) 2018-2025 Jakub Melka and Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef PDFNAMETREELOADER_H
#define PDFNAMETREELOADER_H

#include "pdfdocument.h"

#include <map>
#include <functional>

namespace pdf
{

/// This class can load a number tree into the array
template<typename Type>
class PDFNameTreeLoader
{
public:
    explicit PDFNameTreeLoader() = delete;

    using MappedObjects = std::map<QByteArray, Type>;
    using LoadMethod = std::function<Type(const PDFObjectStorage*, const PDFObject&)>;

    /// Parses the name tree and loads its items into the map. Some errors are ignored,
    /// e.g. when kid is null. Objects are retrieved by \p loadMethod.
    /// \param storage Object storage
    /// \param root Root of the name tree
    /// \param loadMethod Parsing method, which retrieves parsed object
    static MappedObjects parse(const PDFObjectStorage* storage, const PDFObject& root, const LoadMethod& loadMethod)
    {
        MappedObjects result;
        parseImpl(result, storage, root, loadMethod);
        return result;
    }

private:
    static void parseImpl(MappedObjects& objects, const PDFObjectStorage* storage, const PDFObject& root, const LoadMethod& loadMethod)
    {
        if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(root))
        {
            // Jakub Melka: First, load the objects into the map
            const PDFObject& namedItems = storage->getObject(dictionary->get("Names"));
            if (namedItems.isArray())
            {
                const PDFArray* namedItemsArray = namedItems.getArray();
                const size_t count = namedItemsArray->getCount() / 2;
                for (size_t i = 0; i < count; ++i)
                {
                    const size_t numberIndex = 2 * i;
                    const size_t valueIndex = 2 * i + 1;

                    const PDFObject& name = storage->getObject(namedItemsArray->getItem(numberIndex));
                    if (!name.isString())
                    {
                        continue;
                    }

                    objects[name.getString()] = loadMethod(storage, namedItemsArray->getItem(valueIndex));
                }
            }

            // Then, follow the kids
            const PDFObject&  kids = storage->getObject(dictionary->get("Kids"));
            if (kids.isArray())
            {
                const PDFArray* kidsArray = kids.getArray();
                const size_t count = kidsArray->getCount();
                for (size_t i = 0; i < count; ++i)
                {
                    parseImpl(objects, storage, kidsArray->getItem(i), loadMethod);
                }
            }
        }
    }
};

}   // namespace pdf

#endif // PDFNAMETREELOADER_H
