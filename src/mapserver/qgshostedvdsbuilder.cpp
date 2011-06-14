/***************************************************************************
                              qgshostedvdsbuilder.cpp
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

#include "qgshostedvdsbuilder.h"
#include "qgsmslayercache.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayer.h"
#include <QDomElement>

QgsHostedVDSBuilder::QgsHostedVDSBuilder(): QgsMSLayerBuilder()
{

}

QgsHostedVDSBuilder::~QgsHostedVDSBuilder()
{

}

QgsMapLayer* QgsHostedVDSBuilder::createMapLayer( const QDomElement& elem, const QString& layerName, QList<QTemporaryFile*>& filesToRemove, QList<QgsMapLayer*>& layersToRemove, bool allowCaching ) const
{
  if ( elem.isNull() )
  {
    return 0;
  }

  QString providerType = elem.attribute( "providerType", "not found" );
  QString uri = elem.attribute( "uri", "not found" );

  if ( providerType == "not found" || uri == "not found" )
  {
    QgsDebugMsg( "error, provider type not found" );
    return 0;
  }

  QgsMapLayer* ml = 0;

  if ( allowCaching ) //take layer from cache if allowed
  {
    QgsDebugMsg( "Taking hostedvds layer from cash" );
    ml = QgsMSLayerCache::instance()->searchLayer( uri, layerName );
  }

  if ( !ml )
  {
    QgsDebugMsg( "hostedvds layer not in cash, so create and insert it" );
    ml = new QgsVectorLayer( uri, layerNameFromUri( uri ), providerType );

    if ( !ml || !ml->isValid() )
    {
      QgsDebugMsg( "error, VectorLayer is 0 or invalid" );
      delete ml;
      return 0;
    }

    if ( allowCaching )
    {
      QgsMSLayerCache::instance()->insertLayer( uri, layerName, ml );
    }
    else
    {
      layersToRemove.push_back( ml );
    }
  }

  //projection
  if ( ml )
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
        ml->setCrs( srs );
      }
    }
  }

  return ml;
}
