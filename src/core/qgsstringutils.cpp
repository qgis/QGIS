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
#include "qgslogger.h"
#include <QVector>
#include <QStringList>
#include <QTextBoundaryFinder>
#include <QRegularExpression>
#include <cstdlib> // for std::abs

QString QgsStringUtils::capitalize( const QString &string, Qgis::Capitalization capitalization )
{
  if ( string.isEmpty() )
    return QString();

  switch ( capitalization )
  {
    case Qgis::Capitalization::MixedCase:
    case Qgis::Capitalization::SmallCaps:
      return string;

    case Qgis::Capitalization::AllUppercase:
      return string.toUpper();

    case Qgis::Capitalization::AllLowercase:
    case Qgis::Capitalization::AllSmallCaps:
      return string.toLower();

    case Qgis::Capitalization::ForceFirstLetterToCapital:
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

    case Qgis::Capitalization::TitleCase:
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

      const bool allSameCase = string.toLower() == string || string.toUpper() == string;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      const QStringList parts = ( allSameCase ? string.toLower() : string ).split( splitWords, QString::SkipEmptyParts );
#else
      const QStringList parts = ( allSameCase ? string.toLower() : string ).split( splitWords, Qt::SkipEmptyParts );
#endif
      QString result;
      bool firstWord = true;
      int i = 0;
      int lastWord = parts.count() - 1;
      for ( const QString &word : std::as_const( parts ) )
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

    case Qgis::Capitalization::UpperCamelCase:
      QString result = QgsStringUtils::capitalize( string.toLower(), Qgis::Capitalization::ForceFirstLetterToCapital ).simplified();
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
      encoded += QLatin1String( "&amp;" );
    else if ( ch.unicode() == 60 )
      encoded += QLatin1String( "&lt;" );
    else if ( ch.unicode() == 62 )
      encoded += QLatin1String( "&gt;" );
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


double QgsStringUtils::fuzzyScore( const QString &candidate, const QString &search )
{
  QString candidateNormalized = candidate.simplified().normalized( QString:: NormalizationForm_C ).toLower();
  QString searchNormalized = search.simplified().normalized( QString:: NormalizationForm_C ).toLower();

  int candidateLength = candidateNormalized.length();
  int searchLength = searchNormalized.length();
  int score = 0;

  // if the candidate and the search term are empty, no other option than 0 score
  if ( candidateLength == 0 || searchLength == 0 )
    return score;

  int candidateIdx = 0;
  int searchIdx = 0;
  // there is always at least one word
  int maxScore = FUZZY_SCORE_WORD_MATCH;

  bool isPreviousIndexMatching = false;
  bool isWordOpen = true;

  // loop trough each candidate char and calculate the potential max score
  while ( candidateIdx < candidateLength )
  {
    QChar candidateChar = candidateNormalized[ candidateIdx++ ];
    bool isCandidateCharWordEnd = candidateChar == ' ' || candidateChar.isPunct();

    // the first char is always the default score
    if ( candidateIdx == 1 )
      maxScore += FUZZY_SCORE_NEW_MATCH;
    // every space character or underscore is a opportunity for a new word
    else if ( isCandidateCharWordEnd )
      maxScore += FUZZY_SCORE_WORD_MATCH;
    // potentially we can match every other character
    else
      maxScore += FUZZY_SCORE_CONSECUTIVE_MATCH;

    // we looped through all the characters
    if ( searchIdx >= searchLength )
      continue;

    QChar searchChar = searchNormalized[ searchIdx ];
    bool isSearchCharWordEnd = searchChar == ' ' || searchChar.isPunct();

    // match!
    if ( candidateChar == searchChar || ( isCandidateCharWordEnd && isSearchCharWordEnd ) )
    {
      searchIdx++;

      // if we have just successfully finished a word, give higher score
      if ( isSearchCharWordEnd )
      {
        if ( isWordOpen )
          score += FUZZY_SCORE_WORD_MATCH;
        else if ( isPreviousIndexMatching )
          score += FUZZY_SCORE_CONSECUTIVE_MATCH;
        else
          score += FUZZY_SCORE_NEW_MATCH;

        isWordOpen = true;
      }
      // if we have consecutive characters matching, give higher score
      else if ( isPreviousIndexMatching )
      {
        score += FUZZY_SCORE_CONSECUTIVE_MATCH;
      }
      // normal score for new independent character that matches
      else
      {
        score += FUZZY_SCORE_NEW_MATCH;
      }

      isPreviousIndexMatching = true;
    }
    // if the current character does NOT match, we are sure we cannot build a word for now
    else
    {
      isPreviousIndexMatching = false;
      isWordOpen = false;
    }

    // if the search string is covered, check if the last match is end of word
    if ( searchIdx >= searchLength )
    {
      bool isEndOfWord = ( candidateIdx >= candidateLength )
                         ? true
                         : candidateNormalized[candidateIdx] == ' ' || candidateNormalized[candidateIdx].isPunct();

      if ( isEndOfWord )
        score += FUZZY_SCORE_WORD_MATCH;
    }

    // QgsLogger::debug( QStringLiteral( "TMP: %1 | %2 | %3 | %4 | %5" ).arg( candidateChar, searchChar, QString::number(score), QString::number(isCandidateCharWordEnd), QString::number(isSearchCharWordEnd) ) + QStringLiteral( __FILE__ ) );
  }

  // QgsLogger::debug( QStringLiteral( "RES: %1 | %2" ).arg( QString::number(maxScore),  QString::number(score) ) + QStringLiteral( __FILE__ ) );
  // we didn't loop through all the search chars, it means, that they are not present in the current candidate
  if ( searchIdx < searchLength )
    score = 0;

  return static_cast<float>( std::max( score, 0 ) ) / std::max( maxScore, 1 );
}


QString QgsStringUtils::insertLinks( const QString &string, bool *foundLinks )
{
  QString converted = string;

  // http://alanstorm.com/url_regex_explained
  // note - there's more robust implementations available
  const thread_local QRegularExpression urlRegEx( QStringLiteral( "(\\b(([\\w-]+://?|www[.])[^\\s()<>]+(?:\\([\\w\\d]+\\)|([^!\"#$%&'()*+,\\-./:;<=>?@[\\\\\\]^_`{|}~\\s]|/))))" ) );
  const thread_local QRegularExpression protoRegEx( QStringLiteral( "^(?:f|ht)tps?://|file://" ) );
  const thread_local QRegularExpression emailRegEx( QStringLiteral( "([\\w._%+-]+@[\\w.-]+\\.[A-Za-z]+)" ) );

  int offset = 0;
  bool found = false;
  QRegularExpressionMatch match = urlRegEx.match( converted );
  while ( match.hasMatch() )
  {
    found = true;
    QString url = match.captured( 1 );
    QString protoUrl = url;
    if ( !protoRegEx.match( protoUrl ).hasMatch() )
    {
      protoUrl.prepend( "http://" );
    }
    QString anchor = QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( protoUrl.toHtmlEscaped(), url.toHtmlEscaped() );
    converted.replace( match.capturedStart( 1 ), url.length(), anchor );
    offset = match.capturedStart( 1 ) + anchor.length();
    match = urlRegEx.match( converted, offset );
  }

  offset = 0;
  match = emailRegEx.match( converted );
  while ( match.hasMatch() )
  {
    found = true;
    QString email = match.captured( 1 );
    QString anchor = QStringLiteral( "<a href=\"mailto:%1\">%1</a>" ).arg( email.toHtmlEscaped() );
    converted.replace( match.capturedStart( 1 ), email.length(), anchor );
    offset = match.capturedStart( 1 ) + anchor.length();
    match = emailRegEx.match( converted, offset );
  }

  if ( foundLinks )
    *foundLinks = found;

  return converted;
}

bool QgsStringUtils::isUrl( const QString &string )
{
  const thread_local QRegularExpression rxUrl( QStringLiteral( "^(http|https|ftp|file)://\\S+$" ) );
  return rxUrl.match( string ).hasMatch();
}

QString QgsStringUtils::htmlToMarkdown( const QString &html )
{
  // Any changes in this function must be copied to qgscrashreport.cpp too
  QString converted = html;
  converted.replace( QLatin1String( "<br>" ), QLatin1String( "\n" ) );
  converted.replace( QLatin1String( "<b>" ), QLatin1String( "**" ) );
  converted.replace( QLatin1String( "</b>" ), QLatin1String( "**" ) );
  converted.replace( QLatin1String( "<pre>" ), QLatin1String( "\n```\n" ) );
  converted.replace( QLatin1String( "</pre>" ), QLatin1String( "```\n" ) );

  const thread_local QRegularExpression hrefRegEx( QStringLiteral( "<a\\s+href\\s*=\\s*([^<>]*)\\s*>([^<>]*)</a>" ) );

  int offset = 0;
  QRegularExpressionMatch match = hrefRegEx.match( converted );
  while ( match.hasMatch() )
  {
    QString url = match.captured( 1 ).replace( QLatin1String( "\"" ), QString() );
    url.replace( '\'', QString() );
    QString name = match.captured( 2 );
    QString anchor = QStringLiteral( "[%1](%2)" ).arg( name, url );
    converted.replace( match.capturedStart(), match.capturedLength(), anchor );
    offset = match.capturedStart() + anchor.length();
    match = hrefRegEx.match( converted, offset );
  }

  return converted;
}

QString QgsStringUtils::wordWrap( const QString &string, const int length, const bool useMaxLineLength, const QString &customDelimiter )
{
  if ( string.isEmpty() || length == 0 )
    return string;

  QString newstr;
  QRegularExpression rx;
  int delimiterLength = 0;

  if ( !customDelimiter.isEmpty() )
  {
    rx.setPattern( QRegularExpression::escape( customDelimiter ) );
    delimiterLength = customDelimiter.length();
  }
  else
  {
    // \x{200B} is a ZERO-WIDTH SPACE, needed for worwrap to support a number of complex scripts (Indic, Arabic, etc.)
    rx.setPattern( QStringLiteral( "[\\x{200B}\\s]" ) );
    delimiterLength = 1;
  }

  const QStringList lines = string.split( '\n' );
  int strLength, strCurrent, strHit, lastHit;

  for ( int i = 0; i < lines.size(); i++ )
  {
    const QString line = lines.at( i );
    strLength = line.length();
    if ( strLength <= length )
    {
      // shortcut, no wrapping required
      newstr.append( line );
      if ( i < lines.size() - 1 )
        newstr.append( '\n' );
      continue;
    }
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
        strHit = ( strCurrent + length >= strLength ) ? -1 : line.lastIndexOf( rx, strCurrent + length );
        if ( strHit == lastHit || strHit == -1 )
        {
          //if no new backward delimiter found, try to locate forward
          strHit = ( strCurrent + std::abs( length ) >= strLength ) ? -1 : line.indexOf( rx, strCurrent + std::abs( length ) );
        }
        lastHit = strHit;
      }
      else
      {
        strHit = ( strCurrent + std::abs( length ) >= strLength ) ? -1 : line.indexOf( rx, strCurrent + std::abs( length ) );
      }
      if ( strHit > -1 )
      {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
        newstr.append( line.midRef( strCurrent, strHit - strCurrent ) );
#else
        newstr.append( QStringView {line} .mid( strCurrent, strHit - strCurrent ) );
#endif
        newstr.append( '\n' );
        strCurrent = strHit + delimiterLength;
      }
      else
      {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 2)
        newstr.append( line.midRef( strCurrent ) );
#else
        newstr.append( QStringView {line} .mid( strCurrent ) );
#endif
        strCurrent = strLength;
      }
    }
    if ( i < lines.size() - 1 )
      newstr.append( '\n' );
  }

  return newstr;
}

