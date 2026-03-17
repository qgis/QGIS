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

#include <cstring>
#include <nlohmann/json.hpp>

#include "qgsblockingnetworkrequest.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsgltfutils.h"
#include "qgsjsonutils.h"
#include "qgslogger.h"
#include "qgsmatrix4x4.h"
#include "qgsorientedbox3d.h"
#include "qgssphere.h"
#include "qgstiledsceneboundingvolume.h"
#include "tiny_gltf.h"

#include <QFile>
#include <QGenericMatrix>
#include <QIODevice>
#include <QMatrix3x3>
#include <QNetworkRequest>
#include <QString>
#include <QUrlQuery>
#include <QtCore/QBuffer>

using namespace Qt::StringLiterals;

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
        std::sqrt( transform.constData()[0] * transform.constData()[0] + transform.constData()[1] * transform.constData()[1] + transform.constData()[2] * transform.constData()[2] ),
        std::sqrt( transform.constData()[4] * transform.constData()[4] + transform.constData()[5] * transform.constData()[5] + transform.constData()[6] * transform.constData()[6] )
      ),
      std::sqrt( transform.constData()[8] * transform.constData()[8] + transform.constData()[9] * transform.constData()[9] + transform.constData()[10] * transform.constData()[10] )
    );

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
        const auto &rtcCenterJson = featureTable["RTC_CENTER"];
        if ( rtcCenterJson.is_array() && rtcCenterJson.size() == 3 )
        {
          res.rtcCenter.setX( rtcCenterJson[0].get<double>() );
          res.rtcCenter.setY( rtcCenterJson[1].get<double>() );
          res.rtcCenter.setZ( rtcCenterJson[2].get<double>() );
        }
        else
        {
          QgsDebugError( u"Invalid RTC_CENTER value"_s );
        }
      }
    }
    catch ( json::parse_error &ex )
    {
      QgsDebugError( u"Error parsing feature table JSON: %1"_s.arg( ex.what() ) );
    }
  }

  res.gltf = tileContent.mid( sizeof( b3dmHeader ) + hdr.featureTableJsonByteLength + hdr.featureTableBinaryByteLength + hdr.batchTableJsonByteLength + hdr.batchTableBinaryByteLength );
  return res;
}

static QQuaternion quaternionFromNormalUpRight( const QVector3D &normalUp, const QVector3D &normalRight )
{
  const QVector3D right = normalRight.normalized();
  const QVector3D up = normalUp.normalized();
  // In a right-handed coordinate system with X=right, Z=up:
  // Y = cross(up, right) = forward (pointing North for ENU)
  const QVector3D forward = QVector3D::crossProduct( up, right ).normalized();

  // Build rotation matrix with columns [right, forward, up]
  // QGenericMatrix constructor takes row-major input
  float matData[9] = { right.x(), forward.x(), up.x(), right.y(), forward.y(), up.y(), right.z(), forward.z(), up.z() };
  QMatrix3x3 rotMatrix( matData );
  return QQuaternion::fromRotationMatrix( rotMatrix );
}

/**
 * Computes EAST_NORTH_UP orientation quaternions for each instance position.
 *
 * Positions are in tile-local Z-up space, relative to \a rtcCenter.
 * The \a tileTransform converts from tile-local space to ECEF (EPSG:4978),
 * so that the ENU frame can be computed at the correct geographic location.
 *
 * \param positions per-instance positions in tile-local space
 * \param rtcCenter RTC_CENTER offset in tile-local space
 * \param tileTransform tile transform from tileset.json (tile-local → ECEF)
 * \param rotations output quaternions (one per instance)
 * \since QGIS 4.2
 */
