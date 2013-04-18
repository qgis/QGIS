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

static QString DefaultFieldName( "field_%1" );
static QRegExp InvalidFieldRegexp( "^\\d*(\\.\\d*)?$" );
// field_ is optional in following regexp to simplify QgsDelimitedTextFile::fieldNumber()
static QRegExp DefaultFieldRegexp( "^(?:field_)?(\\d+)$", Qt::CaseInsensitive );

QgsDelimitedTextFile::QgsDelimitedTextFile( QString url ) :
    mFileName( QString() ),
    mEncoding( "UTF-8" ),
    mFile( 0 ),
    mStream( 0 ),
    mDefinitionValid( false ),
    mUseHeader( true ),
    mDiscardEmptyFields( false ),
    mTrimFields( false ),
    mSkipLines( 0 ),
    mMaxFields( 0 ),
    mLineNumber( 0 ),
    mRecordLineNumber( 0 ),
    mMaxFieldCount( 0 )
{
  // The default type is CSV
  setTypeCSV();
  if ( ! url.isNull() ) setFromUrl( url );
}


QgsDelimitedTextFile::~QgsDelimitedTextFile()
{
  close();
}

void QgsDelimitedTextFile::close()
{
  if ( mStream )
  {
    delete mStream;
    mStream = 0;
  }
  if ( mFile )
  {
    delete mFile;
    mFile = 0;
  }
}

bool QgsDelimitedTextFile::open()
{
  if ( ! mFile )
  {
    close();
    mFile = new QFile( mFileName );
    if ( ! mFile->open( QIODevice::ReadOnly ) )
    {
      QgsDebugMsg( "Data file " + mFileName + " could not be opened" );
      delete mFile;
      mFile = 0;
      return false;
    }
    mStream = new QTextStream( mFile );
    if ( ! mEncoding.isEmpty() )
    {
      QTextCodec *codec =  QTextCodec::codecForName( mEncoding.toAscii() );
      mStream->setCodec( codec );
    }
  }
  return true;
}

