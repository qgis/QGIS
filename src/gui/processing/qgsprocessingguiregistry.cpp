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

#include "qgis.h"
#include "qgslogger.h"
#include "qgsprocessingaggregatewidgetwrapper.h"
#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgsprocessingalignrasterlayerswidgetwrapper.h"
#include "qgsprocessingconfigurationwidgets.h"
#include "qgsprocessingdxflayerswidgetwrapper.h"
#include "qgsprocessingfieldmapwidgetwrapper.h"
#include "qgsprocessingmeshdatasetwidget.h"
#include "qgsprocessingparameters.h"
#include "qgsprocessingrasteroptionswidgetwrapper.h"
#include "qgsprocessingtininputlayerswidget.h"
#include "qgsprocessingvectortilewriterlayerswidgetwrapper.h"
#include "qgsprocessingwidgetwrapperimpl.h"

QgsProcessingGuiRegistry::QgsProcessingGuiRegistry()
{
  addAlgorithmConfigurationWidgetFactory( new QgsFilterAlgorithmConfigurationWidgetFactory() );
  addAlgorithmConfigurationWidgetFactory( new QgsConditionalBranchAlgorithmConfigurationWidgetFactory() );

  addParameterWidgetFactory( new QgsProcessingAlignRasterLayersWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingBooleanWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingCrsWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingStringWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingNumericWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingDistanceWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingAreaWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingVolumeWidgetWrapper() );
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
  addParameterWidgetFactory( new QgsProcessingPointCloudAttributeWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingVectorTileDestinationWidgetWrapper() );
  addParameterWidgetFactory( new QgsProcessingRasterOptionsWidgetWrapper() );
}

QgsProcessingGuiRegistry::~QgsProcessingGuiRegistry()
{
  const QList<QgsProcessingAlgorithmConfigurationWidgetFactory *> factories = mAlgorithmConfigurationWidgetFactories;
  for ( QgsProcessingAlgorithmConfigurationWidgetFactory *factory : factories )
    removeAlgorithmConfigurationWidgetFactory( factory );
  const QMap<QString, QgsProcessingParameterWidgetFactoryInterface *> paramFactories = mParameterWidgetFactories;
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
      std::unique_ptr<QgsProcessingAlgorithmConfigurationWidget> widget( factory->create( algorithm ) );
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
    QgsLogger::warning( u"Duplicate parameter factory for %1 registered"_s.arg( factory->parameterType() ) );
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

QgsAbstractProcessingParameterWidgetWrapper *QgsProcessingGuiRegistry::createParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter, Qgis::ProcessingMode type )
{
  if ( !parameter )
    return nullptr;

  const QVariantMap metadata = parameter->metadata();
  const QString widgetType = metadata.value( u"widget_wrapper"_s ).toMap().value( u"widget_type"_s ).toString();
  const QString parameterType = !widgetType.isEmpty() ? widgetType : parameter->type();
  if ( !mParameterWidgetFactories.contains( parameterType ) )
    return nullptr;

  if ( QgsProcessingParameterWidgetFactoryInterface *factory = mParameterWidgetFactories.value( parameterType ) )
  {
    return factory->createWidgetWrapper( parameter, type );
  }
  return nullptr;
}

QgsProcessingModelerParameterWidget *QgsProcessingGuiRegistry::createModelerParameterWidget( QgsProcessingModelAlgorithm *model, const QString &childId, const QgsProcessingParameterDefinition *parameter, QgsProcessingContext &context )
{
  if ( !parameter )
    return nullptr;

  const QString parameterType = parameter->type();
  auto it = mParameterWidgetFactories.constFind( parameterType );
  if ( it == mParameterWidgetFactories.constEnd() )
    return nullptr;

  return it.value()->createModelerWidgetWrapper( model, childId, parameter, context );
}

QgsProcessingAbstractParameterDefinitionWidget *QgsProcessingGuiRegistry::createParameterDefinitionWidget( const QString &type, QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition, const QgsProcessingAlgorithm *algorithm )
{
  auto it = mParameterWidgetFactories.constFind( type );
  if ( it == mParameterWidgetFactories.constEnd() )
    return nullptr;

  return it.value()->createParameterDefinitionWidget( context, widgetContext, definition, algorithm );
}
