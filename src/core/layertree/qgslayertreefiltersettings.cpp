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

QgsLayerTreeFilterSettings::QgsLayerTreeFilterSettings( const QgsMapSettings &settings )
  : mMapSettings( std::make_unique<QgsMapSettings>( settings ) )
{}

QgsLayerTreeFilterSettings::~QgsLayerTreeFilterSettings() = default;

QgsLayerTreeFilterSettings::QgsLayerTreeFilterSettings( const QgsLayerTreeFilterSettings &other )
  : mLayerFilterExpressions( other.mLayerFilterExpressions )
  , mMapSettings( other.mMapSettings ? new QgsMapSettings( *other.mMapSettings ) : nullptr )
  , mFilterPolygon( other.mFilterPolygon )
  , mFlags( other.mFlags )
{

}

QgsLayerTreeFilterSettings &QgsLayerTreeFilterSettings::operator=( const QgsLayerTreeFilterSettings &other )
{
  mLayerFilterExpressions = other.mLayerFilterExpressions;
  mMapSettings.reset( other.mMapSettings ? new QgsMapSettings( *other.mMapSettings ) : nullptr );
  mFilterPolygon = other.mFilterPolygon;
  mFlags = other.mFlags;
  return *this;
}

void QgsLayerTreeFilterSettings::setMapSettings( const QgsMapSettings &settings )
{
  mMapSettings.reset( new QgsMapSettings( settings ) );
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

QList<QgsMapLayer *> QgsLayerTreeFilterSettings::layers() const
{
  return mMapSettings->layers( true );
}
