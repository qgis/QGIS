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

#include <QImage>
#include <QMatrix4x4>

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


bool QgsGltfUtils::accessorToMapCoordinates( tinygltf::Model &model, int accessorIndex, const QgsMatrix4x4 &tileTransform, const QgsCoordinateTransform *ecefToTargetCrs, const QgsVector3D &tileTranslationEcef, QMatrix4x4 *nodeTransform, QVector<double> &vx, QVector<double> &vy, QVector<double> &vz )
{
  tinygltf::Accessor &accessor = model.accessors[accessorIndex];
  tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
  tinygltf::Buffer &b = model.buffers[bv.buffer];

  if ( accessor.componentType != TINYGLTF_PARAMETER_TYPE_FLOAT || accessor.type != TINYGLTF_TYPE_VEC3 )
  {
    // we may support more input types in the future if needed
    return false;
  }

  unsigned char *ptr = b.data.data() + bv.byteOffset + accessor.byteOffset;

  vx.resize( accessor.count );
  vy.resize( accessor.count );
  vz.resize( accessor.count );
  double *vxOut = vx.data();
  double *vyOut = vy.data();
  double *vzOut = vz.data();
  for ( int i = 0; i < static_cast<int>( accessor.count ); ++i )
  {
    float *fptr = reinterpret_cast<float *>( ptr );
    QVector3D vOrig( fptr[0], fptr[1], fptr[2] );

    if ( nodeTransform )
      vOrig = nodeTransform->map( vOrig );

    // go from y-up to z-up according to 3D Tiles spec
    QVector3D vFlip( vOrig.x(), -vOrig.z(), vOrig.y() );

    // apply also transform of the node
    QgsVector3D v = tileTransform.map( QgsVector3D( vFlip ) + tileTranslationEcef );

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

bool QgsGltfUtils::extractTextureCoordinates( tinygltf::Model &model, int accessorIndex, QVector<double> &x, QVector<double> &y )
{
  tinygltf::Accessor &accessor = model.accessors[accessorIndex];
  tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
  tinygltf::Buffer &b = model.buffers[bv.buffer];

  if ( accessor.componentType != TINYGLTF_PARAMETER_TYPE_FLOAT || accessor.type != TINYGLTF_TYPE_VEC2 )
  {
    return false;
  }

  unsigned char *ptr = b.data.data() + bv.byteOffset + accessor.byteOffset;
  x.resize( accessor.count );
  y.resize( accessor.count );

  double *xOut = x.data();
  double *yOut = y.data();

  for ( std::size_t i = 0; i < accessor.count; i++ )
  {
    float *fptr = reinterpret_cast< float * >( ptr );

    *xOut++ = fptr[0];
    *yOut++ = fptr[1];

    if ( bv.byteStride )
      ptr += bv.byteStride;
    else
      ptr += 2 * sizeof( float );
  }
  return true;
}

QgsGltfUtils::ResourceType QgsGltfUtils::imageResourceType( tinygltf::Model &model, int index )
{
  tinygltf::Image &img = model.images[index];

  if ( !img.image.empty() )
  {
    return ResourceType::Embedded;
  }
  else
  {
    return ResourceType::Linked;
  }
}

QImage QgsGltfUtils::extractEmbeddedImage( tinygltf::Model &model, int index )
{
  tinygltf::Image &img = model.images[index];
  if ( !img.image.empty() )
    return QImage( img.image.data(), img.width, img.height, QImage::Format_ARGB32 );
  else
    return QImage();
}

QString QgsGltfUtils::linkedImagePath( tinygltf::Model &model, int index )
{
  tinygltf::Image &img = model.images[index];
  return QString::fromStdString( img.uri );
}

std::unique_ptr<QMatrix4x4> QgsGltfUtils::parseNodeTransform( tinygltf::Node &node )
{
  // read node's transform: either specified with 4x4 "matrix" element
  // -OR- given by "translation", "rotation" and "scale" elements (to be combined as T * R * S)
  std::unique_ptr<QMatrix4x4> matrix;
  if ( !node.matrix.empty() )
  {
    matrix.reset( new QMatrix4x4 );
    float *mdata = matrix->data();
    for ( int i = 0; i < 16; ++i )
      mdata[i] = node.matrix[i];
  }
  else if ( node.translation.size() || node.rotation.size() || node.scale.size() )
  {
    matrix.reset( new QMatrix4x4 );
    if ( node.scale.size() )
    {
      matrix->scale( node.scale[0], node.scale[1], node.scale[2] );
    }
    if ( node.rotation.size() )
    {
      matrix->rotate( QQuaternion( node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2] ) );
    }
    if ( node.translation.size() )
    {
      matrix->translate( node.translation[0], node.translation[1], node.translation[2] );
    }
  }
  return matrix;
}


QgsVector3D QgsGltfUtils::extractTileTranslation( tinygltf::Model &model )
{
  tinygltf::Scene &scene = model.scenes[model.defaultScene];

  QgsVector3D tileTranslationEcef;
  if ( model.extensions.find( "CESIUM_RTC" ) != model.extensions.end() )
  {
    tinygltf::Value v = model.extensions["CESIUM_RTC"];
    if ( v.IsObject() && v.Has( "center" ) )
    {
      tinygltf::Value center = v.Get( "center" );
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
      // we flip Y/Z axes here because GLTF uses Y-up convention, while 3D Tiles use Z-up convention
      tileTranslationEcef = QgsVector3D( rootTranslation.x(), -rootTranslation.z(), rootTranslation.y() );
      rootNode.translation[0] = rootNode.translation[1] = rootNode.translation[2] = 0;
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
  std::copy( img.constBits(), img.constBits() + image->width * image->height * image->component * ( image->bits / 8 ), image->image.begin() );

  return true;
}

bool QgsGltfUtils::loadGltfModel( const QByteArray &data, tinygltf::Model &model, QString *errors, QString *warnings )
{
  tinygltf::TinyGLTF loader;

  loader.SetImageLoader( QgsGltfUtils::loadImageDataWithQImage, nullptr );

  std::string baseDir;  // TODO: may be useful to set it from baseUri
  std::string err, warn;

  bool res;
  if ( data.startsWith( "glTF" ) )   // 4-byte magic value in binary GLTF
  {
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
    *warnings = QString::fromStdString( warn );

  return res;
}

///@endcond
