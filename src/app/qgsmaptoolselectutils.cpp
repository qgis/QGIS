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

#include <limits>

#include "qgsmaptoolselectutils.h"
#include "qgsfeatureiterator.h"
#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgsrenderer.h"
#include "qgsrubberband.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgis.h"
#include "qgsproject.h"

#include <QMouseEvent>
#include <QApplication>

QgsVectorLayer *QgsMapToolSelectUtils::getCurrentVectorLayer( QgsMapCanvas *canvas )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( canvas->currentLayer() );
  if ( !vlayer )
  {
    QgisApp::instance()->messageBar()->pushMessage(
      QObject::tr( "No active vector layer" ),
      QObject::tr( "To select features, choose a vector layer in the legend" ),
      Qgis::Info,
      QgisApp::instance()->messageTimeout() );
  }
  return vlayer;
}

void QgsMapToolSelectUtils::setRubberBand( QgsMapCanvas *canvas, QRect &selectRect, QgsRubberBand *rubberBand )
{
  const QgsMapToPixel *transform = canvas->getCoordinateTransform();
  QgsPointXY ll = transform->toMapCoordinates( selectRect.left(), selectRect.bottom() );
  QgsPointXY lr = transform->toMapCoordinates( selectRect.right(), selectRect.bottom() );
  QgsPointXY ul = transform->toMapCoordinates( selectRect.left(), selectRect.top() );
  QgsPointXY ur = transform->toMapCoordinates( selectRect.right(), selectRect.top() );

  if ( rubberBand )
  {
    rubberBand->reset( QgsWkbTypes::PolygonGeometry );
    rubberBand->addPoint( ll, false );
    rubberBand->addPoint( lr, false );
    rubberBand->addPoint( ur, false );
    rubberBand->addPoint( ul, true );
  }
}

QgsRectangle QgsMapToolSelectUtils::expandSelectRectangle( QgsPointXY mapPoint, QgsMapCanvas *canvas, QgsVectorLayer *vlayer )
{
  int boxSize = 0;
  if ( !vlayer || vlayer->geometryType() != QgsWkbTypes::PolygonGeometry )
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

  const QgsMapToPixel *transform = canvas->getCoordinateTransform();
  QgsPointXY point = transform->transform( mapPoint );
  QgsPointXY ll = transform->toMapCoordinates( static_cast<int>( point.x() - boxSize ), static_cast<int>( point.y() + boxSize ) );
  QgsPointXY ur = transform->toMapCoordinates( static_cast<int>( point.x() + boxSize ), static_cast<int>( point.y() - boxSize ) );
  return QgsRectangle( ll, ur );
}

void QgsMapToolSelectUtils::selectMultipleFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, const Qt::KeyboardModifiers &modifiers )
{
  QgsVectorLayer::SelectBehavior behavior = QgsVectorLayer::SetSelection;
  if ( modifiers & Qt::ShiftModifier && modifiers & Qt::ControlModifier )
    behavior = QgsVectorLayer::IntersectSelection;
  else if ( modifiers & Qt::ShiftModifier )
    behavior = QgsVectorLayer::AddToSelection;
  else if ( modifiers & Qt::ControlModifier )
    behavior = QgsVectorLayer::RemoveFromSelection;

  bool doContains = modifiers & Qt::AltModifier;
  setSelectedFeatures( canvas, selectGeometry, behavior, doContains );
}

void QgsMapToolSelectUtils::selectSingleFeature( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, const Qt::KeyboardModifiers &modifiers )
{
  QgsVectorLayer *vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( canvas );
  if ( !vlayer )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsFeatureIds selectedFeatures = getMatchingFeatures( canvas, selectGeometry, false, true );
  if ( selectedFeatures.isEmpty() )
  {
    if ( !( modifiers & Qt::ShiftModifier || modifiers & Qt::ControlModifier ) )
    {
      // if no modifiers then clicking outside features clears the selection
      // but if there's a shift or ctrl modifier, then it's likely the user was trying
      // to modify an existing selection by adding or subtracting features and just
      // missed the feature
      vlayer->removeSelection();
    }
    QApplication::restoreOverrideCursor();
    return;
  }

  QgsVectorLayer::SelectBehavior behavior = QgsVectorLayer::SetSelection;

  //either shift or control modifier switches to "toggle" selection mode
  if ( modifiers & Qt::ShiftModifier || modifiers & Qt::ControlModifier )
  {
    QgsFeatureId selectId = *selectedFeatures.constBegin();
    QgsFeatureIds layerSelectedFeatures = vlayer->selectedFeatureIds();
    if ( layerSelectedFeatures.contains( selectId ) )
      behavior = QgsVectorLayer::RemoveFromSelection;
    else
      behavior = QgsVectorLayer::AddToSelection;
  }

  vlayer->selectByIds( selectedFeatures, behavior );

  QApplication::restoreOverrideCursor();
}

