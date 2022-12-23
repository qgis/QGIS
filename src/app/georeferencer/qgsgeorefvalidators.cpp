/***************************************************************************
     qgsgeorefvalidators.cpp
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QStringList>
#include <QRegularExpression>

#include "qgsgeorefvalidators.h"

QgsDMSAndDDValidator::QgsDMSAndDDValidator( QObject *parent )
  : QValidator( parent )
{
}

QValidator::State QgsDMSAndDDValidator::validate( QString &input, int &pos ) const
{
  Q_UNUSED( pos )

  const thread_local QRegularExpression rx( QRegularExpression::anchoredPattern( QStringLiteral( "-?\\d*" ) ) );
  if ( rx.match( input ).hasMatch() )
  {
    return Acceptable;
  }

  if ( input.length() == 4 )
  {
    if ( input.toInt() > 179 )
      return Invalid;
  }
  else if ( input.startsWith( '-' ) && input.length() == 5 )
  {
    if ( input.toInt() <= -180 )
      return Invalid;
  }

  if ( !input.contains( ' ' ) )
  {
    const thread_local QRegularExpression rx2( QRegularExpression::anchoredPattern( QStringLiteral( "-?\\d*(\\.|,)(\\d+)?" ) ) );
    if ( rx2.match( input ).hasMatch() )
      return Acceptable;
  }
  else
  {
    const thread_local QRegularExpression rx3( QRegularExpression::anchoredPattern( QStringLiteral( "-?\\d{1,3}\\s(\\d{1,2}(\\s(\\d{1,2}((\\.|,)(\\d{1,3})?)?)?)?)?" ) ) );
    if ( rx3.match( input ).hasMatch() )
    {
      const thread_local QRegularExpression rx4( QRegularExpression::anchoredPattern( QStringLiteral( "-?\\d{1,3}\\s60" ) ) );
      if ( rx4.match( input ).hasMatch() )
      {
        const int in = input.left( input.indexOf( ' ' ) ).toInt();
        const int grad = input.startsWith( '-' ) ? in - 1 : in + 1;
        if ( grad <= 180 )
          input = QString::number( grad );

        return Acceptable;
      }

      const thread_local QRegularExpression rx5( QRegularExpression::anchoredPattern( QStringLiteral( "-?\\d{1,3}\\s\\d{1,2}\\s60" ) ) );
      if ( rx5.match( input ).hasMatch() )
      {
        const int min = input.split( ' ' ).at( 1 ).toInt() + 1;
        if ( min <= 60 )
          input = input.left( input.indexOf( ' ' ) ) + ' ' + QString::number( min );

        return Acceptable;
      }

      if ( input.at( input.size() - 1 ) == ' ' )
        return Intermediate;

      const int pos = input.lastIndexOf( ' ' );
      const QString valStr = input.mid( pos + 1, input.size() - 1 );
      const int val = valStr.toInt();
      if ( val <= 60 )
        return Acceptable;
      else
        return Invalid;
    }
    else
      return Invalid;
  }

  return Invalid;
}
