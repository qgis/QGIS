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

#include "qgsapplication.h"
#include "qgsfield.h"
#include "qgssettings.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

const QString QgsDateTimeFieldFormatter::DATE_FORMAT = u"yyyy-MM-dd"_s;
const QString QgsDateTimeFieldFormatter::TIME_FORMAT = u"HH:mm:ss"_s;
const QString QgsDateTimeFieldFormatter::DATETIME_FORMAT = u"yyyy-MM-dd HH:mm:ss"_s;
// we need to use Qt::ISODate rather than a string format definition in QDate::fromString
const QString QgsDateTimeFieldFormatter::QT_ISO_FORMAT = u"Qt ISO Date"_s;
// but QDateTimeEdit::setDisplayFormat only accepts string formats, so use with time zone by default
const QString QgsDateTimeFieldFormatter::DISPLAY_FOR_ISO_FORMAT = u"yyyy-MM-dd HH:mm:ss+t"_s;
QString QgsDateTimeFieldFormatter::DATE_DISPLAY_FORMAT = u"yyyy-MM-dd"_s;
QString QgsDateTimeFieldFormatter::DATETIME_DISPLAY_FORMAT = u"yyyy-MM-dd HH:mm:ss"_s;


QString QgsDateTimeFieldFormatter::id() const
{
  return u"DateTime"_s;
}

QString QgsDateTimeFieldFormatter::representValue( QgsVectorLayer *layer, int fieldIndex, const QVariantMap &config, const QVariant &cache, const QVariant &value ) const
{
  Q_UNUSED( cache )

  QString result;

  if ( QgsVariantUtils::isNull( value ) )
  {
    return QgsApplication::nullRepresentation();
  }

  if ( fieldIndex < 0 || fieldIndex >= layer->fields().size() )
  {
    return value.toString();
  }

  const QgsField field = layer->fields().at( fieldIndex );
  const bool fieldIsoFormat = config.value( u"field_iso_format"_s, false ).toBool();
  const QString fieldFormat = config.value( u"field_format"_s, defaultFormat( field.type() ) ).toString();
  const QString displayFormat = config.value( u"display_format"_s, defaultDisplayFormat( field.type() ) ).toString();

  QDateTime date;
  bool showTimeZone = false;
  if ( static_cast<QMetaType::Type>( value.userType() ) == QMetaType::QDate )
  {
    date = value.toDateTime();
  }
  else if ( static_cast<QMetaType::Type>( value.userType() ) == QMetaType::QDateTime )
  {
    date = value.toDateTime();
    // we always show time zones for datetime values
    showTimeZone = true;
  }
  else if ( static_cast<QMetaType::Type>( value.userType() ) == QMetaType::QTime )
  {
    return  value.toTime().toString( displayFormat );
  }
  else
  {
    if ( fieldIsoFormat )
    {
      date = QDateTime::fromString( value.toString(), Qt::ISODate );
    }
    else
    {
      date = QDateTime::fromString( value.toString(), fieldFormat );
    }
  }

  if ( date.isValid() )
  {
    if ( showTimeZone && displayFormat == QgsDateTimeFieldFormatter::DATETIME_DISPLAY_FORMAT )
    {
      // using default display format for datetimes, so ensure we include the timezone
      result = u"%1 (%2)"_s.arg( date.toString( displayFormat ), date.timeZoneAbbreviation() );
    }
    else
    {
      // Convert to UTC if the format string includes a Z, as QLocale::toString() doesn't do it
      if ( displayFormat.indexOf( "Z" ) > 0 )
        date = date.toUTC();

      result = date.toString( displayFormat );
    }
  }
  else
  {
    result = value.toString();
  }

  return result;
}

QString QgsDateTimeFieldFormatter::defaultFormat( QMetaType::Type type )
{
  switch ( type )
  {
    case QMetaType::Type::QDateTime:
      return QgsDateTimeFieldFormatter::DATETIME_FORMAT;
    case QMetaType::Type::QTime:
      return QgsDateTimeFieldFormatter::TIME_FORMAT;
    default:
      return QgsDateTimeFieldFormatter::DATE_FORMAT;
  }
}

QString QgsDateTimeFieldFormatter::defaultFormat( QVariant::Type type )
{
  return defaultFormat( QgsVariantUtils::variantTypeToMetaType( type ) );
}


QString QgsDateTimeFieldFormatter::defaultDisplayFormat( QMetaType::Type type )
{
  switch ( type )
  {
    case QMetaType::Type::QDateTime:
      return QgsDateTimeFieldFormatter::DATETIME_DISPLAY_FORMAT;
    case QMetaType::Type::QTime:
      return QgsDateTimeFieldFormatter::TIME_FORMAT;
    default:
      return QgsDateTimeFieldFormatter::DATE_DISPLAY_FORMAT;
  }
}

QString QgsDateTimeFieldFormatter::defaultDisplayFormat( QVariant::Type type )
{
  return defaultDisplayFormat( QgsVariantUtils::variantTypeToMetaType( type ) );
}

void QgsDateTimeFieldFormatter::applyLocaleChange()
{
  QString dateFormat = QLocale().dateFormat( QLocale::FormatType::ShortFormat );
  QgsDateTimeFieldFormatter::DATETIME_DISPLAY_FORMAT = QString( "%1 %2" ).arg( dateFormat, QgsDateTimeFieldFormatter::TIME_FORMAT );
  QgsDateTimeFieldFormatter::DATE_DISPLAY_FORMAT = dateFormat;
}
