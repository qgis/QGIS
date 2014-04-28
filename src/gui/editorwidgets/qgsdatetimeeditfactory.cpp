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

QgsEditorWidgetConfig QgsDateTimeEditFactory::readConfig( const QDomElement &configElement, QgsVectorLayer *layer, int fieldIdx )
{
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
  QMap<QString, QVariant> cfg;

  cfg.insert( "type", configElement.attribute( "type" ) );
  cfg.insert( "format", configElement.attribute( "format" ) );
  cfg.insert( "calendar_popup", configElement.attribute( "calendar_popup" ) == "1" );

  return cfg;
}

void QgsDateTimeEditFactory::writeConfig( const QgsEditorWidgetConfig &config, QDomElement &configElement, const QDomDocument &doc, const QgsVectorLayer *layer, int fieldIdx )
{
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  configElement.setAttribute( "type", config["type"].toString() );
  configElement.setAttribute( "display_format", config["display_format"].toString() );
  configElement.setAttribute( "format", config["format"].toString() );
  configElement.setAttribute( "calendar_popup", config["calendar_popup"].toBool() );
}
