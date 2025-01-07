/***************************************************************************
    qgspointcloudlayereditutils.cpp
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudlayereditutils.h"
#include "qgspointcloudlayer.h"
#include "qgslazdecoder.h"


QgsPointCloudLayerEditUtils::QgsPointCloudLayerEditUtils( QgsPointCloudLayer *layer )
  : mIndex( layer->index() )
{
}

bool QgsPointCloudLayerEditUtils::changeAttributeValue( const QgsPointCloudNodeId &n, const QVector<int> &pts, const QgsPointCloudAttribute &attribute, double value )
{
  // Cannot allow x,y,z editing as points may get moved outside the node extents
  if ( attribute.name().compare( QLatin1String( "X" ), Qt::CaseInsensitive ) == 0 ||
       attribute.name().compare( QLatin1String( "Y" ), Qt::CaseInsensitive ) == 0 ||
       attribute.name().compare( QLatin1String( "Z" ), Qt::CaseInsensitive ) == 0 )
    return false;

  if ( !n.isValid() || !mIndex.hasNode( n ) ) // todo: should not have to check if n.isValid
    return false;

  const QgsPointCloudAttributeCollection attributeCollection = mIndex.attributes();

  int attributeOffset;
  const QgsPointCloudAttribute *at = attributeCollection.find( attribute.name(), attributeOffset );

  if ( !at ||
       at->size() != attribute.size() ||
       at->type() != attribute.type() )
  {
    return false;
  }

  if ( !isAttributeValueValid( attribute, value ) )
  {
    return false;
  }

  const QSet<int> uniquePoints( pts.constBegin(), pts.constEnd() );
  QVector<int> sortedPoints( uniquePoints.constBegin(), uniquePoints.constEnd() );
  std::sort( sortedPoints.begin(), sortedPoints.end() );

  if ( sortedPoints.constFirst() < 0 ||
       sortedPoints.constLast() > mIndex.getNode( n ).pointCount() )
    return false;

  QgsPointCloudRequest req;
  req.setAttributes( attributeCollection );

  std::unique_ptr<QgsPointCloudBlock> block = mIndex.nodeData( n, req );
  const int count = block->pointCount();
  const int recordSize = attributeCollection.pointRecordSize();

  // copy data
  QByteArray data( block->data(), count * recordSize );

  char *ptr = data.data();

  for ( int i : sortedPoints )
  {
    // replace attribute for selected point
    lazStoreToStream_( ptr, i * recordSize + attributeOffset, attribute.type(), value );
  }

  return mIndex.updateNodeData( {{n, data}} );;
}

QByteArray QgsPointCloudLayerEditUtils::dataForAttributes( const QgsPointCloudAttributeCollection &allAttributes, const QByteArray &data, const QgsPointCloudRequest &request )
{
  const QVector<QgsPointCloudAttribute> attributes = allAttributes.attributes();
  const int nPoints = data.size() / allAttributes.pointRecordSize();
  const char *ptr = data.data();

  QByteArray outData;
  for ( int i = 0; i < nPoints; ++i )
  {
    for ( const QgsPointCloudAttribute &attr : attributes )
    {
      if ( request.attributes().indexOf( attr.name() ) >= 0 )
      {
        outData.append( ptr, attr.size() );
      }
      ptr += attr.size();
    }
  }

  //
  Q_ASSERT( nPoints == outData.size() / request.attributes().pointRecordSize() );

  return outData;
}

bool QgsPointCloudLayerEditUtils::isAttributeValueValid( const QgsPointCloudAttribute &attribute, double value )
{
  const QString name = attribute.name().toUpper();

  if ( name == QLatin1String( "INTENSITY" ) )
    return value >= 0 && value <= 65535;
  if ( name == QLatin1String( "RETURNNUMBER" ) )
    return value >= 0 && value <= 15;
  if ( name == QLatin1String( "NUMBEROFRETURNS" ) )
    return value >= 0 && value <= 15;
  if ( name == QLatin1String( "SCANCHANNEL" ) )
    return value >= 0 && value <= 3;
  if ( name == QLatin1String( "SCANDIRECTIONFLAG" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "EDGEOFFLIGHTLINE" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "CLASSIFICATION" ) )
    return value >= 0 && value <= 255;
  if ( name == QLatin1String( "USERDATA" ) )
    return value >= 0 && value <= 255;
  if ( name == QLatin1String( "SCANANGLE" ) )
    return value >= -30'000 && value <= 30'000;
  if ( name == QLatin1String( "POINTSOURCEID" ) )
    return value >= 0 && value <= 65535;
  if ( name == QLatin1String( "GPSTIME" ) )
    return value >= 0;
  if ( name == QLatin1String( "SYNTHETIC" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "KEYPOINT" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "WITHHELD" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "OVERLAP" ) )
    return value >= 0 && value <= 1;
  if ( name == QLatin1String( "RED" ) )
    return value >= 0 && value <= 65535;
  if ( name == QLatin1String( "GREEN" ) )
    return value >= 0 && value <= 65535;
  if ( name == QLatin1String( "BLUE" ) )
    return value >= 0 && value <= 65535;

  return true;
}
