/***************************************************************************
                                  qgshstoreutils.h
                              ---------------------
    begin                : Sept 2018
    copyright            : (C) 2018 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshstoreutils.h"

#include <QRegularExpression>

QVariantMap QgsHstoreUtils::parse( const QString &string )
{
  QVariantMap map;
  QList<QString> bits;
  static const QList<QString > sSeps{ "=>", "," };

  int i = 0;
  while ( i < string.length() )
  {
    while ( i < string.length() && string.at( i ).isSpace() )
      ++i;
    QString current = string.mid( i );
    QString sep = sSeps.at( bits.length() );
    if ( current.startsWith( '"' ) )
    {
      QRegularExpression re( QStringLiteral( "^\"((?:\\\\.|[^\"\\\\])*)\".*" ) );
      QRegularExpressionMatch match = re.match( current );
      bits << QString();
      if ( match.hasMatch() )
      {
        bits[bits.length() - 1] = match.captured( 1 ).replace( QLatin1String( "\\\"" ), QLatin1String( "\"" ) ).replace( QLatin1String( "\\\\" ), QLatin1String( "\\" ) );
        i += match.captured( 1 ).length() + 2;
        while ( i < string.length() && string.at( i ).isSpace() )
          ++i;

        if ( string.midRef( i ).startsWith( sep ) )
        {
          i += sep.length();
        }
        else if ( i < string.length() )
        {
          // hstore string format broken, end construction
          i += current.length();
        }
      }
      else
      {
        // hstore string format broken, end construction
        i += current.length();
        bits[bits.length() - 1] = current.trimmed();
      }
    }
    else
    {
      int sepPos = current.indexOf( sep );
      if ( sepPos < 0 )
      {
        i += current.length();
        bits << current.trimmed();
      }
      else
      {
        i += sepPos + sep.length();
        bits << current.left( sepPos ).trimmed();
      }
    }

    if ( bits.length() == 2 )
    {
      if ( !bits.at( 0 ).isEmpty() && !bits.at( 1 ).isEmpty() )
        map[ bits.at( 0 ) ] = bits.at( 1 );
      bits.clear();
    }
  }

  return map;
}
