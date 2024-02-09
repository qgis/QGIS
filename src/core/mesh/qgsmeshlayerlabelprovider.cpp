/***************************************************************************
                         qgsmeshlayerlabelprovider.cpp
                         ---------------------
    begin                : November 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshlayerlabelprovider.h"

#include "qgsgeometry.h"
#include "qgslabelsearchtree.h"
#include "qgspallabeling.h"
#include "qgstextlabelfeature.h"
#include "qgsmeshlayer.h"
#include "qgsvectorlayer.h"
#include "qgsrenderer.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmultipolygon.h"
#include "qgslogger.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmaskidprovider.h"
#include "qgstextcharacterformat.h"
#include "qgstextfragment.h"
#include "qgslabelingresults.h"
#include "qgstextrenderer.h"
#include "qgstriangularmesh.h"

#include "feature.h"
#include "labelposition.h"
#include "qgssymbol.h"
#include "qgsmarkersymbol.h"

#include "pal/layer.h"

#include <QPicture>
#include <QTextDocument>
#include <QTextFragment>

using namespace pal;

QgsMeshLayerLabelProvider::QgsMeshLayerLabelProvider( QgsMeshLayer *layer, const QString &providerId, const QgsPalLayerSettings *settings, const QString &layerName, bool labelFaces )
  : QgsAbstractLabelProvider( layer, providerId )
  , mSettings( settings ? * settings : QgsPalLayerSettings() )
  , mLabelFaces( labelFaces )
  , mCrs( layer->crs() )
{
  mName = layerName.isEmpty() ? layer->id() : layerName;

  init();
}

void QgsMeshLayerLabelProvider::init()
{
  mPlacement = mSettings.placement;

  mFlags = Flags();
  if ( mSettings.drawLabels )
    mFlags |= DrawLabels;
  if ( mSettings.lineSettings().mergeLines() && !mSettings.lineSettings().addDirectionSymbol() )
    mFlags |= MergeConnectedLines;
  if ( mSettings.centroidInside )
    mFlags |= CentroidMustBeInside;

  mPriority = 1 - mSettings.priority / 10.0; // convert 0..10 --> 1..0

  mVectorLabelProvider = std::make_unique<QgsVectorLayerLabelProvider>(
                           mLabelFaces ? Qgis::GeometryType::Polygon : Qgis::GeometryType::Point,
                           QgsFields(),
                           mCrs,
                           QString(),
                           &mSettings,
                           mLayer );

  if ( mLabelFaces )
  {
    //override obstacle type to treat any intersection of a label with the point symbol as a high cost conflict
    mObstacleType = QgsLabelObstacleSettings::ObstacleType::PolygonWhole;
  }
  else
  {
    mObstacleType = mSettings.obstacleSettings().type();
  }

  mUpsidedownLabels = mSettings.upsidedownLabels;
}


QgsMeshLayerLabelProvider::~QgsMeshLayerLabelProvider()
{
  qDeleteAll( mLabels );
}

bool QgsMeshLayerLabelProvider::prepare( QgsRenderContext &context, QSet<QString> &attributeNames )
{
  const QgsMapSettings &mapSettings = mEngine->mapSettings();

  mVectorLabelProvider->setEngine( mEngine );

  return mSettings.prepare( context, attributeNames, QgsFields(), mapSettings, mCrs );
}

void QgsMeshLayerLabelProvider::startRender( QgsRenderContext &context )
{
  QgsAbstractLabelProvider::startRender( context );
  mSettings.startRender( context );
}

void QgsMeshLayerLabelProvider::stopRender( QgsRenderContext &context )
{
  QgsAbstractLabelProvider::stopRender( context );
  mSettings.stopRender( context );
}

QList<QgsLabelFeature *> QgsMeshLayerLabelProvider::labelFeatures( QgsRenderContext &ctx )
{
  Q_UNUSED( ctx );
  return mLabels;
}

QList< QgsLabelFeature * > QgsMeshLayerLabelProvider::registerFeature( const QgsFeature &feature, QgsRenderContext &context, const QgsGeometry &obstacleGeometry, const QgsSymbol *symbol )
{
  std::unique_ptr< QgsLabelFeature > label = mSettings.registerFeatureWithDetails( feature, context, obstacleGeometry, symbol );
  QList< QgsLabelFeature * > res;
  if ( label )
  {
    res << label.get();
    mLabels << label.release();
  }
  return res;
}

const QgsPalLayerSettings &QgsMeshLayerLabelProvider::settings() const
{
  return mSettings;
}

void QgsMeshLayerLabelProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  mVectorLabelProvider->drawLabel( context, label );
}
