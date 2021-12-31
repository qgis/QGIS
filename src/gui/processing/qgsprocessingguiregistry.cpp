/***************************************************************************
                         qgsprocessingguiregistry.cpp
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingguiregistry.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingconfigurationwidgets.h"
#include "qgsprocessingvectortilewriterlayerswidgetwrapper.h"
#include "qgsprocessingfieldmapwidgetwrapper.h"
#include "qgsprocessingaggregatewidgetwrapper.h"
#include "qgsprocessingdxflayerswidgetwrapper.h"
#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsprocessingtininputlayerswidget.h"
#include "qgsprocessingmeshdatasetwidget.h"
#include "qgsprocessingparameters.h"
#include "qgis.h"
#include "qgslogger.h"

QgsProcessingGuiRegistry::QgsProcessingGuiRegistry()
{
  addAlgorithmConfigurationWidgetFactory( new QgsFilterAlgorithmConfigurationWidgetFactory() );
  addAlgorithmConfigurationWidgetFactory( new QgsConditionalBranchAlgorithmConfigurationWidgetFactory() );

  addParameterWidgetFactory( new QgsProcessingBooleanWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingCrsWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingStringWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingNumericWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingDistanceWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingDurationWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingScaleWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingRangeWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingAuthConfigWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingMatrixWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingFileWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingExpressionWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingEnumWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingLayoutWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingLayoutItemWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingPointWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingGeometryWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingColorWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingCoordinateOperationWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingFieldWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingMapThemeWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingDateTimeWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingProviderConnectionWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingDatabaseSchemaWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingDatabaseTableWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingExtentWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingMapLayerWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingVectorLayerWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingFeatureSourceWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingRasterLayerWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingMeshLayerWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingBandWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingMultipleLayerWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingVectorTileWriterLayersWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingFeatureSinkWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingVectorDestinationWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingRasterDestinationWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingFileDestinationWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingFolderDestinationWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingFieldMapWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingAggregateWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingTinInputLayersWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingDxfLayersWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingMeshDatasetGroupsWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingMeshDatasetTimeWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingPointCloudLayerWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingAnnotationLayerWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingPointCloudDestinationWidgetWrapper() );
}

QgsProcessingGuiRegistry::~QgsProcessingGuiRegistry()
{
  const QList< QgsProcessingAlgorithmConfigurationWidgetFactory * > factories = mAlgorithmConfigurationWidgetFactories;
  for ( QgsProcessingAlgorithmConfigurationWidgetFactory *factory : factories )
    removeAlgorithmConfigurationWidgetFactory( factory );
  const QMap< QString, QgsProcessingParameterWidgetFactoryInterface * > paramFactories = mParameterWidgetFactories;
  for ( auto it = paramFactories.constBegin(); it != paramFactories.constEnd(); ++it )
    removeParameterWidgetFactory( it.value() );
}

void QgsProcessingGuiRegistry::addAlgorithmConfigurationWidgetFactory( QgsProcessingAlgorithmConfigurationWidgetFactory *factory )
{
  mAlgorithmConfigurationWidgetFactories.append( factory );
}

void QgsProcessingGuiRegistry::removeAlgorithmConfigurationWidgetFactory( QgsProcessingAlgorithmConfigurationWidgetFactory *factory )
{
  mAlgorithmConfigurationWidgetFactories.removeAll( factory );
  delete factory;
}

QgsProcessingAlgorithmConfigurationWidget *QgsProcessingGuiRegistry::algorithmConfigurationWidget( const QgsProcessingAlgorithm *algorithm ) const
{
  for ( const auto *factory : mAlgorithmConfigurationWidgetFactories )
  {
    if ( factory->canCreateFor( algorithm ) )
    {
      std::unique_ptr< QgsProcessingAlgorithmConfigurationWidget > widget( factory->create( algorithm ) );
      if ( widget )
        widget->setAlgorithm( algorithm );
      return widget.release();
    }
  }

  return nullptr;
}

bool QgsProcessingGuiRegistry::addParameterWidgetFactory( QgsProcessingParameterWidgetFactoryInterface *factory )
{
  if ( !factory )
    return false;

  if ( mParameterWidgetFactories.contains( factory->parameterType() ) )
  {
    QgsLogger::warning( QStringLiteral( "Duplicate parameter factory for %1 registered" ).arg( factory->parameterType() ) );
    return false;
  }

  mParameterWidgetFactories.insert( factory->parameterType(), factory );
  return true;
}

void QgsProcessingGuiRegistry::removeParameterWidgetFactory( QgsProcessingParameterWidgetFactoryInterface *factory )
{
  if ( !factory )
    return;

  mParameterWidgetFactories.remove( factory->parameterType() );
  delete factory;
}

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingGuiRegistry::createParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type )
{
  if ( !parameter )
    return nullptr;

  const QString parameterType = parameter->type();
  if ( !mParameterWidgetFactories.contains( parameterType ) )
    return nullptr;

  return mParameterWidgetFactories.value( parameterType )->createWidgetWrapper( parameter, type );
}

QgsProcessingModelerParameterWidget *QgsProcessingGuiRegistry::createModelerParameterWidget( QgsProcessingModelAlgorithm *model, const QString &childId, const QgsProcessingParameterDefinition *parameter, QgsProcessingContext &context )
{
  if ( !parameter )
    return nullptr;

  const QString parameterType = parameter->type();
  if ( !mParameterWidgetFactories.contains( parameterType ) )
    return nullptr;

  return mParameterWidgetFactories.value( parameterType )->createModelerWidgetWrapper( model, childId, parameter, context );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingGuiRegistry::createParameterDefinitionWidget( const QString &type,
    QgsProcessingContext &context,
    const QgsProcessingParameterWidgetContext &widgetContext,
    const QgsProcessingParameterDefinition *definition,
    const QgsProcessingAlgorithm *algorithm )
{
  if ( !mParameterWidgetFactories.contains( type ) )
    return nullptr;

  return mParameterWidgetFactories.value( type )->createParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}

