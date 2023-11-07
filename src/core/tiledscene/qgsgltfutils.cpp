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

#include "qgsgltfutils.h"

#include "qgscoordinatetransform.h"
#include "qgsexception.h"
#include "qgsmatrix4x4.h"
#include "qgsconfig.h"
#include "qgslogger.h"

#include <QImage>
#include <QMatrix4x4>
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
        QgsDebugError( QStringLiteral( "X up translation not yet supported" ) );
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
    matrix.reset( new QMatrix4x4 );
    float *mdata = matrix->data();
    for ( int i = 0; i < 16; ++i )
      mdata[i] = static_cast< float >( node.matrix[i] );
  }
  else if ( node.translation.size() || node.rotation.size() || node.scale.size() )
  {
    matrix.reset( new QMatrix4x4 );
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
          QgsDebugError( QStringLiteral( "X up translation not yet supported" ) );
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
    const thread_local QRegularExpression rxFailedToLoadExternalUriForImage( QStringLiteral( "Failed to load external 'uri' for image\\[\\d+\\] name = \".*?\"\\n?" ) );
    warnings->replace( rxFailedToLoadExternalUriForImage, QString() );
    const thread_local QRegularExpression rxFileNotFound( QStringLiteral( "File not found : .*?\\n" ) );
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

///@endcond
