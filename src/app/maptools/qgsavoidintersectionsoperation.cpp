/***************************************************************************
    qgsavoidintersectionsoperation.cpp
    ---------------------
    begin                : 2023-09-20
    copyright            : (C) 2023 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPushButton>

#include "qgis.h"
#include "qgisapp.h"
#include "qgsavoidintersectionsoperation.h"
#include "moc_qgsavoidintersectionsoperation.cpp"
#include "qgsmessagebar.h"
#include "qgsmessagebaritem.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

QgsAvoidIntersectionsOperation::Result QgsAvoidIntersectionsOperation::apply( QgsVectorLayer *layer, QgsFeatureId fid, QgsGeometry &geom, const QHash<QgsVectorLayer *, QSet<QgsFeatureId>> &ignoreFeatures )
{
  QgsAvoidIntersectionsOperation::Result result;

  QList<QgsVectorLayer *> avoidIntersectionsLayers;
  switch ( QgsProject::instance()->avoidIntersectionsMode() )
  {
    case Qgis::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer:
      avoidIntersectionsLayers.append( layer );
      break;
    case Qgis::AvoidIntersectionsMode::AvoidIntersectionsLayers:
      avoidIntersectionsLayers = QgsProject::instance()->avoidIntersectionsLayers();
      break;
    case Qgis::AvoidIntersectionsMode::AllowIntersections:
      break;
  }

  if ( avoidIntersectionsLayers.isEmpty() )
    return result;

  result.operationResult = geom.avoidIntersectionsV2( avoidIntersectionsLayers, ignoreFeatures );
  switch ( result.operationResult )
  {
    case Qgis::GeometryOperationResult::GeometryTypeHasChanged: // Geometry type was changed, let's try our best to make it compatible with the target layer
    {
      result.geometryHasChanged = true;
      const QVector<QgsGeometry> newGeoms = geom.coerceToType( layer->wkbType() );
      if ( newGeoms.count() == 1 )
      {
        geom = newGeoms.at( 0 );
        result.operationResult = Qgis::GeometryOperationResult::Success;
      }
      else // handle multi geometries
      {
        QgsFeatureList removedFeatures;
        double largest = 0;
        QgsFeature originalFeature = layer->getFeature( fid );
        int largestPartIndex = -1;
        for ( int i = 0; i < newGeoms.size(); ++i )
        {
          QgsGeometry currentPart = newGeoms.at( i );
          const double currentPartSize = layer->geometryType() == Qgis::GeometryType::Polygon ? currentPart.area() : currentPart.length();

          QgsFeature partFeature( layer->fields() );
          partFeature.setAttributes( originalFeature.attributes() );
          partFeature.setGeometry( currentPart );
          removedFeatures.append( partFeature );
          if ( currentPartSize > largest )
          {
            geom = currentPart;
            largestPartIndex = i;
            largest = currentPartSize;
          }
        }
        removedFeatures.removeAt( largestPartIndex );
        QgsMessageBarItem *messageBarItem = QgisApp::instance()->messageBar()->createMessage( tr( "Avoid overlaps" ), tr( "Only the largest of multiple created geometries was preserved." ) );
        QPushButton *restoreButton = new QPushButton( tr( "Restore others" ) );
        QPointer<QgsVectorLayer> layerPtr( layer );
        connect( restoreButton, &QPushButton::clicked, restoreButton, [=] {
          if ( !layerPtr )
            return;
          layerPtr->beginEditCommand( tr( "Restored geometry parts removed by avoid overlaps" ) );
          QgsFeatureList unconstFeatures = removedFeatures;
          QgisApp::instance()->pasteFeatures( layerPtr.data(), 0, removedFeatures.size(), unconstFeatures );
        } );
        messageBarItem->layout()->addWidget( restoreButton );
        QgisApp::instance()->messageBar()->pushWidget( messageBarItem, Qgis::MessageLevel::Info, 15 );
      }
      break;
    }

    case Qgis::GeometryOperationResult::InvalidBaseGeometry:
      result.geometryHasChanged = true;
      emit messageEmitted( tr( "At least one geometry intersected is invalid. These geometries must be manually repaired." ), Qgis::MessageLevel::Warning );
      break;

    case Qgis::GeometryOperationResult::Success:
      result.geometryHasChanged = true;
      break;

    case Qgis::GeometryOperationResult::NothingHappened:
    case Qgis::GeometryOperationResult::InvalidInputGeometryType:
    case Qgis::GeometryOperationResult::SelectionIsEmpty:
    case Qgis::GeometryOperationResult::SelectionIsGreaterThanOne:
    case Qgis::GeometryOperationResult::GeometryEngineError:
    case Qgis::GeometryOperationResult::LayerNotEditable:
    case Qgis::GeometryOperationResult::AddPartSelectedGeometryNotFound:
    case Qgis::GeometryOperationResult::AddPartNotMultiGeometry:
    case Qgis::GeometryOperationResult::AddRingNotClosed:
    case Qgis::GeometryOperationResult::AddRingNotValid:
    case Qgis::GeometryOperationResult::AddRingCrossesExistingRings:
    case Qgis::GeometryOperationResult::AddRingNotInExistingFeature:
    case Qgis::GeometryOperationResult::SplitCannotSplitPoint:
      break;
  }

  if ( QgsProject::instance()->topologicalEditing() && result.geometryHasChanged )
  {
    // then add the new points generated by avoidIntersections
    QgsGeometry oldGeom = layer->getGeometry( fid ).convertToType( Qgis::GeometryType::Point, true );
    QgsGeometry difference = geom.convertToType( Qgis::GeometryType::Point, true ).difference( oldGeom );
    for ( auto it = difference.vertices_begin(); it != difference.vertices_end(); ++it )
      for ( QgsVectorLayer *targetLayer : avoidIntersectionsLayers )
      {
        targetLayer->addTopologicalPoints( *it );
      }
  }

  return result;
}