static void computeEastNorthUpQuaternions( const QVector<QVector3D> &positions, const QgsVector3D &rtcCenter, const QgsMatrix4x4 &tileTransform, QVector<QQuaternion> &rotations )
{
  const int count = positions.size();
  rotations.resize( count );

  if ( count == 0 )
    return;

  const QgsCoordinateReferenceSystem ecefCrs( u"EPSG:4978"_s );
  const QgsCoordinateReferenceSystem geodeticCrs( u"EPSG:4979"_s );
  QgsCoordinateTransform ecefToGeodetic( ecefCrs, geodeticCrs, QgsCoordinateTransformContext() );
  QgsCoordinateTransform geodeticToEcef( geodeticCrs, ecefCrs, QgsCoordinateTransformContext() );

  constexpr double delta = 0.001; // degrees

  // Batch transform: we need base points + east perturbation + north perturbation = 3*count points
  const int totalPts = 3 * count;
  QVector<double> gx( totalPts ), gy( totalPts ), gz( totalPts );

  // First, convert tile-local positions to ECEF using the tile transform,
  // then convert ECEF positions to geodetic
  QVector<double> px( count ), py( count ), pz( count );
  for ( int i = 0; i < count; ++i )
  {
    const QgsVector3D posLocal( static_cast<double>( positions[i].x() ) + rtcCenter.x(), static_cast<double>( positions[i].y() ) + rtcCenter.y(), static_cast<double>( positions[i].z() ) + rtcCenter.z() );
    const QgsVector3D ecef = tileTransform.map( posLocal );
    px[i] = ecef.x();
    py[i] = ecef.y();
    pz[i] = ecef.z();
  }

  try
  {
    ecefToGeodetic.transformCoords( count, px.data(), py.data(), pz.data() );
  }
  catch ( QgsCsException & )
  {
    // fallback to identity rotations
    for ( int i = 0; i < count; ++i )
      rotations[i] = QQuaternion();
    return;
  }

  // px/py/pz are now geodetic (lon, lat, h)
  // Build batch: base, east-perturbed, north-perturbed
  for ( int i = 0; i < count; ++i )
  {
    // base point
    gx[i] = px[i];
    gy[i] = py[i];
    gz[i] = pz[i];

    // east-perturbed
    gx[count + i] = px[i] + delta;
    gy[count + i] = py[i];
    gz[count + i] = pz[i];

    // north-perturbed
    gx[2 * count + i] = px[i];
    gy[2 * count + i] = py[i] + delta;
    gz[2 * count + i] = pz[i];
  }

  try
  {
    geodeticToEcef.transformCoords( totalPts, gx.data(), gy.data(), gz.data() );
  }
  catch ( QgsCsException & )
  {
    for ( int i = 0; i < count; ++i )
      rotations[i] = QQuaternion();
    return;
  }

  // gx/gy/gz are now ECEF for all 3*count points

  // Extract the tile transform's 3×3 rotation part (column-major in QgsMatrix4x4).
  // We need its transpose (= inverse for orthogonal matrices) to transform
  // ECEF direction vectors back to tile-local space, because the instance
  // rotation quaternion is applied in tile-local space, not ECEF.
  const double *td = tileTransform.constData();
  // Columns of the 3×3 rotation part (column-major: col0=[0,1,2], col1=[4,5,6], col2=[8,9,10])
  const QVector3D tileCol0( static_cast<float>( td[0] ), static_cast<float>( td[1] ), static_cast<float>( td[2] ) );
  const QVector3D tileCol1( static_cast<float>( td[4] ), static_cast<float>( td[5] ), static_cast<float>( td[6] ) );
  const QVector3D tileCol2( static_cast<float>( td[8] ), static_cast<float>( td[9] ), static_cast<float>( td[10] ) );

  // Normalize columns to handle any scale in the tile transform
  const float len0 = tileCol0.length();
  const float len1 = tileCol1.length();
  const float len2 = tileCol2.length();
  const bool hasTileRotation = len0 > 0 && len1 > 0 && len2 > 0 && !tileTransform.isIdentity();

  for ( int i = 0; i < count; ++i )
  {
    const QVector3D base( static_cast<float>( gx[i] ), static_cast<float>( gy[i] ), static_cast<float>( gz[i] ) );
    const QVector3D eastPt( static_cast<float>( gx[count + i] ), static_cast<float>( gy[count + i] ), static_cast<float>( gz[count + i] ) );
    const QVector3D northPt( static_cast<float>( gx[2 * count + i] ), static_cast<float>( gy[2 * count + i] ), static_cast<float>( gz[2 * count + i] ) );

    QVector3D east = ( eastPt - base ).normalized();
    QVector3D north = ( northPt - base ).normalized();
    QVector3D up = QVector3D::crossProduct( east, north ).normalized();

    if ( hasTileRotation )
    {
      // Transform ECEF directions to tile-local space using R_tile^T
      // (transpose of rotation = inverse for orthogonal matrices).
      // R^T × v = (dot(col0,v)/|col0|², dot(col1,v)/|col1|², dot(col2,v)/|col2|²)
      // With normalized columns: R^T × v = (dot(col0/|col0|, v), ...)
      const QVector3D nc0 = tileCol0 / len0;
      const QVector3D nc1 = tileCol1 / len1;
      const QVector3D nc2 = tileCol2 / len2;

      const QVector3D eastLocal( QVector3D::dotProduct( nc0, east ), QVector3D::dotProduct( nc1, east ), QVector3D::dotProduct( nc2, east ) );
      const QVector3D upLocal( QVector3D::dotProduct( nc0, up ), QVector3D::dotProduct( nc1, up ), QVector3D::dotProduct( nc2, up ) );
      east = eastLocal;
      up = upLocal;
    }

    rotations[i] = quaternionFromNormalUpRight( up, east );
  }
}


