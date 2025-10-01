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

#ifndef PDFXREFTABLE_H
#define PDFXREFTABLE_H

#include "pdfglobal.h"
#include "pdfobject.h"

#include <vector>

namespace pdf
{
class PDFParsingContext;

/// Represents table of references in the PDF file. It contains
/// scanned table in the PDF file, together with information, if entry
/// is occupied, or it is free.
class PDFXRefTable
{
    Q_DECLARE_TR_FUNCTIONS(pdf::PDFXRefTable)

public:
    inline explicit PDFXRefTable() = default;

    // Enforce default copy constructor and default move constructor
    inline PDFXRefTable(const PDFXRefTable&) = default;
    inline PDFXRefTable(PDFXRefTable&&) = default;

    // Enforce default copy assignment operator and move assignment operator
    inline PDFXRefTable& operator=(const PDFXRefTable&) = default;
    inline PDFXRefTable& operator=(PDFXRefTable&&) = default;

    enum class EntryType
    {
        Free,           ///< Entry represents a free item (no object)
        Occupied,       ///< Entry represents a occupied item (object)
        InObjectStream  ///< Entry in object stream
    };

    struct Entry
    {
        PDFObjectReference reference;
        PDFObjectReference objectStream;
        PDFInteger offset = -1;
        PDFInteger indexInObjectStream = -1;
        EntryType type = EntryType::Free;
    };

    /// Tries to read reference table from the byte array. If error occurs, then exception
    /// is raised. This fuction also checks redundant entries.
    /// \param context Current parsing context
    /// \param byteArray Input byte array (containing the PDF file)
    /// \param startTableOffset Offset of first reference table
    void readXRefTable(PDFParsingContext* context, const QByteArray& byteArray, PDFInteger startTableOffset);

    /// Filters only occupied entries and returns them
    std::vector<Entry> getOccupiedEntries() const;

    /// Filters only object stream entries and returns them
    std::vector<Entry> getObjectStreamEntries() const;

    /// Returns size of the reference table
    std::size_t getSize() const { return m_entries.size(); }

    /// Gets the entry for given reference. If entry for given reference is not found,
    /// then free entry is returned.
    const Entry& getEntry(PDFObjectReference reference) const;

    /// Returns the trailer dictionary
    const PDFObject& getTrailerDictionary() const { return m_trailerDictionary; }

private:
    /// Reference table entries
    std::vector<Entry> m_entries;

    /// Trailer dictionary
    PDFObject m_trailerDictionary;
};

}   // namespace pdf

#endif // PDFXREFTABLE_H
