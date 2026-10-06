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
#include "qgslayertreeregistrybridge.h"
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

bool QgsProcessingResultsHandler::handleAlgorithmResults( const QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, const QVariantMap &parameters, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsProcessingFeedback > localFeedback;
  if ( !feedback )
  {
    localFeedback = std::make_unique< QgsProcessingFeedback >();
    feedback = localFeedback.get();
  }

  return handleAlgorithmResultsProtected( algorithm, context, parameters, feedback ).succeeded;
}

QgsProcessingResultsHandler::ResultDetails QgsProcessingResultsHandler::handleAlgorithmResultsProtected(
  const QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, const QVariantMap &parameters, QgsProcessingFeedback *feedback
)
{
  QgsProcessingLayerPostProcessor *layerPostProcessor = QgsApplication::processingRegistry()->layerPostProcessor();

  feedback->setProgressText( QObject::tr( "Loading resulting layers" ) );

  ResultDetails result;
  QStringList invalidLayers;


  struct LayerToPostProcess
  {
      QgsMapLayer *layer;
      QgsProcessingContext::LayerDetails details;
  };

  QList< LayerToPostProcess > layersToPostProcess;

  const QMap< QString, QgsProcessingContext::LayerDetails > layersToLoad = context.layersToLoadOnCompletion();
  int i = 0;
  for ( auto it = layersToLoad.constBegin(); it != layersToLoad.constEnd(); ++it, ++i )
  {
    if ( feedback->isCanceled() )
      return result;

    if ( layersToLoad.size() > 2 )
    {
      // only show progress feedback if we're loading a bunch of layers
      feedback->setProgress( 100 * i / static_cast< float >( layersToLoad.size() ) );
    }

    const QString destId = it.key();
    QgsProcessingContext::LayerDetails details = it.value();

    QgsMapLayer *layer = QgsProcessingUtils::mapLayerFromString( destId, context, true, details.layerTypeHint );
    if ( layer )
    {
      details.setOutputLayerName( layer );
      const QString outputName = determineOutputName( destId, details, algorithm, context, parameters );
      layerPostProcessor->postProcessLayer( layer, outputName, algorithm );

      // Load layer to layer tree root or to a specific group
      QgsLayerTreeGroup *resultsGroup = layerTreeResultsGroup( details, context );

      // note here that we may not retrieve an owned layer -- eg if the
      // output layer already exists in the destination project
      std::unique_ptr< QgsMapLayer> ownedMapLayer( context.temporaryLayerStore()->takeMapLayer( layer ) );
      if ( ownedMapLayer )
      {
        // we don't add the layer to the tree yet -- that's done
        // later, after we've sorted all added layers
        ResultLayerDetails resultLayerDetails = ResultLayerDetails( ownedMapLayer.release() );
        resultLayerDetails.targetLayerTreeGroup = resultsGroup;
        resultLayerDetails.sortKey = details.layerSortKey;
        resultLayerDetails.destinationProject = details.project;
        result.addedLayers.append( resultLayerDetails );
      }

      if ( details.postProcessor() )
      {
        // we defer calling the postProcessor set in the context
        // until the layer has been added to the project's layer
        // tree, just in case the postProcessor contains logic
        // relating to layer tree handling
        layersToPostProcess.append( LayerToPostProcess { layer, details } );
      }
    }
    else
    {
      invalidLayers.append( destId );
    }
  }

  result.succeeded = invalidLayers.empty();

  QgsMapLayer *newActiveLayer = handleAddResultLayers( result, context );

  // all layers have been added to the layer tree, so safe to call
  // postProcessors now
  for ( const LayerToPostProcess &layer : std::as_const( layersToPostProcess ) )
  {
    layer.details.postProcessor()->postProcessLayer( layer.layer, context, feedback );
  }

  setNewActiveLayer( newActiveLayer );

  feedback->setProgress( 100 );

  if ( !invalidLayers.empty() )
  {
    QString msg = QObject::tr( "The following layers were not correctly generated." );
    QStringList layerList;
    layerList.reserve( invalidLayers.size() );
    for ( const QString &layerId : std::as_const( invalidLayers ) )
    {
      layerList.append( u"• %1"_s.arg( layerId ) );
    }
    msg += u"\n%1\n"_s.arg( layerList.join( '\n' ) );
    msg += QObject::tr( "You can check the 'Log Messages Panel' in QGIS main window to find more information about the execution of the algorithm." );
    feedback->reportError( msg );
  }

  return result;
}

