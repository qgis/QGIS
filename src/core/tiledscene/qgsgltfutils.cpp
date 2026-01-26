/***************************************************************************
  qgsgltfutils.cpp
  --------------------------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsconfig.h"
#include "qgsgltfutils.h"

#include <memory>

#include "qgsexception.h"
#include "qgslogger.h"
#include "qgsmatrix4x4.h"
#include "qgstiledscenetile.h"
#include "qgsziputils.h"

#include <QImage>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QRegularExpression>

#define TINYGLTF_IMPLEMENTATION       // should be defined just in one CPP file

// decompression of meshes with Draco is optional, but recommended
// because some 3D Tiles datasets use it (KHR_draco_mesh_compression is an optional extension of GLTF)
#ifdef HAVE_DRACO
#define TINYGLTF_ENABLE_DRACO
#endif

#define TINYGLTF_NO_STB_IMAGE         // we use QImage-based reading of images
#define TINYGLTF_NO_STB_IMAGE_WRITE   // we don't need writing of images
//#define TINYGLTF_NO_FS

//#include <fstream>
#include "tiny_gltf.h"

///@cond PRIVATE


bool QgsGltfUtils::accessorToMapCoordinates( const tinygltf::Model &model, int accessorIndex, const QgsMatrix4x4 &tileTransform, const QgsCoordinateTransform *ecefToTargetCrs, const QgsVector3D &tileTranslationEcef, const QMatrix4x4 *nodeTransform, Qgis::Axis gltfUpAxis, QVector<double> &vx, QVector<double> &vy, QVector<double> &vz )
{
  const tinygltf::Accessor &accessor = model.accessors[accessorIndex];
  const tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
  const tinygltf::Buffer &b = model.buffers[bv.buffer];

  if ( accessor.componentType != TINYGLTF_PARAMETER_TYPE_FLOAT || accessor.type != TINYGLTF_TYPE_VEC3 )
  {
    // we may support more input types in the future if needed
    return false;
  }

  const unsigned char *ptr = b.data.data() + bv.byteOffset + accessor.byteOffset;

  vx.resize( accessor.count );
  vy.resize( accessor.count );
  vz.resize( accessor.count );
  double *vxOut = vx.data();
  double *vyOut = vy.data();
  double *vzOut = vz.data();
  for ( int i = 0; i < static_cast<int>( accessor.count ); ++i )
  {
    const float *fptr = reinterpret_cast<const float *>( ptr );
    QVector3D vOrig( fptr[0], fptr[1], fptr[2] );

    if ( nodeTransform )
      vOrig = nodeTransform->map( vOrig );

    QgsVector3D v;
    switch ( gltfUpAxis )
    {
      case Qgis::Axis::X:
      {
        QgsDebugError( u"X up translation not yet supported"_s );
        v = tileTransform.map( tileTranslationEcef );
        break;
      }

      case Qgis::Axis::Y:
      {
        // go from y-up to z-up according to 3D Tiles spec
        QVector3D vFlip( vOrig.x(), -vOrig.z(), vOrig.y() );
        v = tileTransform.map( QgsVector3D( vFlip ) + tileTranslationEcef );
        break;
      }

      case Qgis::Axis::Z:
      {
        v = tileTransform.map( QgsVector3D( vOrig ) + tileTranslationEcef );
        break;
      }
    }

    *vxOut++ = v.x();
    *vyOut++ = v.y();
    *vzOut++ = v.z();

    if ( bv.byteStride )
      ptr += bv.byteStride;
    else
      ptr += 3 * sizeof( float );
  }

  if ( ecefToTargetCrs )
  {
    try
    {
      ecefToTargetCrs->transformCoords( accessor.count, vx.data(), vy.data(), vz.data() );
    }
    catch ( QgsCsException & )
    {
      return false;
    }
  }

  return true;
}

bool QgsGltfUtils::extractTextureCoordinates( const tinygltf::Model &model, int accessorIndex, QVector<float> &x, QVector<float> &y )
{
  const tinygltf::Accessor &accessor = model.accessors[accessorIndex];
  const tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
  const tinygltf::Buffer &b = model.buffers[bv.buffer];

  if ( accessor.componentType != TINYGLTF_PARAMETER_TYPE_FLOAT || accessor.type != TINYGLTF_TYPE_VEC2 )
  {
    return false;
  }

  const unsigned char *ptr = b.data.data() + bv.byteOffset + accessor.byteOffset;
  x.resize( accessor.count );
  y.resize( accessor.count );

  float *xOut = x.data();
  float *yOut = y.data();

  for ( std::size_t i = 0; i < accessor.count; i++ )
  {
    const float *fptr = reinterpret_cast< const float * >( ptr );

    *xOut++ = fptr[0];
    *yOut++ = fptr[1];

    if ( bv.byteStride )
      ptr += bv.byteStride;
    else
      ptr += 2 * sizeof( float );
  }
  return true;
}

QgsGltfUtils::ResourceType QgsGltfUtils::imageResourceType( const tinygltf::Model &model, int index )
{
  const tinygltf::Image &img = model.images[index];

  if ( !img.image.empty() )
  {
    return ResourceType::Embedded;
  }
  else
  {
    return ResourceType::Linked;
  }
}

QImage QgsGltfUtils::extractEmbeddedImage( const tinygltf::Model &model, int index )
{
  const tinygltf::Image &img = model.images[index];
  if ( !img.image.empty() )
    return QImage( img.image.data(), img.width, img.height, QImage::Format_ARGB32 );
  else
    return QImage();
}

QString QgsGltfUtils::linkedImagePath( const tinygltf::Model &model, int index )
{
  const tinygltf::Image &img = model.images[index];
  return QString::fromStdString( img.uri );
}

std::unique_ptr<QMatrix4x4> QgsGltfUtils::parseNodeTransform( const tinygltf::Node &node )
{
  // read node's transform: either specified with 4x4 "matrix" element
  // -OR- given by "translation", "rotation" and "scale" elements (to be combined as T * R * S)
  std::unique_ptr<QMatrix4x4> matrix;
  if ( !node.matrix.empty() )
  {
    matrix = std::make_unique<QMatrix4x4>( );
    float *mdata = matrix->data();
    for ( int i = 0; i < 16; ++i )
      mdata[i] = static_cast< float >( node.matrix[i] );
  }
  else if ( node.translation.size() || node.rotation.size() || node.scale.size() )
  {
    matrix = std::make_unique<QMatrix4x4>( );
    if ( node.scale.size() )
    {
      matrix->scale( static_cast< float >( node.scale[0] ), static_cast< float >( node.scale[1] ), static_cast< float >( node.scale[2] ) );
    }
    if ( node.rotation.size() )
    {
      matrix->rotate( QQuaternion( static_cast< float >( node.rotation[3] ), static_cast< float >( node.rotation[0] ), static_cast< float >( node.rotation[1] ), static_cast< float >( node.rotation[2] ) ) );
    }
    if ( node.translation.size() )
    {
      matrix->translate( static_cast< float >( node.translation[0] ), static_cast< float >( node.translation[1] ), static_cast< float >( node.translation[2] ) );
    }
  }
  return matrix;
}


QgsVector3D QgsGltfUtils::extractTileTranslation( tinygltf::Model &model, Qgis::Axis upAxis )
{
  bool sceneOk = false;
  const std::size_t sceneIndex = QgsGltfUtils::sourceSceneForModel( model, sceneOk );
  if ( !sceneOk )
  {
    return QgsVector3D();
  }

  const tinygltf::Scene &scene = model.scenes[sceneIndex];

  QgsVector3D tileTranslationEcef;
  auto it = model.extensions.find( "CESIUM_RTC" );
  if ( it != model.extensions.end() )
  {
    const tinygltf::Value v = it->second;
    if ( v.IsObject() && v.Has( "center" ) )
    {
      const tinygltf::Value center = v.Get( "center" );
      if ( center.IsArray() && center.Size() == 3 )
      {
        tileTranslationEcef = QgsVector3D( center.Get( 0 ).GetNumberAsDouble(), center.Get( 1 ).GetNumberAsDouble(), center.Get( 2 ).GetNumberAsDouble() );
      }
    }
  }

  if ( scene.nodes.size() == 0 )
    return QgsVector3D();

  int rootNodeIndex = scene.nodes[0];
  tinygltf::Node &rootNode = model.nodes[rootNodeIndex];

  if ( tileTranslationEcef.isNull() && rootNode.translation.size() )
  {
    QgsVector3D rootTranslation( rootNode.translation[0], rootNode.translation[1], rootNode.translation[2] );

    // if root node of a GLTF contains translation by a large amount, let's handle it as the tile translation.
    // this will ensure that we keep double precision rather than losing precision when dealing with floats
    if ( rootTranslation.length() > 1e6 )
    {
      switch ( upAxis )
      {
        case Qgis::Axis::X:
          QgsDebugError( u"X up translation not yet supported"_s );
          break;
        case Qgis::Axis::Y:
        {
          // we flip Y/Z axes here because GLTF uses Y-up convention, while 3D Tiles use Z-up convention
          tileTranslationEcef = QgsVector3D( rootTranslation.x(), -rootTranslation.z(), rootTranslation.y() );
          rootNode.translation[0] = rootNode.translation[1] = rootNode.translation[2] = 0;
          break;
        }
        case Qgis::Axis::Z:
        {
          tileTranslationEcef = QgsVector3D( rootTranslation.x(), rootTranslation.y(), rootTranslation.z() );
          rootNode.translation[0] = rootNode.translation[1] = rootNode.translation[2] = 0;
          break;
        }
      }
    }
  }

  return tileTranslationEcef;
}


bool QgsGltfUtils::loadImageDataWithQImage(
  tinygltf::Image *image, const int image_idx, std::string *err,
  std::string *warn, int req_width, int req_height,
  const unsigned char *bytes, int size, void *user_data )
{

  if ( req_width != 0 || req_height != 0 )
  {
    if ( err )
    {
      ( *err ) += "Expecting zero req_width/req_height.\n";
    }
    return false;
  }

  ( void )warn;
  ( void )user_data;

  QImage img;
  if ( !img.loadFromData( bytes, size ) )
  {
    if ( err )
    {
      ( *err ) +=
        "Unknown image format. QImage cannot decode image data for image[" +
        std::to_string( image_idx ) + "] name = \"" + image->name + "\".\n";
    }
    return false;
  }

  if ( img.format() != QImage::Format_RGB32 && img.format() != QImage::Format_ARGB32 )
  {
    // we may want to natively support other formats as well as long as such texture formats are allowed
    img.convertTo( QImage::Format_RGB32 );
  }

  image->width = img.width();
  image->height = img.height();
  image->component = 4;
  image->bits = 8;
  image->pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;

  image->image.resize( static_cast<size_t>( image->width * image->height * image->component ) * size_t( image->bits / 8 ) );
  std::copy( img.constBits(), img.constBits() + static_cast< std::size_t >( image->width ) * image->height * image->component * ( image->bits / 8 ), image->image.begin() );

  return true;
}

bool QgsGltfUtils::loadGltfModel( const QByteArray &data, tinygltf::Model &model, QString *errors, QString *warnings )
{
  tinygltf::TinyGLTF loader;

  loader.SetImageLoader( QgsGltfUtils::loadImageDataWithQImage, nullptr );

  // in QGIS we always tend towards permissive handling of datasets, allowing
  // users to load data wherever we can even if it's not strictly conformant
  // with specifications...
  // (and there's a lot of non-compliant GLTF out there!)
  loader.SetParseStrictness( tinygltf::ParseStrictness::Permissive );

  std::string baseDir;  // TODO: may be useful to set it from baseUri
  std::string err, warn;

  bool res;
  if ( data.startsWith( "glTF" ) )   // 4-byte magic value in binary GLTF
  {
    if ( data.at( 4 ) == 1 )
    {
      *errors = QObject::tr( "GLTF version 1 tiles cannot be loaded" );
      return false;
    }
    res = loader.LoadBinaryFromMemory( &model, &err, &warn,
                                       ( const unsigned char * )data.constData(), data.size(), baseDir );
  }
  else
  {
    res = loader.LoadASCIIFromString( &model, &err, &warn,
                                      data.constData(), data.size(), baseDir );
  }

  if ( errors )
    *errors = QString::fromStdString( err );
  if ( warnings )
  {
    *warnings = QString::fromStdString( warn );

    // strip unwanted warnings
    const thread_local QRegularExpression rxFailedToLoadExternalUriForImage( u"Failed to load external 'uri' for image\\[\\d+\\] name = \".*?\"\\n?"_s );
    warnings->replace( rxFailedToLoadExternalUriForImage, QString() );
    const thread_local QRegularExpression rxFileNotFound( u"File not found : .*?\\n"_s );
    warnings->replace( rxFileNotFound, QString() );
  }

  return res;
}

std::size_t QgsGltfUtils::sourceSceneForModel( const tinygltf::Model &model, bool &ok )
{
  ok = false;
  if ( model.scenes.empty() )
  {
    return 0;
  }

  ok = true;
  int index = model.defaultScene;
  if ( index >= 0 && static_cast< std::size_t>( index ) < model.scenes.size() )
  {
    return index;
  }

  // just return first scene
  return 0;
}


#ifdef HAVE_DRACO

void dumpDracoModelInfo( draco::Mesh *dracoMesh )
{
  std::cout << "Decoded Draco Mesh:" << dracoMesh->num_points() << " points / " << dracoMesh->num_faces() << " faces" << std::endl;
  draco::GeometryMetadata *geometryMetadata = dracoMesh->metadata();

  std::cout << "Global Geometry Metadata:" << std::endl;
  for ( const auto &entry : geometryMetadata->entries() )
  {
    std::cout << "  Key: " << entry.first << ", Value: " << entry.second.data().size() << std::endl;
  }

  std::cout << "\nAttribute Metadata:" << std::endl;
  for ( int32_t i = 0; i < dracoMesh->num_attributes(); ++i )
  {
    const draco::PointAttribute *attribute = dracoMesh->attribute( i );
    if ( !attribute )
      continue;

    std::cout << "  Attribute ID: " << attribute->unique_id() << " / " << draco::PointAttribute::TypeToString( attribute->attribute_type() ) << std::endl;
    if ( const draco::AttributeMetadata *attributeMetadata = geometryMetadata->attribute_metadata( attribute->unique_id() ) )
    {
      for ( const auto &entry : attributeMetadata->entries() )
      {
        std::cout << "    Key: " << entry.first << ", Length: " << entry.second.data().size() << std::endl;
      }
    }
  }
}


bool QgsGltfUtils::loadDracoModel( const QByteArray &data, const I3SNodeContext &context, tinygltf::Model &model, QString *errors )
{
  //
  // SLPK and Extracted SLPK have the files gzipped
  //

  QByteArray dataExtracted;
  if ( data.startsWith( QByteArray( "\x1f\x8b", 2 ) ) )
  {
    if ( !QgsZipUtils::decodeGzip( data, dataExtracted ) )
    {
      if ( errors )
        *errors = "Failed to decode gzipped model";
      return false;
    }
  }
  else
  {
    dataExtracted = data;
  }

  //
  // load the model in decoder and do basic sanity checks
  //

  draco::Decoder decoder;
  draco::DecoderBuffer decoderBuffer;
  decoderBuffer.Init( dataExtracted.constData(), dataExtracted.size() );

  draco::StatusOr<draco::EncodedGeometryType> geometryTypeStatus = decoder.GetEncodedGeometryType( &decoderBuffer );
  if ( !geometryTypeStatus.ok() )
  {
    if ( errors )
      *errors = "Failed to get geometry type: " + QString( geometryTypeStatus.status().error_msg() );
    return false;
  }
  if ( geometryTypeStatus.value() != draco::EncodedGeometryType::TRIANGULAR_MESH )
  {
    if ( errors )
      *errors = "Not a triangular mesh";
    return false;
  }

  draco::StatusOr<std::unique_ptr<draco::Mesh>> meshStatus = decoder.DecodeMeshFromBuffer( &decoderBuffer );
  if ( !meshStatus.ok() )
  {
    if ( errors )
      *errors = "Failed to decode mesh: " + QString( meshStatus.status().error_msg() );
    return false;
  }

  std::unique_ptr<draco::Mesh> dracoMesh = std::move( meshStatus ).value();

  draco::GeometryMetadata *geometryMetadata = dracoMesh->metadata();
  if ( !geometryMetadata )
  {
    if ( errors )
      *errors = "Geometry metadata missing";
    return false;
  }

  int posAccessorIndex = -1;
  int normalAccessorIndex = -1;
  int uvAccessorIndex = -1;
  int indicesAccessorIndex = -1;

  //
  // parse XYZ position coordinates
  //

  const draco::PointAttribute *posAttribute = dracoMesh->GetNamedAttribute( draco::GeometryAttribute::POSITION );
  if ( posAttribute )
  {
    double scaleX = 1, scaleY = 1;
    const draco::AttributeMetadata *posMetadata = geometryMetadata->attribute_metadata( posAttribute->unique_id() );
    if ( posMetadata )
    {
      posMetadata->GetEntryDouble( "i3s-scale_x", &scaleX );
      posMetadata->GetEntryDouble( "i3s-scale_y", &scaleY );
    }

    QgsVector3D nodeCenterLonLat = context.datasetToSceneTransform.transform( context.nodeCenterEcef, Qgis::TransformDirection::Reverse );

    std::vector<unsigned char> posData( dracoMesh->num_points() * 3 * sizeof( float ) );
    float *posPtr = reinterpret_cast<float *>( posData.data() );

    float values[4];
    for ( draco::PointIndex i( 0 ); i < dracoMesh->num_points(); ++i )
    {
      posAttribute->ConvertValue<float>( posAttribute->mapped_index( i ), posAttribute->num_components(), values );

      // when using EPSG:4326, the X,Y coordinates are in degrees(!) relative to the node's center (in lat/lon degrees),
      // but they are scaled (because they are several orders of magnitude smaller than Z coordinates).
      // That scaling is applied so that Draco's compression works well.
      // when using local CRS, scaling is not applied (not needed)
      if ( context.isGlobalMode )
      {
        double lonDeg = double( values[0] ) * scaleX + nodeCenterLonLat.x();
        double latDeg = double( values[1] ) * scaleY + nodeCenterLonLat.y();
        double alt = double( values[2] ) + nodeCenterLonLat.z();
        QgsVector3D ecef = context.datasetToSceneTransform.transform( QgsVector3D( lonDeg, latDeg, alt ) );
        QgsVector3D localPos = ecef - context.nodeCenterEcef;

        values[0] = static_cast<float>( localPos.x() );
        values[1] = static_cast<float>( localPos.y() );
        values[2] = static_cast<float>( localPos.z() );
      }

      posPtr[i.value() * 3 + 0] = values[0];
      posPtr[i.value() * 3 + 1] = values[1];
      posPtr[i.value() * 3 + 2] = values[2];
    }

    tinygltf::Buffer posBuffer;
    posBuffer.data = posData;
    model.buffers.emplace_back( std::move( posBuffer ) );

    tinygltf::BufferView posBufferView;
    posBufferView.buffer = static_cast<int>( model.buffers.size() ) - 1;
    posBufferView.byteOffset = 0;
    posBufferView.byteLength = posData.size();
    posBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    model.bufferViews.emplace_back( std::move( posBufferView ) );

    tinygltf::Accessor posAccessor;
    posAccessor.bufferView = static_cast<int>( model.bufferViews.size() ) - 1;
    posAccessor.byteOffset = 0;
    posAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    posAccessor.count = dracoMesh->num_points();
    posAccessor.type = TINYGLTF_TYPE_VEC3;
    model.accessors.emplace_back( std::move( posAccessor ) );

    posAccessorIndex = static_cast<int>( model.accessors.size() ) - 1;
  }

  //
  // parse normal vectors
  //

  const draco::PointAttribute *normalAttribute = dracoMesh->GetNamedAttribute( draco::GeometryAttribute::NORMAL );
  if ( normalAttribute )
  {
    std::vector<unsigned char> normalData( dracoMesh->num_points() * 3 * sizeof( float ) );
    float *normalPtr = reinterpret_cast<float *>( normalData.data() );

    float values[3];
    for ( draco::PointIndex i( 0 ); i < dracoMesh->num_points(); ++i )
    {
      normalAttribute->ConvertValue<float>( normalAttribute->mapped_index( i ), normalAttribute->num_components(), values );

      normalPtr[i.value() * 3 + 0] = values[0];
      normalPtr[i.value() * 3 + 1] = values[1];
      normalPtr[i.value() * 3 + 2] = values[2];
    }

    tinygltf::Buffer normalBuffer;
    normalBuffer.data = normalData;
    model.buffers.emplace_back( std::move( normalBuffer ) );

    tinygltf::BufferView normalBufferView;
    normalBufferView.buffer = static_cast<int>( model.buffers.size() ) - 1;
    normalBufferView.byteOffset = 0;
    normalBufferView.byteLength = normalData.size();
    normalBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    model.bufferViews.emplace_back( std::move( normalBufferView ) );

    tinygltf::Accessor normalAccessor;
    normalAccessor.bufferView = static_cast<int>( model.bufferViews.size() ) - 1;
    normalAccessor.byteOffset = 0;
    normalAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    normalAccessor.count = dracoMesh->num_points();
    normalAccessor.type = TINYGLTF_TYPE_VEC3;
    model.accessors.emplace_back( std::move( normalAccessor ) );

    normalAccessorIndex = static_cast<int>( model.accessors.size() ) - 1;
  }

  //
  // parse UV texture coordinates
  //

  const draco::PointAttribute *uvAttribute = dracoMesh->GetNamedAttribute( draco::GeometryAttribute::TEX_COORD );
  if ( uvAttribute )
  {
    std::vector<unsigned char> uvData( dracoMesh->num_points() * 2 * sizeof( float ) );
    float *uvPtr = reinterpret_cast<float *>( uvData.data() );

    // try to find UV region attribute - if it exists, we will need to adjust
    // texture UV values based on its regions
    const draco::PointAttribute *uvRegionAttribute = nullptr;
    for ( int32_t i = 0; i < dracoMesh->num_attributes(); ++i )
    {
      const draco::PointAttribute *attribute = dracoMesh->attribute( i );
      if ( !attribute )
        continue;

      draco::AttributeMetadata *attributeMetadata = geometryMetadata->attribute_metadata( attribute->unique_id() );
      if ( !attributeMetadata )
        continue;

      std::string i3sAttributeType;
      if ( attributeMetadata->GetEntryString( "i3s-attribute-type", &i3sAttributeType ) && i3sAttributeType == "uv-region" )
      {
        uvRegionAttribute = attribute;
      }
    }

    float values[2];
    for ( draco::PointIndex i( 0 ); i < dracoMesh->num_points(); ++i )
    {
      uvAttribute->ConvertValue<float>( uvAttribute->mapped_index( i ), uvAttribute->num_components(), values );

      if ( uvRegionAttribute )
      {
        // UV regions are 4 x uint16 per each vertex [uMin, vMin, uMax, vMax], and they define
        // a sub-region within a texture to which UV coordinates of each vertex belong.
        // I have no idea why there's such extra complication for clients... the final
        // UV coordinates could have been easily calculated by the dataset producer.
        uint16_t uvRegion[4];
        uvRegionAttribute->ConvertValue<uint16_t>( uvRegionAttribute->mapped_index( i ), uvRegionAttribute->num_components(), uvRegion );
        float uMin = static_cast<float>( uvRegion[0] ) / 65535.f;
        float vMin = static_cast<float>( uvRegion[1] ) / 65535.f;
        float uMax = static_cast<float>( uvRegion[2] ) / 65535.f;
        float vMax = static_cast<float>( uvRegion[3] ) / 65535.f;
        values[0] = uMin + values[0] * ( uMax - uMin );
        values[1] = vMin + values[1] * ( vMax - vMin );
      }

      uvPtr[i.value() * 2 + 0] = values[0];
      uvPtr[i.value() * 2 + 1] = values[1];
    }

    tinygltf::Buffer uvBuffer;
    uvBuffer.data = uvData;
    model.buffers.emplace_back( std::move( uvBuffer ) );

    tinygltf::BufferView uvBufferView;
    uvBufferView.buffer = static_cast<int>( model.buffers.size() ) - 1;
    uvBufferView.byteOffset = 0;
    uvBufferView.byteLength = uvData.size();
    uvBufferView.target = TINYGLTF_TARGET_ARRAY_BUFFER;
    model.bufferViews.emplace_back( std::move( uvBufferView ) ) ;

    tinygltf::Accessor uvAccessor;
    uvAccessor.bufferView = static_cast<int>( model.bufferViews.size() ) - 1;
    uvAccessor.byteOffset = 0;
    uvAccessor.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
    uvAccessor.count = dracoMesh->num_points();
    uvAccessor.type = TINYGLTF_TYPE_VEC2;
    model.accessors.emplace_back( std::move( uvAccessor ) );

    uvAccessorIndex = static_cast<int>( model.accessors.size() ) - 1;
  }

  //
  // parse indices of triangles
  //

  // TODO: to save some memory, we could use only 1 or 2 bytes per vertex if the mesh is small enough
  std::vector<unsigned char> indexData;
  indexData.resize( dracoMesh->num_faces() * 3 * sizeof( quint32 ) );
  Q_ASSERT( sizeof( dracoMesh->face( draco::FaceIndex( 0 ) )[0] ) == sizeof( quint32 ) );
  memcpy( indexData.data(), &dracoMesh->face( draco::FaceIndex( 0 ) )[0], indexData.size() );

  tinygltf::Buffer gltfIndexBuffer;
  gltfIndexBuffer.data = indexData;
  model.buffers.emplace_back( std::move( gltfIndexBuffer ) );

  tinygltf::BufferView indexBufferView;
  indexBufferView.buffer = static_cast<int>( model.buffers.size() ) - 1;
  indexBufferView.byteLength = dracoMesh->num_faces() * 3 * sizeof( quint32 );
  indexBufferView.byteOffset = 0;
  indexBufferView.byteStride = 0;
  indexBufferView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;
  model.bufferViews.emplace_back( std::move( indexBufferView ) );

  tinygltf::Accessor indicesAccessor;
  indicesAccessor.bufferView = static_cast<int>( model.bufferViews.size() ) - 1;
  indicesAccessor.byteOffset = 0;
  indicesAccessor.count = dracoMesh->num_faces() * 3;
  indicesAccessor.type = TINYGLTF_TYPE_SCALAR;
  indicesAccessor.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
  model.accessors.emplace_back( std::move( indicesAccessor ) );

  indicesAccessorIndex = static_cast<int>( model.accessors.size() ) - 1;

  //
  // construct the GLTF model
  //

  tinygltf::Material material;
  int materialIndex = loadMaterialFromMetadata( context.materialInfo, model );

  tinygltf::Primitive primitive;
  primitive.mode = TINYGLTF_MODE_TRIANGLES;
  primitive.material = materialIndex;
  primitive.indices = indicesAccessorIndex;
  if ( posAccessorIndex != -1 )
    primitive.attributes["POSITION"] = posAccessorIndex;
  if ( normalAccessorIndex != -1 )
    primitive.attributes["NORMAL"] = normalAccessorIndex;
  if ( uvAccessorIndex != -1 )
    primitive.attributes["TEXCOORD_0"] = uvAccessorIndex;

  tinygltf::Mesh tiny_mesh;
  tiny_mesh.primitives.emplace_back( std::move( primitive ) );
  model.meshes.emplace_back( std::move( tiny_mesh ) );

  tinygltf::Node node;
  node.mesh = 0;
  model.nodes.emplace_back( std::move( node ) );

  tinygltf::Scene scene;
  scene.nodes.push_back( 0 );
  model.scenes.emplace_back( std::move( scene ) );

  model.defaultScene = 0;
  model.asset.version = "2.0";

  return true;
}
#else

bool QgsGltfUtils::loadDracoModel( const QByteArray &data, const I3SNodeContext &context, tinygltf::Model &model, QString *errors )
{
  Q_UNUSED( data );
  Q_UNUSED( context );
  Q_UNUSED( model );
  if ( errors )
    *errors = "Cannot load geometry - QGIS was built without Draco library.";
  return false;
}

#endif

int QgsGltfUtils::loadMaterialFromMetadata( const QVariantMap &materialInfo, tinygltf::Model &model )
{
  tinygltf::Material material;
  material.name = "DefaultMaterial";

  QVariantList colorList = materialInfo["pbrBaseColorFactor"].toList();
  material.pbrMetallicRoughness.baseColorFactor = { colorList[0].toDouble(), colorList[1].toDouble(), colorList[2].toDouble(), colorList[3].toDouble() };

  if ( materialInfo.contains( "pbrBaseColorTexture" ) )
  {
    QString baseColorTextureUri = materialInfo["pbrBaseColorTexture"].toString();

    tinygltf::Image img;
    img.uri = baseColorTextureUri.toStdString();   // file:/// or http:// ... will be fetched by QGIS
    model.images.emplace_back( std::move( img ) );

    tinygltf::Texture tex;
    tex.source = static_cast<int>( model.images.size() ) - 1;
    model.textures.emplace_back( std::move( tex ) );

    material.pbrMetallicRoughness.baseColorTexture.index = static_cast<int>( model.textures.size() ) - 1;
  }

  if ( materialInfo.contains( "doubleSided" ) )
  {
    material.doubleSided = materialInfo["doubleSided"].toInt();
  }

  // add the new material to the model
  model.materials.emplace_back( std::move( material ) );

  return static_cast<int>( model.materials.size() ) - 1;
}

bool QgsGltfUtils::writeGltfModel( const tinygltf::Model &model, const QString &outputFilename )
{
  tinygltf::TinyGLTF gltf;
  bool res = gltf.WriteGltfSceneToFile( &model,
                                        outputFilename.toStdString(),
                                        false,    // embedImages
                                        true,     // embedBuffers
                                        false,    // prettyPrint
                                        true );   // writeBinary
  return res;
}

void QgsGltfUtils::I3SNodeContext::initFromTile( const QgsTiledSceneTile &tile, const QgsCoordinateReferenceSystem &layerCrs, const QgsCoordinateReferenceSystem &sceneCrs, const QgsCoordinateTransformContext &transformContext )
{
  const QVariantMap tileMetadata = tile.metadata();

  materialInfo = tileMetadata[u"material"_s].toMap();
  isGlobalMode = sceneCrs.type() == Qgis::CrsType::Geocentric;
  if ( isGlobalMode )
  {
    nodeCenterEcef = tile.boundingVolume().box().center();
    datasetToSceneTransform = QgsCoordinateTransform( layerCrs, sceneCrs, transformContext );
  }
}

///@endcond
