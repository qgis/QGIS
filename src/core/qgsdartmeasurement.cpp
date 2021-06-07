/***************************************************************************
    qgsdartmeasurement.cpp
     --------------------------------------
    Date                 : 8.11.2014
    Copyright            : (C) 2014 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
  QString elementName = QStringLiteral( "DartMeasurement" );
  if ( mType == ImagePng )
  {
    elementName = QStringLiteral( "DartMeasurementFile" );
  }

  QString dashMessage = QStringLiteral( "<%1 name=\"%2\" type=\"%3\">%4</%1>" )
                        .arg( elementName,
                              mName,
                              typeToString( mType ),
                              mValue );
  return dashMessage;
}

void QgsDartMeasurement::send() const
{
  QTextStream out( stdout );
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  out << toString() << endl;
#else
  out << toString() << Qt::endl;
#endif
}

const QString QgsDartMeasurement::typeToString( QgsDartMeasurement::Type type )
{
  QString str;

  switch ( type )
  {
    case Text:
      str = QStringLiteral( "text/text" );
      break;

    case ImagePng:
      str = QStringLiteral( "image/png" );
      break;

    case Integer:
      str = QStringLiteral( "numeric/integer" );
      break;
  }

  return str;
}