// Helper: build the axis flip matrix from gltfUpAxis
static QMatrix4x4 axisFlipMatrix( Qgis::Axis gltfUpAxis )
{
  QMatrix4x4 F;
  switch ( gltfUpAxis )
  {
    case Qgis::Axis::Y:
      // Y-up → Z-up: (x,y,z) → (x,-z,y)
      F = QMatrix4x4( 1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 );
      break;
    case Qgis::Axis::X:
    case Qgis::Axis::Z:
      // identity (already Z-up or X-up not supported yet)
      break;
  }
  return F;
}


// Helper: parse EXT_mesh_gpu_instancing from a glTF node
static bool parseExtMeshGpuInstancing(
  const tinygltf::Model &model, const tinygltf::Node &node, QVector<QVector3D> &translations, QVector<QQuaternion> &rotations, QVector<QVector3D> &scales, int &instanceCount
)
{
  auto extIt = node.extensions.find( "EXT_mesh_gpu_instancing" );
  if ( extIt == node.extensions.end() )
    return false;

  const tinygltf::Value &extValue = extIt->second;
  if ( !extValue.IsObject() || !extValue.Has( "attributes" ) )
    return false;

  const tinygltf::Value &attributes = extValue.Get( "attributes" );
  if ( !attributes.IsObject() )
    return false;

  instanceCount = 0;

  // Helper lambda: read a VEC3 FLOAT accessor into a QVector<QVector3D>
  auto readVec3Accessor = [&model, &instanceCount]( const tinygltf::Value &attributes, const std::string &name, QVector<QVector3D> &out, const QVector3D &defaultValue ) -> bool {
    if ( attributes.Has( name ) )
    {
      int accessorIdx = attributes.Get( name ).GetNumberAsInt();
      if ( accessorIdx < 0 || accessorIdx >= static_cast<int>( model.accessors.size() ) )
        return false;

      const tinygltf::Accessor &accessor = model.accessors[accessorIdx];
      if ( accessor.type != TINYGLTF_TYPE_VEC3 || accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT )
        return false;

      const int count = static_cast<int>( accessor.count );
      if ( instanceCount == 0 )
        instanceCount = count;
      else if ( instanceCount != count )
        return false;

      const tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
      const tinygltf::Buffer &buf = model.buffers[bv.buffer];
      const unsigned char *ptr = buf.data.data() + bv.byteOffset + accessor.byteOffset;
      const int stride = bv.byteStride ? bv.byteStride : 3 * static_cast<int>( sizeof( float ) );

      out.resize( count );
      for ( int i = 0; i < count; ++i )
      {
        const float *fptr = reinterpret_cast<const float *>( ptr + i * stride );
        out[i] = QVector3D( fptr[0], fptr[1], fptr[2] );
      }
    }
    else
    {
      // use default
      if ( instanceCount > 0 )
        out.fill( defaultValue, instanceCount );
    }
    return true;
  };

  if ( !readVec3Accessor( attributes, "TRANSLATION", translations, QVector3D( 0, 0, 0 ) ) )
    return false;

  if ( !readVec3Accessor( attributes, "SCALE", scales, QVector3D( 1, 1, 1 ) ) )
    return false;

  // Read ROTATION: VEC4 FLOAT accessor → per-instance quaternions
  if ( attributes.Has( "ROTATION" ) )
  {
    int accessorIdx = attributes.Get( "ROTATION" ).GetNumberAsInt();
    if ( accessorIdx < 0 || accessorIdx >= static_cast<int>( model.accessors.size() ) )
      return false;

    const tinygltf::Accessor &accessor = model.accessors[accessorIdx];
    if ( accessor.type != TINYGLTF_TYPE_VEC4 )
      return false;

    const int count = static_cast<int>( accessor.count );
    if ( instanceCount == 0 )
      instanceCount = count;
    else if ( instanceCount != count )
      return false;

    const tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer &buf = model.buffers[bv.buffer];
    const unsigned char *ptr = buf.data.data() + bv.byteOffset + accessor.byteOffset;

    rotations.resize( count );

    if ( accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT )
    {
      const int stride = bv.byteStride ? bv.byteStride : 4 * static_cast<int>( sizeof( float ) );
      for ( int i = 0; i < count; ++i )
      {
        const float *fptr = reinterpret_cast<const float *>( ptr + i * stride );
        // glTF quaternion: (x, y, z, w)
        rotations[i] = QQuaternion( fptr[3], fptr[0], fptr[1], fptr[2] );
      }
    }
    else if ( accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT )
    {
      const int stride = bv.byteStride ? bv.byteStride : 4 * static_cast<int>( sizeof( short ) );
      for ( int i = 0; i < count; ++i )
      {
        const short *sptr = reinterpret_cast<const short *>( ptr + i * stride );
        // Normalized short: divide by 32767.0
        rotations[i] = QQuaternion( static_cast<float>( sptr[3] ) / 32767.0f, static_cast<float>( sptr[0] ) / 32767.0f, static_cast<float>( sptr[1] ) / 32767.0f, static_cast<float>( sptr[2] ) / 32767.0f );
      }
    }
    else if ( accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE )
    {
      const int stride = bv.byteStride ? bv.byteStride : 4 * static_cast<int>( sizeof( char ) );
      for ( int i = 0; i < count; ++i )
      {
        const signed char *bptr = reinterpret_cast<const signed char *>( ptr + i * stride );
        // Normalized byte: divide by 127.0
        rotations[i] = QQuaternion( static_cast<float>( bptr[3] ) / 127.0f, static_cast<float>( bptr[0] ) / 127.0f, static_cast<float>( bptr[1] ) / 127.0f, static_cast<float>( bptr[2] ) / 127.0f );
      }
    }
    else
    {
      return false;
    }
  }
  else
  {
    if ( instanceCount > 0 )
      rotations.fill( QQuaternion(), instanceCount );
  }

  // Fill defaults if only some attributes were specified
  if ( instanceCount > 0 )
  {
    if ( translations.isEmpty() )
      translations.fill( QVector3D( 0, 0, 0 ), instanceCount );
    if ( rotations.isEmpty() )
      rotations.fill( QQuaternion(), instanceCount );
    if ( scales.isEmpty() )
      scales.fill( QVector3D( 1, 1, 1 ), instanceCount );
  }

  return instanceCount > 0;
}


