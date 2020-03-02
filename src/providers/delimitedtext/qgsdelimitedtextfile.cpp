/***************************************************************************
  qgsdelimitedtextfile.cpp -  Data provider for delimited text
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
#include <QFileInfo>
#include <QDataStream>
#include <QTextStream>
#include <QFileSystemWatcher>
#include <QTextCodec>
#include <QStringList>
#include <QRegExp>
#include <QUrl>
#include <QUrlQuery>

QgsDelimitedTextFile::QgsDelimitedTextFile( const QString &url )
  : mFileName( QString() )
  , mEncoding( QStringLiteral( "UTF-8" ) )
  , mDefaultFieldName( QStringLiteral( "field_%1" ) )
  , mDefaultFieldRegexp( "^(?:field_)(\\d+)$", Qt::CaseInsensitive )
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
    mStream = nullptr;
  }
  if ( mFile )
  {
    delete mFile;
    mFile = nullptr;
  }
  if ( mWatcher )
  {
    delete mWatcher;
    mWatcher = nullptr;
  }
  mLineNumber = -1;
  mRecordLineNumber = -1;
  mRecordNumber = -1;
  mMaxRecordNumber = -1;
  mHoldCurrentRecord = false;
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
      mFile = nullptr;
    }
    if ( mFile )
    {
      mStream = new QTextStream( mFile );
      if ( ! mEncoding.isEmpty() )
      {
        QTextCodec *codec = QTextCodec::codecForName( mEncoding.toLatin1() );
        mStream->setCodec( codec );
      }
      if ( mUseWatcher )
      {
        mWatcher = new QFileSystemWatcher();
        mWatcher->addPath( mFileName );
        connect( mWatcher, &QFileSystemWatcher::fileChanged, this, &QgsDelimitedTextFile::updateFile );
      }
    }
  }
  return nullptr != mFile;
}

void QgsDelimitedTextFile::updateFile()
{
  close();
  emit fileUpdated();
}

// Clear information based on current definition of file
void QgsDelimitedTextFile::resetDefinition()
{
  close();
  mFieldNames.clear();
  mMaxFieldCount = 0;
}

// Extract the provider definition from the url
bool QgsDelimitedTextFile::setFromUrl( const QString &url )
{
  QUrl qurl = QUrl::fromEncoded( url.toLatin1() );
  return setFromUrl( qurl );
}

// Extract the provider definition from the url
bool QgsDelimitedTextFile::setFromUrl( const QUrl &url )
{
  // Close any existing definition
  resetDefinition();

  // Extract the file name
  setFileName( url.toLocalFile() );

  // Extract the encoding
  const QUrlQuery query( url );
  if ( query.hasQueryItem( QStringLiteral( "encoding" ) ) )
  {
    mEncoding = query.queryItemValue( QStringLiteral( "encoding" ) );
  }

  //
  if ( query.hasQueryItem( QStringLiteral( "watchFile" ) ) )
  {
    mUseWatcher = query.queryItemValue( QStringLiteral( "watchFile" ) ).toUpper().startsWith( 'Y' );
  }

  // The default type is csv, to be consistent with the
  // previous implementation (except that quoting should be handled properly)

  QString type( QStringLiteral( "csv" ) );
  QString delimiter( QStringLiteral( "," ) );
  QString quote = QStringLiteral( "\"" );
  QString escape = QStringLiteral( "\"" );
  mUseHeader = true;
  mSkipLines = 0;

  // Prefer simple "type" for delimiter type, but include delimiterType
  // as optional name  for backwards compatibility
  if ( query.hasQueryItem( QStringLiteral( "type" ) ) || query.hasQueryItem( QStringLiteral( "delimiterType" ) ) )
  {
    if ( query.hasQueryItem( QStringLiteral( "type" ) ) )
      type = query.queryItemValue( QStringLiteral( "type" ) );
    else if ( query.hasQueryItem( QStringLiteral( "delimiterType" ) ) )
      type = query.queryItemValue( QStringLiteral( "delimiterType" ) );

    // Support for previous version of Qgs - plain chars had
    // quote characters ' or "
    if ( type == QLatin1String( "plain" ) )
    {
      quote = QStringLiteral( "'\"" );
      escape.clear();
    }
    else if ( type == QLatin1String( "regexp " ) )
    {
      delimiter.clear();
      quote.clear();
      escape.clear();
    }
  }
  if ( query.hasQueryItem( QStringLiteral( "delimiter" ) ) )
  {
    delimiter = query.queryItemValue( QStringLiteral( "delimiter" ) );
  }
  if ( query.hasQueryItem( QStringLiteral( "quote" ) ) )
  {
    quote = query.queryItemValue( QStringLiteral( "quote" ) );
  }
  if ( query.hasQueryItem( QStringLiteral( "escape" ) ) )
  {
    escape = query.queryItemValue( QStringLiteral( "escape" ) );
  }
  if ( query.hasQueryItem( QStringLiteral( "skipLines" ) ) )
  {
    mSkipLines = query.queryItemValue( QStringLiteral( "skipLines" ) ).toInt();
  }
  if ( query.hasQueryItem( QStringLiteral( "useHeader" ) ) )
  {
    mUseHeader = ! query.queryItemValue( QStringLiteral( "useHeader" ) ).toUpper().startsWith( 'N' );
  }
  if ( query.hasQueryItem( QStringLiteral( "skipEmptyFields" ) ) )
  {
    mDiscardEmptyFields = ! query.queryItemValue( QStringLiteral( "skipEmptyFields" ) ).toUpper().startsWith( 'N' );
  }
  if ( query.hasQueryItem( QStringLiteral( "trimFields" ) ) )
  {
    mTrimFields = ! query.queryItemValue( QStringLiteral( "trimFields" ) ).toUpper().startsWith( 'N' );
  }
  if ( query.hasQueryItem( QStringLiteral( "maxFields" ) ) )
  {
    mMaxFields = query.queryItemValue( QStringLiteral( "maxFields" ) ).toInt();
  }

  QgsDebugMsg( "Delimited text file is: " + mFileName );
  QgsDebugMsg( "Encoding is: " + mEncoding );
  QgsDebugMsg( "Delimited file type is: " + type );
  QgsDebugMsg( "Delimiter is: [" + delimiter + ']' );
  QgsDebugMsg( "Quote character is: [" + quote + ']' );
  QgsDebugMsg( "Escape character is: [" + escape + ']' );
  QgsDebugMsg( "Skip lines: " + QString::number( mSkipLines ) );
  QgsDebugMsg( "Maximum number of fields in record: " + QString::number( mMaxFields ) );
  QgsDebugMsg( "Use headers: " + QString( mUseHeader ? "Yes" : "No" ) );
  QgsDebugMsg( "Discard empty fields: " + QString( mDiscardEmptyFields ? "Yes" : "No" ) );
  QgsDebugMsg( "Trim fields: " + QString( mTrimFields ? "Yes" : "No" ) );

  // Support for previous version of plain characters
  if ( type == QLatin1String( "csv" ) || type == QLatin1String( "plain" ) )
  {
    setTypeCSV( delimiter, quote, escape );
  }
  else if ( type == QLatin1String( "whitespace" ) )
  {
    setTypeWhitespace();
  }
  else if ( type == QLatin1String( "regexp" ) )
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
  QUrlQuery query( url );
  if ( mEncoding != QLatin1String( "UTF-8" ) )
  {
    query.addQueryItem( QStringLiteral( "encoding" ), mEncoding );
  }

  if ( mUseWatcher )
  {
    query.addQueryItem( QStringLiteral( "watchFile" ), QStringLiteral( "yes" ) );
  }

  query.addQueryItem( QStringLiteral( "type" ), type() );
  if ( mType == DelimTypeRegexp )
  {
    query.addQueryItem( QStringLiteral( "delimiter" ), mDelimRegexp.pattern() );
  }
  if ( mType == DelimTypeCSV )
  {
    if ( mDelimChars != QLatin1String( "," ) ) query.addQueryItem( QStringLiteral( "delimiter" ), encodeChars( mDelimChars ) );
    if ( mQuoteChar != QLatin1String( "\"" ) ) query.addQueryItem( QStringLiteral( "quote" ), encodeChars( mQuoteChar ) );
    if ( mEscapeChar != QLatin1String( "\"" ) ) query.addQueryItem( QStringLiteral( "escape" ), encodeChars( mEscapeChar ) );
  }
  if ( mSkipLines > 0 )
  {
    query.addQueryItem( QStringLiteral( "skipLines" ), QString::number( mSkipLines ) );
  }
  if ( ! mUseHeader )
  {
    query.addQueryItem( QStringLiteral( "useHeader" ), QStringLiteral( "No" ) );
  }
  if ( mTrimFields )
  {
    query.addQueryItem( QStringLiteral( "trimFields" ), QStringLiteral( "Yes" ) );
  }
  if ( mDiscardEmptyFields && mType != DelimTypeWhitespace )
  {
    query.addQueryItem( QStringLiteral( "skipEmptyFields" ), QStringLiteral( "Yes" ) );
  }
  if ( mMaxFields > 0 )
  {
    query.addQueryItem( QStringLiteral( "maxFields" ), QString::number( mMaxFields ) );
  }
  url.setQuery( query );
  return url;
}

void QgsDelimitedTextFile::setFileName( const QString &filename )
{
  resetDefinition();
  mFileName = filename;
}

void QgsDelimitedTextFile::setEncoding( const QString &encoding )
{
  resetDefinition();
  mEncoding = encoding;
}

void QgsDelimitedTextFile::setUseWatcher( bool useWatcher )
{
  resetDefinition();
  mUseWatcher = useWatcher;
}

QString QgsDelimitedTextFile::type()
{
  if ( mType == DelimTypeWhitespace ) return QStringLiteral( "whitespace" );
  if ( mType == DelimTypeCSV ) return QStringLiteral( "csv" );
  if ( mType == DelimTypeRegexp ) return QStringLiteral( "regexp" );
  return QStringLiteral( "csv" );
}

void QgsDelimitedTextFile::setTypeWhitespace()
{
  setTypeRegexp( QStringLiteral( "\\s+" ) );
  mDiscardEmptyFields = true;
  mType = DelimTypeWhitespace;
}

void QgsDelimitedTextFile::setTypeRegexp( const QString &regexp )
{
  resetDefinition();
  mType = DelimTypeRegexp;
  mDelimRegexp.setPattern( regexp );
  mAnchoredRegexp = regexp.startsWith( '^' );
  mParser = &QgsDelimitedTextFile::parseRegexp;
  mDefinitionValid = !regexp.isEmpty() && mDelimRegexp.isValid();
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
  chars = chars.replace( QLatin1String( "\\t" ), QLatin1String( "\t" ) );
  return chars;
}

QString QgsDelimitedTextFile::encodeChars( QString chars )
{
  chars = chars.replace( '\t', QLatin1String( "\\t" ) );
  return chars;
}

void QgsDelimitedTextFile::setTypeCSV( const QString &delim, const QString &quote, const QString &escape )
{
  resetDefinition();
  mType = DelimTypeCSV;
  mDelimChars = decodeChars( delim );
  mQuoteChar = decodeChars( quote );
  mEscapeChar = decodeChars( escape );
  mParser = &QgsDelimitedTextFile::parseQuoted;
  mDefinitionValid = !mDelimChars.isEmpty();
  if ( ! mDefinitionValid )
  {
    QgsDebugMsg( QStringLiteral( "Invalid empty delimiter defined for text file delimiter" ) );
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
  mFieldNames.clear();
  const auto constNames = names;
  for ( QString name : constNames )
  {
    bool nameOk = true;
    int fieldNo = mFieldNames.size() + 1;
    name = name.trimmed();
    if ( name.length() > mMaxNameLength ) name = name.mid( 0, mMaxNameLength );

    // If the name is empty then reset it to default name
    if ( name.length() == 0 )
    {
      name = mDefaultFieldName.arg( fieldNo );
    }
    // If the name looks like a default field name (field_##), then it is
    // valid if the number matches its column number..
    else if ( mDefaultFieldRegexp.indexIn( name ) == 0 )
    {
      int col = mDefaultFieldRegexp.capturedTexts().at( 1 ).toInt();
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
        // Not OK if it is already in the name list
        if ( mFieldNames.contains( name, Qt::CaseInsensitive ) ) continue;
        // Not OK if it is already in proposed names
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
      mFieldNames.append( mDefaultFieldName.arg( i ) );
    }
  }
  return mFieldNames;
}

int QgsDelimitedTextFile::fieldIndex( const QString &name )
{
  // If not yet opened then reset file to read column headers
  //
  if ( mUseHeader && ! mFile ) reset();
  // Try to determine the field based on a default field name, includes
  // Field_### and simple integer fields.
  if ( mDefaultFieldRegexp.indexIn( name ) == 0 )
  {
    return mDefaultFieldRegexp.capturedTexts().at( 1 ).toInt() - 1;
  }
  for ( int i = 0; i < mFieldNames.size(); i++ )
  {
    if ( mFieldNames[i].compare( name, Qt::CaseInsensitive ) == 0 ) return i;
  }
  return -1;

}

bool QgsDelimitedTextFile::setNextRecordId( long nextRecordId )
{
  if ( ! mFile ) reset();

  mHoldCurrentRecord = nextRecordId == mRecordLineNumber;
  if ( mHoldCurrentRecord ) return true;
  return setNextLineNumber( nextRecordId );
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::nextRecord( QStringList &record )
{

  record.clear();
  Status status = RecordOk;

  if ( mHoldCurrentRecord )
  {
    mHoldCurrentRecord = false;
  }
  else
  {
    // Invalidate the record line number, in get EOF
    mRecordLineNumber = -1;

    // Find the first non-blank line to read
    QString buffer;
    status = nextLine( buffer, true );
    if ( status != RecordOk ) return RecordEOF;

    mCurrentRecord.clear();
    mRecordLineNumber = mLineNumber;
    if ( mRecordNumber >= 0 )
    {
      mRecordNumber++;
      if ( mRecordNumber > mMaxRecordNumber ) mMaxRecordNumber = mRecordNumber;
    }
    status = ( this->*mParser )( buffer, mCurrentRecord );
  }
  if ( status == RecordOk )
  {
    record.append( mCurrentRecord );
  }
  return status;
}


QgsDelimitedTextFile::Status  QgsDelimitedTextFile::reset()
{
  // Make sure the file is valid open
  if ( ! isValid() || ! open() ) return InvalidDefinition;

  // Reset the file pointer
  mStream->seek( 0 );
  mLineNumber = 0;
  mRecordNumber = -1;
  mRecordLineNumber = -1;

  // Skip header lines
  for ( int i = mSkipLines; i-- > 0; )
  {
    if ( mStream->readLine().isNull() ) return RecordEOF;
    mLineNumber++;
  }
  // Read the column names
  Status result = RecordOk;
  if ( mUseHeader )
  {
    QStringList names;
    result = nextRecord( names );
    setFieldNames( names );
  }
  if ( result == RecordOk ) mRecordNumber = 0;
  return result;
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

bool QgsDelimitedTextFile::setNextLineNumber( long nextLineNumber )
{
  if ( ! mStream ) return false;
  if ( mLineNumber > nextLineNumber - 1 )
  {
    mRecordNumber = -1;
    mStream->seek( 0 );
    mLineNumber = 0;
  }
  QString buffer;
  while ( mLineNumber < nextLineNumber - 1 )
  {
    if ( nextLine( buffer, false ) != RecordOk ) return false;
  }
  return true;

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
  if ( record.size() > mMaxFieldCount && ! field.isEmpty() )
  {
    mMaxFieldCount = record.size();
  }
}

QgsDelimitedTextFile::Status QgsDelimitedTextFile::parseRegexp( QString &buffer, QStringList &fields )
{

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

QgsDelimitedTextFile::Status QgsDelimitedTextFile::parseQuoted( QString &buffer, QStringList &fields )
{
  Status status = RecordOk;
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
          ended = true;
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
  return mDefinitionValid && QFile::exists( mFileName ) && QFileInfo( mFileName ).size() > 0;
}