QString QgsStringUtils::substituteVerticalCharacters( QString string )
{
  string = string.replace( ',', QChar( 65040 ) ).replace( QChar( 8229 ), QChar( 65072 ) ); // comma & two-dot leader
  string = string.replace( QChar( 12289 ), QChar( 65041 ) ).replace( QChar( 12290 ), QChar( 65042 ) ); // ideographic comma & full stop
  string = string.replace( ':', QChar( 65043 ) ).replace( ';', QChar( 65044 ) );
  string = string.replace( '!', QChar( 65045 ) ).replace( '?', QChar( 65046 ) );
  string = string.replace( QChar( 12310 ), QChar( 65047 ) ).replace( QChar( 12311 ), QChar( 65048 ) ); // white lenticular brackets
  string = string.replace( QChar( 8230 ), QChar( 65049 ) ); // three-dot ellipse
  string = string.replace( QChar( 8212 ), QChar( 65073 ) ).replace( QChar( 8211 ), QChar( 65074 ) ); // em & en dash
  string = string.replace( '_', QChar( 65075 ) ).replace( QChar( 65103 ), QChar( 65076 ) ); // low line & wavy low line
  string = string.replace( '(', QChar( 65077 ) ).replace( ')', QChar( 65078 ) );
  string = string.replace( '{', QChar( 65079 ) ).replace( '}', QChar( 65080 ) );
  string = string.replace( '<', QChar( 65087 ) ).replace( '>', QChar( 65088 ) );
  string = string.replace( '[', QChar( 65095 ) ).replace( ']', QChar( 65096 ) );
  string = string.replace( QChar( 12308 ), QChar( 65081 ) ).replace( QChar( 12309 ), QChar( 65082 ) );   // tortoise shell brackets
  string = string.replace( QChar( 12304 ), QChar( 65083 ) ).replace( QChar( 12305 ), QChar( 65084 ) );   // black lenticular brackets
  string = string.replace( QChar( 12298 ), QChar( 65085 ) ).replace( QChar( 12299 ), QChar( 65086 ) ); // double angle brackets
  string = string.replace( QChar( 12300 ), QChar( 65089 ) ).replace( QChar( 12301 ), QChar( 65090 ) );   // corner brackets
  string = string.replace( QChar( 12302 ), QChar( 65091 ) ).replace( QChar( 12303 ), QChar( 65092 ) );   // white corner brackets
  return string;
}

