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

#include <memory>

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

  QgsVectorLayer *layer = mModel->feature().layer();
  if ( layer )
  {
    QgsCoordinateTransform transf( layer->crs(), mMapSettings->destinationCrs(), mMapSettings->transformContext() );

    QgsFeature feature = mModel->feature().feature();
    if ( feature.hasGeometry() )
    {
      QgsGeometry geom( feature.geometry() );
      geom.transform( transf );
      std::unique_ptr<QgsQuickHighlightSGNode> rb( new QgsQuickHighlightSGNode( geom, mColor, mWidth ) );
      rb->setFlag( QSGNode::OwnedByParent );
      n->appendChildNode( rb.release() );
    }
  }
  mDirty = false;

  return n;
}
