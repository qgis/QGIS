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

#ifndef PDFENCODING_H
#define PDFENCODING_H

#include "pdfglobal.h"

#include <QString>
#include <QDateTime>

#include <array>

namespace pdf
{

namespace encoding
{
using EncodingTable = std::array<QChar, 256>;
}

/// This class can convert byte stream to the QString in unicode encoding.
/// PDF has several encodings, see PDF Reference 1.7, Appendix D.
class PDF4QTLIBCORESHARED_EXPORT PDFEncoding
{
public:
    explicit PDFEncoding() = delete;

    enum class Encoding
    {
        Standard,       ///< Appendix D, Section D.1, StandardEncoding
        MacRoman,       ///< Appendix D, Section D.1, MacRomanEncoding
        WinAnsi,        ///< Appendix D, Section D.1, WinAnsiEncoding
        PDFDoc,         ///< Appendix D, Section D.1/D.2, PDFDocEncoding
        MacExpert,      ///< Appendix D, Section D.3, MacExpertEncoding
        Symbol,         ///< Appendix D, Section D.4, Symbol Set and Encoding
        ZapfDingbats,   ///< Appendix D, Section D.5, Zapf Dingbats Encoding

        // Following encodings are used for internal use only and are not a part of PDF reference
        MacOsRoman,     ///< Encoding for Mac OS, differs from MacRoman for 15 characters
        Custom,
        Invalid
    };

    /// Converts byte array to the unicode string using specified encoding
    /// \param stream Stream (byte array string) to be processed
    /// \param encoding Encoding used to convert to unicode string
    /// \returns Converted unicode string
    static QString convert(const QByteArray& stream, Encoding encoding);

    /// Converts unicode string to the byte array using the specified encoding.
    /// It performs reverse functionality than function \p convert. If the character
    /// in the encoding is not found, then it is converted to character code 0.
    /// \param string String to be converted
    /// \param encoding Encoding used in the conversion
    /// \sa convert
    static QByteArray convertToEncoding(const QString& string, Encoding encoding);

    /// Verifies, if string with given unicode characters can be converted using
    /// the specified encoding (so, all unicode characters present in the string
    /// are also present in given encoding).
    /// \param string String to be tested
    /// \param encoding Encoding used in verification of conversion
    /// \param[out] invalidCharacters Storage, where not convertible characters are inserted
    static bool canConvertToEncoding(const QString& string, Encoding encoding, QString* invalidCharacters);

    /// Checks, if stream can be converted to string using encoding (i.e. all
    /// characters are defined). If all characters are valid, then true is
    /// returned. This is only guess.
    /// \param stream Stream
    /// \param encoding Target encoding
    static bool canConvertFromEncoding(const QByteArray& stream, Encoding encoding);

    /// Convert text string to the unicode string, using either PDFDocEncoding,
    /// or UTF-16BE encoding. Please see PDF Reference 1.7, Chapter 3.8.1. If
    /// UTF-16BE encoding is used, then leading bytes should be 0xFE and 0xFF
    /// \param Stream
    /// \returns Converted unicode string
    static QString convertTextString(const QByteArray& stream);

    /// Converts byte array from UTF-16BE encoding to QString with same encoding.
    /// \param Stream
    /// \returns Converted unicode string
    static QString convertFromUnicode(const QByteArray& stream);

    /// Convert stream to date time according to PDF Reference 1.7, Chapter 3.8.1.
    /// If date cannot be converted (string is invalid), then invalid QDateTime
    /// is returned.
    /// \param stream Stream, from which date/time is read
    static QDateTime convertToDateTime(const QByteArray& stream);

    /// Convert date/time to string according to PDF Reference 1.7, Chapter 3.8.1.
    /// If date is invalid, empty byte array is returned.
    /// \param dateTime Date and time to be converted
    static QByteArray convertDateTimeToString(QDateTime dateTime);

    /// Returns conversion table for particular encoding
    /// \param encoding Encoding
    static const encoding::EncodingTable* getTableForEncoding(Encoding encoding);

    /// Tries to convert stream to unicode string. Stream can be binary.
    /// If this is the case, then hexadecimal representation of stream is returned.
    /// Function checks if stream can be converted to unicode by heuristic
    /// way, it is not always reliable.
    /// \param stream Stream
    /// \param[out] isBinary If specified, it is set to true if conversion failed
    /// \returns Unicode string or string converted to hexadecimal representation
    static QString convertSmartFromByteStringToUnicode(const QByteArray& stream, bool* isBinary);

    /// Tries to convert stream to representable string. If it cannot be done,
    /// percentage encoding is used.
    /// \param stream Stream
    /// \returns Unicode string or string converted to percentage representation
    static QString convertSmartFromByteStringToRepresentableQString(const QByteArray& stream);

    /// Returns all characters of the given encoding
    /// \param encoding Encoding
    /// \returns All characters reprezentable by encoding.
    static QString getEncodingCharacters(Encoding encoding);

    /// Returns all printable characters
    static QByteArray getPrintableCharacters();

private:
    /// Returns true, if byte array has UTF-16BE/LE unicode marking bytes at the
    /// stream start. If they are present, then byte stream is probably encoded
    /// as unicode.
    /// \param stream Stream to be tested
    static bool hasUnicodeLeadMarkings(const QByteArray& stream);

    /// Returns true, if byte array has UTF-8 unicode marking bytes at the stream
    /// start. If they are present, then byte stream is probably encoded
    /// as UTF-8 string.
    /// \note UTF-8 strings were added in PDF 2.0 specification
    /// \param stream Stream to be tested
    static bool hasUTF8LeadMarkings(const QByteArray& stream);
};

}   // namespace pdf

#endif // PDFENCODING_H
