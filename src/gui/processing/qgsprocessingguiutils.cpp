/***************************************************************************
                             qgsprocessingguiutils.cpp
                             ------------------------
    Date                 : June 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprocessingguiutils.h"

#include <optional>

#include "qgslayertree.h"
#include "qgslayertreelayer.h"
#include "qgslayertreeregistrybridge.h"
#include "qgslayertreeview.h"

void QgsProcessingGuiUtils::configureResultLayerTreeLayer( QgsLayerTreeLayer *layerTreeLayer )
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

QgsLayerTreeGroup *QgsProcessingGuiUtils::layerTreeResultsGroup( const QgsProcessingContext::LayerDetails &layerDetails, const QgsProcessingContext &context )
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
      resultsGroup = destinationProject->layerTreeRoot()->insertGroup(
        0, resultsGroupName
      );
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

void QgsProcessingGuiUtils::addResultLayers( const QVector<ResultLayerDetails> &layers, const QgsProcessingContext &context, QgsLayerTreeView *view )
{
  // sort added layer tree layers
  QVector<ResultLayerDetails> sortedLayers = layers;
  std::sort(
    sortedLayers.begin(), sortedLayers.end(), []( const ResultLayerDetails &a, const ResultLayerDetails &b ) {
      return a.sortKey < b.sortKey;
    }
  );

  bool haveSetActiveLayer = false;
  QgsLayerTreeNode *currentSelectedNode = nullptr;
  if ( view )
  {
    currentSelectedNode = view->currentNode();
  }
  QgsLayerTreeGroup *defaultTargetGroup = nullptr;
  int defaultTargetGroupIndex = 0;
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

  for ( const ResultLayerDetails &layerDetails : std::as_const( sortedLayers ) )
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
        insertionPoint.emplace( QgsLayerTreeRegistryBridge::InsertionPoint(
          defaultTargetGroup, defaultTargetGroupIndex
        ) );
      }
      else if ( project )
      {
        insertionPoint.emplace( QgsLayerTreeRegistryBridge::InsertionPoint(
          project->layerTreeRoot(), 0
        ) );
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
      configureResultLayerTreeLayer( layerTreeLayer );
    }

    if ( !haveSetActiveLayer && view )
    {
      view->setCurrentLayer( layerDetails.layer );
      haveSetActiveLayer = true;
    }

    // reset to the previous insertion point
    if ( project && previousInsertionPoint.has_value() )
    {
      project->layerTreeRegistryBridge()->setLayerInsertionPoint(
        *previousInsertionPoint
      );
    }
  }
}