// Helper: build a 4x4 matrix from translation, rotation, scale
static QMatrix4x4 trsMatrix( const QVector3D &t, const QQuaternion &r, const QVector3D &s )
{
  QMatrix4x4 mat;
  mat.translate( t );
  mat.rotate( r );
  mat.scale( s );
  return mat;
}


QVector<QgsGltfUtils::InstancedPrimitive> QgsCesiumUtils::resolveInstancing(
  const tinygltf::Model &model, const std::optional<TileI3dmData> &tileInstancing, Qgis::Axis gltfUpAxis, const QgsMatrix4x4 &tileTransform, const QgsVector3D &rtcCenter
)
{
  QVector<QgsGltfUtils::InstancedPrimitive> result;

  bool sceneOk = false;
  const std::size_t sceneIndex = QgsGltfUtils::sourceSceneForModel( model, sceneOk );
  if ( !sceneOk )
    return result;

  const tinygltf::Scene &scene = model.scenes[sceneIndex];
  const QMatrix4x4 F = axisFlipMatrix( gltfUpAxis );

  // Recursive node walker
  std::function<void( int nodeIndex, const QMatrix4x4 &parentTransform )> walkNode;
  walkNode = [&]( int nodeIndex, const QMatrix4x4 &parentTransform ) {
    if ( nodeIndex < 0 || nodeIndex >= static_cast<int>( model.nodes.size() ) )
      return;

    const tinygltf::Node &node = model.nodes[nodeIndex];

    // Compute accumulated local transform M for this node
    std::unique_ptr<QMatrix4x4> localTransform = QgsGltfUtils::parseNodeTransform( node );
    QMatrix4x4 M = parentTransform;
    if ( localTransform )
      M = parentTransform * *localTransform;

    if ( node.mesh >= 0 )
    {
      const tinygltf::Mesh &mesh = model.meshes[node.mesh];

      if ( tileInstancing.has_value() )
      {
        // i3dm path: every mesh node gets the same instances
        // fullMatrix = T(pos_i) × R_i × S_i × F × M
        const QMatrix4x4 FM = F * M;
        QgsCesiumUtils::TileI3dmData inst = tileInstancing.value();

        // Deferred EAST_NORTH_UP: compute ENU rotations now that the tile transform is available
        if ( inst.eastNorthUp )
        {
          computeEastNorthUpQuaternions( inst.translations, rtcCenter, tileTransform, inst.rotations );
        }

        for ( int pIdx = 0; pIdx < static_cast<int>( mesh.primitives.size() ); ++pIdx )
        {
          QgsGltfUtils::InstancedPrimitive entry;
          entry.meshIndex = node.mesh;
          entry.primitiveIndex = pIdx;
          entry.materialIndex = mesh.primitives[pIdx].material;
          entry.instanceTransforms.resize( inst.instanceCount );

          for ( int i = 0; i < inst.instanceCount; ++i )
          {
            entry.instanceTransforms[i] = trsMatrix( inst.translations[i], inst.rotations[i], inst.scales[i] ) * FM;
          }

          result.append( std::move( entry ) );
        }
      }
      else
      {
        // Check for EXT_mesh_gpu_instancing on this node
        QVector<QVector3D> translations;
        QVector<QQuaternion> rotations;
        QVector<QVector3D> scales;
        int instanceCount = 0;

        if ( parseExtMeshGpuInstancing( model, node, translations, rotations, scales, instanceCount ) )
        {
          // EXT path: fullMatrix = F × M × T_i × R_i × S_i
          const QMatrix4x4 FM = F * M;

          for ( int pIdx = 0; pIdx < static_cast<int>( mesh.primitives.size() ); ++pIdx )
          {
            QgsGltfUtils::InstancedPrimitive entry;
            entry.meshIndex = node.mesh;
            entry.primitiveIndex = pIdx;
            entry.materialIndex = mesh.primitives[pIdx].material;
            entry.instanceTransforms.resize( instanceCount );

            for ( int i = 0; i < instanceCount; ++i )
            {
              entry.instanceTransforms[i] = FM * trsMatrix( translations[i], rotations[i], scales[i] );
            }

            result.append( std::move( entry ) );
          }
        }
        // else: non-instanced node, skip — handled by existing code path
      }
    }

    // Recurse to children
    for ( int childIndex : node.children )
    {
      walkNode( childIndex, M );
    }
  };

  for ( int nodeIndex : scene.nodes )
  {
    walkNode( nodeIndex, QMatrix4x4() );
  }

  return result;
}


