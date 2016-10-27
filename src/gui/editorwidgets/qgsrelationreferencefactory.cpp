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

  cfg.insert( QStringLiteral( "AllowNULL" ), configElement.attribute( QStringLiteral( "AllowNULL" ) ) == QLatin1String( "1" ) );
  cfg.insert( QStringLiteral( "OrderByValue" ), configElement.attribute( QStringLiteral( "OrderByValue" ) ) == QLatin1String( "1" ) );
  cfg.insert( QStringLiteral( "ShowForm" ), configElement.attribute( QStringLiteral( "ShowForm" ) ) == QLatin1String( "1" ) );
  cfg.insert( QStringLiteral( "Relation" ), configElement.attribute( QStringLiteral( "Relation" ) ) );
  cfg.insert( QStringLiteral( "MapIdentification" ), configElement.attribute( QStringLiteral( "MapIdentification" ) ) == QLatin1String( "1" ) );
  cfg.insert( QStringLiteral( "ReadOnly" ), configElement.attribute( QStringLiteral( "ReadOnly" ) ) == QLatin1String( "1" ) );
  cfg.insert( QStringLiteral( "AllowAddFeatures" ), configElement.attribute( QStringLiteral( "AllowAddFeatures" ) ) == QLatin1String( "1" ) );

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

    cfg.insert( QStringLiteral( "ChainFilters" ), filterNode.toElement().attribute( QStringLiteral( "ChainFilters" ) ) == QLatin1String( "1" ) );
  }
  return cfg;
}

void QgsRelationReferenceFactory::writeConfig( const QgsEditorWidgetConfig& config, QDomElement& configElement, QDomDocument& doc, const QgsVectorLayer* layer, int fieldIdx )
{
  Q_UNUSED( doc );
  Q_UNUSED( layer );
  Q_UNUSED( fieldIdx );

  configElement.setAttribute( QStringLiteral( "AllowNULL" ), config[QStringLiteral( "AllowNULL" )].toBool() );
  configElement.setAttribute( QStringLiteral( "OrderByValue" ), config[QStringLiteral( "OrderByValue" )].toBool() );
  configElement.setAttribute( QStringLiteral( "ShowForm" ), config[QStringLiteral( "ShowForm" )].toBool() );
  configElement.setAttribute( QStringLiteral( "Relation" ), config[QStringLiteral( "Relation" )].toString() );
  configElement.setAttribute( QStringLiteral( "MapIdentification" ), config[QStringLiteral( "MapIdentification" )].toBool() );
  configElement.setAttribute( QStringLiteral( "ReadOnly" ), config[QStringLiteral( "ReadOnly" )].toBool() );
  configElement.setAttribute( QStringLiteral( "AllowAddFeatures" ), config[QStringLiteral( "AllowAddFeatures" )].toBool() );

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

    filterFields.setAttribute( QStringLiteral( "ChainFilters" ), config[QStringLiteral( "ChainFilters" )].toBool() );
  }
}

QHash<const char*, int> QgsRelationReferenceFactory::supportedWidgetTypes()
{
  QHash<const char*, int> map = QHash<const char*, int>();
  map.insert( QgsRelationReferenceWidget::staticMetaObject.className(), 10 );
  return map;
}

QString QgsRelationReferenceFactory::representValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  Q_UNUSED( cache );

  // Some sanity checks
  if ( !config.contains( QStringLiteral( "Relation" ) ) )
  {
    QgsDebugMsg( "Missing Relation in configuration" );
    return value.toString();
  }
  QgsRelation relation = QgsProject::instance()->relationManager()->relation( config[QStringLiteral( "Relation" )].toString() );
  if ( !relation.isValid() )
  {
    QgsDebugMsg( "Invalid relation" );
    return value.toString();
  }
  QgsVectorLayer* referencingLayer = relation.referencingLayer();
  if ( vl != referencingLayer )
  {
    QgsDebugMsg( "representValue() with inconsistent vl parameter w.r.t relation referencingLayer" );
    return value.toString();
  }
  int referencingFieldIdx = referencingLayer->fields().lookupField( relation.fieldPairs().at( 0 ).first );
  if ( referencingFieldIdx != fieldIdx )
  {
    QgsDebugMsg( "representValue() with inconsistent fieldIdx parameter w.r.t relation referencingFieldIdx" );
    return value.toString();
  }
  QgsVectorLayer* referencedLayer = relation.referencedLayer();
  if ( !referencedLayer )
  {
    QgsDebugMsg( "Cannot find referenced layer" );
    return value.toString();
  }

  // Attributes from the referencing layer
  QgsAttributes attrs = QgsAttributes( vl->fields().count() );
  // Set the value on the foreign key field of the referencing record
  attrs[ referencingFieldIdx ] = value;

  QgsFeatureRequest request = relation.getReferencedFeatureRequest( attrs );
  QgsFeature feature;
  referencedLayer->getFeatures( request ).nextFeature( feature );
  if ( !feature.isValid() )
    return value.toString();

  QgsExpression expr( referencedLayer->displayExpression() );
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( referencedLayer );
  context.setFeature( feature );
  QString title = expr.evaluate( &context ).toString();
  if ( expr.hasEvalError() )
  {
    int referencedFieldIdx = referencedLayer->fields().lookupField( relation.fieldPairs().at( 0 ).second );
    title = feature.attribute( referencedFieldIdx ).toString();
  }
  return title;
}

QVariant QgsRelationReferenceFactory::sortValue( QgsVectorLayer* vl, int fieldIdx, const QgsEditorWidgetConfig& config, const QVariant& cache, const QVariant& value ) const
{
  return representValue( vl, fieldIdx, config, cache, value );
}
