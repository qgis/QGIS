/***************************************************************************
  qgsqguickfeaturehighlight.cpp
  --------------------------------------
  Date                 : 9.12.2014
  Copyright            : (C) 2014 by Matthias Kuhn
  Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayer.h"

#include "qgsquickfeaturemodel.h"
#include "qgsquickfeaturehighlight.h"
#include "qgsquickmapsettings.h"
#include "qgsquickhighlightsgnode.h"


QgsQuickFeatureHighlight::QgsQuickFeatureHighlight( QQuickItem *parent )
  : QQuickItem( parent )
{
  setFlags( QQuickItem::ItemHasContents );
  setAntialiasing( true );

  connect( this, &QgsQuickFeatureHighlight::modelChanged, this, &QgsQuickFeatureHighlight::onDataChanged );
}

void QgsQuickFeatureHighlight::onDataChanged()
{
  if ( mModel )
  {
    connect( mModel, &QgsQuickFeatureModel::modelReset, this, &QgsQuickFeatureHighlight::onModelDataChanged );
    connect( mModel, &QgsQuickFeatureModel::rowsRemoved, this, &QgsQuickFeatureHighlight::onModelDataChanged );
  }

  onModelDataChanged();
}

void QgsQuickFeatureHighlight::onModelDataChanged()
{
  mDirty = true;
  update();
}

QSGNode *QgsQuickFeatureHighlight::updatePaintNode( QSGNode *n, QQuickItem::UpdatePaintNodeData * )
{
  if ( !mDirty || !mMapSettings )
    return n;

  delete n;
  n = new QSGNode;

  if ( !mModel )
    return n;

  QgsVectorLayer *layer = mModel->layer();
  if ( layer )
  {
    QgsCoordinateTransform transf( layer->crs(), mMapSettings->destinationCrs(), mMapSettings->transformContext() );

    QgsFeature feature = mModel->feature();
    QgsGeometry geom( feature.geometry() );
    geom.transform( transf );

    // TODO: this is very crude conversion! QgsQuickHighlightsNode should accept any type of geometry
    QVector<QgsPoint> points;
    for ( auto it = geom.vertices_begin(); it != geom.vertices_end(); ++it )
      points.append( *it );

    QgsQuickHighlightSGNode *rb = new QgsQuickHighlightSGNode( points, geom.type(), mColor, mWidth );
    rb->setFlag( QSGNode::OwnedByParent );
    n->appendChildNode( rb );
  }
  mDirty = false;

  return n;
}
