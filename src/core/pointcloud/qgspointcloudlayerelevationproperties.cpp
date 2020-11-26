/***************************************************************************
                         qgspointcloudlayerelevationproperties.cpp
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayerelevationproperties.h"

QgsPointCloudLayerElevationProperties::QgsPointCloudLayerElevationProperties( QObject *parent )
  : QgsMapLayerElevationProperties( parent )
{
}

bool QgsPointCloudLayerElevationProperties::hasElevation() const
{
  return true;
}

QDomElement QgsPointCloudLayerElevationProperties::writeXml( QDomElement &, QDomDocument &document, const QgsReadWriteContext & )
{
  QDomElement element = document.createElement( QStringLiteral( "elevation" ) );
  return element;
}

bool QgsPointCloudLayerElevationProperties::readXml( const QDomElement &, const QgsReadWriteContext & )
{
  return true;
}

bool QgsPointCloudLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange & ) const
{
  // TODO -- test actual point cloud z range
  return true;
}

QgsDoubleRange QgsPointCloudLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  // TODO - retrieve range from point cloud data provider
  return QgsDoubleRange();
}
