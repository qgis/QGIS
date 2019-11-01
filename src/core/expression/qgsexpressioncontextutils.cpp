/***************************************************************************
     qgsexpressioncontextutils.cpp
     ------------------------
    Date                 : April 2015
    Copyright            : (C) 2015 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsexpressioncontextutils.h"
#include "qgsapplication.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsexpression.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingalgorithm.h"
#include "qgsmapsettings.h"
#include "qgssymbollayerutils.h"
#include "qgslayout.h"
#include "qgslayoutitem.h"
#include "qgsexpressionutils.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutatlas.h"
#include "qgslayoutmultiframe.h"

QgsExpressionContextScope *QgsExpressionContextUtils::globalScope()
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Global" ) );

  QVariantMap customVariables = QgsApplication::customVariables();

  for ( QVariantMap::const_iterator it = customVariables.constBegin(); it != customVariables.constEnd(); ++it )
  {
    scope->setVariable( it.key(), it.value(), true );
  }

  //add some extra global variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "qgis_version" ), Qgis::version(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "qgis_version_no" ), Qgis::versionInt(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "qgis_short_version" ), QStringLiteral( "%1.%2" ).arg( Qgis::versionInt() / 10000 ).arg( Qgis::versionInt() / 100 % 100 ), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "qgis_release_name" ), Qgis::releaseName(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "qgis_platform" ), QgsApplication::platform(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "qgis_os_name" ), QgsApplication::osName(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "qgis_locale" ), QgsApplication::locale(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "user_account_name" ), QgsApplication::userLoginName(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "user_full_name" ), QgsApplication::userFullName(), true, true ) );

  return scope;
}

void QgsExpressionContextUtils::setGlobalVariable( const QString &name, const QVariant &value )
{
  QgsApplication::setCustomVariable( name, value );
}

void QgsExpressionContextUtils::setGlobalVariables( const QVariantMap &variables )
{
  QgsApplication::setCustomVariables( variables );
}

void QgsExpressionContextUtils::removeGlobalVariable( const QString &name )
{
  QVariantMap vars = QgsApplication::customVariables();
  if ( vars.remove( name ) )
    QgsApplication::setCustomVariables( vars );
}

/// @cond PRIVATE

class GetLayoutItemVariables : public QgsScopedExpressionFunction
{
  public:
    GetLayoutItemVariables( const QgsLayout *c )
      : QgsScopedExpressionFunction( QStringLiteral( "item_variables" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "id" ) ), QStringLiteral( "Layout" ) )
      , mLayout( c )
    {}

    QVariant func( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * ) override
    {
      if ( !mLayout )
        return QVariant();

      QString id = values.at( 0 ).toString();

      const QgsLayoutItem *item = mLayout->itemById( id );
      if ( !item )
        return QVariant();

      QgsExpressionContext c = item->createExpressionContext();

      return c.variablesToMap();
    }

    QgsScopedExpressionFunction *clone() const override
    {
      return new GetLayoutItemVariables( mLayout );
    }

  private:

    const QgsLayout *mLayout = nullptr;

};

class GetCurrentFormFieldValue : public QgsScopedExpressionFunction
{
  public:
    GetCurrentFormFieldValue( )
      : QgsScopedExpressionFunction( QStringLiteral( "current_value" ), QgsExpressionFunction::ParameterList() << QStringLiteral( "field_name" ), QStringLiteral( "Form" ) )
    {}

    QVariant func( const QVariantList &values, const QgsExpressionContext *context, QgsExpression *, const QgsExpressionNodeFunction * ) override
    {
      QString fieldName( values.at( 0 ).toString() );
      const QgsFeature feat( context->variable( QStringLiteral( "current_feature" ) ).value<QgsFeature>() );
      if ( fieldName.isEmpty() || ! feat.isValid( ) )
      {
        return QVariant();
      }
      return feat.attribute( fieldName ) ;
    }

    QgsScopedExpressionFunction *clone() const override
    {
      return new GetCurrentFormFieldValue( );
    }

};


class GetProcessingParameterValue : public QgsScopedExpressionFunction
{
  public:
    GetProcessingParameterValue( const QVariantMap &params )
      : QgsScopedExpressionFunction( QStringLiteral( "parameter" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "name" ) ), QStringLiteral( "Processing" ) )
      , mParams( params )
    {}

    QVariant func( const QVariantList &values, const QgsExpressionContext *, QgsExpression *, const QgsExpressionNodeFunction * ) override
    {
      return mParams.value( values.at( 0 ).toString() );
    }

    QgsScopedExpressionFunction *clone() const override
    {
      return new GetProcessingParameterValue( mParams );
    }

  private:

    const QVariantMap mParams;

};

///@endcond


QgsExpressionContextScope *QgsExpressionContextUtils::formScope( const QgsFeature &formFeature, const QString &formMode )
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Form" ) );
  scope->addFunction( QStringLiteral( "current_value" ), new GetCurrentFormFieldValue( ) );
  scope->setVariable( QStringLiteral( "current_geometry" ), formFeature.geometry( ), true );
  scope->setVariable( QStringLiteral( "current_feature" ), formFeature, true );
  scope->setVariable( QStringLiteral( "form_mode" ), formMode, true );
  return scope;
}

QgsExpressionContextScope *QgsExpressionContextUtils::projectScope( const QgsProject *project )
{
  if ( !project )
  {
    QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Project" ) );
    return scope;
  }
  else
    return project->createExpressionContextScope();
}

void QgsExpressionContextUtils::setProjectVariable( QgsProject *project, const QString &name, const QVariant &value )
{
  if ( !project )
    return;

  QVariantMap vars = project->customVariables();

  vars.insert( name, value );

  project->setCustomVariables( vars );
}

void QgsExpressionContextUtils::setProjectVariables( QgsProject *project, const QVariantMap &variables )
{
  if ( !project )
    return;

  project->setCustomVariables( variables );
}

void QgsExpressionContextUtils::removeProjectVariable( QgsProject *project, const QString &name )
{
  if ( !project )
  {
    return;
  }

  QVariantMap vars = project->customVariables();
  if ( vars.remove( name ) )
    project->setCustomVariables( vars );
}

QgsExpressionContextScope *QgsExpressionContextUtils::layerScope( const QgsMapLayer *layer )
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Layer" ) );

  if ( !layer )
    return scope;

  //add variables defined in layer properties
  const QStringList variableNames = layer->customProperty( QStringLiteral( "variableNames" ) ).toStringList();
  const QStringList variableValues = layer->customProperty( QStringLiteral( "variableValues" ) ).toStringList();

  int varIndex = 0;
  for ( const QString &variableName : variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QVariant varValue = variableValues.at( varIndex );
    varIndex++;
    scope->setVariable( variableName, varValue, true );
  }

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layer_name" ), layer->name(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layer_id" ), layer->id(), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "_layer_crs" ), QVariant::fromValue<QgsCoordinateReferenceSystem>( layer->crs() ), true, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layer" ), QVariant::fromValue<QgsWeakMapLayerPointer >( QgsWeakMapLayerPointer( const_cast<QgsMapLayer *>( layer ) ) ), true, true ) );

  const QgsVectorLayer *vLayer = qobject_cast< const QgsVectorLayer * >( layer );
  if ( vLayer )
  {
    scope->setFields( vLayer->fields() );
  }

  //TODO - add functions. Possibilities include:
  //is_selected
  //field summary stats

  return scope;
}

QList<QgsExpressionContextScope *> QgsExpressionContextUtils::globalProjectLayerScopes( const QgsMapLayer *layer )
{
  QList<QgsExpressionContextScope *> scopes;
  scopes << globalScope();

  QgsProject *project = QgsProject::instance();  // TODO: use project associated with layer
  if ( project )
    scopes << projectScope( project );

  if ( layer )
    scopes << layerScope( layer );
  return scopes;
}


void QgsExpressionContextUtils::setLayerVariable( QgsMapLayer *layer, const QString &name, const QVariant &value )
{
  if ( !layer )
    return;

  //write variable to layer
  QStringList variableNames = layer->customProperty( QStringLiteral( "variableNames" ) ).toStringList();
  QStringList variableValues = layer->customProperty( QStringLiteral( "variableValues" ) ).toStringList();

  variableNames << name;
  variableValues << value.toString();

  layer->setCustomProperty( QStringLiteral( "variableNames" ), variableNames );
  layer->setCustomProperty( QStringLiteral( "variableValues" ), variableValues );
}

void QgsExpressionContextUtils::setLayerVariables( QgsMapLayer *layer, const QVariantMap &variables )
{
  if ( !layer )
    return;

  QStringList variableNames;
  QStringList variableValues;

  QVariantMap::const_iterator it = variables.constBegin();
  for ( ; it != variables.constEnd(); ++it )
  {
    variableNames << it.key();
    variableValues << it.value().toString();
  }

  layer->setCustomProperty( QStringLiteral( "variableNames" ), variableNames );
  layer->setCustomProperty( QStringLiteral( "variableValues" ), variableValues );
}

QgsExpressionContextScope *QgsExpressionContextUtils::mapSettingsScope( const QgsMapSettings &mapSettings )
{
  // IMPORTANT: ANY CHANGES HERE ALSO NEED TO BE MADE TO QgsLayoutItemMap::createExpressionContext()
  // (rationale is described in QgsLayoutItemMap::createExpressionContext() )

  // and because people don't read that ^^, I'm going to blast it all over this function

  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Map Settings" ) );

  //add known map settings context variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_id" ), "canvas", true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_rotation" ), mapSettings.rotation(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_scale" ), mapSettings.scale(), true ) );

  // IMPORTANT: ANY CHANGES HERE ALSO NEED TO BE MADE TO QgsLayoutItemMap::createExpressionContext()
  // (rationale is described in QgsLayoutItemMap::createExpressionContext() )

  QgsGeometry extent = QgsGeometry::fromRect( mapSettings.visibleExtent() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent" ), QVariant::fromValue( extent ), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_width" ), mapSettings.visibleExtent().width(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_height" ), mapSettings.visibleExtent().height(), true ) );

  // IMPORTANT: ANY CHANGES HERE ALSO NEED TO BE MADE TO QgsLayoutItemMap::createExpressionContext()
  // (rationale is described in QgsLayoutItemMap::createExpressionContext() )

  QgsGeometry centerPoint = QgsGeometry::fromPointXY( mapSettings.visibleExtent().center() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_extent_center" ), QVariant::fromValue( centerPoint ), true ) );

  // IMPORTANT: ANY CHANGES HERE ALSO NEED TO BE MADE TO QgsLayoutItemMap::createExpressionContext()
  // (rationale is described in QgsLayoutItemMap::createExpressionContext() )

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs" ), mapSettings.destinationCrs().authid(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_definition" ), mapSettings.destinationCrs().toProj4(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_units" ), QgsUnitTypes::toString( mapSettings.mapUnits() ), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_description" ), mapSettings.destinationCrs().description(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_acronym" ), mapSettings.destinationCrs().projectionAcronym(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_ellipsoid" ), mapSettings.destinationCrs().ellipsoidAcronym(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_proj4" ), mapSettings.destinationCrs().toProj4(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_crs_wkt" ), mapSettings.destinationCrs().toWkt(), true ) );

  // IMPORTANT: ANY CHANGES HERE ALSO NEED TO BE MADE TO QgsLayoutItemMap::createExpressionContext()
  // (rationale is described in QgsLayoutItemMap::createExpressionContext() )

  QVariantList layersIds;
  QVariantList layers;
  const QList<QgsMapLayer *> layersInMap = mapSettings.layers();
  layersIds.reserve( layersInMap.count() );
  layers.reserve( layersInMap.count() );
  for ( QgsMapLayer *layer : layersInMap )
  {
    layersIds << layer->id();
    layers << QVariant::fromValue<QgsWeakMapLayerPointer>( QgsWeakMapLayerPointer( layer ) );
  }

  // IMPORTANT: ANY CHANGES HERE ALSO NEED TO BE MADE TO QgsLayoutItemMap::createExpressionContext()
  // (rationale is described in QgsLayoutItemMap::createExpressionContext() )

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_layer_ids" ), layersIds, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "map_layers" ), layers, true ) );

  // IMPORTANT: ANY CHANGES HERE ALSO NEED TO BE MADE TO QgsLayoutItemMap::createExpressionContext()
  // (rationale is described in QgsLayoutItemMap::createExpressionContext() )

  scope->addFunction( QStringLiteral( "is_layer_visible" ), new GetLayerVisibility( mapSettings.layers(), mapSettings.scale() ) );

  // IMPORTANT: ANY CHANGES HERE ALSO NEED TO BE MADE TO QgsLayoutItemMap::createExpressionContext()
  // (rationale is described in QgsLayoutItemMap::createExpressionContext() )

  return scope;
}

QgsExpressionContextScope *QgsExpressionContextUtils::mapToolCaptureScope( const QList<QgsPointLocator::Match> &matches )
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Map Tool Capture" ) );

  QVariantList matchList;

  for ( const QgsPointLocator::Match &match : matches )
  {
    QVariantMap matchMap;

    matchMap.insert( QStringLiteral( "valid" ), match.isValid() );
    matchMap.insert( QStringLiteral( "layer" ), QVariant::fromValue<QgsWeakMapLayerPointer>( QgsWeakMapLayerPointer( match.layer() ) ) );
    matchMap.insert( QStringLiteral( "feature_id" ), match.featureId() );
    matchMap.insert( QStringLiteral( "vertex_index" ), match.vertexIndex() );
    matchMap.insert( QStringLiteral( "distance" ), match.distance() );

    matchList.append( matchMap );
  }

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "snapping_results" ), matchList ) );

  return scope;
}

QgsExpressionContextScope *QgsExpressionContextUtils::updateSymbolScope( const QgsSymbol *symbol, QgsExpressionContextScope *symbolScope )
{
  if ( !symbolScope )
    return nullptr;

  symbolScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_SYMBOL_COLOR, symbol ? symbol->color() : QColor(), true ) );

  double angle = 0.0;
  const QgsMarkerSymbol *markerSymbol = dynamic_cast< const QgsMarkerSymbol * >( symbol );
  if ( markerSymbol )
  {
    angle = markerSymbol->angle();
  }
  symbolScope->addVariable( QgsExpressionContextScope::StaticVariable( QgsExpressionContext::EXPR_SYMBOL_ANGLE, angle, true ) );

  return symbolScope;
}

QgsExpressionContextScope *QgsExpressionContextUtils::layoutScope( const QgsLayout *layout )
{
  std::unique_ptr< QgsExpressionContextScope > scope( new QgsExpressionContextScope( QObject::tr( "Layout" ) ) );
  if ( !layout )
    return scope.release();

  //add variables defined in layout properties
  const QStringList variableNames = layout->customProperty( QStringLiteral( "variableNames" ) ).toStringList();
  const QStringList variableValues = layout->customProperty( QStringLiteral( "variableValues" ) ).toStringList();

  int varIndex = 0;

  for ( const QString &variableName : variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QVariant varValue = variableValues.at( varIndex );
    varIndex++;
    scope->setVariable( variableName, varValue );
  }

  //add known layout context variables
  if ( const QgsMasterLayoutInterface *l = dynamic_cast< const QgsMasterLayoutInterface * >( layout ) )
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_name" ), l->name(), true ) );

  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_numpages" ), layout->pageCollection()->pageCount(), true ) );
  if ( layout->pageCollection()->pageCount() > 0 )
  {
    // just take first page size
    QSizeF s = layout->pageCollection()->page( 0 )->sizeWithUnits().toQSizeF();
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_pageheight" ), s.height(), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_pagewidth" ), s.width(), true ) );
  }
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_dpi" ), layout->renderContext().dpi(), true ) );

  scope->addFunction( QStringLiteral( "item_variables" ), new GetLayoutItemVariables( layout ) );

  if ( layout->reportContext().layer() )
  {
    scope->setFields( layout->reportContext().layer()->fields() );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_layerid" ), layout->reportContext().layer()->id(), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_layername" ), layout->reportContext().layer()->name(), true ) );
  }

  if ( layout->reportContext().feature().isValid() )
  {
    QgsFeature atlasFeature = layout->reportContext().feature();
    scope->setFeature( atlasFeature );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_feature" ), QVariant::fromValue( atlasFeature ), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_featureid" ), atlasFeature.id(), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_geometry" ), QVariant::fromValue( atlasFeature.geometry() ), true ) );
  }

  return scope.release();
}

void QgsExpressionContextUtils::setLayoutVariable( QgsLayout *layout, const QString &name, const QVariant &value )
{
  if ( !layout )
    return;

  //write variable to layout
  QStringList variableNames = layout->customProperty( QStringLiteral( "variableNames" ) ).toStringList();
  QStringList variableValues = layout->customProperty( QStringLiteral( "variableValues" ) ).toStringList();

  variableNames << name;
  variableValues << value.toString();

  layout->setCustomProperty( QStringLiteral( "variableNames" ), variableNames );
  layout->setCustomProperty( QStringLiteral( "variableValues" ), variableValues );
}

void QgsExpressionContextUtils::setLayoutVariables( QgsLayout *layout, const QVariantMap &variables )
{
  if ( !layout )
    return;

  QStringList variableNames;
  QStringList variableValues;

  QVariantMap::const_iterator it = variables.constBegin();
  for ( ; it != variables.constEnd(); ++it )
  {
    variableNames << it.key();
    variableValues << it.value().toString();
  }

  layout->setCustomProperty( QStringLiteral( "variableNames" ), variableNames );
  layout->setCustomProperty( QStringLiteral( "variableValues" ), variableValues );
}

QgsExpressionContextScope *QgsExpressionContextUtils::atlasScope( const QgsLayoutAtlas *atlas )
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Atlas" ) );
  if ( !atlas )
  {
    //add some dummy atlas variables. This is done so that as in certain contexts we want to show
    //users that these variables are available even if they have no current value
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_pagename" ), QString(), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_feature" ), QVariant::fromValue( QgsFeature() ), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_featureid" ), 0, true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_geometry" ), QVariant::fromValue( QgsGeometry() ), true ) );
    return scope;
  }

  //add known atlas variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_totalfeatures" ), atlas->count(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_featurenumber" ), atlas->currentFeatureNumber() + 1, true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_filename" ), atlas->currentFilename(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_pagename" ), atlas->nameForPage( atlas->currentFeatureNumber() ), true ) );

  if ( atlas->enabled() && atlas->coverageLayer() )
  {
    scope->setFields( atlas->coverageLayer()->fields() );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_layerid" ), atlas->coverageLayer()->id(), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_layername" ), atlas->coverageLayer()->name(), true ) );
  }

  if ( atlas->enabled() )
  {
    QgsFeature atlasFeature = atlas->layout()->reportContext().feature();
    scope->setFeature( atlasFeature );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_feature" ), QVariant::fromValue( atlasFeature ), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_featureid" ), atlasFeature.id(), true ) );
    scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "atlas_geometry" ), QVariant::fromValue( atlasFeature.geometry() ), true ) );
  }

  return scope;
}

QgsExpressionContextScope *QgsExpressionContextUtils::layoutItemScope( const QgsLayoutItem *item )
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Layout Item" ) );
  if ( !item )
    return scope;

  //add variables defined in layout item properties
  const QStringList variableNames = item->customProperty( QStringLiteral( "variableNames" ) ).toStringList();
  const QStringList variableValues = item->customProperty( QStringLiteral( "variableValues" ) ).toStringList();

  int varIndex = 0;
  for ( const QString &variableName : variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QVariant varValue = variableValues.at( varIndex );
    varIndex++;
    scope->setVariable( variableName, varValue );
  }

  //add known layout item context variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "item_id" ), item->id(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "item_uuid" ), item->uuid(), true ) );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_page" ), item->page() + 1, true ) );

  if ( item->layout() )
  {
    const QgsLayoutItemPage *page = item->layout()->pageCollection()->page( item->page() );
    if ( page )
    {
      const QSizeF s = page->sizeWithUnits().toQSizeF();
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_pageheight" ), s.height(), true ) );
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_pagewidth" ), s.width(), true ) );
    }
    else
    {
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_pageheight" ), QVariant(), true ) );
      scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "layout_pagewidth" ), QVariant(), true ) );
    }
  }

  return scope;
}

void QgsExpressionContextUtils::setLayoutItemVariable( QgsLayoutItem *item, const QString &name, const QVariant &value )
{
  if ( !item )
    return;

  //write variable to layout item
  QStringList variableNames = item->customProperty( QStringLiteral( "variableNames" ) ).toStringList();
  QStringList variableValues = item->customProperty( QStringLiteral( "variableValues" ) ).toStringList();

  variableNames << name;
  variableValues << value.toString();

  item->setCustomProperty( QStringLiteral( "variableNames" ), variableNames );
  item->setCustomProperty( QStringLiteral( "variableValues" ), variableValues );
}

void QgsExpressionContextUtils::setLayoutItemVariables( QgsLayoutItem *item, const QVariantMap &variables )
{
  if ( !item )
    return;

  QStringList variableNames;
  QStringList variableValues;

  QVariantMap::const_iterator it = variables.constBegin();
  for ( ; it != variables.constEnd(); ++it )
  {
    variableNames << it.key();
    variableValues << it.value().toString();
  }

  item->setCustomProperty( QStringLiteral( "variableNames" ), variableNames );
  item->setCustomProperty( QStringLiteral( "variableValues" ), variableValues );
}

QgsExpressionContextScope *QgsExpressionContextUtils::multiFrameScope( const QgsLayoutMultiFrame *frame )
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope( QObject::tr( "Multiframe Item" ) );
  if ( !frame )
    return scope;

  //add variables defined in layout item properties
  const QStringList variableNames = frame->customProperty( QStringLiteral( "variableNames" ) ).toStringList();
  const QStringList variableValues = frame->customProperty( QStringLiteral( "variableValues" ) ).toStringList();

  int varIndex = 0;
  for ( const QString &variableName : variableNames )
  {
    if ( varIndex >= variableValues.length() )
    {
      break;
    }

    QVariant varValue = variableValues.at( varIndex );
    varIndex++;
    scope->setVariable( variableName, varValue );
  }

  return scope;
}

void QgsExpressionContextUtils::setLayoutMultiFrameVariable( QgsLayoutMultiFrame *frame, const QString &name, const QVariant &value )
{
  if ( !frame )
    return;

  //write variable to layout multiframe
  QStringList variableNames = frame->customProperty( QStringLiteral( "variableNames" ) ).toStringList();
  QStringList variableValues = frame->customProperty( QStringLiteral( "variableValues" ) ).toStringList();

  variableNames << name;
  variableValues << value.toString();

  frame->setCustomProperty( QStringLiteral( "variableNames" ), variableNames );
  frame->setCustomProperty( QStringLiteral( "variableValues" ), variableValues );
}

void QgsExpressionContextUtils::setLayoutMultiFrameVariables( QgsLayoutMultiFrame *frame, const QVariantMap &variables )
{
  if ( !frame )
    return;

  QStringList variableNames;
  QStringList variableValues;

  QVariantMap::const_iterator it = variables.constBegin();
  for ( ; it != variables.constEnd(); ++it )
  {
    variableNames << it.key();
    variableValues << it.value().toString();
  }

  frame->setCustomProperty( QStringLiteral( "variableNames" ), variableNames );
  frame->setCustomProperty( QStringLiteral( "variableValues" ), variableValues );
}

QgsExpressionContext QgsExpressionContextUtils::createFeatureBasedContext( const QgsFeature &feature, const QgsFields &fields )
{
  QgsExpressionContextScope *scope = new QgsExpressionContextScope();
  scope->setFeature( feature );
  scope->setFields( fields );
  return QgsExpressionContext() << scope;
}

QgsExpressionContextScope *QgsExpressionContextUtils::processingAlgorithmScope( const QgsProcessingAlgorithm *algorithm, const QVariantMap &parameters, QgsProcessingContext &context )
{
  // set aside for future use
  Q_UNUSED( context )

  std::unique_ptr< QgsExpressionContextScope > scope( new QgsExpressionContextScope( QObject::tr( "Algorithm" ) ) );
  scope->addFunction( QStringLiteral( "parameter" ), new GetProcessingParameterValue( parameters ) );

  if ( !algorithm )
    return scope.release();

  //add standard algorithm variables
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "algorithm_id" ), algorithm->id(), true ) );

  return scope.release();
}

QgsExpressionContextScope *QgsExpressionContextUtils::processingModelAlgorithmScope( const QgsProcessingModelAlgorithm *model, const QVariantMap &, QgsProcessingContext &context )
{
  std::unique_ptr< QgsExpressionContextScope > modelScope( new QgsExpressionContextScope( QObject::tr( "Model" ) ) );
  QString modelPath;
  if ( !model->sourceFilePath().isEmpty() )
  {
    modelPath = model->sourceFilePath();
  }
  else if ( context.project() )
  {
    // fallback to project path -- the model may be embedded in a project, OR an unsaved model. In either case the
    // project path is a logical value to fall back to
    modelPath = context.project()->projectStorage() ? context.project()->fileName() : context.project()->absoluteFilePath();
  }

  const QString modelFolder = !modelPath.isEmpty() ? QFileInfo( modelPath ).path() : QString();
  modelScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "model_path" ), QDir::toNativeSeparators( modelPath ), true ) );
  modelScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "model_folder" ), QDir::toNativeSeparators( modelFolder ), true, true ) );
  modelScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "model_name" ), model->displayName(), true ) );
  modelScope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "model_group" ), model->group(), true ) );

  // custom variables
  const QVariantMap customVariables = model->variables();
  for ( auto it = customVariables.constBegin(); it != customVariables.constEnd(); ++it )
  {
    modelScope->addVariable( QgsExpressionContextScope::StaticVariable( it.key(), it.value(), true ) );
  }

  return modelScope.release();
}

QgsExpressionContextScope *QgsExpressionContextUtils::notificationScope( const QString &message )
{
  std::unique_ptr< QgsExpressionContextScope > scope( new QgsExpressionContextScope() );
  scope->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "notification_message" ), message, true ) );
  return scope.release();
}

void QgsExpressionContextUtils::registerContextFunctions()
{
  QgsExpression::registerFunction( new GetNamedProjectColor( nullptr ) );
  QgsExpression::registerFunction( new GetLayoutItemVariables( nullptr ) );
  QgsExpression::registerFunction( new GetLayerVisibility( QList<QgsMapLayer *>(), 0.0 ) );
  QgsExpression::registerFunction( new GetProcessingParameterValue( QVariantMap() ) );
  QgsExpression::registerFunction( new GetCurrentFormFieldValue( ) );
}

bool QgsScopedExpressionFunction::usesGeometry( const QgsExpressionNodeFunction *node ) const
{
  Q_UNUSED( node )
  return mUsesGeometry;
}

QSet<QString> QgsScopedExpressionFunction::referencedColumns( const QgsExpressionNodeFunction *node ) const
{
  Q_UNUSED( node )
  return mReferencedColumns;
}

bool QgsScopedExpressionFunction::isStatic( const QgsExpressionNodeFunction *node, QgsExpression *parent, const QgsExpressionContext *context ) const
{
  return allParamsStatic( node, parent, context );
}

//
// GetLayerVisibility
//

QgsExpressionContextUtils::GetLayerVisibility::GetLayerVisibility( const QList<QgsMapLayer *> &layers, double scale )
  : QgsScopedExpressionFunction( QStringLiteral( "is_layer_visible" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "id" ) ), QStringLiteral( "General" ) )
  , mLayers( _qgis_listRawToQPointer( layers ) )
  , mScale( scale )
{
  for ( const auto &layer : mLayers )
  {
    if ( layer->hasScaleBasedVisibility() )
    {
      mScaleBasedVisibilityDetails[ layer ] = qMakePair( layer->minimumScale(), layer->maximumScale() );
    }
  }
}

QgsExpressionContextUtils::GetLayerVisibility::GetLayerVisibility()
  : QgsScopedExpressionFunction( QStringLiteral( "is_layer_visible" ), QgsExpressionFunction::ParameterList() << QgsExpressionFunction::Parameter( QStringLiteral( "id" ) ), QStringLiteral( "General" ) )
{}

QVariant QgsExpressionContextUtils::GetLayerVisibility::func( const QVariantList &values, const QgsExpressionContext *, QgsExpression *parent, const QgsExpressionNodeFunction * )
{
  if ( mLayers.isEmpty() )
  {
    return false;
  }

  bool isVisible = false;
  QgsMapLayer *layer = QgsExpressionUtils::getMapLayer( values.at( 0 ), parent );
  if ( layer && mLayers.contains( layer ) )
  {
    isVisible = true;
    if ( mScaleBasedVisibilityDetails.contains( layer ) && !qgsDoubleNear( mScale, 0.0 ) )
    {
      if ( ( !qgsDoubleNear( mScaleBasedVisibilityDetails[ layer ].first, 0.0 ) && mScale > mScaleBasedVisibilityDetails[ layer ].first ) ||
           ( !qgsDoubleNear( mScaleBasedVisibilityDetails[ layer ].second, 0.0 ) && mScale < mScaleBasedVisibilityDetails[ layer ].second ) )
      {
        isVisible = false;
      }
    }
  }

  return isVisible;
}

QgsScopedExpressionFunction *QgsExpressionContextUtils::GetLayerVisibility::clone() const
{
  GetLayerVisibility *func = new GetLayerVisibility();
  func->mLayers = mLayers;
  func->mScale = mScale;
  func->mScaleBasedVisibilityDetails = mScaleBasedVisibilityDetails;
  return func;
}
