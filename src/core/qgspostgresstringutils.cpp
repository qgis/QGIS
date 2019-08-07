/***************************************************************************
                                  qgspostgresstringutils.cpp
                              ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspostgresstringutils.h"
#include "qgsmessagelog.h"
#include <QDebug>
#include <nlohmann/json.hpp>

using namespace nlohmann;

static void jumpSpace( const QString &txt, int &i )
{
  while ( i < txt.length() && txt.at( i ).isSpace() )
    ++i;
}

QString QgsPostgresStringUtils::getNextString( const QString &txt, int &i, const QString &sep )
{
  jumpSpace( txt, i );
  QString cur = txt.mid( i );
  if ( cur.startsWith( '"' ) )
  {
    QRegExp stringRe( "^\"((?:\\\\.|[^\"\\\\])*)\".*" );
    if ( !stringRe.exactMatch( cur ) )
    {
      QgsMessageLog::logMessage( QObject::tr( "Cannot find end of double quoted string: %1" ).arg( txt ), QObject::tr( "PostgresStringUtils" ) );
      return QString();
    }
    i += stringRe.cap( 1 ).length() + 2;
    jumpSpace( txt, i );
    if ( !txt.midRef( i ).startsWith( sep ) && i < txt.length() )
    {
      QgsMessageLog::logMessage( QObject::tr( "Cannot find separator: %1" ).arg( txt.mid( i ) ), QObject::tr( "PostgresStringUtils" ) );
      return QString();
    }
    i += sep.length();
    return stringRe.cap( 1 ).replace( QLatin1String( "\\\"" ), QLatin1String( "\"" ) ).replace( QLatin1String( "\\\\" ), QLatin1String( "\\" ) );
  }
  else
  {
    int sepPos = cur.indexOf( sep );
    if ( sepPos < 0 )
    {
      i += cur.length();
      return cur.trimmed();
    }
    i += sepPos + sep.length();
    return cur.left( sepPos ).trimmed();
  }
}

QVariantList QgsPostgresStringUtils::parseArray( const QString &string )
{
  QVariantList variantList;

  //it's a postgres array
  QString newVal = string.mid( 1, string.length() - 2 );

  if ( newVal.trimmed().startsWith( '{' ) )
  {
    //it's a multidimensional array
    QStringList values;
    QString subarray = newVal;
    while ( !subarray.isEmpty() )
    {
      bool escaped = false;
      int openedBrackets = 1;
      int i = 0;
      while ( i < subarray.length()  && openedBrackets > 0 )
      {
        ++i;

        if ( subarray.at( i ) == '}' && !escaped ) openedBrackets--;
        else if ( subarray.at( i ) == '{' && !escaped ) openedBrackets++;

        escaped = !escaped ? subarray.at( i ) == '\\' : false;
      }

      variantList.append( subarray.left( ++i ) );
      i = subarray.indexOf( ',', i );
      i = i > 0 ? subarray.indexOf( '{', i ) : -1;
      if ( i == -1 )
        break;

      subarray = subarray.mid( i );
    }
  }
  else
  {
    int i = 0;
    while ( i < newVal.length() )
    {
      const QString value = getNextString( newVal, i, QStringLiteral( "," ) );
      if ( value.isNull() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error parsing PG like array: %1" ).arg( newVal ), QObject::tr( "PostgresStringUtils" ) );
        break;
      }
      variantList.append( value );
    }
  }

  return variantList;

}

QString QgsPostgresStringUtils::buildArray( const QVariantList &list )
{
  QStringList sl;
  for ( const QVariant &v : qgis::as_const( list ) )
  {
    // Convert to proper type
    switch ( v.type() )
    {
      case QVariant::Type::Int:
      case QVariant::Type::LongLong:
        sl.push_back( v.toString() );
        break;
      default:
        QString newS = v.toString();
        if ( newS.startsWith( '{' ) )
        {
          sl.push_back( newS );
        }
        else
        {
          newS.replace( '\\', QStringLiteral( R"(\\)" ) );
          newS.replace( '\"', QStringLiteral( R"(\")" ) );
          sl.push_back( "\"" + newS + "\"" );
        }
        break;
    }
  }
  //store as a formatted string because the fields supports only string
  QString s = sl.join( ',' ).prepend( '{' ).append( '}' );

  return s;
}