QString QgsStringUtils::qRegExpEscape( const QString &string )
{
  // code and logic taken from the Qt source code
  const QLatin1Char backslash( '\\' );
  const int count = string.count();

  QString escaped;
  escaped.reserve( count * 2 );
  for ( int i = 0; i < count; i++ )
  {
    switch ( string.at( i ).toLatin1() )
    {
      case '$':
      case '(':
      case ')':
      case '*':
      case '+':
      case '.':
      case '?':
      case '[':
      case '\\':
      case ']':
      case '^':
      case '{':
      case '|':
      case '}':
        escaped.append( backslash );
    }
    escaped.append( string.at( i ) );
  }
  return escaped;
}

QString QgsStringUtils::truncateMiddleOfString( const QString &string, int maxLength )
{
  const int charactersToTruncate = string.length() - maxLength;
  if ( charactersToTruncate <= 0 )
    return string;

  // note we actually truncate an extra character, as we'll be replacing it with the ... character
  const int truncateFrom = string.length() / 2 - ( charactersToTruncate + 1 ) / 2;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  return string.leftRef( truncateFrom ) + QString( QChar( 0x2026 ) ) + string.midRef( truncateFrom + charactersToTruncate + 1 );
#else
  return QStringView( string ).first( truncateFrom ) + QString( QChar( 0x2026 ) ) + QStringView( string ).sliced( truncateFrom + charactersToTruncate + 1 );
#endif
}

QgsStringReplacement::QgsStringReplacement( const QString &match, const QString &replacement, bool caseSensitive, bool wholeWordOnly )
  : mMatch( match )
  , mReplacement( replacement )
  , mCaseSensitive( caseSensitive )
  , mWholeWordOnly( wholeWordOnly )
{
  if ( mWholeWordOnly )
  {
    mRx.setPattern( QStringLiteral( "\\b%1\\b" ).arg( mMatch ) );
    mRx.setPatternOptions( mCaseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption );
  }
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
  map.insert( QStringLiteral( "caseSensitive" ), mCaseSensitive ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  map.insert( QStringLiteral( "wholeWord" ), mWholeWordOnly ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
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
  for ( const QgsStringReplacement &r : mReplacements )
  {
    result = r.process( result );
  }
  return result;
}

void QgsStringReplacementCollection::writeXml( QDomElement &elem, QDomDocument &doc ) const
{
  for ( const QgsStringReplacement &r : mReplacements )
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
