//    Copyright (C) 2018-2022 Jakub Melka
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


#ifndef PDFCONSTANTS_H
#define PDFCONSTANTS_H

#include <cstddef>

namespace pdf
{

#ifndef PDF4QT_PROJECT_VERSION
#define PDF4QT_PROJECT_VERSION "UNKNOWN"
#endif

// Name of the library, together with version
static constexpr const char* PDF_LIBRARY_NAME = "PDF4QT " PDF4QT_PROJECT_VERSION;
static constexpr const char* PDF_LIBRARY_VERSION = PDF4QT_PROJECT_VERSION;

// Structure file constants
static constexpr const char* PDF_END_OF_FILE_MARK = "%%EOF";
static constexpr const char* PDF_START_OF_XREF_MARK = "startxref";

static constexpr const char* PDF_FILE_HEADER_V1 = "%PDF-?.?";
static constexpr const char* PDF_FILE_HEADER_V2 = "%!PS-Adobe-?.? PDF-?.?";
static constexpr const char* PDF_FILE_HEADER_REGEXP = "%PDF-([[:digit:]]\\.[[:digit:]])|%!PS-Adobe-[[:digit:]]\\.[[:digit:]] PDF-([[:digit:]]\\.[[:digit:]])";

static constexpr const int PDF_HEADER_SCAN_LIMIT = 1024;
static constexpr const int PDF_FOOTER_SCAN_LIMIT = 1024;

// Stream dictionary constants - entries common to all stream dictionaries
static constexpr const char* PDF_STREAM_DICT_LENGTH = "Length";
static constexpr const char* PDF_STREAM_DICT_FILTER = "Filter";
static constexpr const char* PDF_STREAM_DICT_DECODE_PARMS = "DecodeParms";
static constexpr const char* PDF_STREAM_DICT_FILE_SPECIFICATION = "F";
static constexpr const char* PDF_STREAM_DICT_FILE_FILTER = "FFilter";
static constexpr const char* PDF_STREAM_DICT_FDECODE_PARMS = "FDecodeParms";
static constexpr const char* PDF_STREAM_DICT_DECODED_LENGTH = "DL";

// xref table constants
static constexpr const char* PDF_XREF_HEADER = "xref";
static constexpr const char* PDF_XREF_TRAILER = "trailer";
static constexpr const char* PDF_XREF_TRAILER_PREVIOUS = "Prev";
static constexpr const char* PDF_XREF_TRAILER_XREFSTM = "XRefStm";
static constexpr const char* PDF_XREF_FREE = "f";
static constexpr const char* PDF_XREF_OCCUPIED = "n";

// objects

static constexpr const char* PDF_OBJECT_START_MARK = "obj";
static constexpr const char* PDF_OBJECT_END_MARK = "endobj";

// maximum generation limit
static constexpr const int PDF_MAX_OBJECT_GENERATION = 65535;

// Colors
static constexpr const int PDF_MAX_COLOR_COMPONENTS = 32;

// Cache limits
static constexpr size_t DEFAULT_FONT_CACHE_LIMIT = 32;
static constexpr size_t DEFAULT_REALIZED_FONT_CACHE_LIMIT = 128;

}   // namespace pdf

#endif // PDFCONSTANTS_H
