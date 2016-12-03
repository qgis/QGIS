/***************************************************************************
  qgsdatetimefieldkit.cpp - QgsDateTimeFieldKit

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
#include "qgsdatetimefieldkit.h"

#include <QSettings>

#include "qgsfield.h"
#include "qgsvectorlayer.h"

QgsDateTimeFieldKit::QgsDateTimeFieldKit()
{

}

bool QgsDateTimeFieldKit::supportsField( QgsVectorLayer* layer, int fieldIdx )
{
}

QString QgsDateTimeFieldKit::representValue( QgsVectorLayer* layer, int fieldIdx, const QVariantMap& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( cache )

  QString result;

  if ( value.isNull() )
  {
    QSettings settings;
    return settings.value( QStringLiteral( "qgis/nullValue" ), "NULL" ).toString();
  }

  const QgsField field = layer->fields().at( fieldIdx );
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

QString QgsDateTimeFieldKit::defaultFormat( const QVariant::Type type )
{
  switch ( type )
  {
    case QVariant::DateTime:
      return QGSDATETIMEEDIT_DATETIMEFORMAT;
      break;
    case QVariant::Time:
      return QGSDATETIMEEDIT_TIMEFORMAT;
      break;
    default:
      return QGSDATETIMEEDIT_DATEFORMAT;
  }
}