static QgsCesiumUtils::TileContents extractGltfFromI3dm( const QByteArray &tileContent, const QString &baseUri )
{
  struct i3dmHeader
  {
    quint32 featureTableJsonByteLength;
    quint32 featureTableBinaryByteLength;
    quint32 batchTableJsonByteLength;
    quint32 batchTableBinaryByteLength;
    quint32 gltfFormat;
  };

  QgsCesiumUtils::TileContents res;

  if ( tileContent.size() < static_cast<int>( sizeof( i3dmHeader ) ) )
    return res;

  i3dmHeader hdr;
  memcpy( &hdr, tileContent.constData(), sizeof( i3dmHeader ) );

  const int featureTableJsonOffset = sizeof( i3dmHeader );
  const int featureTableBinaryOffset = featureTableJsonOffset + hdr.featureTableJsonByteLength;

  // Parse feature table JSON
  const QString featureTableJson( tileContent.mid( featureTableJsonOffset, hdr.featureTableJsonByteLength ) );
  if ( featureTableJson.isEmpty() )
  {
    QgsDebugError( u"i3dm: empty feature table JSON"_s );
    return res;
  }

  int instanceCount = 0;
  bool eastNorthUp = false;
  int positionByteOffset = -1;
  int normalUpByteOffset = -1;
  int normalRightByteOffset = -1;
  int scaleByteOffset = -1;
  int scaleNonUniformByteOffset = -1;

  try
  {
    const json featureTable = json::parse( featureTableJson.toStdString() );

    if ( !featureTable.contains( "INSTANCES_LENGTH" ) )
    {
      QgsDebugError( u"i3dm: INSTANCES_LENGTH not found in feature table"_s );
      return res;
    }
    instanceCount = featureTable["INSTANCES_LENGTH"].get<int>();

    if ( featureTable.contains( "RTC_CENTER" ) )
    {
      const auto &rtcCenterJson = featureTable["RTC_CENTER"];
      if ( rtcCenterJson.is_array() && rtcCenterJson.size() == 3 )
      {
        res.rtcCenter.setX( rtcCenterJson[0].get<double>() );
        res.rtcCenter.setY( rtcCenterJson[1].get<double>() );
        res.rtcCenter.setZ( rtcCenterJson[2].get<double>() );
      }
    }

    if ( featureTable.contains( "EAST_NORTH_UP" ) )
    {
      eastNorthUp = featureTable["EAST_NORTH_UP"].get<bool>();
    }

    // Get byte offsets for binary properties
    if ( featureTable.contains( "POSITION" ) )
    {
      const auto &posJson = featureTable["POSITION"];
      if ( posJson.is_object() && posJson.contains( "byteOffset" ) )
        positionByteOffset = posJson["byteOffset"].get<int>();
    }
    if ( featureTable.contains( "NORMAL_UP" ) )
    {
      const auto &nuJson = featureTable["NORMAL_UP"];
      if ( nuJson.is_object() && nuJson.contains( "byteOffset" ) )
        normalUpByteOffset = nuJson["byteOffset"].get<int>();
    }
    if ( featureTable.contains( "NORMAL_RIGHT" ) )
    {
      const auto &nrJson = featureTable["NORMAL_RIGHT"];
      if ( nrJson.is_object() && nrJson.contains( "byteOffset" ) )
        normalRightByteOffset = nrJson["byteOffset"].get<int>();
    }
    if ( featureTable.contains( "SCALE" ) )
    {
      const auto &sJson = featureTable["SCALE"];
      if ( sJson.is_object() && sJson.contains( "byteOffset" ) )
        scaleByteOffset = sJson["byteOffset"].get<int>();
    }
    if ( featureTable.contains( "SCALE_NON_UNIFORM" ) )
    {
      const auto &snuJson = featureTable["SCALE_NON_UNIFORM"];
      if ( snuJson.is_object() && snuJson.contains( "byteOffset" ) )
        scaleNonUniformByteOffset = snuJson["byteOffset"].get<int>();
    }
  }
  catch ( json::parse_error &ex )
  {
    QgsDebugError( u"i3dm: error parsing feature table JSON: %1"_s.arg( ex.what() ) );
    return res;
  }

  if ( instanceCount <= 0 || positionByteOffset < 0 )
  {
    QgsDebugError( u"i3dm: invalid instance count or missing POSITION"_s );
    return res;
  }

  const char *featureBinaryPtr = tileContent.constData() + featureTableBinaryOffset;
  const int featureBinarySize = hdr.featureTableBinaryByteLength;

  QgsCesiumUtils::TileI3dmData instancing;
  instancing.instanceCount = instanceCount;

  // Read POSITION: instanceCount × float32[3]
  {
    const int requiredSize = positionByteOffset + instanceCount * 3 * static_cast<int>( sizeof( float ) );
    if ( requiredSize > featureBinarySize )
    {
      QgsDebugError( u"i3dm: POSITION data exceeds feature table binary size"_s );
      return res;
    }
    instancing.translations.resize( instanceCount );
    const float *posPtr = reinterpret_cast<const float *>( featureBinaryPtr + positionByteOffset );
    for ( int i = 0; i < instanceCount; ++i )
    {
      instancing.translations[i] = QVector3D( posPtr[i * 3], posPtr[i * 3 + 1], posPtr[i * 3 + 2] );
    }
  }

  // Read rotations from NORMAL_UP + NORMAL_RIGHT, or EAST_NORTH_UP, or identity
  if ( normalUpByteOffset >= 0 && normalRightByteOffset >= 0 )
  {
    const int requiredUp = normalUpByteOffset + instanceCount * 3 * static_cast<int>( sizeof( float ) );
    const int requiredRight = normalRightByteOffset + instanceCount * 3 * static_cast<int>( sizeof( float ) );
    if ( requiredUp > featureBinarySize || requiredRight > featureBinarySize )
    {
      QgsDebugError( u"i3dm: NORMAL_UP/NORMAL_RIGHT data exceeds feature table binary size"_s );
      return res;
    }
    instancing.rotations.resize( instanceCount );
    const float *upPtr = reinterpret_cast<const float *>( featureBinaryPtr + normalUpByteOffset );
    const float *rightPtr = reinterpret_cast<const float *>( featureBinaryPtr + normalRightByteOffset );
    for ( int i = 0; i < instanceCount; ++i )
    {
      QVector3D normalUp( upPtr[i * 3], upPtr[i * 3 + 1], upPtr[i * 3 + 2] );
      QVector3D normalRight( rightPtr[i * 3], rightPtr[i * 3 + 1], rightPtr[i * 3 + 2] );
      instancing.rotations[i] = quaternionFromNormalUpRight( normalUp, normalRight );
    }
  }
  else if ( eastNorthUp )
  {
    // Defer ENU computation: the tile transform (from tileset.json) is needed
    // to convert tile-local positions to ECEF, but is not available at parse time.
    // Store the flag so resolveInstancing() can compute ENU rotations later.
    instancing.eastNorthUp = true;
    instancing.rotations.fill( QQuaternion(), instanceCount );
  }
  else
  {
    instancing.rotations.fill( QQuaternion(), instanceCount );
  }

  // Read SCALE or SCALE_NON_UNIFORM
  if ( scaleNonUniformByteOffset >= 0 )
  {
    const int required = scaleNonUniformByteOffset + instanceCount * 3 * static_cast<int>( sizeof( float ) );
    if ( required > featureBinarySize )
    {
      QgsDebugError( u"i3dm: SCALE_NON_UNIFORM data exceeds feature table binary size"_s );
      return res;
    }
    instancing.scales.resize( instanceCount );
    const float *scalePtr = reinterpret_cast<const float *>( featureBinaryPtr + scaleNonUniformByteOffset );
    for ( int i = 0; i < instanceCount; ++i )
    {
      instancing.scales[i] = QVector3D( scalePtr[i * 3], scalePtr[i * 3 + 1], scalePtr[i * 3 + 2] );
    }
  }
  else if ( scaleByteOffset >= 0 )
  {
    const int required = scaleByteOffset + instanceCount * static_cast<int>( sizeof( float ) );
    if ( required > featureBinarySize )
    {
      QgsDebugError( u"i3dm: SCALE data exceeds feature table binary size"_s );
      return res;
    }
    instancing.scales.resize( instanceCount );
    const float *scalePtr = reinterpret_cast<const float *>( featureBinaryPtr + scaleByteOffset );
    for ( int i = 0; i < instanceCount; ++i )
    {
      const float s = scalePtr[i];
      instancing.scales[i] = QVector3D( s, s, s );
    }
  }
  else
  {
    instancing.scales.fill( QVector3D( 1, 1, 1 ), instanceCount );
  }

  res.instancing = std::move( instancing );

  // Extract embedded glTF body
  const int gltfOffset = featureTableBinaryOffset + hdr.featureTableBinaryByteLength + hdr.batchTableJsonByteLength + hdr.batchTableBinaryByteLength;
  QByteArray gltfContent = tileContent.mid( gltfOffset );

  if ( hdr.gltfFormat == 1 ) // embedded
  {
    res.gltf = gltfContent;
  }
  else if ( hdr.gltfFormat == 0 ) // gltf is actually only a URI, not real content
  {
    QString gltfUri = QString::fromUtf8( gltfContent );
    // URI may be relative to the .i3dm location
    QUrl url = QUrl( baseUri ).resolved( gltfUri );

    if ( url.scheme().startsWith( "http" ) )
    {
      QNetworkRequest request = QNetworkRequest( url );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
      QgsBlockingNetworkRequest networkRequest;
      // TODO: setup auth, setup headers
      if ( networkRequest.get( request ) != QgsBlockingNetworkRequest::NoError )
      {
        QgsDebugError( u"i3dm: Failed to download GLTF: %1"_s.arg( url.toString() ) );
      }
      else
      {
        const QgsNetworkReplyContent content = networkRequest.reply();
        res.gltf = content.content();
      }
    }
    else if ( url.isLocalFile() )
    {
      QString localFilePath = url.toLocalFile();
      if ( QFile::exists( localFilePath ) )
      {
        QFile f( localFilePath );
        if ( f.open( QIODevice::ReadOnly ) )
        {
          res.gltf = f.readAll();
        }
      }
      else
      {
        QgsDebugError( u"i3dm: Failed to open local GLTF: %1"_s.arg( url.toString() ) );
      }
    }
  }
  else
  {
    QgsDebugError( u"i3dm: Unknown gltf format: %1"_s.arg( hdr.gltfFormat ) );
  }

  return res;
}

