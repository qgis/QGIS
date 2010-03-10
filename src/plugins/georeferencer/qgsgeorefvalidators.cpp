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
/* $Id$ */

#include <QStringList>

#include "qgsgeorefvalidators.h"

QgsDMSAndDDValidator::QgsDMSAndDDValidator( QObject *parent )
    : QValidator( parent )
{
}

QValidator::State QgsDMSAndDDValidator::validate( QString &input, int &pos ) const
{
  Q_UNUSED( pos );

  QRegExp rx( "-?\\d*" );
  if ( rx.exactMatch( input ) )
  {
    return Acceptable;
  }

  if ( input.length() == 4 )
  {
    if ( input.toInt() > 179 )
      return Invalid;
  }
  else if ( input.startsWith( "-" ) && input.length() == 5 )
  {
    if ( input.toInt() <= -180 )
      return Invalid;
  }

  if ( !input.contains( " " ) )
  {
    rx.setPattern( "-?\\d*(\\.|,)(\\d+)?" );
    if ( rx.exactMatch( input ) )
      return Acceptable;
  }
  else
  {
    rx.setPattern( "-?\\d{1,3}\\s(\\d{1,2}(\\s(\\d{1,2}((\\.|,)(\\d{1,3})?)?)?)?)?" );
    if ( rx.exactMatch( input ) )
    {
      rx.setPattern( "-?\\d{1,3}\\s60" );
      if ( rx.exactMatch( input ) )
      {
        int in = input.left( input.indexOf( " " ) ).toInt();
        int grad =  input.startsWith( "-" ) ? in - 1 : in + 1;
        if ( grad <= 180 )
          input = QString::number( grad );

        return Acceptable;
      }

      rx.setPattern( "-?\\d{1,3}\\s\\d{1,2}\\s60" );
      if ( rx.exactMatch( input ) )
      {
        int min = input.split( " " ).at( 1 ).toInt() + 1;
        if ( min <= 60 )
          input = input.left( input.indexOf( " " ) ) + " " + QString::number( min );

        return Acceptable;
      }

      if ( input.at( input.size() - 1 ) == ' ' )
        return Intermediate;

      int pos = input.lastIndexOf( ' ' );
      QString valStr = input.mid( pos + 1, input.size() - 1 );
      int val = valStr.toInt();
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
