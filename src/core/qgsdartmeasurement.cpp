/***************************************************************************
    qgsdartmeasurement.cpp
     --------------------------------------
    Date                 : 8.11.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias dot kuhn at gmx dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdartmeasurement.h"

#include <QDebug>

QgsDartMeasurement::QgsDartMeasurement( const QString& name, Type type, const QString& value )
    : mName( name )
    , mType( type )
    , mValue( value )
{
}

const QString QgsDartMeasurement::toString() const
{
  QString elementName = "DartMeasurement";
  if ( mType == ImagePng )
  {
    elementName = "DartMeasurementFile";
  }

  QString dashMessage = QString( "<%1 name=\"%2\" type=\"%3\">%4</%1>" )
                        .arg( elementName )
                        .arg( mName )
                        .arg( typeToString( mType ) )
                        .arg( mValue );
  return dashMessage;
}

void QgsDartMeasurement::send() const
{
  qDebug() << toString() + "\n";
}

const QString QgsDartMeasurement::typeToString( QgsDartMeasurement::Type type )
{
  QString str;

  switch ( type )
  {
    case Text:
      str = "text/text";
      break;

    case ImagePng:
      str = "image/png";
      break;

    case Integer:
      str = "numeric/integer";
      break;
  }

  return str;
}
