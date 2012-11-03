/***************************************************************************
      qgsdelimitedtextreader.h  -  Reader for delimited text file
                             -------------------
    begin                : 2004-02-27
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QStringList>

class QgsFeature;
class QgsField;
class QFile;
class QTextStream;


/**
\class QgsDelimitedTextReader
\brief Delimited text file reader extracts records as QStringList.
*
*
* The delimited text reader is used by the QgsDelimitedTextProvider to parse
* a QTextStream into records of QStringList.  It provides a number of variants
* for parsing each record.  The following options are supported:
* - Basic whitespace parsing.  Each line in the file is treated as a record.
*   Extracts all contiguous sequences of non-whitespace
*   characters.  Leading and trailing whitespace are ignored.  
* - Regular expression parsing.  Each line in the file is treated as a record. 
*   The string is split into fields based on a regular expression. 
* - Character delimited, based on three characters - a delimiter, a quote, and 
*   an escape character.  The escape treats the next character as a part of a field.
*   Fields may start and end with quote characters, in which case any non-escaped 
*   character within the field is treated literally, including end of line characters.
*   The escape character within a string causes the next character to be read literally
*   (this includes new line characters).  If the escape and quote characters are the 
*   same, then only quote characters will be escaped (ie to include a quote in a 
*   quoted field it is entered as two quotes.  All other characters in quoted fields
*   are treated literally, including newlines.
* - CSV format files - these are a special case of character delimited, in which the
*   delimiter is a comma, and the quote and escape characters are double quotes (")
*
* The delimiters can be encode in and decoded from a QUrl as query items.  The two
* items used are:
* - delimiterType, one of plain (delimiter is any of a set of characters), 
*   regexp, csv, whitespace, quoted
* - delimiter, interpreted according to the type.  For plain and quoted this is
*   a sequence of characters.  The backslash is treated as an escape, in which
*   \t is tab, \b is a blank, \\ is a backslash. For quoted this the sequence 
*   comprises the delimter, quote, and escape characters in order.  For regexp
*   the item specifies the reqular expression.  The field is ignored for csv and
*   whitespace types.
*/
class QgsDelimitedTextReader 
{

  public:

    enum Status
    {
        RecordOk,
        RecordEmpty,
        RecordInvalid,
        EOF
    };

    enum DelimiterType
    {
        DelimTypeWhitespace,
        DelimTypeCSV,
        DelimTypeRegexp,
        DelimTypePlain,
        DelimTypeQuoted
    };

    QgsDelimitedTextReader();

    virtual ~QgsDelimitedTextReader();

    /** Decode the reader settings from a QUrl 
     *  @param uri  The uri from which the delimiter and delimiterType items are read
     */
    bool decodeFromUrl( QUrl &uri );

    /** Encode the reader settings into a QUrl
     *  @param uri  The uri from which the delimiter and delimiterType items are read
     */
    bool encodeIntoUrl( QUrl &uri );

    /** Set the reader for parsing CSV files
     */
    void setTypeCSV();
    /** Set the reader for parsing whitespace delimited files
     */
    void setTypeWhitespace();
    /** Set the reader for parsing files delimited by an arbitrary sequence of
        a specified set of characters
        @param chars  A string containing the set of characters on which to
                      split the string
     */
    void setTypePlainChars( QString chars );
    /** Set the reader for parsing based on a reqular expression delimiter
        @param regexp A string defining the regular expression
    */
    void setTypeRegexp( QString regexp );
    /** Set the reader to use a quoted type delimiter.
     *  @param delim  The field delimiter character
     *  @param quote  The quote character, used to define quoted fields
     *  @param escape The escape character used to escape quote or delim 
     *                characters.
     */
    void setTypeQuoted( QChar delim, QChar quote, QChar escape );
    /** Set the reader to use a quoted type delimiter.
     *  @param delim  A string comprising the delimiter, quote, and escape
     *                characters
     */
    void setTypeQuoted( QChar delim, QChar quote, QChar escape );
    /* Specify the maximum number of fields that will be read into a record.
     *
     * Any fields after this a record will be silently ignored
     * @param maxFields  The maximum number of fields into which a record will be split
     */
    void setMaxFields( int maxFields );

    /** Reads the next record from the QTextStream and splits into string fields. 
     *  @param stream  The text stream to read the record from
     *  @param fields  The string list to populate with the fields
     *  @return status The result of trying to parse a record.  RecordOk
     *                 if read successfully, EOF if reached the end of the
     *                 file, RecordEmpty if no data on the next line, and
     *                 RecordInvalid if the record is ill-formatted.
     */

    Status readRecord( QTextStream &stream, QStringList &fields );

  private:

    DelimiterType mType;
    QString mDelimiterString;

    /* Parser functions */

    /** Parse reqular expression delimited fields */
    Status parseRegexp( QTextStream &stream, QStringList &fields );
    /** Parse quote delimited fields, where quote and escape are different */
    Status parseQuoted( QTextStream &stream, QStringList &fields );
    /** Parse quote delimited fields, where quote and escape are the same */
    Status parseQuoteEscaped( QTextStream &stream, QStringList &fields );

    Status (QgsDelimitedTextReader::*parser)( QTextStream &stream, QStringList &fields );
    /* Parameters used by parsers */
    bool mTrimString;
    QRegExp mDelimRegexp;
    QChar mDelimChar;
    QChar mQuoteChar;
    QChar mEscapeChar;
};
