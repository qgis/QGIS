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

#include "qgsproject.h"
#include "qgsrelationreferencefactory.h"

#include "qgsfeatureiterator.h"
#include "qgsrelation.h"
#include "qgsrelationmanager.h"
#include "qgsrelationreferencewidgetwrapper.h"
#include "qgsrelationreferenceconfigdlg.h"
#include "qgsrelationreferencesearchwidgetwrapper.h"
#include "qgsrelationreferencewidget.h"
#include "qgslogger.h"

QgsRelationReferenceFactory::QgsRelationReferenceFactory( const QString& name, QgsMapCanvas* canvas, QgsMessageBar* messageBar )
    : QgsEditorWidgetFactory( name )
    , mCanvas( canvas )
    , mMessageBar( messageBar )
{
}

QgsEditorWidgetWrapper* QgsRelationReferenceFactory::create( QgsVectorLayer* vl, int fieldIdx, QWidget* editor, QWidget* parent ) const
{
  return new QgsRelationReferenceWidgetWrapper( vl, fieldIdx, editor, mCanvas, mMessageBar, parent );
}

QgsSearchWidgetWrapper*QgsRelationReferenceFactory::createSearchWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsRelationReferenceSearchWidgetWrapper( vl, fieldIdx, mCanvas, parent );
}

QgsEditorConfigWidget* QgsRelationReferenceFactory::configWidget( QgsVectorLayer* vl, int fieldIdx, QWidget* parent ) const
{
  return new QgsRelationReferenceConfigDlg( vl, fieldIdx, parent );
}

QgsEditorWidgetConfig QgsRelationReferenceFactory::readConfig( const QDomElement& configElement, QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );
  QgsEditorWidgetConfig cfg;

  xml2config( configElement, cfg, QStringLiteral( "AllowNULL" ) );
  xml2config( configElement, cfg, QStringLiteral( "OrderByValue" ) );
  xml2config( configElement, cfg, QStringLiteral( "ShowForm" ) );
  xml2config( configElement, cfg, QStringLiteral( "Relation" ) );
  xml2config( configElement, cfg, QStringLiteral( "MapIdentification" ) );
  xml2config( configElement, cfg, QStringLiteral( "ReadOnly" ) );
  xml2config( configElement, cfg, QStringLiteral( "AllowAddFeatures" ) );

  QDomNode filterNode = configElement.elementsByTagName( QStringLiteral( "FilterFields" ) ).at( 0 );
  if ( !filterNode.isNull() )
  {
    QStringList filterFields;
    QDomNodeList fieldNodes = filterNode.toElement().elementsByTagName( QStringLiteral( "field" ) );
    filterFields.reserve( fieldNodes.size() );
    for ( int i = 0; i < fieldNodes.size(); i++ )
    {
      QDomElement fieldElement = fieldNodes.at( i ).toElement();
      filterFields << fieldElement.attribute( QStringLiteral( "name" ) );
    }
    cfg.insert( QStringLiteral( "FilterFields" ), filterFields );

    xml2config( configElement, cfg , QStringLiteral( "ChainFilters" ) );
  }
  return cfg;
}

void QgsRelationReferenceFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  config2xml( config, configElement, QStringLiteral( "AllowNULL" ) );
  config2xml( config, configElement, QStringLiteral( "OrderByValue" ) );
  config2xml( config, configElement, QStringLiteral( "ShowForm" ) );
  config2xml( config, configElement, QStringLiteral( "Relation" ) );
  config2xml( config, configElement, QStringLiteral( "MapIdentification" ) );
  config2xml( config, configElement, QStringLiteral( "ReadOnly" ) );
  config2xml( config, configElement, QStringLiteral( "AllowAddFeatures" ) );

  if ( config.contains( QStringLiteral( "FilterFields" ) ) )
  {
    QDomElement filterFields = doc.createElement( QStringLiteral( "FilterFields" ) );

    Q_FOREACH ( const QString& field, config["FilterFields"].toStringList() )
    {
      QDomElement fieldElem = doc.createElement( QStringLiteral( "field" ) );
      fieldElem.setAttribute( QStringLiteral( "name" ), field );
      filterFields.appendChild( fieldElem );
    }
    configElement.appendChild( filterFields );

    config2xml( config, configElement, QStringLiteral( "ChainFilters" ) );
  }
}

QHash<const char*, int> QgsRelationReferenceFactory::supportedWidgetTypes()
{
  QHash<const char*, int> map = QHash<const char*, int>();
  map.insert( QgsRelationReferenceWidget::staticMetaObject.className(), 10 );
  return map;
}

unsigned int QgsRelationReferenceFactory::fieldScore( const QgsVectorLayer* vl, int fieldIdx ) const
{
  const QList<QgsRelation> relations = vl->referencingRelations( fieldIdx );
  return !relations.isEmpty() ? 21 /*A bit stronger than the range widget*/ : 5;
}
