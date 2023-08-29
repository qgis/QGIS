/***************************************************************************
                         qgscesiumutils.cpp
                         --------------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ******************************************************************
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscesiumutils.h"
#include "nlohmann/json.hpp"
#include "qgsjsonutils.h"
#include "qgsmatrix4x4.h"
#include "qgssphere.h"
#include "qgsorientedbox3d.h"
#include "qgslogger.h"

#include <QtCore/QBuffer>
#include <QIODevice>

QgsBox3D QgsCesiumUtils::parseRegion( const json &region )
{
  try
  {
    // The latitude and longitude values are given in radians!
    // TODO -- is this ALWAYS the case? What if there's a region root bounding volume, but a transform object present? What if there's crs metadata specifying a different crs?

    const double west = region[0].get<double>() * 180 / M_PI;
    const double south = region[1].get<double>() * 180 / M_PI;
    const double east = region[2].get<double>() * 180 / M_PI;
    const double north = region[3].get<double>() * 180 / M_PI;
    double minHeight = region[4].get<double>();
    double maxHeight = region[5].get<double>();

    return QgsBox3D( west, south, minHeight, east, north, maxHeight );
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsBox3D();
  }
}

QgsBox3D QgsCesiumUtils::parseRegion( const QVariantList &region )
{
  if ( region.size() != 6 )
    return QgsBox3D();

  return parseRegion( QgsJsonUtils::jsonFromVariant( region ) );
}

QgsOrientedBox3D QgsCesiumUtils::parseBox( const json &box )
{
  if ( box.size() != 12 )
    return QgsOrientedBox3D();

  try
  {
    QgsOrientedBox3D res;
    for ( int i = 0; i < 3; ++i )
    {
      res.mCenter[i] = box[i].get<double>();
    }
    for ( int i = 0; i < 9; ++i )
    {
      res.mHalfAxes[i] = box[i + 3].get<double>();
    }
    return res;
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsOrientedBox3D();
  }
}

QgsOrientedBox3D QgsCesiumUtils::parseBox( const QVariantList &box )
{
  if ( box.size() != 12 )
    return QgsOrientedBox3D();

  return parseBox( QgsJsonUtils::jsonFromVariant( box ) );
}

QgsSphere QgsCesiumUtils::parseSphere( const json &sphere )
{
  if ( sphere.size() != 4 )
    return QgsSphere();

  try
  {
    const double centerX = sphere[0].get<double>();
    const double centerY = sphere[1].get<double>();
    const double centerZ = sphere[2].get<double>();
    const double radius = sphere[3].get<double>();
    return QgsSphere( centerX, centerY, centerZ, radius );
  }
  catch ( nlohmann::json::exception & )
  {
    return QgsSphere();
  }
}

QgsSphere QgsCesiumUtils::parseSphere( const QVariantList &sphere )
{
  if ( sphere.size() != 4 )
    return QgsSphere();

  return parseSphere( QgsJsonUtils::jsonFromVariant( sphere ) );
}

QgsSphere QgsCesiumUtils::transformSphere( const QgsSphere &sphere, const QgsMatrix4x4 &transform )
{
  if ( !transform.isIdentity() )
  {
    // center is transformed, radius is scaled by maximum scalar from transform
    // see https://github.com/CesiumGS/cesium-native/blob/fd20f5e272850dde6b58c74059e6de767fe25df6/Cesium3DTilesSelection/src/BoundingVolume.cpp#L33
    const QgsVector3D center = transform.map( sphere.centerVector() );
    const double uniformScale = std::max(
                                  std::max(
                                    std::sqrt(
                                      transform.constData()[0] * transform.constData()[0] +
                                      transform.constData()[1] * transform.constData()[1] +
                                      transform.constData()[2] * transform.constData()[2] ),
                                    std::sqrt(
                                      transform.constData()[4] * transform.constData()[4] +
                                      transform.constData()[5] * transform.constData()[5] +
                                      transform.constData()[6] * transform.constData()[6] ) ),
                                  std::sqrt(
                                    transform.constData()[8] * transform.constData()[8] +
                                    transform.constData()[9] * transform.constData()[9] +
                                    transform.constData()[10] * transform.constData()[10] ) );

    return QgsSphere( center.x(), center.y(), center.z(), sphere.radius() * uniformScale );
  }
  return sphere;
}

QgsCesiumUtils::B3DMContents QgsCesiumUtils::extractGltfFromB3dm( const QByteArray &tileContent )
{
  struct b3dmHeader
  {
    unsigned char magic[4];
    quint32 version;
    quint32 byteLength;
    quint32 featureTableJsonByteLength;
    quint32 featureTableBinaryByteLength;
    quint32 batchTableJsonByteLength;
    quint32 batchTableBinaryByteLength;
  };

  QgsCesiumUtils::B3DMContents res;
  if ( tileContent.size() < static_cast<int>( sizeof( b3dmHeader ) ) )
    return res;

  b3dmHeader hdr;
  memcpy( &hdr, tileContent.constData(), sizeof( b3dmHeader ) );

  const QString featureTableJson( tileContent.mid( sizeof( b3dmHeader ), hdr.featureTableJsonByteLength ) );
  if ( !featureTableJson.isEmpty() )
  {
    try
    {
      const json featureTable = json::parse( featureTableJson.toStdString() );
      if ( featureTable.contains( "RTC_CENTER" ) )
      {
        const auto &rtcCenterJson = featureTable[ "RTC_CENTER" ];
        if ( rtcCenterJson.is_array() && rtcCenterJson.size() == 3 )
        {
          res.rtcCenter.setX( rtcCenterJson[0].get<double>() );
          res.rtcCenter.setY( rtcCenterJson[1].get<double>() );
          res.rtcCenter.setZ( rtcCenterJson[2].get<double>() );
        }
        else
        {
          QgsDebugError( QStringLiteral( "Invalid RTC_CENTER value" ) );
        }
      }
    }
    catch ( json::parse_error &ex )
    {
      QgsDebugError( QStringLiteral( "Error parsing feature table JSON: %1" ).arg( ex.what() ) );
    }
  }

  res.gltf = tileContent.mid( sizeof( b3dmHeader ) +
                              hdr.featureTableJsonByteLength + hdr.featureTableBinaryByteLength +
                              hdr.batchTableJsonByteLength + hdr.batchTableBinaryByteLength );
  return res;
}

QgsCesiumUtils::TileContents QgsCesiumUtils::extractGltfFromTileContent( const QByteArray &tileContent )
{
  TileContents res;
  if ( tileContent.startsWith( QByteArray( "b3dm" ) ) )
  {
    const B3DMContents b3dmContents = QgsCesiumUtils::extractGltfFromB3dm( tileContent );
    res.gltf = b3dmContents.gltf;
    res.rtcCenter = b3dmContents.rtcCenter;
    return res;
  }
  else if ( tileContent.startsWith( QByteArray( "glTF" ) ) )
  {
    res.gltf = tileContent;
    return res;
  }
  else
  {
    // unsupported tile content type
    // TODO: we could extract "b3dm" data from a composite tile ("cmpt")
    return res;
  }
}
