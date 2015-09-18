/***************************************************************************
    qgsrelationreferencefactory.cpp
     --------------------------------------
    Date                 : 29.5.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationreferencefactory.h"

#include "qgsrelationreferencewidgetwrapper.h"
#include "qgsrelationreferenceconfigdlg.h"

QgsRelationReferenceFactory::QgsRelationReferenceFactory( QString name, QgsMapCanvas* canvas, QgsMessageBar* messageBar )
    : QgsEditorWidgetFactory( name )
    , mCanvas( canvas )
    , mMessageBar( messageBar )
{
}

QgsEditorWidgetWrapper* QgsRelationReferenceFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsRelationReferenceWidgetWrapper( vl, fieldIdx, editor, mCanvas, mMessageBar, parent );
}

QgsEditorConfigWidget* QgsRelationReferenceFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsRelationReferenceConfigDlg( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsRelationReferenceFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
  QMap<QString, QVariant> cfg;

  cfg.insert( "AllowNULL", configElement.attribute( "AllowNULL" ) == "1" );
  cfg.insert( "OrderByValue", configElement.attribute( "OrderByValue" ) == "1" );
  cfg.insert( "ShowForm", configElement.attribute( "ShowForm" ) == "1" );
  cfg.insert( "Relation", configElement.attribute( "Relation" ) );
  cfg.insert( "MapIdentification", configElement.attribute( "MapIdentification" ) == "1" );
  cfg.insert( "ReadOnly", configElement.attribute( "ReadOnly" ) == "1" );

  QDomNode filterNode = configElement.elementsByTagName( "FilterFields" ).at( 0 );
  if ( !filterNode.isNull() )
  {
    QStringList filterFields;
    QDomNodeList fieldNodes = filterNode.toElement().elementsByTagName( "field" );
    for ( int i = 0; i < fieldNodes.size(); i++ )
    {
      QDomElement fieldElement = fieldNodes.at( i ).toElement();
      filterFields << fieldElement.attribute( "name" );
    }
    cfg.insert( "FilterFields", filterFields );

    cfg.insert( "ChainFilters", filterNode.toElement().attribute( "ChainFilters" ) == "1" );
  }
  return cfg;
}

void QgsRelationReferenceFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  configElement.setAttribute( "AllowNULL", config["AllowNULL"].toBool() );
  configElement.setAttribute( "OrderByValue", config["OrderByValue"].toBool() );
  configElement.setAttribute( "ShowForm", config["ShowForm"].toBool() );
  configElement.setAttribute( "Relation", config["Relation"].toString() );
  configElement.setAttribute( "MapIdentification", config["MapIdentification"].toBool() );
  configElement.setAttribute( "ReadOnly", config["ReadOnly"].toBool() );

  if ( config.contains( "FilterFields" ) )
  {
    QDomElement filterFields = doc.createElement( "FilterFields" );

    Q_FOREACH ( const QString& field, config["FilterFields"].toStringList() )
    {
      QDomElement fieldElem = doc.createElement( "field" );
      fieldElem.setAttribute( "name", field );
      filterFields.appendChild( fieldElem );
    }
    configElement.appendChild( filterFields );

    filterFields.setAttribute( "ChainFilters", config["ChainFilters"].toBool() );
  }
}

QMap<const char*, int> QgsRelationReferenceFactory::supportedWidgetTypes()
{
  QMap<const char*, int> map = QMap<const char*, int>();
  map.insert( QgsRelationReferenceWidget::staticMetaObject.className(), 10 );
  return map;
}
