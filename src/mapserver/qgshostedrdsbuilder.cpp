
/***************************************************************************
                              qgshostedrdsbuilder.cpp
                              -----------------------
  begin                : July, 2008
  copyright            : (C) 2008 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshostedrdsbuilder.h"
#include "qgslogger.h"
#include "qgsmslayercache.h"
#include "qgsrasterlayer.h"
#include "qgscoordinatereferencesystem.h"
#include <QDomElement>

QgsHostedRDSBuilder::QgsHostedRDSBuilder(): QgsMSLayerBuilder()
{

}

QgsHostedRDSBuilder::~QgsHostedRDSBuilder()
{

}

QgsMapLayer* QgsHostedRDSBuilder::createMapLayer( const QDomElement& elem,
    const QString &layerName,
    QList<QTemporaryFile*> &filesToRemove,
    QList<QgsMapLayer*> &layersToRemove,
    bool allowCaching ) const
{
  Q_UNUSED( filesToRemove );
  QgsDebugMsg( "entering." );

  if ( elem.isNull() )
  {
    return 0;
  }

  QString uri = elem.attribute( "uri", "not found" );
  if ( uri == "not found" )
  {
    QgsDebugMsg( "Uri not found" );
    return 0;
  }
  else
  {
    QgsDebugMsg( "Trying to get hostedrds layer from cache with uri: " + uri );
    QgsRasterLayer* rl = 0;
    if ( allowCaching )
    {
      rl = dynamic_cast<QgsRasterLayer*>( QgsMSLayerCache::instance()->searchLayer( uri, layerName ) );
    }
    if ( !rl )
    {
      QgsDebugMsg( "hostedrds layer not in cache, so create and insert it" );
      rl = new QgsRasterLayer( uri, layerNameFromUri( uri ) );
      if ( allowCaching )
      {
        QgsMSLayerCache::instance()->insertLayer( uri, layerName, rl, "" /*todo: add project file path*/ );
      }
      else
      {
        layersToRemove.push_back( rl );
      }
    }

    clearRasterSymbology( rl );

    //projection
    if ( rl )
    {
      QString epsg = elem.attribute( "epsg" );
      if ( !epsg.isEmpty() )
      {
        bool conversionOk;
        int epsgnr = epsg.toInt( &conversionOk );
        if ( conversionOk )
        {
          //set spatial ref sys
          QgsCoordinateReferenceSystem srs;
          srs.createFromOgcWmsCrs( QString( "EPSG:%1" ).arg( epsgnr ) );
          rl->setCrs( srs );
        }
      }
    }

    return rl;
  }
}