QgsMapLayer *QgsProcessingResultsHandler::handleAddResultLayers( const ResultDetails &details, const QgsProcessingContext &context )
{
  return addResultLayers( details.addedLayers, context, nullptr );
}

void QgsProcessingResultsHandler::setNewActiveLayer( QgsMapLayer * )
{}

QgsMapLayer *QgsProcessingResultsHandler::addResultLayers( const QVector<ResultLayerDetails> &layers, const QgsProcessingContext &context, QgsLayerTreeNode *currentSelectedNode )
{
  // sort added layer tree layers
  QVector<QgsProcessingResultsHandler::ResultLayerDetails> sortedLayers = layers;
  std::sort( sortedLayers.begin(), sortedLayers.end(), []( const QgsProcessingResultsHandler::ResultLayerDetails &a, const QgsProcessingResultsHandler::ResultLayerDetails &b ) {
    return a.sortKey < b.sortKey;
  } );

  bool haveSetActiveLayer = false;
  QgsLayerTreeGroup *defaultTargetGroup = nullptr;
  qsizetype defaultTargetGroupIndex = 0;
  if ( auto currentSelectedLayer = qobject_cast< QgsLayerTreeLayer * >( currentSelectedNode ) )
  {
    defaultTargetGroup = qobject_cast< QgsLayerTreeGroup * >( currentSelectedLayer->parent() );
    if ( defaultTargetGroup )
      defaultTargetGroupIndex = defaultTargetGroup->children().indexOf( currentSelectedNode );
  }
  if ( auto currentSelectedGroup = qobject_cast< QgsLayerTreeGroup * >( currentSelectedNode ) )
  {
    defaultTargetGroup = currentSelectedGroup;
  }

  QgsMapLayer *resultLayer = nullptr;
  for ( const QgsProcessingResultsHandler::ResultLayerDetails &layerDetails : std::as_const( sortedLayers ) )
  {
    QgsProject *project = layerDetails.destinationProject;
    if ( !project )
      project = context.project();

    // store the current insertion point to restore it later
    std::optional< QgsLayerTreeRegistryBridge::InsertionPoint > previousInsertionPoint;
    if ( project )
    {
      previousInsertionPoint.emplace( project->layerTreeRegistryBridge()->layerInsertionPoint() );
    }

    std::optional< QgsLayerTreeRegistryBridge::InsertionPoint > insertionPoint;
    if ( layerDetails.targetLayerTreeGroup )
    {
      insertionPoint.emplace( QgsLayerTreeRegistryBridge::InsertionPoint( layerDetails.targetLayerTreeGroup, 0 ) );
    }
    else
    {
      // no destination group for this layer, so should be placed
      // above the current layer if one was selected, or at top of group if a group was selected
      if ( defaultTargetGroup )
      {
        insertionPoint.emplace( QgsLayerTreeRegistryBridge::InsertionPoint( defaultTargetGroup, static_cast< int >( defaultTargetGroupIndex ) ) );
      }
      else if ( project )
      {
        insertionPoint.emplace( QgsLayerTreeRegistryBridge::InsertionPoint( project->layerTreeRoot(), 0 ) );
      }
    }

    if ( project && insertionPoint.has_value() )
    {
      project->layerTreeRegistryBridge()->setLayerInsertionPoint( *insertionPoint );
    }

    if ( project )
    {
      project->addMapLayer( layerDetails.layer );
      QgsLayerTreeLayer *layerTreeLayer = project->layerTreeRoot()->findLayer( layerDetails.layer );
      QgsProcessingResultsHandler::configureResultLayerTreeLayer( layerTreeLayer );
    }

    if ( !haveSetActiveLayer )
    {
      resultLayer = layerDetails.layer;
      haveSetActiveLayer = true;
    }

    // reset to the previous insertion point
    if ( project && previousInsertionPoint.has_value() )
    {
      project->layerTreeRegistryBridge()->setLayerInsertionPoint( *previousInsertionPoint );
    }
  }
  return resultLayer;
}
