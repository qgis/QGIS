/***************************************************************************
                        qgspalobjectpositionmanager.cpp  -  description
                        ---------------------------------
   begin                : October 2008
   copyright            : (C) 2008 by Marco Hugentobler
   email                : marco dot hugentobler at karto dot baug dot ethz dot ch
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspalobjectpositionmanager.h"
#include "qgsgeometry.h"
#include "qgspalgeometry.h"
#include "qgsoverlayobject.h"
#include "qgsrendercontext.h"
#include "qgsvectorlayer.h"
#include "qgsvectoroverlay.h"
#include "pal.h"
#include "labelposition.h"
#include "feature.h"
#include "layer.h"

QgsPALObjectPositionManager::QgsPALObjectPositionManager(): mNumberOfLayers( 0 )
{

}

QgsPALObjectPositionManager::~QgsPALObjectPositionManager()
{
  deletePALGeometries();
}

void QgsPALObjectPositionManager::addLayer( QgsVectorLayer* vl, QList<QgsVectorOverlay*>& overlays )
{
  if ( overlays.size() < 1 )
  {
    return;
  }

  //set arrangement based on vector type
  pal::Arrangement labelArrangement;
  switch ( vl->geometryType() )
  {
    case QGis::Point:
      labelArrangement = pal::P_POINT;
      break;
    case QGis::Line:
      labelArrangement = pal::P_LINE;
      break;
    case QGis::Polygon:
      labelArrangement = pal::P_HORIZ;
      break;
    default:
      return; //error
  }

  pal::Layer* positionLayer = mPositionEngine.addLayer( QString::number( mNumberOfLayers ).toLocal8Bit().data(), -1, -1, labelArrangement, pal::PIXEL, 0.5, true, true, true );
  ++mNumberOfLayers;

  if ( !positionLayer )
  {
    return;
  }

  //register the labeling objects in the layer
  int objectNr = 0;
  QList<QgsVectorOverlay*>::const_iterator overlayIt = overlays.begin();
  for ( ; overlayIt != overlays.end(); ++overlayIt )
  {
    if ( !( *overlayIt ) )
    {
      continue;
    }

    QMap<int, QgsOverlayObject*>* positionObjects = ( *overlayIt )->overlayObjects();
    if ( !positionObjects )
    {
      continue;
    }

    QMap<int, QgsOverlayObject*>::const_iterator objectIt = positionObjects->begin();
    for ( ; objectIt != positionObjects->end(); ++objectIt )
    {
      QgsPALGeometry* palGeom = new QgsPALGeometry( objectIt.value() );
      mPALGeometries.push_back( palGeom ); //insert object into list to delete memory later
      char* featureLabel = QString::number( objectNr ).toAscii().data();
      positionLayer->registerFeature( featureLabel, palGeom, objectIt.value()->width(), objectIt.value()->height() );
      ++objectNr;
    }
  }
}

void QgsPALObjectPositionManager::findObjectPositions( const QgsRenderContext& renderContext, QGis::UnitType unitType )
{
  //trigger label placement
  QgsRectangle viewExtent = renderContext.extent();
  //PAL needs projected view extent
  if ( renderContext.coordinateTransform() )
  {
    viewExtent = renderContext.coordinateTransform()->transformBoundingBox( viewExtent );
  }
  double bbox[4]; bbox[0] = viewExtent.xMinimum(); bbox[1] = viewExtent.yMinimum(); bbox[2] = viewExtent.xMaximum(); bbox[3] = viewExtent.yMaximum();


  //set map units
  pal::Units mapUnits;
  switch ( unitType )
  {
    case QGis::Meters:
      mapUnits = pal::METER;
      break;

    case QGis::Feet:
      mapUnits = pal::FOOT;
      break;

    case QGis::Degrees:
      mapUnits = pal::DEGREE;
      break;
    default:
      return;
  }

  mPositionEngine.setMapUnit( mapUnits );
  mPositionEngine.setDpi( renderContext.scaleFactor() * 25.4 );

  std::list<pal::LabelPosition*>* resultLabelList = mPositionEngine.labeller( renderContext.rendererScale(), bbox, NULL, false );

  //and read the positions back to the overlay objects
  if ( !resultLabelList )
  {
    return;
  }

  //pal geometry that the current label object refers to
  QgsPALGeometry* referredGeometry = 0;
  QgsOverlayObject* referredOverlayObject = 0;
  pal::FeaturePart* referredPart = 0;

  std::list<pal::LabelPosition*>::iterator labelIt = resultLabelList->begin();
  for ( ; labelIt != resultLabelList->end(); ++labelIt )
  {
    if ( !*labelIt )
    {
      continue;
    }

    referredPart = ( *labelIt )->getFeaturePart();
    if ( !referredPart )
    {
      continue;
    }
    referredGeometry = dynamic_cast<QgsPALGeometry*>( referredPart->getUserGeometry() );
    if ( !referredGeometry )
    {
      continue;
    }
    referredOverlayObject = referredGeometry->overlayObjectPtr();
    if ( !referredOverlayObject )
    {
      continue;
    }

    pal::LabelPosition* lp = *labelIt;

    //QGIS takes the coordinates of the middle points
    double x = ( lp->getX( 0 ) + lp->getX( 1 ) + lp->getX( 2 ) + lp->getX( 3 ) ) / 4;
    double y = ( lp->getY( 0 ) + lp->getY( 1 ) + lp->getY( 2 ) + lp->getY( 3 ) ) / 4;
    referredOverlayObject->addPosition( QgsPoint( x, y ) );
  }

  //release memory for QgsPALGeometries
  deletePALGeometries();
}

void QgsPALObjectPositionManager::removeLayers()
{
  std::list<pal::Layer*>* layerList = mPositionEngine.getLayers();
  if ( !layerList )
  {
    return;
  }

  //Iterators get invalid if elements are deleted in a std::list
  //Therefore we have to get the layer pointers in a first step and remove them in a second
  QList<pal::Layer*> layersToRemove;
  std::list<pal::Layer*>::iterator layerIt = layerList->begin();
  for ( ; layerIt != layerList->end(); ++layerIt )
  {
    layersToRemove.push_back( *layerIt );
  }

  QList<pal::Layer*>::iterator removeIt = layersToRemove.begin();
  for ( ; removeIt != layersToRemove.end(); ++removeIt )
  {
    mPositionEngine.removeLayer( *removeIt );
  }
}
//Chain, Popmusic tabu chain, Popmusic tabu, Popmusic chain
void QgsPALObjectPositionManager::setPlacementAlgorithm( const QString& algorithmName )
{
  if ( algorithmName == "Popmusic tabu chain" )
  {
    mPositionEngine.setSearch( pal::POPMUSIC_TABU_CHAIN );
  }
  else if ( algorithmName == "Popmusic tabu" )
  {
    mPositionEngine.setSearch( pal::POPMUSIC_TABU );
  }
  else if ( algorithmName == "Popmusic chain" )
  {
    mPositionEngine.setSearch( pal::POPMUSIC_CHAIN );
  }
  else //default is "Chain"
  {
    mPositionEngine.setSearch( pal::CHAIN );
  }
}

void QgsPALObjectPositionManager::deletePALGeometries()
{
  QList<QgsPALGeometry*>::iterator geomIt = mPALGeometries.begin();
  for ( ; geomIt != mPALGeometries.end(); ++geomIt )
  {
    delete( *geomIt );
  }
  mPALGeometries.clear();
}
