/***************************************************************************
    qgsdartmeasurement.cpp
     --------------------------------------
    Date                 : 8.11.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdartmeasurement.h"

#include <QTextStream>

QgsDartMeasurement::QgsDartMeasurement( const QString &name, Type type, const QString &value )
  : mName( name )
  , mType( type )
  , mValue( value )
{
}

const QString QgsDartMeasurement::toString() const
{
  QString elementName = u"DartMeasurement"_s;
  if ( mType == ImagePng )
  {
    elementName = u"DartMeasurementFile"_s;
  }

  QString dashMessage = u"<%1 name=\"%2\" type=\"%3\">%4</%1>"_s
                        .arg( elementName,
                              mName,
                              typeToString( mType ),
                              mValue );
  return dashMessage;
}

void QgsDartMeasurement::send() const
{
  QTextStream out( stdout );
  out << toString() << Qt::endl;
}

const QString QgsDartMeasurement::typeToString( QgsDartMeasurement::Type type )
{
  QString str;

  switch ( type )
  {
    case Text:
      str = u"text/text"_s;
      break;

    case ImagePng:
      str = u"image/png"_s;
      break;

    case Integer:
      str = u"numeric/integer"_s;
      break;
  }

  return str;
}
