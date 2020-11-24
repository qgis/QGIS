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
#include "qgshighlight.h"
#include "qgsrenderer.h"
#include "qgsrubberband.h"
#include "qgsexception.h"
#include "qgslogger.h"
#include "qgis.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmessagelog.h"

#include <QMouseEvent>
#include <QApplication>
#include <QAction>

QgsVectorLayer *QgsMapToolSelectUtils::getCurrentVectorLayer( QgsMapCanvas *canvas )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( canvas->currentLayer() );
  if ( !vlayer )
  {
    QgisApp::instance()->messageBar()->pushMessage(
      QObject::tr( "No active vector layer" ),
      QObject::tr( "To select features, choose a vector layer in the layers panel" ),
      Qgis::Info );
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

void QgsMapToolSelectUtils::selectMultipleFeatures( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, Qt::KeyboardModifiers modifiers )
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

void QgsMapToolSelectUtils::selectSingleFeature( QgsMapCanvas *canvas, const QgsGeometry &selectGeometry, Qt::KeyboardModifiers modifiers )
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
    Q_UNUSED( cse )
    // catch exception for 'invalid' point and leave existing selection unchanged
    QgsDebugMsg( QStringLiteral( "Caught CRS exception " ) );
    QgisApp::instance()->messageBar()->pushMessage(
      QObject::tr( "CRS Exception" ),
      QObject::tr( "Selection extends beyond layer's coordinate system" ),
      Qgis::Warning );
    return newSelectedFeatures;
  }

  QgsDebugMsgLevel( "Selection layer: " + vlayer->name(), 3 );
  QgsDebugMsgLevel( "Selection polygon: " + selectGeomTrans.asWkt(), 3 );
  QgsDebugMsgLevel( "doContains: " + QString( doContains ? "T" : "F" ), 3 );

  // make sure the selection geometry is valid, or intersection tests won't work correctly...
  if ( !selectGeomTrans.isGeosValid( ) )
  {
    // a zero width buffer is safer than calling make valid here!
    selectGeomTrans = selectGeomTrans.buffer( 0, 1 );
  }

  std::unique_ptr< QgsGeometryEngine > selectionGeometryEngine( QgsGeometry::createGeometryEngine( selectGeomTrans.constGet() ) );
  selectionGeometryEngine->setLogErrors( false );
  selectionGeometryEngine->prepareGeometry();

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
    request.setNoAttributes();

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
    QString errorMessage;
    if ( doContains )
    {
      // if we get an error from the contains check then it indicates that the geometry is invalid and GEOS choked on it.
      // in this case we consider the bounding box intersection check which has already been performed by the iterator as sufficient and
      // allow the feature to be selected
      const bool notContained = !selectionGeometryEngine->contains( g.constGet(), &errorMessage ) &&
                                ( errorMessage.isEmpty() || /* message will be non empty if geometry g is invalid */
                                  !selectionGeometryEngine->contains( g.makeValid().constGet(), &errorMessage ) ); /* second chance for invalid geometries, repair and re-test */

      if ( !errorMessage.isEmpty() )
      {
        // contains relation test still failed, even after trying to make valid!
        QgsMessageLog::logMessage( QObject::tr( "Error determining selection: %1" ).arg( errorMessage ), QString(), Qgis::Warning );
      }

      if ( notContained )
        continue;
    }
    else
    {
      // if we get an error from the intersects check then it indicates that the geometry is invalid and GEOS choked on it.
      // in this case we consider the bounding box intersection check which has already been performed by the iterator as sufficient and
      // allow the feature to be selected
      const bool notIntersects = !selectionGeometryEngine->intersects( g.constGet(), &errorMessage ) &&
                                 ( errorMessage.isEmpty() || /* message will be non empty if geometry g is invalid */
                                   !selectionGeometryEngine->intersects( g.makeValid().constGet(), &errorMessage ) ); /* second chance for invalid geometries, repair and re-test */

      if ( !errorMessage.isEmpty() )
      {
        // intersects relation test still failed, even after trying to make valid!
        QgsMessageLog::logMessage( QObject::tr( "Error determining selection: %1" ).arg( errorMessage ), QString(), Qgis::Warning );
      }

      if ( notIntersects )
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

  QgsDebugMsgLevel( "Number of new selected features: " + QString::number( newSelectedFeatures.size() ), 2 );

  return newSelectedFeatures;
}


QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::QgsMapToolSelectMenuActions(
  QgsMapCanvas *canvas,
  QgsVectorLayer *vectorLayer,
  QgsVectorLayer::SelectBehavior behavior,
  QObject *parent ):
  QObject( parent ),
  mCanvas( canvas ),
  mVectorLayer( vectorLayer ),
  mBehavior( behavior )
{
  connect( mVectorLayer, &QgsMapLayer::destroyed, this, &QgsMapToolSelectMenuActions::onLayerDestroyed );
}

