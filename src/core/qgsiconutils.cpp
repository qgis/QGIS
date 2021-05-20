/***************************************************************************
                             qgsiconutils.cpp
                             -------------------
    begin                : May 2021
    copyright            : (C) 2021 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsiconutils.h"
#include "qgsapplication.h"
#include <QIcon>

QIcon QgsIconUtils::iconForWkbType( QgsWkbTypes::Type type )
{
  QgsWkbTypes::GeometryType geomType = QgsWkbTypes::geometryType( QgsWkbTypes::Type( type ) );
  switch ( geomType )
  {
    case QgsWkbTypes::NullGeometry:
      return iconTable();
    case QgsWkbTypes::PointGeometry:
      return iconPoint();
    case QgsWkbTypes::LineGeometry:
      return iconLine();
    case QgsWkbTypes::PolygonGeometry:
      return iconPolygon();
    default:
      break;
  }
  return iconDefaultLayer();
}

QIcon QgsIconUtils::iconPoint()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) );
}

QIcon QgsIconUtils::iconLine()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) );
}

QIcon QgsIconUtils::iconPolygon()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) );
}

QIcon QgsIconUtils::iconTable()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconTableLayer.svg" ) );
}

QIcon QgsIconUtils::iconRaster()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconRaster.svg" ) );
}

QIcon QgsIconUtils::iconMesh()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconMeshLayer.svg" ) );
}

QIcon QgsIconUtils::iconVectorTile()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconVectorTileLayer.svg" ) );
}

QIcon QgsIconUtils::iconPointCloud()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointCloudLayer.svg" ) );
}

QIcon QgsIconUtils::iconDefaultLayer()
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconLayer.png" ) );
}

