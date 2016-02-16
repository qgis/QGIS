/***************************************************************************
    qgsdatumtransformstore.cpp
    ---------------------
    begin                : June 2014
    copyright            : (C) 2014 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatumtransformstore.h"

#include "qgscrscache.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"

QgsDatumTransformStore::QgsDatumTransformStore( const QgsCoordinateReferenceSystem& destCrs )
    : mDestCRS( destCrs )
{
}

void QgsDatumTransformStore::clear()
{
  mEntries.clear();
}

void QgsDatumTransformStore::setDestinationCrs( const QgsCoordinateReferenceSystem& destCrs )
{
  mDestCRS = destCrs;
  clear();
}

void QgsDatumTransformStore::addEntry( const QString& layerId, const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform, int destDatumTransform )
{
  Entry lt;
  lt.srcAuthId = srcAuthId;
  lt.destAuthId = destAuthId;
  lt.srcDatumTransform = srcDatumTransform;
  lt.destDatumTransform = destDatumTransform;
  mEntries.insert( layerId, lt );
}

bool QgsDatumTransformStore::hasEntryForLayer( QgsMapLayer* layer ) const
{
  return mEntries.contains( layer->id() );
}

const QgsCoordinateTransform* QgsDatumTransformStore::transformation( QgsMapLayer* layer ) const
{
  if ( !layer )
    return nullptr;

  QString srcAuthId = layer->crs().authid();
  QString dstAuthId = mDestCRS.authid();

  if ( srcAuthId == dstAuthId )
  {
    return nullptr;
  }

  QHash< QString, Entry >::const_iterator ctIt = mEntries.find( layer->id() );
  if ( ctIt != mEntries.constEnd() && ctIt->srcAuthId == srcAuthId && ctIt->destAuthId == dstAuthId )
  {
    return QgsCoordinateTransformCache::instance()->transform( ctIt->srcAuthId, ctIt->destAuthId, ctIt->srcDatumTransform, ctIt->destDatumTransform );
  }
  else
  {
    return QgsCoordinateTransformCache::instance()->transform( srcAuthId, dstAuthId );
  }
}

void QgsDatumTransformStore::readXML( const QDomNode& parentNode )
{
  clear();

  QDomElement layerCoordTransformInfoElem = parentNode.firstChildElement( "layer_coordinate_transform_info" );
  if ( !layerCoordTransformInfoElem.isNull() )
  {
    QDomNodeList layerCoordinateTransformList = layerCoordTransformInfoElem.elementsByTagName( "layer_coordinate_transform" );
    QDomElement layerCoordTransformElem;
    for ( int i = 0; i < layerCoordinateTransformList.size(); ++i )
    {
      layerCoordTransformElem = layerCoordinateTransformList.at( i ).toElement();
      QString layerId = layerCoordTransformElem.attribute( "layerid" );
      if ( layerId.isEmpty() )
      {
        continue;
      }

      addEntry( layerId,
                layerCoordTransformElem.attribute( "srcAuthId" ),
                layerCoordTransformElem.attribute( "destAuthId" ),
                layerCoordTransformElem.attribute( "srcDatumTransform", "-1" ).toInt(),
                layerCoordTransformElem.attribute( "destDatumTransform", "-1" ).toInt()
              );
    }
  }
}

void QgsDatumTransformStore::writeXML( QDomNode& parentNode, QDomDocument& theDoc ) const
{
  // layer coordinate transform infos
  QDomElement layerCoordTransformInfo = theDoc.createElement( "layer_coordinate_transform_info" );

  for ( QHash< QString, Entry >::const_iterator coordIt = mEntries.constBegin(); coordIt != mEntries.constEnd(); ++coordIt )
  {
    QDomElement layerCoordTransformElem = theDoc.createElement( "layer_coordinate_transform" );
    layerCoordTransformElem.setAttribute( "layerid", coordIt.key() );
    layerCoordTransformElem.setAttribute( "srcAuthId", coordIt->srcAuthId );
    layerCoordTransformElem.setAttribute( "destAuthId", coordIt->destAuthId );
    layerCoordTransformElem.setAttribute( "srcDatumTransform", QString::number( coordIt->srcDatumTransform ) );
    layerCoordTransformElem.setAttribute( "destDatumTransform", QString::number( coordIt->destDatumTransform ) );
    layerCoordTransformInfo.appendChild( layerCoordTransformElem );
  }
  parentNode.appendChild( layerCoordTransformInfo );
}
