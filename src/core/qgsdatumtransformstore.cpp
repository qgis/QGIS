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
#include "qgscoordinatetransform.h"
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

QgsCoordinateTransform QgsDatumTransformStore::transformation( QgsMapLayer* layer ) const
{
  if ( !layer )
    return QgsCoordinateTransform();

  QString srcAuthId = layer->crs().authid();
  QString dstAuthId = mDestCRS.authid();

  if ( srcAuthId == dstAuthId )
  {
    return QgsCoordinateTransform();
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

void QgsDatumTransformStore::readXml( const QDomNode& parentNode )
{
  clear();

  QDomElement layerCoordTransformInfoElem = parentNode.firstChildElement( QStringLiteral( "layer_coordinate_transform_info" ) );
  if ( !layerCoordTransformInfoElem.isNull() )
  {
    QDomNodeList layerCoordinateTransformList = layerCoordTransformInfoElem.elementsByTagName( QStringLiteral( "layer_coordinate_transform" ) );
    QDomElement layerCoordTransformElem;
    for ( int i = 0; i < layerCoordinateTransformList.size(); ++i )
    {
      layerCoordTransformElem = layerCoordinateTransformList.at( i ).toElement();
      QString layerId = layerCoordTransformElem.attribute( QStringLiteral( "layerid" ) );
      if ( layerId.isEmpty() )
      {
        continue;
      }

      addEntry( layerId,
                layerCoordTransformElem.attribute( QStringLiteral( "srcAuthId" ) ),
                layerCoordTransformElem.attribute( QStringLiteral( "destAuthId" ) ),
                layerCoordTransformElem.attribute( QStringLiteral( "srcDatumTransform" ), QStringLiteral( "-1" ) ).toInt(),
                layerCoordTransformElem.attribute( QStringLiteral( "destDatumTransform" ), QStringLiteral( "-1" ) ).toInt()
              );
    }
  }
}

void QgsDatumTransformStore::writeXml( QDomNode& parentNode, QDomDocument& theDoc ) const
{
  // layer coordinate transform infos
  QDomElement layerCoordTransformInfo = theDoc.createElement( QStringLiteral( "layer_coordinate_transform_info" ) );

  for ( QHash< QString, Entry >::const_iterator coordIt = mEntries.constBegin(); coordIt != mEntries.constEnd(); ++coordIt )
  {
    QDomElement layerCoordTransformElem = theDoc.createElement( QStringLiteral( "layer_coordinate_transform" ) );
    layerCoordTransformElem.setAttribute( QStringLiteral( "layerid" ), coordIt.key() );
    layerCoordTransformElem.setAttribute( QStringLiteral( "srcAuthId" ), coordIt->srcAuthId );
    layerCoordTransformElem.setAttribute( QStringLiteral( "destAuthId" ), coordIt->destAuthId );
    layerCoordTransformElem.setAttribute( QStringLiteral( "srcDatumTransform" ), QString::number( coordIt->srcDatumTransform ) );
    layerCoordTransformElem.setAttribute( QStringLiteral( "destDatumTransform" ), QString::number( coordIt->destDatumTransform ) );
    layerCoordTransformInfo.appendChild( layerCoordTransformElem );
  }
  parentNode.appendChild( layerCoordTransformInfo );
}
