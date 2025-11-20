/***************************************************************************
                          qgsdevelopersmapcanvas.cpp
                             -------------------
    begin                : November 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "moc_qgsdevelopersmapcanvas.cpp"
#include "qgsdevelopersmapcanvas.h"
#include "qgsapplication.h"


QgsDevelopersMapCanvas::QgsDevelopersMapCanvas( QWidget *parent )
  : QgsMapCanvas( parent )
{
  mDevelopersMapBaseLayer = std::make_unique<QgsRasterLayer>( QStringLiteral( "type=xyz&tilePixelRatio=1&url=https://tile.openstreetmap.org/%7Bz%7D/%7Bx%7D/%7By%7D.png&zmax=19&zmin=0&crs=EPSG3857" ), QStringLiteral( "OpenStreetMap" ), QLatin1String( "wms" ) );
  mDevelopersMapLayer = std::make_unique<QgsVectorLayer>( QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/contributors.json" ), tr( "Contributors" ), QLatin1String( "ogr" ) );

  QgsCoordinateTransform transform( mDevelopersMapLayer->crs(), mDevelopersMapBaseLayer->crs(), QgsProject::instance()->transformContext() );
  QgsRectangle extent = mDevelopersMapLayer->extent();
  try
  {
    extent = transform.transform( extent );
  }
  catch ( const QgsException &e )
  {
    extent = mDevelopersMapBaseLayer->extent();
  }

  mapSettings().setOutputSize( size() );
  mapSettings().setLayers( QList<QgsMapLayer *>() << mDevelopersMapLayer.get() << mDevelopersMapBaseLayer.get() );
  mapSettings().setDestinationCrs( mDevelopersMapBaseLayer->crs() );
  mapSettings().setExtent( extent );
  refresh();

  mDevelopersMapTool = std::make_unique<QgsMapToolPan>( this );
  setMapTool( mDevelopersMapTool.get() );
}