static QVector<QgsCesiumUtils::TileContents> extractGltfFromCmpt( const QByteArray &tileContent, int depth = 0 )
{
  struct cmptHeader
  {
      unsigned char magic[4];
      quint32 version;
      quint32 byteLength;
      quint32 tilesLength;
  };

  QVector<QgsCesiumUtils::TileContents> result;

  if ( depth > 10 )
  {
    // avoid infinite recursion with badly formed tiles
    QgsDebugError( u"cmpt recursion depth exceeded"_s );
    return result;
  }

  if ( tileContent.size() < static_cast<int>( sizeof( cmptHeader ) ) )
    return result;

  cmptHeader hdr;
  memcpy( &hdr, tileContent.constData(), sizeof( cmptHeader ) );

  if ( hdr.version != 1 )
  {
    QgsDebugError( u"Unsupported cmpt version %1"_s.arg( hdr.version ) );
    return result;
  }

  if ( static_cast<quint32>( tileContent.size() ) < hdr.byteLength )
    return result;

  int offset = static_cast<int>( sizeof( cmptHeader ) );
  for ( quint32 i = 0; i < hdr.tilesLength; ++i )
  {
    // all inner tiles have the following header: magic (4 bytes), version (uint32), byteLength (uint32)
    const quint32 innerByteLength = *reinterpret_cast<const quint32 *>( tileContent.constData() + offset + 8 );

    if ( innerByteLength < 12 || offset + static_cast<int>( innerByteLength ) > static_cast<int>( hdr.byteLength ) )
    {
      QgsDebugError( u"cmpt with bad inner tile (at index %1)"_s.arg( i ) );
      break;
    }

    const QByteArray innerTile = tileContent.mid( offset, innerByteLength );

    if ( innerTile.startsWith( QByteArray( "cmpt" ) ) )
    {
      result.append( extractGltfFromCmpt( innerTile, depth + 1 ) );
    }
    else
    {
      result.append( QgsCesiumUtils::extractTileContent( innerTile ) );
    }

    offset += static_cast<int>( innerByteLength );
  }

  return result;
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
    return res;
  }
}

