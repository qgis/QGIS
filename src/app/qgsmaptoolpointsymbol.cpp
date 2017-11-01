/***************************************************************************
    qgsmaptoolpointsymbol.cpp
    -------------------------
    begin                : April 2016
    copyright            : (C) 2016 by Marco Hugentobler, Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolpointsymbol.h"
#include "qgsfeatureiterator.h"
#include "qgsrenderer.h"
#include "qgsvectorlayer.h"
#include "qgsmapcanvas.h"
#include "qgssnappingutils.h"

#include <QMouseEvent>

QgsMapToolPointSymbol::QgsMapToolPointSymbol( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mFeatureNumber( -1 )
{}

void QgsMapToolPointSymbol::canvasPressEvent( QgsMapMouseEvent *e )
{
  if ( !mCanvas )
  {
    return;
  }

  mActiveLayer = currentVectorLayer();
  if ( !mActiveLayer )
  {
    notifyNotVectorLayer();
    return;
  }

  if ( !mActiveLayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  if ( mActiveLayer->geometryType() != QgsWkbTypes::PointGeometry )
  {
    return;
  }

  //find the closest feature to the pressed position
  QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Vertex );
  if ( !m.isValid() )
  {
    emit messageEmitted( tr( "No point feature was detected at the clicked position. Please click closer to the feature or enhance the search tolerance under Settings->Options->Digitizing->Search radius for vertex edits" ), QgsMessageBar::CRITICAL );
    return; //error during snapping
  }

  mFeatureNumber = m.featureId();
  mSnappedPoint = toCanvasCoordinates( m.point() );

  QgsFeature feature;
  if ( !mActiveLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeatureNumber ) ).nextFeature( feature ) )
  {
    return;
  }

  //check whether selected feature has a modifiable symbol
  QgsFeatureRenderer *renderer = mActiveLayer->renderer();
  if ( !renderer )
    return;
  QgsRenderContext context = QgsRenderContext::fromMapSettings( mCanvas->mapSettings() );
  context.expressionContext() << QgsExpressionContextUtils::layerScope( mActiveLayer );
  context.expressionContext().setFeature( feature );
  renderer->startRender( context, mActiveLayer->fields() );

  //test whether symbol is compatible with map tool
  bool hasCompatibleSymbol = false;
  if ( renderer->capabilities() & QgsFeatureRenderer::MoreSymbolsPerFeature )
  {
    //could be multiple symbols for this feature, so check them all
    Q_FOREACH ( QgsSymbol *s, renderer->originalSymbolsForFeature( feature, context ) )
    {
      if ( s && s->type() == QgsSymbol::Marker )
      {
        hasCompatibleSymbol = hasCompatibleSymbol || checkSymbolCompatibility( static_cast< QgsMarkerSymbol * >( s ), context );
      }
    }
  }
  else
  {
    QgsSymbol *s = renderer->originalSymbolForFeature( feature, context );
    if ( s && s->type() == QgsSymbol::Marker )
    {
      hasCompatibleSymbol = hasCompatibleSymbol || checkSymbolCompatibility( static_cast< QgsMarkerSymbol * >( s ), context );
    }
  }

  renderer->stopRender( context );
  if ( hasCompatibleSymbol )
    canvasPressOnFeature( e, feature, m.point() );
  else
    noCompatibleSymbols();
}

