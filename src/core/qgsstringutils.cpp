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

int QgsStringUtils::levenshteinDistance( const QString& string1, const QString& string2, bool caseSensitive )
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

  const QChar* s1Char = s1.constData();
  const QChar* s2Char = s2.constData();

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
    qSwap( s1, s2 );
    qSwap( length1, length2 );
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
  const QChar* s2start = s2Char;
  for ( int i = 0; i < length1; ++i )
  {
    col[0] = i + 1;
    s2Char = s2start;
    for ( int j = 0; j < length2; ++j )
    {
      col[j + 1] = qMin( qMin( 1 + col[j], 1 + prevCol[1 + j] ), prevCol[j] + (( *s1Char == *s2Char ) ? 0 : 1 ) );
      s2Char++;
    }
    col.swap( prevCol );
    s1Char++;
  }
  return prevCol[length2];
}

QString QgsStringUtils::longestCommonSubstring( const QString& string1, const QString& string2, bool caseSensitive )
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

  int* currentScores = new int [ s2.length()];
  int* previousScores = new int [ s2.length()];
  int maxCommonLength = 0;
  int lastMaxBeginIndex = 0;

  const QChar* s1Char = s1.constData();
  const QChar* s2Char = s2.constData();
  const QChar* s2Start = s2Char;

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
    qSwap( currentScores, previousScores );
    s1Char++;
    s2Char = s2Start;
  }
  delete [] currentScores;
  delete [] previousScores;
  return string1.mid( lastMaxBeginIndex - maxCommonLength + 1, maxCommonLength );
}

int QgsStringUtils::hammingDistance( const QString& string1, const QString& string2, bool caseSensitive )
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
  const QChar* s1Char = s1.constData();
  const QChar* s2Char = s2.constData();

  for ( int i = 0; i < string1.length(); ++i )
  {
    if ( *s1Char != *s2Char )
      distance++;
    s1Char++;
    s2Char++;
  }

  return distance;
}

QString QgsStringUtils::soundex( const QString& string )
{
  if ( string.isEmpty() )
    return QString();

  QString tmp = string.toUpper();

  //strip non character codes, and vowel like characters after the first character
  QChar* char1 = tmp.data();
  QChar* char2 = tmp.data();
  int outLen = 0;
  for ( int i = 0; i < tmp.length(); ++i, ++char2 )
  {
    if (( *char2 ).unicode() >= 0x41 && ( *char2 ).unicode() <= 0x5A && ( i == 0 || (( *char2 ).unicode() != 0x41 && ( *char2 ).unicode() != 0x45
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

  QChar* tmpChar = tmp.data();
  tmpChar++;
  for ( int i = 1; i < tmp.length(); ++i, ++tmpChar )
  {
    switch (( *tmpChar ).unicode() )
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
