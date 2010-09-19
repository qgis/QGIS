/***************************************************************************
qgsmaptoolselectutils.cpp  -  Utility methods to help with select map tools
---------------------
begin                : May 2010
copyright            : (C) 2010 by Jeremy Palmer
email                : jpalmer at linz dot govt dot nz
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/* $Id$ */

#include <limits>

#include "qgsmaptoolselectutils.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsrubberband.h"
#include "qgscsexception.h"
#include "qgslogger.h"
#include "qgis.h"

#include <QMouseEvent>
#include <QApplication>
#include <QMessageBox>

QgsVectorLayer* QgsMapToolSelectUtils::getCurrentVectorLayer( QgsMapCanvas* canvas )
{
  QgsVectorLayer* vlayer = NULL;
  if ( !canvas->currentLayer()
       || ( vlayer = qobject_cast<QgsVectorLayer *>( canvas->currentLayer() ) ) == NULL )
  {
    QMessageBox::warning( canvas, QObject::tr( "No active vector layer" ),
                          QObject::tr( "To select features, you must choose a "
                                       "vector layer by clicking on its name in the legend"
                                     ) );
  }
  return vlayer;
}

void QgsMapToolSelectUtils::setRubberBand( QgsMapCanvas* canvas, QRect& selectRect, QgsRubberBand* rubberBand )
{
  const QgsMapToPixel* transform = canvas->getCoordinateTransform();
  QgsPoint ll = transform->toMapCoordinates( selectRect.left(), selectRect.bottom() );
  QgsPoint ur = transform->toMapCoordinates( selectRect.right(), selectRect.top() );
  rubberBand->reset( true );
  rubberBand->addPoint( ll, false );
  rubberBand->addPoint( QgsPoint( ur.x(), ll.y() ), false );
  rubberBand->addPoint( ur, false );
  rubberBand->addPoint( QgsPoint( ll.x(), ur.y() ), true );
}

void QgsMapToolSelectUtils::expandSelectRectangle( QRect& selectRect,
    QgsVectorLayer* vlayer,
    QPoint point )
{
  int boxSize = 0;
  if ( vlayer->geometryType() != QGis::Polygon )
  {
    //if point or line use an artificial bounding box of 10x10 pixels
    //to aid the user to click on a feature accurately
    boxSize = 5;
  }
  else
  {
    //otherwise just use the click point for polys
    boxSize = 1;
  }
  selectRect.setLeft( point.x() - boxSize );
  selectRect.setRight( point.x() + boxSize );
  selectRect.setTop( point.y() - boxSize );
  selectRect.setBottom( point.y() + boxSize );
}

void QgsMapToolSelectUtils::setSelectFeatures( QgsMapCanvas* canvas,
    QgsGeometry* selectGeometry,
    bool doContains,
    bool addSelection,
    bool substractSelection,
    bool singleSelect )
{
  if ( selectGeometry->type() != QGis::Polygon )
  {
    return;
  }
  QgsVectorLayer* vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( canvas );
  if ( vlayer == NULL )
  {
    return;
  }

  // toLayerCoordinates will throw an exception for any 'invalid' points in
  // the rubber band.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  QgsGeometry selectGeomTrans( *selectGeometry );

  if ( canvas->mapRenderer()->hasCrsTransformEnabled() )
  {
    try
    {
      QgsCoordinateTransform ct( canvas->mapRenderer()->destinationSrs(), vlayer->crs() );
      selectGeomTrans.transform( ct );
    }
    catch ( QgsCsException &cse )
    {
      Q_UNUSED( cse );
      // catch exception for 'invalid' point and leave existing selection unchanged
      QgsLogger::warning( "Caught CRS exception " + QString( __FILE__ ) + ": " + QString::number( __LINE__ ) );
      QMessageBox::warning( canvas, QObject::tr( "CRS Exception" ),
                            QObject::tr( "Selection extends beyond layer's coordinate system." ) );
      return;
    }
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsDebugMsg( "Selection layer: " + vlayer->name() );
  QgsDebugMsg( "Selection polygon: " + selectGeomTrans.exportToWkt() );
  QgsDebugMsg( "doContains: " + QString( doContains ? "T" : "F" ) );
  QgsDebugMsg( "addSelection: " + QString( addSelection ? "T" : "F" ) );
  QgsDebugMsg( "substractSelection: " + QString( substractSelection ? "T" : "F" ) );

  vlayer->select( QgsAttributeList(), selectGeomTrans.boundingBox(), true, true );

  QgsFeatureIds newSelectedFeatures;
  QgsFeature f;
  int closestFeatureId = 0;
  bool foundSingleFeature = false;
  double closestFeatureDist = std::numeric_limits<double>::max();
  while ( vlayer->nextFeature( f ) )
  {
    QgsGeometry* g = f.geometry();
    if ( doContains && !selectGeomTrans.contains( g ) )
    {
      continue;
    }
    if ( singleSelect )
    {
      foundSingleFeature = true;
      double distance = g->distance( selectGeomTrans );
      if ( distance <= closestFeatureDist )
      {
        closestFeatureDist = distance;
        closestFeatureId = f.id();
      }
    }
    else
    {
      newSelectedFeatures.insert( f.id() );
    }
  }
  if ( singleSelect && foundSingleFeature )
  {
    newSelectedFeatures.insert( closestFeatureId );
  }

  QgsDebugMsg( "Number of selected features: " + QString::number( newSelectedFeatures.size() ) );

  QgsFeatureIds layerSelectedFeatures;
  if ( addSelection )
  {
    layerSelectedFeatures = vlayer->selectedFeaturesIds();
    QgsFeatureIds::const_iterator i = newSelectedFeatures.constEnd();
    while ( i != newSelectedFeatures.constBegin() )
    {
      --i;
      layerSelectedFeatures.insert( *i );
    }
  }
  else if ( substractSelection )
  {
    layerSelectedFeatures = vlayer->selectedFeaturesIds();
    QgsFeatureIds::const_iterator i = newSelectedFeatures.constEnd();
    while ( i != newSelectedFeatures.constBegin() )
    {
      --i;
      if ( layerSelectedFeatures.contains( *i ) )
      {
        layerSelectedFeatures.remove( *i );
      }
    }
  }
  else
  {
    layerSelectedFeatures = newSelectedFeatures;
  }
  vlayer->setSelectedFeatures( layerSelectedFeatures );

  QApplication::restoreOverrideCursor();
}

void QgsMapToolSelectUtils::setSelectFeatures( QgsMapCanvas* canvas, QgsGeometry* selectGeometry, QMouseEvent * e )
{
  bool doContains = e->modifiers() & Qt::AltModifier ? false : true;
  bool addSelection = e->modifiers() & Qt::ControlModifier ? true : false;
  bool substractSelection = e->modifiers() & Qt::ShiftModifier ? true : false;
  setSelectFeatures( canvas, selectGeometry, doContains, addSelection, substractSelection );
}