QVector<QgsCesiumUtils::TileContents> QgsCesiumUtils::extractTileContent( const QByteArray &tileContent, const QString &baseUri )
{
  QVector<TileContents> result;
  if ( tileContent.startsWith( QByteArray( "b3dm" ) ) )
  {
    const B3DMContents b3dmContents = QgsCesiumUtils::extractGltfFromB3dm( tileContent );
    TileContents contents;
    contents.gltf = b3dmContents.gltf;
    contents.rtcCenter = b3dmContents.rtcCenter;
    result.append( contents );
  }
  else if ( tileContent.startsWith( QByteArray( "i3dm" ) ) )
  {
    TileContents contents = extractGltfFromI3dm( tileContent, baseUri );
    result.append( contents );
  }
  else if ( tileContent.startsWith( QByteArray( "glTF" ) ) )
  {
    TileContents contents;
    contents.gltf = tileContent;
    result.append( contents );
  }
  else if ( tileContent.startsWith( QByteArray( "cmpt" ) ) )
  {
    result = extractGltfFromCmpt( tileContent );
  }
  else
  {
    QgsDebugError( u"extractGltfFromTileContent: unknown tile format, size=%1, magic=%2"_s.arg( tileContent.size() ).arg( QString::fromLatin1( tileContent.left( 4 ) ) ) );
  }
  return result;
}