QList<QAction *> QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::actions( const QgsFeatureIds &featureCanditateIds )
{
  QList<QAction *> actionsList;

  QString beginningOfText;

  QgsFeatureIds effectiveFeatureIds = featureCanditateIds;

  switch ( mBehavior )
  {
    case QgsVectorLayer::SetSelection:
      beginningOfText = tr( "Select " );
      break;
    case QgsVectorLayer::AddToSelection:
    {
      beginningOfText = tr( "Add " );
      QgsFeatureIds existingSelection = mVectorLayer->selectedFeatureIds();
      for ( const QgsFeatureId &selected : featureCanditateIds )
      {
        if ( existingSelection.contains( selected ) )
          effectiveFeatureIds.remove( selected );
      }
    }
    break;

    case QgsVectorLayer::IntersectSelection:
    {
      beginningOfText = tr( "Intersect " );
      QgsFeatureIds existingSelection = mVectorLayer->selectedFeatureIds();
      for ( const QgsFeatureId &selected : featureCanditateIds )
      {
        if ( !existingSelection.contains( selected ) )
          effectiveFeatureIds.remove( selected );
      }
    }
    break;
    case QgsVectorLayer::RemoveFromSelection:
    {
      beginningOfText = tr( "Remove " );
      QgsFeatureIds existingSelection = mVectorLayer->selectedFeatureIds();
      for ( const QgsFeatureId &selected : featureCanditateIds )
      {
        if ( !existingSelection.contains( selected ) )
          effectiveFeatureIds.remove( selected );
      }
    }
    break;
  }

  if ( !effectiveFeatureIds.isEmpty() )
  {
    if ( effectiveFeatureIds.size() > 1 )
    {
      QAction *actionAll = new QAction( beginningOfText + tr( "All Features (%1)" ).arg( effectiveFeatureIds.count() ), this );
      connect( actionAll, &QAction::triggered, this, &QgsMapToolSelectMenuActions::selectFeature );
      connect( actionAll, &QAction::hovered, this, &QgsMapToolSelectMenuActions::highLightFeatures );
      QVariantList list;
      for ( const QgsFeatureId &id : effectiveFeatureIds )
        list.append( id );

      actionAll->setData( list );
      actionsList.append( actionAll );
    }
    for ( const QgsFeatureId &id : effectiveFeatureIds )
    {
      QAction *featureAction = new QAction( beginningOfText + tr( "Feature %1" ).arg( id ), this ) ;
      featureAction->setData( id );
      connect( featureAction, &QAction::triggered, this, &QgsMapToolSelectMenuActions::selectFeature );
      connect( featureAction, &QAction::hovered, this, &QgsMapToolSelectMenuActions::highLightFeatures );
      actionsList.append( featureAction );
    }
  }
  return actionsList;
}

QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::~QgsMapToolSelectMenuActions()
{
  removeHighLight();
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::selectFeature()
{
  if ( !mVectorLayer )
    return;
  QAction *senderAction = qobject_cast<QAction *>( sender() );

  if ( !senderAction )
    return;

  QVariant featureVariant = senderAction->data();

  QgsFeatureIds ids;
  if ( featureVariant.type() == QVariant::List )
  {
    QVariantList list = featureVariant.toList();
    for ( const QVariant &var : list )
      ids.insert( var.toLongLong() );
  }

  if ( featureVariant.type() == QVariant::LongLong )
    ids.insert( featureVariant.toLongLong() );

  if ( !ids.empty() )
    mVectorLayer->selectByIds( ids, mBehavior );

}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::highLightFeatures()
{
  removeHighLight();

  if ( !mVectorLayer )
    return;

  QAction *senderAction = qobject_cast<QAction *>( sender() );
  if ( !senderAction )
    return;

  QVariant featureVariant = senderAction->data();

  QgsFeatureIds ids;
  if ( featureVariant.type() == QVariant::List )
  {
    QVariantList list = featureVariant.toList();
    for ( const QVariant &var : list )
      ids.insert( var.toLongLong() );
  }

  if ( featureVariant.type() == QVariant::LongLong )
    ids.insert( featureVariant.toLongLong() );

  if ( !ids.empty() )
  {
    for ( const QgsFeatureId &id : ids )
    {
      QgsFeature feat = mVectorLayer->getFeature( id );
      QgsGeometry geom = feat.geometry();
      if ( !geom.isEmpty() )
      {
        QgsHighlight *hl = new QgsHighlight( mCanvas, geom, mVectorLayer );
        styleHighlight( hl );
        mHighLight.append( hl );
      }
    }
  }

}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::onLayerDestroyed()
{
  mVectorLayer = nullptr;
  removeHighLight();
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::removeHighLight()
{
  for ( QgsHighlight *hl : mHighLight )
    delete hl;

  mHighLight.clear();
}

void QgsMapToolSelectUtils::QgsMapToolSelectMenuActions::styleHighlight( QgsHighlight *highlight )
{
  QgsSettings settings;
  QColor color = QColor( settings.value( QStringLiteral( "Map/highlight/color" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.name() ).toString() );
  int alpha = settings.value( QStringLiteral( "Map/highlight/colorAlpha" ), Qgis::DEFAULT_HIGHLIGHT_COLOR.alpha() ).toInt();
  double buffer = settings.value( QStringLiteral( "Map/highlight/buffer" ), Qgis::DEFAULT_HIGHLIGHT_BUFFER_MM ).toDouble();
  double minWidth = settings.value( QStringLiteral( "Map/highlight/minWidth" ), Qgis::DEFAULT_HIGHLIGHT_MIN_WIDTH_MM ).toDouble();

  highlight->setColor( color ); // sets also fill with default alpha
  color.setAlpha( alpha );
  highlight->setFillColor( color ); // sets fill with alpha
  highlight->setBuffer( buffer );
  highlight->setMinWidth( minWidth );
}