void QgsMapToolSelectUtils::setSelectedFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry,
    QgsVectorLayer::SelectBehavior selectBehavior, bool doContains, bool singleSelect )
{
  QgsVectorLayer *vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( canvas );
  if ( !vlayer )
    return;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsFeatureIds selectedFeatures = getMatchingFeatures( canvas, selectGeometry, doContains, singleSelect );
  vlayer->selectByIds( selectedFeatures, selectBehavior );

  QApplication::restoreOverrideCursor();
}


QgsFeatureIds QgsMapToolSelectUtils::getMatchingFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, bool doContains, bool singleSelect )
{
  QgsFeatureIds newSelectedFeatures;

  if ( selectGeometry.type() != QgsWkbTypes::PolygonGeometry )
    return newSelectedFeatures;

  QgsVectorLayer *vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( canvas );
  if ( !vlayer )
    return newSelectedFeatures;

  // toLayerCoordinates will throw an exception for any 'invalid' points in
  // the rubber band.
  // For example, if you project a world map onto a globe using EPSG 2163
  // and then click somewhere off the globe, an exception will be thrown.
  QgsGeometry selectGeomTrans = selectGeometry;

  try
  {
    QgsCoordinateTransform ct( canvas->mapSettings().destinationCrs(), vlayer->crs(), QgsProject::instance() );

    if ( !ct.isShortCircuited() && selectGeomTrans.type() == QgsWkbTypes::PolygonGeometry )
    {
      // convert add more points to the edges of the rectangle
      // improve transformation result
      QgsPolygonXY poly( selectGeomTrans.asPolygon() );
      if ( poly.size() == 1 && poly.at( 0 ).size() == 5 )
      {
        const QgsPolylineXY &ringIn = poly.at( 0 );

        QgsPolygonXY newpoly( 1 );
        newpoly[0].resize( 41 );
        QgsPolylineXY &ringOut = newpoly[0];

        ringOut[ 0 ] = ringIn.at( 0 );

        int i = 1;
        for ( int j = 1; j < 5; j++ )
        {
          QgsVector v( ( ringIn.at( j ) - ringIn.at( j - 1 ) ) / 10.0 );
          for ( int k = 0; k < 9; k++ )
          {
            ringOut[ i ] = ringOut[ i - 1 ] + v;
            i++;
          }
          ringOut[ i++ ] = ringIn.at( j );
        }
        selectGeomTrans = QgsGeometry::fromPolygonXY( newpoly );
      }
    }

    selectGeomTrans.transform( ct );
  }
  catch ( QgsCsException &cse )
  {
    Q_UNUSED( cse );
    // catch exception for 'invalid' point and leave existing selection unchanged
    QgsDebugMsg( "Caught CRS exception " );
    QgisApp::instance()->messageBar()->pushMessage(
      QObject::tr( "CRS Exception" ),
      QObject::tr( "Selection extends beyond layer's coordinate system" ),
      Qgis::Warning,
      QgisApp::instance()->messageTimeout() );
    return newSelectedFeatures;
  }

  QgsDebugMsgLevel( "Selection layer: " + vlayer->name(), 3 );
  QgsDebugMsgLevel( "Selection polygon: " + selectGeomTrans.asWkt(), 3 );
  QgsDebugMsgLevel( "doContains: " + QString( doContains ? "T" : "F" ), 3 );

  QgsRenderContext context = QgsRenderContext::fromMapSettings( canvas->mapSettings() );
  context.expressionContext() << QgsExpressionContextUtils::layerScope( vlayer );
  std::unique_ptr< QgsFeatureRenderer > r;
  if ( vlayer->renderer() )
  {
    r.reset( vlayer->renderer()->clone() );
    r->startRender( context, vlayer->fields() );
  }

  QgsFeatureRequest request;
  request.setFilterRect( selectGeomTrans.boundingBox() );
  request.setFlags( QgsFeatureRequest::ExactIntersect );
  if ( r )
    request.setSubsetOfAttributes( r->usedAttributes( context ), vlayer->fields() );
  else
    request.setSubsetOfAttributes( QgsAttributeList() );

  QgsFeatureIterator fit = vlayer->getFeatures( request );

  QgsFeature f;
  QgsFeatureId closestFeatureId = 0;
  bool foundSingleFeature = false;
  double closestFeatureDist = std::numeric_limits<double>::max();
  while ( fit.nextFeature( f ) )
  {
    context.expressionContext().setFeature( f );
    // make sure to only use features that are visible
    if ( r && !r->willRenderFeature( f, context ) )
      continue;

    QgsGeometry g = f.geometry();
    if ( doContains )
    {
      if ( !selectGeomTrans.contains( g ) )
        continue;
    }
    else
    {
      if ( !selectGeomTrans.intersects( g ) )
        continue;
    }
    if ( singleSelect )
    {
      foundSingleFeature = true;
      double distance = g.distance( selectGeomTrans );
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

  if ( r )
    r->stopRender( context );

  QgsDebugMsg( "Number of new selected features: " + QString::number( newSelectedFeatures.size() ) );

  return newSelectedFeatures;
}