QgsTiledSceneBoundingVolume QgsCesiumUtils::boundingVolumeFromRegion( const QgsBox3D &region, const QgsCoordinateTransformContext &transformContext )
{
  if ( region.width() > 20 || region.height() > 20 )
  {
    // treat very large regions as global -- these will not transform correctly to EPSG:4978
    return QgsTiledSceneBoundingVolume();
  }

  // Transform the 8 corners of the region from EPSG:4979 to EPSG:4978
  QVector< QgsVector3D > corners = region.corners();
  QVector< double > x;
  x.reserve( 8 );
  QVector< double > y;
  y.reserve( 8 );
  QVector< double > z;
  z.reserve( 8 );
  for ( int i = 0; i < 8; ++i )
  {
    const QgsVector3D &corner = corners[i];
    x.append( corner.x() );
    y.append( corner.y() );
    z.append( corner.z() );
  }
  QgsCoordinateTransform ct( QgsCoordinateReferenceSystem( u"EPSG:4979"_s ), QgsCoordinateReferenceSystem( u"EPSG:4978"_s ), transformContext );
  ct.setBallparkTransformsAreAppropriate( true );
  try
  {
    ct.transformInPlace( x, y, z );
  }
  catch ( QgsCsException & )
  {
    QgsDebugError( u"Cannot transform region bounding volume"_s );
  }

  const auto minMaxX = std::minmax_element( x.constBegin(), x.constEnd() );
  const auto minMaxY = std::minmax_element( y.constBegin(), y.constEnd() );
  const auto minMaxZ = std::minmax_element( z.constBegin(), z.constEnd() );
  // note that matrix transforms are NOT applied to region bounding volumes!
  return QgsTiledSceneBoundingVolume( QgsOrientedBox3D::fromBox3D( QgsBox3D( *minMaxX.first, *minMaxY.first, *minMaxZ.first, *minMaxX.second, *minMaxY.second, *minMaxZ.second ) ) );
}

QString QgsCesiumUtils::appendQueryFromBaseUrl( const QString &contentUri, const QUrl &baseUrl )
{
  // This is to support a case seen with Google's tiles. Root URL is something like this:
  // https://tile.googleapis.com/.../root.json?key=123
  // The returned JSON contains relative links with "session" (e.g. "/.../abc.json?session=456")
  // When fetching such abc.json, we have to include also "key" from the original URL!
  // Then the content of abc.json contains relative links (e.g. "/.../xyz.glb") and we
  // need to add both "key" and "session" (otherwise requests fail).

  QUrlQuery contentQuery( QUrl( contentUri ).query() );
  const QList<QPair<QString, QString>> baseUrlQueryItems = QUrlQuery( baseUrl.query() ).queryItems();
  for ( const QPair<QString, QString> &kv : baseUrlQueryItems )
  {
    contentQuery.addQueryItem( kv.first, kv.second );
  }
  QUrl newContentUrl( contentUri );
  newContentUrl.setQuery( contentQuery );
  return newContentUrl.toString();
}
