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
