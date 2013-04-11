/***************************************************************************
  qgsdelimitedtextfile.cpp -  Data provider for delimted text
  -------------------
          begin                : 2012-01-20
          copyright            : (C) 201 by Chris Crook
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

#include "qgsdelimitedtextfile.h"
#include "qgslogger.h"

#include <QtGlobal>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QTextCodec>
#include <QStringList>
#include <QRegExp>
#include <QUrl>

QgsDelimitedTextFile::QgsDelimitedTextFile( QString url ) :
    mFileName(QString()),
    mEncoding("UTF-8"),
    mFile(0),
    mStream(0),
    mDefinitionValid(false),
    mUseHeader(true),
    mSkipLines(0),
    mMaxFields(0),
    mLineNumber(0),
    mRecordLineNumber(0)
{
    // The default type is CSV
    setTypeCSV();
    if( ! url.isNull()) setFromUrl( url );
}


QgsDelimitedTextFile::~QgsDelimitedTextFile()
{
    close();
}

void QgsDelimitedTextFile::close()
{
    if( mStream ) {
        delete mStream;
        mStream=0;
    }
    if( mFile ) {
        delete mFile;
        mFile=0;
    }
}

bool QgsDelimitedTextFile::open()
{
    if( ! mFile )
    {
        close();
        mFile = new QFile(mFileName);
        if( ! mFile->open(QIODevice::ReadOnly) )
        {
            QgsDebugMsg( "Data file " + mFileName + " could not be opened" );
            delete mFile;
            mFile=0;
            return false;
        }
        mStream = new QTextStream( mFile );
        if( mEncoding.isEmpty() && mEncoding != "System")
        {
            QTextCodec *codec =  QTextCodec::codecForName(mEncoding.toAscii());
            mStream->setCodec(codec);
        }
    }
    return true;
}

// Clear information based on current definition of file
void QgsDelimitedTextFile::resetDefinition()
{
    close();
    mColumnNames.clear();
}

// Extract the provider definition from the url
bool QgsDelimitedTextFile::setFromUrl( QString url )
{
    QUrl qurl = QUrl::fromEncoded( url.toAscii() );
    return setFromUrl( qurl );
}

// Extract the provider definition from the url
bool QgsDelimitedTextFile::setFromUrl( QUrl &url )
{
    // Close any existing definition
    resetDefinition();

    // Extract the file name
    setFileName( url.toLocalFile());

    // Extract the encoding
    if( url.hasQueryItem("encoding"))
    {
        mEncoding =url.queryItemValue("encoding");
    }

    // The default type is csv, to be consistent with the
    // previous implementation (except that quoting should be handled properly)

    QString type("csv");
    QString delimiter(",");
    QString quote="\"";
    QString escape="\"";
    mUseHeader=true;
    mSkipLines=0;

    // Prefer simple "type" for delimiter type, but include delimiterType
    // as optional name  for backwards compatibility
    if( url.hasQueryItem("type") )
    {
        type = url.queryItemValue("type");
        // Support for previous version of Qgs - plain chars had
        // quote characters ' or "
        if( type == "plain" )
        {
            quote="'\"";
            escape="";
        }
    }
    else if( url.hasQueryItem("delimiterType") )
    {
        type = url.queryItemValue("delimiterType");
    }
    if( url.hasQueryItem("delimiter") )
    {
        delimiter = url.queryItemValue("delimiter");
    }
    if( url.hasQueryItem("quote") )
    {
        quote = url.queryItemValue("quote");
    }
    if( url.hasQueryItem("escape") )
    {
        escape = url.queryItemValue("escape");
    }
    if ( url.hasQueryItem( "skipLines" ) )
    {
        mSkipLines = url.queryItemValue( "skipLines" ).toInt();
    }
    if ( url.hasQueryItem( "useHeader" ) )
    {
        mUseHeader = url.queryItemValue( "useHeader" ).toUpper().startsWith('Y');
    }

    QgsDebugMsg( "Delimited text file is: " + mFileName );
    QgsDebugMsg( "Encoding is: " + mEncoding);
    QgsDebugMsg( "Delimited file type is: " + type );
    QgsDebugMsg( "Delimiter is: [" + delimiter + "]" );
    QgsDebugMsg( "Quote character is: [" + quote +"]");
    QgsDebugMsg( "Escape character is: [" + escape + "]");
    QgsDebugMsg( "Skip lines: " + QString::number(mSkipLines) );
    QgsDebugMsg( "Skip lines: " + QString(mUseHeader ? "Yes" : "No") );

    // Support for previous version of plain characters
    if( type == "csv" || type == "plain" )
    {
        setTypeCSV(delimiter,quote,escape);
    }
    else if( type == "whitespace" )
    {
        setTypeWhitespace();
    }
    else if( type == "regexp" )
    {
        setTypeRegexp( delimiter );
    }
    else
    {
        return false;
    }
    return mDefinitionValid;
}

QUrl QgsDelimitedTextFile::url()
{
    QUrl url = QUrl::fromLocalFile( mFileName );
    if( mEncoding != "UTF-8" )
    {
        url.addQueryItem("encoding",mEncoding);
    }
    url.addQueryItem("type",type());
    if( mType == DelimTypeRegexp )
    {
        url.addQueryItem("delimiter",delimiterDefinitionString());
    }
    if( mType == DelimTypeCSV )
    {
        if( mDelimChars != "," ) url.addQueryItem("delimiter",delimiterDefinitionString());
        if( mQuoteChar != "\"" ) url.addQueryItem("quote",mQuoteChar);
        if( mEscapeChar != "\"" ) url.addQueryItem("escape",mEscapeChar);
    }
    if( mSkipLines > 0 )
    {
        url.addQueryItem("skipLines",QString::number(mSkipLines));
    }
    if( ! mUseHeader )
    {
        url.addQueryItem("useHeader","No");
    }
    return url;
}

void QgsDelimitedTextFile::setFileName( QString filename )
{
    resetDefinition();
    mFileName = filename;
}

void QgsDelimitedTextFile::setEncoding( QString encoding )
{
    resetDefinition();
    mEncoding = encoding;
}

QString QgsDelimitedTextFile::type()
{
    if( mType == DelimTypeWhitespace ) return QString("whitespace");
    if( mType == DelimTypeCSV ) return QString("csv");
    if( mType == DelimTypeRegexp ) return QString("regexp");
    return QString("csv");
}

void QgsDelimitedTextFile::setTypeWhitespace()
{
    setTypeRegexp("\\s+");
    mType=DelimTypeWhitespace;
    mDelimDefinition = "";
}

void QgsDelimitedTextFile::setTypeRegexp( QString regexp )
{
    resetDefinition();
    mType=DelimTypeRegexp;
    mDelimRegexp.setPattern(regexp);
    mDelimDefinition=regexp;
    mParser=&QgsDelimitedTextFile::parseRegexp;
    mDefinitionValid = mDelimRegexp.isValid();
    if( ! mDefinitionValid )
    {
        QgsDebugMsg("Invalid regular expression in delimited text file delimiter: "+regexp);
    }
}

void QgsDelimitedTextFile::setTypeCSV( QString delim, QString quote, QString escape )
{
    resetDefinition();
    mType=DelimTypeRegexp;
    mType=DelimTypeCSV;
    mDelimDefinition=delim;
    mDelimChars=delim;
    mQuoteChar=quote;
    mEscapeChar= escape;
    mDelimChars.replace("\\t","\t");
    mParser=&QgsDelimitedTextFile::parseQuoted;
    mDefinitionValid = delim.size() > 0;
    if( ! mDefinitionValid )
    {
        QgsDebugMsg("Invalid empty delimiter defined for text file delimiter");
    }
}

void QgsDelimitedTextFile::setSkipLines( int skiplines )
{
    resetDefinition();
    mSkipLines = skiplines;
}

void QgsDelimitedTextFile::setUseHeader( bool useheader )
{
    resetDefinition();
    mUseHeader = useheader;
}

QStringList &QgsDelimitedTextFile::columnNames()
{
    // If not yet opened then reset file to read column headers
    //
    if( mUseHeader && ! mFile ) reset();
    return mColumnNames;
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::nextRecord( QStringList &record )
{
    return (this->*mParser)(record);
}


QgsDelimitedTextFile::Status  QgsDelimitedTextFile::reset()
{
    // Make sure the file is valid open
    if( ! isValid() || ! open() ) return InvalidDefinition;

    // Reset the file pointer
    mStream->seek(0);
    mLineNumber = 0;
    mRecordLineNumber = 0;

    // Skip header lines
    for( int i = mSkipLines; i-- > 0; )
    {
        if( mStream->readLine().isNull() ) return RecordEOF;
        mLineNumber++;
    }
    // Read the column names
    if( mUseHeader )
    {
        return (this->*mParser)(mColumnNames);
    }
    return RecordOk;
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::nextLine( QString &buffer, bool skipBlank )
{
    if( ! mStream )
    {
        Status status = reset();
        if( status != RecordOk ) return status;
    }

    while( ! mStream->atEnd() )
    {
        buffer = mStream->readLine();
        if( buffer.isNull()) break;
        mLineNumber++;
        if( skipBlank && buffer.isEmpty() ) continue;
        return RecordOk;
    }

    // Null string if at end of stream
    return RecordEOF;
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::parseRegexp( QStringList &fields )
{
    QString buffer;
    Status status = nextLine(buffer,true);
    if( status != RecordOk ) return status;
    mRecordLineNumber = mLineNumber;

    if( mType == DelimTypeWhitespace ) buffer=buffer.trimmed();
    fields = buffer.split(mDelimRegexp);
    if( mMaxFields > 0 && fields.size() > mMaxFields )
    {
        fields = fields.mid(0,mMaxFields);
    }
    return RecordOk;
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::parseQuoted( QStringList &fields )
{
    fields.clear();

    // Find the first non-blank line to read
    QString buffer;
    Status status = nextLine(buffer,true);
    if( status != RecordOk ) return status;
    mRecordLineNumber = mLineNumber;

    QString field;        // String in which to accumulate next field
    bool escaped = false; // Next char is escaped
    bool quoted = false;  // In quotes
    QChar quoteChar = 0;  // Actual quote character used to open quotes
    bool started = false; // Non-blank chars in field or quotes started
    bool ended = false;   // Quoted field ended
    int cp = 0;          // Pointer to the next character in the buffer
    int cpmax = buffer.size(); // End of string

    while( true )
    {
        QChar c = buffer[cp];
        cp++;

        // If end of line then if escaped or buffered then try to get more...
        if( cp > cpmax )
        {
            if( quoted || escaped )
            {
                status = nextLine(buffer,false);
                if( status != RecordOk ) return status;
                field.append('\n');
                cp = 0;
                cpmax = buffer.size();
                escaped = false;
                continue;
            }
            break;
        }

        // If escaped, then just append the character
        if( escaped )
        {
            field.append(c);
            escaped = false;
            continue;
        }

        // Determine if this is a special character - test each class in turn
        // Note that delimiters are not valid as quotes or escape character
        //
        // Somewhat convoluted logic around quote and escape chars is
        // to enforce logic for escape characters that are also quote characters.
        // These escapes can only escape themselves and only in quotes using them
        // as delimiters!

        bool isQuote = false;
        bool isEscape = false;
        bool isDelim = mDelimChars.contains(c);
        if( ! isDelim )
        {
            bool isQuoteChar = mQuoteChar.contains(c);
            isQuote = quoted ? c==quoteChar : isQuoteChar;
            isEscape = mEscapeChar.contains(c);
            if( isQuoteChar && isEscape ) isEscape = isQuote;
        }

        // Start or end of quote ...
        if( isQuote )
        {
            // quote char in quoted field
            if( quoted )
            {
                // if is also escape and next character is quote, then
                // escape the quote..
                if( isEscape && buffer[cp]==quoteChar )
                {
                    field.append(quoteChar);
                    cp++;
                }
                // Otherwise end of quoted field
                else
                {
                    quoted = false;
                    ended =  true;
                }
            }
            // quote char at start of field .. start of quoted fields
            else if( ! started )
            {
                quoteChar = c;
                quoted = true;
                started = true;
            }
            // Cannot have a quote embedded in a field
            else
            {
                fields.clear();
                return RecordInvalid;
            }
        }
        // If escape char, then next char is escaped...
        else if( isEscape )
        {
            escaped = true;
        }
        // If within quotes, then append to the string
        else if( quoted )
        {
            field.append(c);
        }
        // If it is a delimiter, then end of field...
        else if( isDelim )
        {
            if( mMaxFields <= 0 || fields.size() < mMaxFields )
            {
                // If wasn't quoted, then trim..
                if( ! ended ) field = field.trimmed();
                fields.append(field);
            }
            // Clear the field
            field.clear();
            started = false;
            ended = false;
        }
        // Whitespace is permitted before the start of a field, or
        // after the end..
        else if( c.isSpace() )
        {
            if( started && ! ended ) field.append(c);
        }
        // Other chars permitted if not after quoted field
        else
        {
            if( ended )
            {
                fields.clear();
                return RecordInvalid;
            }
            field.append(c);
            started = true;
        }
    }
    // If reached the end of the record, then add the last field...
    if( started && (mMaxFields <=0 || fields.size() < mMaxFields) )
    {
        if( ! ended ) field = field.trimmed();
        fields.append(field);
    }
    return RecordOk;
}

bool QgsDelimitedTextFile::isValid()
{

    return mDefinitionValid && QFile::exists(mFileName);
}

