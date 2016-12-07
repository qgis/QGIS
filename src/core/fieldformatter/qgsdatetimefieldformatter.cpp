/***************************************************************************
  qgsdatetimefieldformatter.cpp - QgsDateTimeFieldFormatter

 ---------------------
 begin                : 2.12.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdatetimefieldformatter.h"

#include <QSettings>

#include "qgsfield.h"
#include "qgsvectorlayer.h"

const QString QgsDateTimeFieldFormatter::DefaultDateFormat = QStringLiteral( "yyyy-MM-dd" );
const QString QgsDateTimeFieldFormatter::DefaultTimeFormat = QStringLiteral( "HH:mm:ss" );
const QString QgsDateTimeFieldFormatter::DefaultDateTimeFormat = QStringLiteral( "yyyy-MM-dd HH:mm:ss" );


QString QgsDateTimeFieldFormatter::id() const
{
  return QStringLiteral( "DateTime" );
}

QString QgsDateTimeFieldFormatter::representValue( QgsVectorLayer* layer, int fieldIndex, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( cache )

  QString result;

  if ( value.isNull() )
  {
    QSettings settings;
    return QgsApplication::nullRepresentation();
  }

  const QgsField field = layer->fields().at( fieldIndex );
  const QString displayFormat = config.value( QStringLiteral( "display_format" ), defaultFormat( field.type() ) ).toString();
  const QString fieldFormat = config.value( QStringLiteral( "field_format" ), defaultFormat( field.type() ) ).toString();

  QDateTime date = QDateTime::fromString( value.toString(), fieldFormat );

  if ( date.isValid() )
  {
    result = date.toString( displayFormat );
  }
  else
  {
    result = value.toString();
  }

  return result;
}

QString QgsDateTimeFieldFormatter::defaultFormat( const QVariant::Type type )
{
  switch ( type )
  {
    case QVariant::DateTime:
      return QgsDateTimeFieldFormatter::DefaultDateTimeFormat;
      break;
    case QVariant::Time:
      return QgsDateTimeFieldFormatter::DefaultTimeFormat;
      break;
    default:
      return QgsDateTimeFieldFormatter::DefaultDateFormat;
  }
}
