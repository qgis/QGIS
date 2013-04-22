/***************************************************************************
      qgsdelimitedtextparser.h  -  File for delimited text file
                             -------------------
    begin                : 2004-02-27
    copyright            : (C) 2013 by Chris Crook
    email                : ccrook at linz.govt.nz
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
#include <QRegExp>
#include <QUrl>

class QgsFeature;
class QgsField;
class QFile;
class QTextStream;


/**
\class QgsDelimitedTextFile
\brief Delimited text file parser extracts records from a QTextStream as a QStringList.
*
*
* The delimited text parser is used by the QgsDelimitedTextProvider to parse
* a QTextStream into records of QStringList.  It provides a number of variants
* for parsing each record.  The following options are supported:
* - Basic whitespace parsing.  Each line in the file is treated as a record.
*   Extracts all contiguous sequences of non-whitespace
*   characters.  Leading and trailing whitespace are ignored.
* - Regular expression parsing.  Each line in the file is treated as a record.
*   The string is split into fields based on a regular expression.
* - Character delimited, based on a delimiter character set, a quote character, and
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
* The delimiters can be encode in and decoded from a QUrl as query items.  The
* items used are:
* - delimiterType, one of plain (delimiter is any of a set of characters),
*   regexp, csv, whitespace
* - delimiter, interpreted according to the type.  For plain characters this is
*   a sequence of characters.  The string \t in the sequence is replaced by a tab.
*   For regexp type delimiters this specifies the reqular expression.
*   The field is ignored for csv and whitespace
* - quoteChar, optional, a single character used for quoting plain fields
* - escapeChar, optional, a single characer used for escaping (may be the same as quoteChar)
*/

// Note: this has been implemented as a single class rather than a set of classes based
// on an abstract base class in order to facilitate changing the type of the parser easily
// eg in the provider dialog

class QgsDelimitedTextFile
{

  public:

    enum Status
    {
      RecordOk,
      InvalidDefinition,
      RecordEmpty,
      RecordInvalid,
      RecordEOF
    };

    enum DelimiterType
    {
      DelimTypeWhitespace,
      DelimTypeCSV,
      DelimTypeRegexp,
    };

    QgsDelimitedTextFile( QString url = QString() );

    virtual ~QgsDelimitedTextFile();

    /** Set the filename
     * @param filename  the name of the file
     */
    void setFileName( QString filename );
    /** Return the filename
     * @return filename  the name of the file
     */
    QString fileName()
    {
      return mFileName;
    }

    /** Set the file encoding (defuault is UTF-8)
     *  @param encoding the encoding to use for the fileName()
     */
    void setEncoding( QString encoding );
    /** Return the file encoding
     *  @return encoding The file encoding
     */
    QString encoding() { return mEncoding; }

    /** Decode the parser settings from a url as a string
     *  @param url  The url from which the delimiter and delimiterType items are read
     */
    bool setFromUrl( QString url );
    /** Decode the parser settings from a url
     *  @param url  The url from which the delimiter and delimiterType items are read
     */
    bool setFromUrl( QUrl &url );

    /** Encode the parser settings into a QUrl
     *  @return url  The url into which the delimiter and delimiterType items are set
     */
    QUrl url();

    /** Set the parser for parsing CSV files
     */
    void setTypeWhitespace();

    /** Set the parser for parsing based on a reqular expression delimiter
        @param regexp A string defining the regular expression
    */
    void setTypeRegexp( QString regexp );
    /** Set the parser to use a character type delimiter.
     *  @param delim  The field delimiter character set
     *  @param quote  The quote character, used to define quoted fields
     *  @param escape The escape character used to escape quote or delim
     *                characters.
     */
    void setTypeCSV( QString delim = QString( "," ), QString quote = QString( "\"" ), QString escape = QString( "\"" ) );

    /* Set the number of header lines to skip
     * @param skiplines The maximum lines to skip
     */
    void setSkipLines( int skiplines );
    /* Return the number of header lines to skip
     * @return skiplines The maximum lines to skip
     */
    int skipLines()
    {
      return mSkipLines;
    }

