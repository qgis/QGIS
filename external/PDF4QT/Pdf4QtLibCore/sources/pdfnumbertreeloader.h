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

#ifndef PDFNUMBERTREELOADER_H
#define PDFNUMBERTREELOADER_H

#include "pdfdocument.h"

#include <vector>

namespace pdf
{

/// This class can load a number tree into the array
template<typename Type>
class PDFNumberTreeLoader
{
public:
    explicit PDFNumberTreeLoader() = delete;

    using Objects = std::vector<Type>;

    /// Parses the number tree and loads its items into the array. Some errors are ignored,
    /// e.g. when kid is null. Type must contain methods to load object array.
    static Objects parse(const PDFObjectStorage* storage, const PDFObject& root)
    {
        Objects result;

        // First, try to load items from the tree into the array
        parseImpl(result, storage, root);

        // Array may not be sorted. Sort it using comparison operator for Type.
        std::stable_sort(result.begin(), result.end());

        return result;
    }

private:
    static void parseImpl(Objects& objects, const PDFObjectStorage* storage, const PDFObject& root)
    {
        if (const PDFDictionary* dictionary = storage->getDictionaryFromObject(root))
        {
            // First, load the objects into the array
            const PDFObject& numberedItems = storage->getObject(dictionary->get("Nums"));
            if (numberedItems.isArray())
            {
                const PDFArray* numberedItemsArray = numberedItems.getArray();
                const size_t count = numberedItemsArray->getCount() / 2;
                objects.reserve(objects.size() + count);
                for (size_t i = 0; i < count; ++i)
                {
                    const size_t numberIndex = 2 * i;
                    const size_t valueIndex = 2 * i + 1;

                    const PDFObject& number = storage->getObject(numberedItemsArray->getItem(numberIndex));
                    if (!number.isInt())
                    {
                        continue;
                    }

                    objects.emplace_back(Type::parse(number.getInteger(), storage, numberedItemsArray->getItem(valueIndex)));
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
                    parseImpl(objects, storage, kidsArray->getItem(i));
                }
            }
        }
    }
};

}   // namespace pdf

#endif // PDFNUMBERTREELOADER_H
