/***************************************************************************
    qgsstringutils.cpp
    ------------------
    begin                : June 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstringutils.h"
#include <QVector>
#include <QRegExp>
#include <QStringList>
#include <QTextBoundaryFinder>
#include <QRegularExpression>

QString QgsStringUtils::capitalize( const QString &string, QgsStringUtils::Capitalization capitalization )
{
  if ( string.isEmpty() )
    return QString();

  switch ( capitalization )
  {
    case MixedCase:
      return string;

    case AllUppercase:
      return string.toUpper();

    case AllLowercase:
      return string.toLower();

    case ForceFirstLetterToCapital:
    {
      QString temp = string;

      QTextBoundaryFinder wordSplitter( QTextBoundaryFinder::Word, string.constData(), string.length(), nullptr, 0 );
      QTextBoundaryFinder letterSplitter( QTextBoundaryFinder::Grapheme, string.constData(), string.length(), nullptr, 0 );

      wordSplitter.setPosition( 0 );
      bool first = true;
      while ( ( first && wordSplitter.boundaryReasons() & QTextBoundaryFinder::StartOfItem )
              || wordSplitter.toNextBoundary() >= 0 )
      {
        first = false;
        letterSplitter.setPosition( wordSplitter.position() );
        letterSplitter.toNextBoundary();
        QString substr = string.mid( wordSplitter.position(), letterSplitter.position() - wordSplitter.position() );
        temp.replace( wordSplitter.position(), substr.length(), substr.toUpper() );
      }
      return temp;
    }

    case TitleCase:
    {
      // yes, this is MASSIVELY simplifying the problem!!

      static QStringList smallWords;
      static QStringList newPhraseSeparators;
      static QRegularExpression splitWords;
      if ( smallWords.empty() )
      {
        smallWords = QObject::tr( "a|an|and|as|at|but|by|en|for|if|in|nor|of|on|or|per|s|the|to|vs.|vs|via" ).split( '|' );
        newPhraseSeparators = QObject::tr( ".|:" ).split( '|' );
        splitWords = QRegularExpression( QStringLiteral( "\\b" ), QRegularExpression::UseUnicodePropertiesOption );
      }

      const QStringList parts = string.split( splitWords, QString::SkipEmptyParts );
      QString result;
      bool firstWord = true;
      int i = 0;
      int lastWord = parts.count() - 1;
      for ( const QString &word : qgis::as_const( parts ) )
      {
        if ( newPhraseSeparators.contains( word.trimmed() ) )
        {
          firstWord = true;
          result += word;
        }
        else if ( firstWord || ( i == lastWord ) || !smallWords.contains( word ) )
        {
          result += word.at( 0 ).toUpper() + word.mid( 1 );
          firstWord = false;
        }
        else
        {
          result += word;
        }
        i++;
      }
      return result;
    }

    case UpperCamelCase:
      QString result = QgsStringUtils::capitalize( string.toLower(), QgsStringUtils::ForceFirstLetterToCapital ).simplified();
      result.remove( ' ' );
      return result;
  }
  // no warnings
  return string;
}

// original code from http://www.qtcentre.org/threads/52456-HTML-Unicode-ampersand-encoding
QString QgsStringUtils::ampersandEncode( const QString &string )
{
  QString encoded;
  for ( int i = 0; i < string.size(); ++i )
  {
    QChar ch = string.at( i );
    if ( ch.unicode() > 160 )
      encoded += QStringLiteral( "&#%1;" ).arg( static_cast< int >( ch.unicode() ) );
    else if ( ch.unicode() == 38 )
      encoded += QStringLiteral( "&amp;" );
    else if ( ch.unicode() == 60 )
      encoded += QStringLiteral( "&lt;" );
    else if ( ch.unicode() == 62 )
      encoded += QStringLiteral( "&gt;" );
    else
      encoded += ch;
  }
  return encoded;
}

int QgsStringUtils::levenshteinDistance( const QString &string1, const QString &string2, bool caseSensitive )
{
  int length1 = string1.length();
  int length2 = string2.length();

  //empty strings? solution is trivial...
  if ( string1.isEmpty() )
  {
    return length2;
  }
  else if ( string2.isEmpty() )
  {
    return length1;
  }

  //handle case sensitive flag (or not)
  QString s1( caseSensitive ? string1 : string1.toLower() );
  QString s2( caseSensitive ? string2 : string2.toLower() );

  const QChar *s1Char = s1.constData();
  const QChar *s2Char = s2.constData();

  //strip out any common prefix
  int commonPrefixLen = 0;
  while ( length1 > 0 && length2 > 0 && *s1Char == *s2Char )
  {
    commonPrefixLen++;
    length1--;
    length2--;
    s1Char++;
    s2Char++;
  }

  //strip out any common suffix
  while ( length1 > 0 && length2 > 0 && s1.at( commonPrefixLen + length1 - 1 ) == s2.at( commonPrefixLen + length2 - 1 ) )
  {
    length1--;
    length2--;
  }

  //fully checked either string? if so, the answer is easy...
  if ( length1 == 0 )
  {
    return length2;
  }
  else if ( length2 == 0 )
  {
    return length1;
  }

  //ensure the inner loop is longer
  if ( length1 > length2 )
  {
    std::swap( s1, s2 );
    std::swap( length1, length2 );
  }

  //levenshtein algorithm begins here
  QVector< int > col;
  col.fill( 0, length2 + 1 );
  QVector< int > prevCol;
  prevCol.reserve( length2 + 1 );
  for ( int i = 0; i < length2 + 1; ++i )
  {
    prevCol << i;
  }
  const QChar *s2start = s2Char;
  for ( int i = 0; i < length1; ++i )
  {
    col[0] = i + 1;
    s2Char = s2start;
    for ( int j = 0; j < length2; ++j )
    {
      col[j + 1] = std::min( std::min( 1 + col[j], 1 + prevCol[1 + j] ), prevCol[j] + ( ( *s1Char == *s2Char ) ? 0 : 1 ) );
      s2Char++;
    }
    col.swap( prevCol );
    s1Char++;
  }
  return prevCol[length2];
}

QString QgsStringUtils::longestCommonSubstring( const QString &string1, const QString &string2, bool caseSensitive )
{
  if ( string1.isEmpty() || string2.isEmpty() )
  {
    //empty strings, solution is trivial...
    return QString();
  }

  //handle case sensitive flag (or not)
  QString s1( caseSensitive ? string1 : string1.toLower() );
  QString s2( caseSensitive ? string2 : string2.toLower() );

  if ( s1 == s2 )
  {
    //another trivial case, identical strings
    return s1;
  }

  int *currentScores = new int [ s2.length()];
  int *previousScores = new int [ s2.length()];
  int maxCommonLength = 0;
  int lastMaxBeginIndex = 0;

  const QChar *s1Char = s1.constData();
  const QChar *s2Char = s2.constData();
  const QChar *s2Start = s2Char;

  for ( int i = 0; i < s1.length(); ++i )
  {
    for ( int j = 0; j < s2.length(); ++j )
    {
      if ( *s1Char != *s2Char )
      {
        currentScores[j] = 0;
      }
      else
      {
        if ( i == 0 || j == 0 )
        {
          currentScores[j] = 1;
        }
        else
        {
          currentScores[j] = 1 + previousScores[j - 1];
        }

        if ( maxCommonLength < currentScores[j] )
        {
          maxCommonLength = currentScores[j];
          lastMaxBeginIndex = i;
        }
      }
      s2Char++;
    }
    std::swap( currentScores, previousScores );
    s1Char++;
    s2Char = s2Start;
  }
  delete [] currentScores;
  delete [] previousScores;
  return string1.mid( lastMaxBeginIndex - maxCommonLength + 1, maxCommonLength );
}

int QgsStringUtils::hammingDistance( const QString &string1, const QString &string2, bool caseSensitive )
{
  if ( string1.isEmpty() && string2.isEmpty() )
  {
    //empty strings, solution is trivial...
    return 0;
  }

  if ( string1.length() != string2.length() )
  {
    //invalid inputs
    return -1;
  }

  //handle case sensitive flag (or not)
  QString s1( caseSensitive ? string1 : string1.toLower() );
  QString s2( caseSensitive ? string2 : string2.toLower() );

  if ( s1 == s2 )
  {
    //another trivial case, identical strings
    return 0;
  }

  int distance = 0;
  const QChar *s1Char = s1.constData();
  const QChar *s2Char = s2.constData();

  for ( int i = 0; i < string1.length(); ++i )
  {
    if ( *s1Char != *s2Char )
      distance++;
    s1Char++;
    s2Char++;
  }

  return distance;
}

QString QgsStringUtils::soundex( const QString &string )
{
  if ( string.isEmpty() )
    return QString();

  QString tmp = string.toUpper();

  //strip non character codes, and vowel like characters after the first character
  QChar *char1 = tmp.data();
  QChar *char2 = tmp.data();
  int outLen = 0;
  for ( int i = 0; i < tmp.length(); ++i, ++char2 )
  {
    if ( ( *char2 ).unicode() >= 0x41 && ( *char2 ).unicode() <= 0x5A && ( i == 0 || ( ( *char2 ).unicode() != 0x41 && ( *char2 ).unicode() != 0x45
         && ( *char2 ).unicode() != 0x48 && ( *char2 ).unicode() != 0x49
         && ( *char2 ).unicode() != 0x4F && ( *char2 ).unicode() != 0x55
         && ( *char2 ).unicode() != 0x57 && ( *char2 ).unicode() != 0x59 ) ) )
    {
      *char1 = *char2;
      char1++;
      outLen++;
    }
  }
  tmp.truncate( outLen );

  QChar *tmpChar = tmp.data();
  tmpChar++;
  for ( int i = 1; i < tmp.length(); ++i, ++tmpChar )
  {
    switch ( ( *tmpChar ).unicode() )
    {
      case 0x42:
      case 0x46:
      case 0x50:
      case 0x56:
        tmp.replace( i, 1, QChar( 0x31 ) );
        break;

      case 0x43:
      case 0x47:
      case 0x4A:
      case 0x4B:
      case 0x51:
      case 0x53:
      case 0x58:
      case 0x5A:
        tmp.replace( i, 1, QChar( 0x32 ) );
        break;

      case 0x44:
      case 0x54:
        tmp.replace( i, 1, QChar( 0x33 ) );
        break;

      case 0x4C:
        tmp.replace( i, 1, QChar( 0x34 ) );
        break;

      case 0x4D:
      case 0x4E:
        tmp.replace( i, 1, QChar( 0x35 ) );
        break;

      case 0x52:
        tmp.replace( i, 1, QChar( 0x36 ) );
        break;
    }
  }

  //remove adjacent duplicates
  char1 = tmp.data();
  char2 = tmp.data();
  char2++;
  outLen = 1;
  for ( int i = 1; i < tmp.length(); ++i, ++char2 )
  {
    if ( *char2 != *char1 )
    {
      char1++;
      *char1 = *char2;
      outLen++;
      if ( outLen == 4 )
        break;
    }
  }
  tmp.truncate( outLen );
  if ( tmp.length() < 4 )
  {
    tmp.append( "000" );
    tmp.truncate( 4 );
  }

  return tmp;
}

QString QgsStringUtils::insertLinks( const QString &string, bool *foundLinks )
{
  QString converted = string;

  // http://alanstorm.com/url_regex_explained
  // note - there's more robust implementations available, but we need one which works within the limitation of QRegExp
  static QRegExp urlRegEx( "(\\b(([\\w-]+://?|www[.])[^\\s()<>]+(?:\\([\\w\\d]+\\)|([^!\"#$%&'()*+,\\-./:;<=>?@[\\\\\\]^_`{|}~\\s]|/))))" );
  static QRegExp protoRegEx( "^(?:f|ht)tps?://" );
  static QRegExp emailRegEx( "([\\w._%+-]+@[\\w.-]+\\.[A-Za-z]+)" );

  int offset = 0;
  bool found = false;
  while ( urlRegEx.indexIn( converted, offset ) != -1 )
  {
    found = true;
    QString url = urlRegEx.cap( 1 );
    QString protoUrl = url;
    if ( protoRegEx.indexIn( protoUrl ) == -1 )
    {
      protoUrl.prepend( "http://" );
    }
    QString anchor = QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( protoUrl.toHtmlEscaped(), url.toHtmlEscaped() );
    converted.replace( urlRegEx.pos( 1 ), url.length(), anchor );
    offset = urlRegEx.pos( 1 ) + anchor.length();
  }
  offset = 0;
  while ( emailRegEx.indexIn( converted, offset ) != -1 )
  {
    found = true;
    QString email = emailRegEx.cap( 1 );
    QString anchor = QStringLiteral( "<a href=\"mailto:%1\">%1</a>" ).arg( email.toHtmlEscaped() );
    converted.replace( emailRegEx.pos( 1 ), email.length(), anchor );
    offset = emailRegEx.pos( 1 ) + anchor.length();
  }

  if ( foundLinks )
    *foundLinks = found;

  return converted;
}

QString QgsStringUtils::wordWrap( const QString &string, const int length, const bool useMaxLineLength, const QString &customDelimiter )
{
  if ( string.isEmpty() || length == 0 )
    return string;

  QString newstr;
  QRegExp rx;
  int delimiterLength = 0;

  if ( !customDelimiter.isEmpty() )
  {
    rx.setPatternSyntax( QRegExp::FixedString );
    rx.setPattern( customDelimiter );
    delimiterLength = customDelimiter.length();
  }
  else
  {
    // \x200B is a ZERO-WIDTH SPACE, needed for worwrap to support a number of complex scripts (Indic, Arabic, etc.)
    rx.setPattern( QStringLiteral( "[\\s\\x200B]" ) );
    delimiterLength = 1;
  }

  const QStringList lines = string.split( '\n' );
  int strLength, strCurrent, strHit, lastHit;

  for ( int i = 0; i < lines.size(); i++ )
  {
    strLength = lines.at( i ).length();
    strCurrent = 0;
    strHit = 0;
    lastHit = 0;

    while ( strCurrent < strLength )
    {
      // positive wrap value = desired maximum line width to wrap
      // negative wrap value = desired minimum line width before wrap
      if ( useMaxLineLength )
      {
        //first try to locate delimiter backwards
        strHit = lines.at( i ).lastIndexOf( rx, strCurrent + length );
        if ( strHit == lastHit || strHit == -1 )
        {
          //if no new backward delimiter found, try to locate forward
          strHit = lines.at( i ).indexOf( rx, strCurrent + std::abs( length ) );
        }
        lastHit = strHit;
      }
      else
      {
        strHit = lines.at( i ).indexOf( rx, strCurrent + std::abs( length ) );
      }
      if ( strHit > -1 )
      {
        newstr.append( lines.at( i ).midRef( strCurrent, strHit - strCurrent ) );
        newstr.append( '\n' );
        strCurrent = strHit + delimiterLength;
      }
      else
      {
        newstr.append( lines.at( i ).midRef( strCurrent ) );
        strCurrent = strLength;
      }
    }
    if ( i < lines.size() - 1 )
      newstr.append( '\n' );
  }

  return newstr;
}

QgsStringReplacement::QgsStringReplacement( const QString &match, const QString &replacement, bool caseSensitive, bool wholeWordOnly )
  : mMatch( match )
  , mReplacement( replacement )
  , mCaseSensitive( caseSensitive )
  , mWholeWordOnly( wholeWordOnly )
{
  if ( mWholeWordOnly )
    mRx = QRegExp( QString( "\\b%1\\b" ).arg( mMatch ),
                   mCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive );
}

QString QgsStringReplacement::process( const QString &input ) const
{
  QString result = input;
  if ( !mWholeWordOnly )
  {
    return result.replace( mMatch, mReplacement, mCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive );
  }
  else
  {
    return result.replace( mRx, mReplacement );
  }
}

QgsStringMap QgsStringReplacement::properties() const
{
  QgsStringMap map;
  map.insert( QStringLiteral( "match" ), mMatch );
  map.insert( QStringLiteral( "replace" ), mReplacement );
  map.insert( QStringLiteral( "caseSensitive" ), mCaseSensitive ? "1" : "0" );
  map.insert( QStringLiteral( "wholeWord" ), mWholeWordOnly ? "1" : "0" );
  return map;
}

QgsStringReplacement QgsStringReplacement::fromProperties( const QgsStringMap &properties )
{
  return QgsStringReplacement( properties.value( QStringLiteral( "match" ) ),
                               properties.value( QStringLiteral( "replace" ) ),
                               properties.value( QStringLiteral( "caseSensitive" ), QStringLiteral( "0" ) ) == QLatin1String( "1" ),
                               properties.value( QStringLiteral( "wholeWord" ), QStringLiteral( "0" ) ) == QLatin1String( "1" ) );
}

QString QgsStringReplacementCollection::process( const QString &input ) const
{
  QString result = input;
  Q_FOREACH ( const QgsStringReplacement &r, mReplacements )
  {
    result = r.process( result );
  }
  return result;
}

void QgsStringReplacementCollection::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  Q_FOREACH ( const QgsStringReplacement &r, mReplacements )
  {
    QgsStringMap props = r.properties();
    QDomElement propEl = doc.createElement( QStringLiteral( "replacement" ) );
    QgsStringMap::const_iterator it = props.constBegin();
    for ( ; it != props.constEnd(); ++it )
    {
      propEl.setAttribute( it.key(), it.value() );
    }
    elem.appendChild( propEl );
  }
}

void QgsStringReplacementCollection::readXml( const QDomElement &elem )
{
  mReplacements.clear();
  QDomNodeList nodelist = elem.elementsByTagName( QStringLiteral( "replacement" ) );
  for ( int i = 0; i < nodelist.count(); i++ )
  {
    QDomElement replacementElem = nodelist.at( i ).toElement();
    QDomNamedNodeMap nodeMap = replacementElem.attributes();

    QgsStringMap props;
    for ( int j = 0; j < nodeMap.count(); ++j )
    {
      props.insert( nodeMap.item( j ).nodeName(), nodeMap.item( j ).nodeValue() );
    }
    mReplacements << QgsStringReplacement::fromProperties( props );
  }

}