    /* Set reading column names from the first record
     * @param useheaders Column names will be read if true
     */
    void setUseHeader( bool useheader = true );
    /* Return the option for reading column names from the first record
     * @return useheaders Column names will be read if true
     */
    bool useHeader()
    {
      return mUseHeader;
    }

    /* Set the option for dicarding empty fields
     * @param useheaders Empty fields will be discarded if true
     */
    void setDiscardEmptyFields( bool discardEmptyFields = true );
    /* Return the option for discarding empty fields
     * @return useheaders Empty fields will be discarded if true
     */
    bool discardEmptyFields()
    {
      return mDiscardEmptyFields;
    }

    /* Set the option for trimming whitespace from fields
     * @param trimFields Fields will be trimmed if true
     */
    void setTrimFields( bool trimFields = true );
    /* Return the option for trimming empty fields
     * @return useheaders Empty fields will be trimmed if true
     */
    bool trimFields()
    {
      return mTrimFields;
    }

    /** Return the column names read from the header, or default names
     *  Col## if none defined.  Will open and read the head of the file
     *  if required, then reset..
     */
    QStringList &columnNames();

    /** Reads the next record from the stream splits into string fields.
     *  @param fields  The string list to populate with the fields
     *  @return status The result of trying to parse a record.  RecordOk
     *                 if read successfully, RecordEOF if reached the end of the
     *                 file, RecordEmpty if no data on the next line, and
     *                 RecordInvalid if the record is ill-formatted.
     */
    Status nextRecord( QStringList &fields );

    /** Return the line number of the start of the last record read
     *  @return linenumber  The line number of the start of the record
     */
    int recordLineNumber()
    {
      return mRecordLineNumber;
    }

    /** Reset the file to reread from the beginning
     */
    Status reset();

    /** Return a string defining the type of the delimiter as a string
     *  @return type The delimiter type as a string
     */
    QString type();

    /** Check that provider is valid (filename and definition valid)
     *
     * @return valid True if the provider is valid
     */
    bool isValid();

    /** Encode characters - used to convert delimiter/quote/escape characters to
     *  encoded form (eg replace tab with \t)
     *  @param string  The unencoded string
     *  @return encstring  The encoded string
     */
    static QString encodeChars( QString string );

    /** Encode characters - used to encoded character strings to
     *  decoded form (eg replace \t with tab)
     *  @param string  The unencoded string
     *  @return decstring  The decoded string
     */
    static QString decodeChars( QString string );




  private:

    /** Open the file
     *
     * @return valid  True if the file is successfully opened
     */
    bool open();

    /** Close the text file
     */
    void close();

    /** Reset the status if the definition is changing (eg clear
     *  existing column names, etc...
     */
    void resetDefinition();

    /** Parse reqular expression delimited fields */
    Status parseRegexp( QStringList &fields );
    /** Parse quote delimited fields, where quote and escape are different */
    Status parseQuoted( QStringList &fields );

    /** Return the next line from the data file.  If skipBlank is true then
     * blank lines will be skipped - this is for compatibility with previous
     * delimited text parser implementation.
     */
    Status nextLine( QString &buffer, bool skipBlank = false );

    // Pointer to the currently selected parser
    Status( QgsDelimitedTextFile::*mParser )( QStringList &fields );

    QString mFileName;
    QString mEncoding;
    QFile *mFile;
    QTextStream *mStream;

    // Parameters common to parsers
    bool mDefinitionValid;
    DelimiterType mType;
    bool mUseHeader;
    bool mDiscardEmptyFields;
    bool mTrimFields;
    int mSkipLines;
    int mMaxFields;

    // Parameters used by parsers
    QRegExp mDelimRegexp;
    QString mDelimChars;
    QString mQuoteChar;
    QString mEscapeChar;

    // Information extracted from file
    QStringList mColumnNames;
    int mLineNumber;
    int mRecordLineNumber;
};
