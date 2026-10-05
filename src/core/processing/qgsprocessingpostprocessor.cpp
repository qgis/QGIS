/***************************************************************************
                         qgsprocessingpostprocessor.cpp
                         ------------------------------
    begin                : October 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingpostprocessor.h"

#include "qgsapplication.h"
#include "qgslayertree.h"
#include "qgslayertreegroup.h"
#include "qgsmaplayer.h"
#include "qgsprocessingalgorithm.h"
#include "qgsprocessingdefaultstyleregistry.h"
#include "qgsprocessingregistry.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

#include <QString>

using namespace Qt::StringLiterals;

QgsProcessingLayerPostProcessor::~QgsProcessingLayerPostProcessor() = default;

void QgsProcessingLayerPostProcessor::postProcessLayer( QgsMapLayer *layer, const QString &outputName, const QgsProcessingAlgorithm *algorithm )
{
  if ( !layer )
    return;

  QString styleFilePath;

  if ( !outputName.isEmpty() && algorithm )
  {
    styleFilePath = QgsApplication::processingRegistry()->defaultStyleRegistry()->defaultStyleForOutput( algorithm->id(), outputName );
  }

  if ( styleFilePath.isEmpty() )
  {
    QgsSettings settings;

    switch ( layer->type() )
    {
      case Qgis::LayerType::Vector:
      {
        QgsVectorLayer *vectorLayer = qobject_cast< QgsVectorLayer * >( layer );
        switch ( vectorLayer->geometryType() )
        {
          case Qgis::GeometryType::Point:
            styleFilePath = settings.value( u"Processing/Configuration/VECTOR_POINT_STYLE"_s, QString() ).toString();
            break;
          case Qgis::GeometryType::Line:
            styleFilePath = settings.value( u"Processing/Configuration/VECTOR_LINE_STYLE"_s, QString() ).toString();
            break;
          case Qgis::GeometryType::Polygon:
            styleFilePath = settings.value( u"Processing/Configuration/VECTOR_POLYGON_STYLE"_s, QString() ).toString();
            break;
          case Qgis::GeometryType::Unknown:
          case Qgis::GeometryType::Null:
            break;
        }
        break;
      }
      case Qgis::LayerType::Raster:
      {
        styleFilePath = settings.value( u"Processing/Configuration/RASTER_STYLE"_s, QString() ).toString();
        break;
      }
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::VectorTile:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
      case Qgis::LayerType::TiledScene:
        break;
    }
  }

  if ( !styleFilePath.isEmpty() )
  {
    bool result = false;
    layer->loadNamedStyle( styleFilePath, result );
  }
}

//
// QgsProcessingResultsHandler
//

QgsProcessingResultsHandler::~QgsProcessingResultsHandler() = default;

QString QgsProcessingResultsHandler::determineOutputName(
  const QString &destinationId, const QgsProcessingContext::LayerDetails &details, const QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, const QVariantMap &parameters
)
{
  if ( !algorithm )
    return details.outputName;

  const QgsProcessingOutputDefinitions outputs = algorithm->outputDefinitions();
  for ( const QgsProcessingOutputDefinition *output : std::as_const( outputs ) )
  {
    auto it = parameters.constFind( output->name() );
    if ( it == parameters.constEnd() || QgsVariantUtils::isNull( it.value() ) )
    {
      continue;
    }

    QVariant outputValue = it.value();
    if ( outputValue.userType() == qMetaTypeId<QgsProcessingOutputLayerDefinition >() )
    {
      QgsProperty sink = outputValue.value< QgsProcessingOutputLayerDefinition >().sink;
      outputValue = sink.valueAsString( context.expressionContext() );
    }
    else
    {
      outputValue = outputValue.toString();
    }
    if ( outputValue.toString() == destinationId )
    {
      return output->name();
    }
  }

  return details.outputName;
}

QgsLayerTreeGroup *QgsProcessingResultsHandler::layerTreeResultsGroup( const QgsProcessingContext::LayerDetails &layerDetails, const QgsProcessingContext &context )
{
  QgsProject *destinationProject = layerDetails.project ? layerDetails.project : context.project();
  if ( !destinationProject )
    return nullptr;

  QgsLayerTreeGroup *resultsGroup = nullptr;

  // if a specific results group is specified in Processing settings,
  // respect it (and create if necessary)
  QgsSettings settings;
  const QString resultsGroupName = settings.value( u"Processing/Configuration/RESULTS_GROUP_NAME"_s, QString() ).toString();

  if ( !resultsGroupName.isEmpty() )
  {
    resultsGroup = destinationProject->layerTreeRoot()->findGroup( resultsGroupName );
    if ( !resultsGroup )
    {
      resultsGroup = destinationProject->layerTreeRoot()->insertGroup( 0, resultsGroupName );
      resultsGroup->setExpanded( true );
    }
  }

  // if this particular output layer has a specific output group assigned,
  // find or create it now
  QgsLayerTreeGroup *group = nullptr;
  if ( !layerDetails.groupName.isEmpty() )
  {
    if ( !resultsGroup )
    {
      resultsGroup = destinationProject->layerTreeRoot();
    }

    group = resultsGroup->findGroup( layerDetails.groupName );
    if ( !group )
    {
      group = resultsGroup->insertGroup( 0, layerDetails.groupName );
      group->setExpanded( true );
    }
  }
  else
  {
    group = resultsGroup;
  }

  return group;
}

void QgsProcessingResultsHandler::configureResultLayerTreeLayer( QgsLayerTreeLayer *layerTreeLayer )
{
  const QgsMapLayer *layer = layerTreeLayer->layer();
  if ( layer && layer->type() == Qgis::LayerType::Vector )
  {
    // post-process vector layer
    QgsSettings settings;
    if ( settings.value( u"Processing/Configuration/VECTOR_FEATURE_COUNT"_s, false ).toBool() )
    {
      layerTreeLayer->setCustomProperty( u"showFeatureCount"_s, true );
    }
  }
}
