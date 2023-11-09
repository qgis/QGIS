/***************************************************************************
  qgslayertreefiltersettings.cpp
  --------------------------------------
  Date                 : March 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayertreefiltersettings.h"
#include "qgsmapsettings.h"
#include "qgslayertreeutils.h"
#include "qgslayertree.h"
#include "qgsmaplayerlistutils_p.h"
#include "qgsreferencedgeometry.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"

QgsLayerTreeFilterSettings::QgsLayerTreeFilterSettings( const QgsMapSettings &settings )
  : mMapSettings( std::make_unique<QgsMapSettings>( settings ) )
{
  mLayers = _qgis_listRawToQPointer( mMapSettings->layers( true ) );
}

QgsLayerTreeFilterSettings::~QgsLayerTreeFilterSettings() = default;

QgsLayerTreeFilterSettings::QgsLayerTreeFilterSettings( const QgsLayerTreeFilterSettings &other )
  : mLayerFilterExpressions( other.mLayerFilterExpressions )
  , mMapSettings( other.mMapSettings ? new QgsMapSettings( *other.mMapSettings ) : nullptr )
  , mFilterPolygon( other.mFilterPolygon )
  , mFlags( other.mFlags )
  , mLayers( other.mLayers )
  , mLayerExtents( other.mLayerExtents )
{

}

QgsLayerTreeFilterSettings &QgsLayerTreeFilterSettings::operator=( const QgsLayerTreeFilterSettings &other )
{
  mLayerFilterExpressions = other.mLayerFilterExpressions;
  mMapSettings.reset( other.mMapSettings ? new QgsMapSettings( *other.mMapSettings ) : nullptr );
  mFilterPolygon = other.mFilterPolygon;
  mFlags = other.mFlags;
  mLayers = other.mLayers;
  mLayerExtents = other.mLayerExtents;
  return *this;
}

QgsMapSettings &QgsLayerTreeFilterSettings::mapSettings()
{
  return *mMapSettings.get();
}

QMap<QString, QString> QgsLayerTreeFilterSettings::layerFilterExpressions() const
{
  return mLayerFilterExpressions;
}

void QgsLayerTreeFilterSettings::setLayerFilterExpressions( const QMap<QString, QString> &expressions )
{
  mLayerFilterExpressions = expressions;
}

void QgsLayerTreeFilterSettings::setLayerFilterExpressionsFromLayerTree( QgsLayerTree *tree )
{
  QMap<QString, QString> legendFilterExpressions;
  const QList<QgsLayerTreeLayer *> layers = tree->findLayers();
  for ( QgsLayerTreeLayer *nodeLayer : layers )
  {
    bool enabled = false;
    const QString legendFilterExpression = QgsLayerTreeUtils::legendFilterByExpression( *nodeLayer, &enabled );
    if ( enabled && !legendFilterExpression.isEmpty() )
    {
      legendFilterExpressions[nodeLayer->layerId()] = legendFilterExpression;
    }
  }
  mLayerFilterExpressions = legendFilterExpressions;
}

QString QgsLayerTreeFilterSettings::layerFilterExpression( const QString &layerId ) const
{
  return mLayerFilterExpressions.value( layerId );
}

QgsGeometry QgsLayerTreeFilterSettings::filterPolygon() const
{
  return mFilterPolygon;
}

void QgsLayerTreeFilterSettings::setFilterPolygon( const QgsGeometry &newFilterPolygon )
{
  mFilterPolygon = newFilterPolygon;
}

Qgis::LayerTreeFilterFlags QgsLayerTreeFilterSettings::flags() const
{
  return mFlags;
}

void QgsLayerTreeFilterSettings::setFlags( Qgis::LayerTreeFilterFlags flags )
{
  mFlags = flags;
}

void QgsLayerTreeFilterSettings::addVisibleExtentForLayer( QgsMapLayer *layer, const QgsReferencedGeometry &polygon )
{
  const QgsCoordinateTransform polygonToLayerTransform( polygon.crs(), layer->crs(), mMapSettings->transformContext() );
  try
  {
    QgsGeometry transformedPoly = polygon;
    transformedPoly.transform( polygonToLayerTransform );
    mLayerExtents[ layer->id() ].append( transformedPoly );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( QStringLiteral( "Error transforming polygon to layer CRS for legend filtering" ) );
  }
  if ( !mLayers.contains( layer ) )
    mLayers << layer;
}

QgsGeometry QgsLayerTreeFilterSettings::combinedVisibleExtentForLayer( const QgsMapLayer *layer )
{
  QVector< QgsGeometry > parts;

  if ( mMapSettings->layerIds( true ).contains( layer->id() ) )
  {
    // add visible polygon in layer CRS
    QgsGeometry mapExtent = mFilterPolygon;
    if ( mapExtent.isEmpty() )
    {
      mapExtent = QgsGeometry::fromQPolygonF( mMapSettings->visiblePolygon() );
    }

    const QgsCoordinateTransform layerToMapTransform = mMapSettings->layerTransform( layer );
    try
    {
      mapExtent.transform( layerToMapTransform, Qgis::TransformDirection::Reverse );
      parts << mapExtent;
    }
    catch ( QgsCsException & )
    {
      QgsDebugError( QStringLiteral( "Error transforming map extent to layer CRS for legend filtering" ) );
    }
  }

  auto additionalIt = mLayerExtents.constFind( layer->id() );
  if ( additionalIt != mLayerExtents.constEnd() )
  {
    parts.append( additionalIt.value() );
  }

  if ( !parts.empty() )
    return QgsGeometry::unaryUnion( parts );
  else
    return QgsGeometry();
}

QList<QgsMapLayer *> QgsLayerTreeFilterSettings::layers() const
{
  return _qgis_listQPointerToRaw( mLayers );
}
