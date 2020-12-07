/***************************************************************************
    qgsmapcanvasutils.cpp
    -------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmapcanvasutils.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"

long QgsMapCanvasUtils::zoomToMatchingFeatures( QgsMapCanvas *canvas, QgsVectorLayer *layer, const QString &filter )
{
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );

  QgsFeatureRequest request = QgsFeatureRequest().setFilterExpression( filter )
                              .setExpressionContext( context )
                              .setNoAttributes();

  QgsFeatureIterator features = layer->getFeatures( request );

  QgsRectangle bbox;
  bbox.setMinimal();
  QgsFeature feat;
  int featureCount = 0;
  while ( features.nextFeature( feat ) )
  {
    QgsGeometry geom = feat.geometry();
    if ( geom.isNull() || geom.constGet()->isEmpty() )
      continue;

    QgsRectangle r = canvas->mapSettings().layerExtentToOutputExtent( layer, geom.boundingBox() );
    bbox.combineExtentWith( r );
    featureCount++;
  }
  features.close();

  if ( featureCount > 0 )
  {
    canvas->zoomToFeatureExtent( bbox );
  }
  return featureCount;
}

long QgsMapCanvasUtils::flashMatchingFeatures( QgsMapCanvas *canvas, QgsVectorLayer *layer, const QString &filter )
{
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) );

  QgsFeatureRequest request = QgsFeatureRequest().setFilterExpression( filter )
                              .setExpressionContext( context )
                              .setNoAttributes();

  QgsFeatureIterator features = layer->getFeatures( request );
  QgsFeature feat;
  QList< QgsGeometry > geoms;
  while ( features.nextFeature( feat ) )
  {
    if ( feat.hasGeometry() )
      geoms << feat.geometry();
  }

  if ( !geoms.empty() )
  {
    canvas->flashGeometries( geoms, layer->crs() );
  }
  return geoms.size();
}