// Clear information based on current definition of file
void QgsDelimitedTextFile::resetDefinition()
{
  close();
  mFieldNames.clear();
  mMaxFieldCount = 0;
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
  setFileName( url.toLocalFile() );

  // Extract the encoding
  if ( url.hasQueryItem( "encoding" ) )
  {
    mEncoding = url.queryItemValue( "encoding" );
  }

  // The default type is csv, to be consistent with the
  // previous implementation (except that quoting should be handled properly)

  QString type( "csv" );
  QString delimiter( "," );
  QString quote = "\"";
  QString escape = "\"";
  mUseHeader = true;
  mSkipLines = 0;

  // Prefer simple "type" for delimiter type, but include delimiterType
  // as optional name  for backwards compatibility
  if ( url.hasQueryItem( "type" ) || url.hasQueryItem( "delimiterType" ) )
  {
    if ( url.hasQueryItem( "type" ) )
      type = url.queryItemValue( "type" );
    else if ( url.hasQueryItem( "delimiterType" ) )
      type = url.queryItemValue( "delimiterType" );

    // Support for previous version of Qgs - plain chars had
    // quote characters ' or "
    if ( type == "plain" )
    {
      quote = "'\"";
      escape = "";
    }
    else if( type == "regexp ")
    {
      delimiter="";
      quote="";
      escape="";
    }
  }
  if ( url.hasQueryItem( "delimiter" ) )
  {
    delimiter = url.queryItemValue( "delimiter" );
  }
  if ( url.hasQueryItem( "quote" ) )
  {
    quote = url.queryItemValue( "quote" );
  }
  if ( url.hasQueryItem( "escape" ) )
  {
    escape = url.queryItemValue( "escape" );
  }
  if ( url.hasQueryItem( "skipLines" ) )
  {
    mSkipLines = url.queryItemValue( "skipLines" ).toInt();
  }
  if ( url.hasQueryItem( "useHeader" ) )
  {
    mUseHeader = ! url.queryItemValue( "useHeader" ).toUpper().startsWith( 'N' );
  }
  if ( url.hasQueryItem( "skipEmptyFields" ) )
  {
    mDiscardEmptyFields = ! url.queryItemValue( "useHeader" ).toUpper().startsWith( 'N' );;
  }
  if ( url.hasQueryItem( "trimFields" ) )
  {
    mTrimFields = ! url.queryItemValue( "trimFields" ).toUpper().startsWith( 'N' );;
  }
  if ( url.hasQueryItem( "maxFields" ) )
  {
    mMaxFields = url.queryItemValue( "maxFields" ).toInt();
  }

  QgsDebugMsg( "Delimited text file is: " + mFileName );
  QgsDebugMsg( "Encoding is: " + mEncoding );
  QgsDebugMsg( "Delimited file type is: " + type );
  QgsDebugMsg( "Delimiter is: [" + delimiter + "]" );
  QgsDebugMsg( "Quote character is: [" + quote + "]" );
  QgsDebugMsg( "Escape character is: [" + escape + "]" );
  QgsDebugMsg( "Skip lines: " + QString::number( mSkipLines ) );
  QgsDebugMsg( "Maximum number of fields in record: " + QString::number( mMaxFields ) );
  QgsDebugMsg( "Use headers: " + QString( mUseHeader ? "Yes" : "No" ) );
  QgsDebugMsg( "Discard empty fields: " + QString( mDiscardEmptyFields ? "Yes" : "No" ) );
  QgsDebugMsg( "Trim fields: " + QString( mTrimFields ? "Yes" : "No" ) );

  // Support for previous version of plain characters
  if ( type == "csv" || type == "plain" )
  {
    setTypeCSV( delimiter, quote, escape );
  }
  else if ( type == "whitespace" )
  {
    setTypeWhitespace();
  }
  else if ( type == "regexp" )
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
  if ( mEncoding != "UTF-8" )
  {
    url.addQueryItem( "encoding", mEncoding );
  }
  url.addQueryItem( "type", type() );
  if ( mType == DelimTypeRegexp )
  {
    url.addQueryItem( "delimiter", mDelimRegexp.pattern() );
  }
  if ( mType == DelimTypeCSV )
  {
    if ( mDelimChars != "," ) url.addQueryItem( "delimiter", encodeChars( mDelimChars ) );
    if ( mQuoteChar != "\"" ) url.addQueryItem( "quote", encodeChars( mQuoteChar ) );
    if ( mEscapeChar != "\"" ) url.addQueryItem( "escape", encodeChars( mEscapeChar ) );
  }
  if ( mSkipLines > 0 )
  {
    url.addQueryItem( "skipLines", QString::number( mSkipLines ) );
  }
  if ( ! mUseHeader )
  {
    url.addQueryItem( "useHeader", "No" );
  }
  if ( mTrimFields )
  {
    url.addQueryItem( "trimFields", "Yes" );
  }
  if ( mDiscardEmptyFields && mType != DelimTypeWhitespace )
  {
    url.addQueryItem( "skipEmptyFields", "Yes" );
  }
  if ( mMaxFields > 0 )
  {
    url.addQueryItem( "maxFields", QString::number( mMaxFields ) );
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
  if ( mType == DelimTypeWhitespace ) return QString( "whitespace" );
  if ( mType == DelimTypeCSV ) return QString( "csv" );
  if ( mType == DelimTypeRegexp ) return QString( "regexp" );
  return QString( "csv" );
}

void QgsDelimitedTextFile::setTypeWhitespace()
{
  setTypeRegexp( "\\s+" );
  mDiscardEmptyFields = true;
  mType = DelimTypeWhitespace;
}

void QgsDelimitedTextFile::setTypeRegexp( QString regexp )
{
  resetDefinition();
  mType = DelimTypeRegexp;
  mDelimRegexp.setPattern( regexp );
  mAnchoredRegexp = regexp.startsWith( "^" );
  mParser = &QgsDelimitedTextFile::parseRegexp;
  mDefinitionValid = regexp.size() > 0 && mDelimRegexp.isValid();
  if ( ! mDefinitionValid )
  {
    QgsDebugMsg( "Invalid regular expression in delimited text file delimiter: " + regexp );
  }
  else if ( mAnchoredRegexp && mDelimRegexp.captureCount() == 0 )
  {
    mDefinitionValid = false;
    QgsDebugMsg( "Invalid anchored regular expression - must have capture groups: " + regexp );
  }
}

QString QgsDelimitedTextFile::decodeChars( QString chars )
{
  chars = chars.replace( "\\t", "\t" );
  return chars;
}

QString QgsDelimitedTextFile::encodeChars( QString chars )
{
  chars = chars.replace( "\t", "\\t" );
  return chars;
}

void QgsDelimitedTextFile::setTypeCSV( QString delim, QString quote, QString escape )
{
  resetDefinition();
  mType = DelimTypeRegexp;
  mType = DelimTypeCSV;
  mDelimChars = decodeChars( delim );
  mQuoteChar = decodeChars( quote );
  mEscapeChar = decodeChars( escape );
  mParser = &QgsDelimitedTextFile::parseQuoted;
  mDefinitionValid = mDelimChars.size() > 0;
  if ( ! mDefinitionValid )
  {
    QgsDebugMsg( "Invalid empty delimiter defined for text file delimiter" );
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

void QgsDelimitedTextFile::setTrimFields( bool trimFields )
{
  resetDefinition();
  mTrimFields = trimFields;
}

void QgsDelimitedTextFile::setMaxFields( int maxFields )
{
  resetDefinition();
  mMaxFields = maxFields;
}

void QgsDelimitedTextFile::setDiscardEmptyFields( bool discardEmptyFields )
{
  resetDefinition();
  mDiscardEmptyFields = discardEmptyFields;
}


void QgsDelimitedTextFile::setFieldNames( const QStringList &names )
{
  mFieldNames.empty();
  foreach ( QString name, names )
  {
    bool nameOk = true;
    int fieldNo = mFieldNames.size() + 1;
    name = name.trimmed();

    // If the name is invalid then reset it to default name
    if ( InvalidFieldRegexp.exactMatch( name ) )
    {
      name = DefaultFieldName.arg( fieldNo );
    }
    // If the name looks like a default field name (field_##), then it is
    // valid if the number matches its column number..
    else if ( DefaultFieldRegexp.indexIn( name ) == 0 )
    {
      int col = DefaultFieldRegexp.capturedTexts()[1].toInt();
      nameOk = col == fieldNo;
    }
    // Otherwise it is valid if isn't the name of an existing field...
    else
    {
      nameOk = ! mFieldNames.contains( name, Qt::CaseInsensitive );
    }
    // If it is not a valid name then try appending a number to generate
    // a valid name.
    if ( ! nameOk )
    {
      int suffix = 0;
      QString basename = name + "_%1";
      while ( true )
      {
        suffix++;
        name = basename.arg( suffix );
        // Not ok if it is already in the name list
        if ( mFieldNames.contains( name, Qt::CaseInsensitive ) ) continue;
        // Not ok if it is already in proposed names
        if ( names.contains( name, Qt::CaseInsensitive ) ) continue;
        break;
      }
    }
    mFieldNames.append( name );
  }
}


QStringList &QgsDelimitedTextFile::fieldNames()
{
  // If not yet opened then reset file to read column headers
  //
  if ( mUseHeader && ! mFile ) reset();
  // If have read more fields than field names, then append field names
  // to match the field count (will only happen if parsed some records)
  if ( mMaxFieldCount > mFieldNames.size() )
  {
    for ( int i = mFieldNames.size() + 1; i <= mMaxFieldCount; i++ )
    {
      mFieldNames.append( DefaultFieldName.arg( i ) );
    }
  }
  return mFieldNames;
}

int QgsDelimitedTextFile::fieldIndex( QString name )
{
  // If not yet opened then reset file to read column headers
  //
  if ( mUseHeader && ! mFile ) reset();
  // Try to determine the field based on a default field name, includes
  // Field_### and simple integer fields.
  if ( DefaultFieldRegexp.indexIn( name ) == 0 )
  {
    return DefaultFieldRegexp.capturedTexts()[1].toInt() - 1;
  }
  for ( int i = 0; i < mFieldNames.size(); i++ )
  {
    if ( mFieldNames[i].compare( name, Qt::CaseInsensitive ) == 0 ) return i;
  }
  return -1;

}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::nextRecord( QStringList &record )
{
  return ( this->*mParser )( record );
}


QgsDelimitedTextFile::Status  QgsDelimitedTextFile::reset()
{
  // Make sure the file is valid open
  if ( ! isValid() || ! open() ) return InvalidDefinition;

  // Reset the file pointer
  mStream->seek( 0 );
  mLineNumber = 0;
  mRecordLineNumber = 0;

  // Skip header lines
  for ( int i = mSkipLines; i-- > 0; )
  {
    if ( mStream->readLine().isNull() ) return RecordEOF;
    mLineNumber++;
  }
  // Read the column names
  if ( mUseHeader )
  {
    QStringList names;
    QgsDelimitedTextFile::Status result = nextRecord( names );
    setFieldNames( names );
    return result;
  }
  return RecordOk;
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::nextLine( QString &buffer, bool skipBlank )
{
  if ( ! mStream )
  {
    Status status = reset();
    if ( status != RecordOk ) return status;
  }

  while ( ! mStream->atEnd() )
  {
    buffer = mStream->readLine();
    if ( buffer.isNull() ) break;
    mLineNumber++;
    if ( skipBlank && buffer.isEmpty() ) continue;
    return RecordOk;
  }

  // Null string if at end of stream
  return RecordEOF;
}

void QgsDelimitedTextFile::appendField( QStringList &record, QString field, bool quoted )
{
  if ( mMaxFields > 0 && record.size() >= mMaxFields ) return;
  if ( quoted )
  {
    record.append( field );
  }
  else
  {
    if ( mTrimFields ) field = field.trimmed();
    if ( !( mDiscardEmptyFields && field.isEmpty() ) ) record.append( field );
  }
  // Keep track of maximum number of non-empty fields in a record
  if ( record.size() > mMaxFieldCount && ! field.isEmpty() ) mMaxFieldCount = record.size();
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::parseRegexp( QStringList &fields )
{
  fields.clear();
  QString buffer;
  Status status = nextLine( buffer, true );
  if ( status != RecordOk ) return status;
  mRecordLineNumber = mLineNumber;

  // If match is anchored, then only interested in records which actually match
  // and extract capture groups
  if ( mAnchoredRegexp )
  {
    if ( mDelimRegexp.indexIn( buffer ) < 0 ) return RecordInvalid;
    QStringList groups = mDelimRegexp.capturedTexts();
    for ( int i = 1; i < groups.size(); i++ )
    {
      appendField( fields, groups[i] );
    }
    return RecordOk;
  }

  int pos = 0;
  int size = buffer.size();
  while ( true )
  {
    if ( pos >= size ) break;
    int matchPos = mDelimRegexp.indexIn( buffer, pos );
    // If match won't advance cursor, then need to force it along one place
    // to avoid infinite loop.
    int matchLen = mDelimRegexp.matchedLength();
    if ( matchPos == pos && matchLen == 0 )
    {
      matchPos = mDelimRegexp.indexIn( buffer, pos + 1 );
      matchLen = mDelimRegexp.matchedLength();
    }
    // If no match, then field is to end of record
    if ( matchPos < 0 )
    {
      appendField( fields, buffer.mid( pos ) );
      break;
    }
    // Else append up to matched string, then any capture
    // groups from match
    appendField( fields, buffer.mid( pos, matchPos - pos ) );
    if ( mDelimRegexp.captureCount() > 0 )
    {
      QStringList groups = mDelimRegexp.capturedTexts();
      for ( int i = 1; i < groups.size(); i++ )
      {
        appendField( fields, groups[i] );
      }
    }
    // Advance the buffer pointer
    pos = matchPos + matchLen;

    // Quit loop if we have enough fields.
    if ( mMaxFields > 0 && fields.size() >= mMaxFields ) break;
  }
  return RecordOk;
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::parseQuoted( QStringList &fields )
{
  fields.clear();

  // Find the first non-blank line to read
  QString buffer;
  Status status = nextLine( buffer, true );
  if ( status != RecordOk ) return status;
  mRecordLineNumber = mLineNumber;

  QString field;        // String in which to accumulate next field
  bool escaped = false; // Next char is escaped
  bool quoted = false;  // In quotes
  QChar quoteChar = 0;  // Actual quote character used to open quotes
  bool started = false; // Non-blank chars in field or quotes started
  bool ended = false;   // Quoted field ended
  int cp = 0;          // Pointer to the next character in the buffer
  int cpmax = buffer.size(); // End of string

  while ( true )
  {
    QChar c = buffer[cp];
    cp++;

    // If end of line then if escaped or buffered then try to get more...
    if ( cp > cpmax )
    {
      if ( quoted || escaped )
      {
        status = nextLine( buffer, false );
        if ( status != RecordOk )
        {
          status = RecordInvalid;
          break;
        }
        field.append( '\n' );
        cp = 0;
        cpmax = buffer.size();
        escaped = false;
        continue;
      }
      break;
    }

    // If escaped, then just append the character
    if ( escaped )
    {
      field.append( c );
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
    bool isDelim = mDelimChars.contains( c );
    if ( ! isDelim )
    {
      bool isQuoteChar = mQuoteChar.contains( c );
      isQuote = quoted ? c == quoteChar : isQuoteChar;
      isEscape = mEscapeChar.contains( c );
      if ( isQuoteChar && isEscape ) isEscape = isQuote;
    }

    // Start or end of quote ...
    if ( isQuote )
    {
      // quote char in quoted field
      if ( quoted )
      {
        // if is also escape and next character is quote, then
        // escape the quote..
        if ( isEscape && buffer[cp] == quoteChar )
        {
          field.append( quoteChar );
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
      else if ( ! started )
      {
        field.clear();
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
    else if ( isEscape )
    {
      escaped = true;
    }
    // If within quotes, then append to the string
    else if ( quoted )
    {
      field.append( c );
    }
    // If it is a delimiter, then end of field...
    else if ( isDelim )
    {
      appendField( fields, field, ended );

      // Clear the field
      field.clear();
      started = false;
      ended = false;
    }
    // Whitespace is permitted before the start of a field, or
    // after the end..
    else if ( c.isSpace() )
    {
      if ( ! ended ) field.append( c );
    }
    // Other chars permitted if not after quoted field
    else
    {
      if ( ended )
      {
        fields.clear();
        return RecordInvalid;
      }
      field.append( c );
      started = true;
    }
  }
  // If reached the end of the record, then add the last field...
  if ( started )
  {
    appendField( fields, field, ended );

  }
  return status;
}

bool QgsDelimitedTextFile::isValid()
{

  return mDefinitionValid && QFile::exists( mFileName );
}

