/***************************************************************************
  qgsqguickfeaturehighlight.cpp
  --------------------------------------
  Date                 : May 2018
  Copyright            : (C) 2018 by Peter Petrik
  Email                : zilolv at gmail dot com
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

#include "qgsquickfeaturehighlight.h"
#include "qgsquickmapsettings.h"
#include "qgsquickhighlightsgnode.h"


QgsQuickFeatureHighlight::QgsQuickFeatureHighlight( QQuickItem *parent )
  : QQuickItem( parent )
{
  setFlags( QQuickItem::ItemHasContents );
  setAntialiasing( true );

  // transform to device coords
  mTransform.appendToItem( this );

  connect( this, &QgsQuickFeatureHighlight::mapSettingsChanged, this, &QgsQuickFeatureHighlight::onMapSettingsChanged );
  connect( this, &QgsQuickFeatureHighlight::featureLayerPairChanged, this, &QgsQuickFeatureHighlight::markDirty );
  connect( this, &QgsQuickFeatureHighlight::colorChanged, this, &QgsQuickFeatureHighlight::markDirty );
  connect( this, &QgsQuickFeatureHighlight::widthChanged, this, &QgsQuickFeatureHighlight::markDirty );
}

void QgsQuickFeatureHighlight::markDirty()
{
  mDirty = true;
  update();
}

void QgsQuickFeatureHighlight::onMapSettingsChanged()
{
  mTransform.setMapSettings( mMapSettings );
  markDirty();
}

QSGNode *QgsQuickFeatureHighlight::updatePaintNode( QSGNode *n, QQuickItem::UpdatePaintNodeData * )
{
  if ( !mDirty || !mMapSettings || !mFeatureLayerPair.isValid() )
    return n;

  delete n;
  n = new QSGNode;

  QgsVectorLayer *layer = mFeatureLayerPair.layer();
  Q_ASSERT( layer ); // we checked the validity of feature-layer pair
  QgsCoordinateTransform transf( layer->crs(), mMapSettings->destinationCrs(), mMapSettings->transformContext() );

  QgsFeature feature = mFeatureLayerPair.feature();
  if ( feature.hasGeometry() )
  {
    QgsGeometry geom( feature.geometry() );
    try
    {
      geom.transform( transf );
      std::unique_ptr<QgsQuickHighlightSGNode> rb( new QgsQuickHighlightSGNode( geom, mColor, mWidth ) );
      rb->setFlag( QSGNode::OwnedByParent );
      n->appendChildNode( rb.release() );
    }
    catch ( QgsCsException &e )
    {
      Q_UNUSED( e )
      // Caught an error in transform
    }
  }
  mDirty = false;

  return n;
}
