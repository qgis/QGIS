/***************************************************************************
    qgsdatetimeeditfactory.cpp
     --------------------------------------
    Date                 : 03.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatetimeeditfactory.h"
#include "qgsdatetimeeditconfig.h"
#include "qgsdatetimeeditwrapper.h"

#include <QSettings>

QgsDateTimeEditFactory::QgsDateTimeEditFactory( QString name ) :
    QgsEditorWidgetFactory( name )
{
}

QgsEditorWidgetWrapper *QgsDateTimeEditFactory::create( QgsVectorLayer *vl, int fieldIdx, QWidget *editor, QWidget *parent ) const
{
  return new QgsDateTimeEditWrapper( vl, fieldIdx, editor, parent );
}

QgsEditorConfigWidget *QgsDateTimeEditFactory::configWidget( QgsVectorLayer *vl, int fieldIdx, QWidget *parent ) const
{
  return new QgsDateTimeEditConfig( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsDateTimeEditFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
  QgsEditorWidgetConfig cfg;

  cfg.insert( "field_format", configElement.attribute( "field_format" ) );
  cfg.insert( "display_format", configElement.attribute( "display_format" ) );
  cfg.insert( "calendar_popup", configElement.attribute( "calendar_popup" ) == "1" );

  return cfg;
}

void QgsDateTimeEditFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  configElement.setAttribute( "field_format", config["field_format"].toString() );
  configElement.setAttribute( "display_format", config["display_format"].toString() );
  configElement.setAttribute( "calendar_popup", config["calendar_popup"].toBool() );
}

QString QgsDateTimeEditFactory::representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( vl )
  Q_UNUSED( fieldIdx )
  Q_UNUSED( cache )

  QString result;

  if ( value.isNull() )
  {
    QSettings settings;
    return settings.value( "qgis/nullValue", "NULL" ).toString();
  }

  const QString displayFormat = config.value( "display_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();
  const QString fieldFormat = config.value( "field_format", QGSDATETIMEEDIT_DATEFORMAT ).toString();

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
