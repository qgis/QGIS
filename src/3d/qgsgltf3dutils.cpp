/***************************************************************************
  qgsgltf3dutils.cpp
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


#include "qgsgltf3dutils.h"

#include <memory>

#include "qgs3dutils.h"
#include "qgsblockingnetworkrequest.h"
#include "qgscoordinatetransform.h"
#include "qgsgltfutils.h"
#include "qgslogger.h"
#include "qgsmaterial3dhandler.h"
#include "qgsmetalroughmaterial.h"
#include "qgstexturematerial.h"
#include "qgsziputils.h"

#include <QFile>
#include <QFileInfo>
#include <QMatrix4x4>
#include <QString>
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QEntity>
#include <Qt3DCore/QGeometry>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTexture>

using namespace Qt::StringLiterals;

///@cond PRIVATE

static Qt3DCore::QAttribute::VertexBaseType parseVertexBaseType( int componentType )
{
  switch ( componentType )
  {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      return Qt3DCore::QAttribute::Byte;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      return Qt3DCore::QAttribute::UnsignedByte;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      return Qt3DCore::QAttribute::Short;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      return Qt3DCore::QAttribute::UnsignedShort;
    case TINYGLTF_COMPONENT_TYPE_INT:
      return Qt3DCore::QAttribute::Int;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      return Qt3DCore::QAttribute::UnsignedInt;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      return Qt3DCore::QAttribute::Float;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      return Qt3DCore::QAttribute::Double;
  }
  Q_ASSERT( false );
  return Qt3DCore::QAttribute::UnsignedInt;
}


static Qt3DRender::QAbstractTexture::Filter parseTextureFilter( int filter )
{
  switch ( filter )
  {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
      return Qt3DRender::QTexture2D::Nearest;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
      return Qt3DRender::QTexture2D::Linear;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
      return Qt3DRender::QTexture2D::NearestMipMapNearest;
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
      return Qt3DRender::QTexture2D::LinearMipMapNearest;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
      return Qt3DRender::QTexture2D::NearestMipMapLinear;
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
      return Qt3DRender::QTexture2D::LinearMipMapLinear;
  }

  // play it safe and handle malformed models
  return Qt3DRender::QTexture2D::Nearest;
}

static Qt3DRender::QTextureWrapMode::WrapMode parseTextureWrapMode( int wrapMode )
{
  switch ( wrapMode )
  {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
      return Qt3DRender::QTextureWrapMode::Repeat;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
      return Qt3DRender::QTextureWrapMode::ClampToEdge;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
      return Qt3DRender::QTextureWrapMode::MirroredRepeat;
  }
  // some malformed GLTF models have incorrect texture wrap modes (eg
  // https://qld.digitaltwin.terria.io/api/v0/data/b73ccb60-66ef-4470-8c3c-44af36c4d69b/CBD/tileset.json )
  return Qt3DRender::QTextureWrapMode::Repeat;
}


static Qt3DCore::QAttribute *parseAttribute( tinygltf::Model &model, int accessorIndex )
{
  tinygltf::Accessor &accessor = model.accessors[accessorIndex];
  tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
  tinygltf::Buffer &b = model.buffers[bv.buffer];

  // TODO: only ever create one QBuffer for a buffer even if it is used multiple times
  QByteArray byteArray( reinterpret_cast<const char *>( b.data.data() ),
                        static_cast<int>( b.data.size() ) ); // makes a deep copy
  Qt3DCore::QBuffer *buffer = new Qt3DCore::QBuffer();
  buffer->setData( byteArray );

  Qt3DCore::QAttribute *attribute = new Qt3DCore::QAttribute();

  // "target" is optional, can be zero
  if ( bv.target == TINYGLTF_TARGET_ARRAY_BUFFER )
    attribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  else if ( bv.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER )
    attribute->setAttributeType( Qt3DCore::QAttribute::IndexAttribute );

  attribute->setBuffer( buffer );
  attribute->setByteOffset( bv.byteOffset + accessor.byteOffset );
  attribute->setByteStride( bv.byteStride ); // could be zero, it seems that's fine (assuming packed)
  attribute->setCount( accessor.count );
  attribute->setVertexBaseType( parseVertexBaseType( accessor.componentType ) );
  attribute->setVertexSize( tinygltf::GetNumComponentsInType( accessor.type ) );

  return attribute;
}


static Qt3DCore::QAttribute *reprojectPositions( tinygltf::Model &model, int accessorIndex, const QgsGltf3DUtils::EntityTransform &transform, const QgsVector3D &tileTranslationEcef, QMatrix4x4 *matrix )
{
  tinygltf::Accessor &accessor = model.accessors[accessorIndex];

  QVector<double> vx, vy, vz;
  bool res = QgsGltfUtils::accessorToMapCoordinates( model, accessorIndex, transform.tileTransform, transform.ecefToTargetCrs, tileTranslationEcef, matrix, transform.gltfUpAxis, vx, vy, vz );
  if ( !res )
    return nullptr;

  QByteArray byteArray;
  byteArray.resize( accessor.count * 4 * 3 );
  float *out = reinterpret_cast<float *>( byteArray.data() );

  QgsVector3D sceneOrigin = transform.chunkOriginTargetCrs;
  for ( int i = 0; i < static_cast<int>( accessor.count ); ++i )
  {
    double x = vx[i] - sceneOrigin.x();
    double y = vy[i] - sceneOrigin.y();
    double z = ( vz[i] * transform.zValueScale ) + transform.zValueOffset - sceneOrigin.z();

    out[i * 3 + 0] = static_cast<float>( x );
    out[i * 3 + 1] = static_cast<float>( y );
    out[i * 3 + 2] = static_cast<float>( z );
  }

  Qt3DCore::QBuffer *buffer = new Qt3DCore::QBuffer();
  buffer->setData( byteArray );

  Qt3DCore::QAttribute *attribute = new Qt3DCore::QAttribute();
  attribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  attribute->setBuffer( buffer );
  attribute->setByteOffset( 0 );
  attribute->setByteStride( 12 );
  attribute->setCount( accessor.count );
  attribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  attribute->setVertexSize( 3 );

  return attribute;
}

class TinyGltfTextureImageDataGenerator : public Qt3DRender::QTextureImageDataGenerator
{
  public:
    TinyGltfTextureImageDataGenerator( Qt3DRender::QTextureImageDataPtr imagePtr )
      : mImagePtr( imagePtr )
    {}

    Qt3DRender::QTextureImageDataPtr operator()() override { return mImagePtr; }

    qintptr id() const override { return reinterpret_cast<qintptr>( &Qt3DCore::FunctorType<TinyGltfTextureImageDataGenerator>::id ); }

    bool operator==( const QTextureImageDataGenerator &other ) const override
    {
      const TinyGltfTextureImageDataGenerator *otherFunctor = dynamic_cast<const TinyGltfTextureImageDataGenerator *>( &other );
      return otherFunctor && mImagePtr.get() == otherFunctor->mImagePtr.get();
    }

    Qt3DRender::QTextureImageDataPtr mImagePtr;
};

class TinyGltfTextureImage : public Qt3DRender::QAbstractTextureImage
{
    Q_OBJECT
  public:
    TinyGltfTextureImage( tinygltf::Image &image )
    {
      Q_ASSERT( image.bits == 8 );
      Q_ASSERT( image.component == 4 );
      Q_ASSERT( image.pixel_type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE );

      imgDataPtr.reset( new Qt3DRender::QTextureImageData );
      imgDataPtr->setWidth( image.width );
      imgDataPtr->setHeight( image.height );
      imgDataPtr->setDepth( 1 ); // not sure what this is
      imgDataPtr->setFaces( 1 );
      imgDataPtr->setLayers( 1 );
      imgDataPtr->setMipLevels( 1 );
      QByteArray imageBytes( reinterpret_cast<const char *>( image.image.data() ), image.image.size() );
      imgDataPtr->setData( imageBytes, 4 );
      imgDataPtr->setFormat( QOpenGLTexture::RGBA8_UNorm );
      imgDataPtr->setPixelFormat( QOpenGLTexture::BGRA ); // when using tinygltf with STB_image, pixel format is QOpenGLTexture::RGBA
      imgDataPtr->setPixelType( QOpenGLTexture::UInt8 );
      imgDataPtr->setTarget( QOpenGLTexture::Target2D );
    }

    Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override { return Qt3DRender::QTextureImageDataGeneratorPtr( new TinyGltfTextureImageDataGenerator( imgDataPtr ) ); }

    Qt3DRender::QTextureImageDataPtr imgDataPtr;
};


// TODO: move elsewhere
static QByteArray fetchUri( const QUrl &url, QStringList *errors )
{
  if ( url.scheme().startsWith( "http" ) )
  {
    QNetworkRequest request = QNetworkRequest( url );
    request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
    request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
    QgsBlockingNetworkRequest networkRequest;
    // TODO: setup auth, setup headers
    if ( networkRequest.get( request ) != QgsBlockingNetworkRequest::NoError )
    {
      if ( errors )
        *errors << u"Failed to download image: %1"_s.arg( url.toString() );
    }
    else
    {
      const QgsNetworkReplyContent content = networkRequest.reply();
      return content.content();
    }
  }
  else if ( url.isLocalFile() )
  {
    QString localFilePath = url.toLocalFile();
    if ( localFilePath.contains( ".slpk/" ) ) // we need to extract the image from SLPK archive
    {
      const QStringList parts = localFilePath.split( u".slpk/"_s );
      if ( parts.size() == 2 )
      {
        QString slpkPath = parts[0] + ".slpk";
        QString imagePath = parts[1];

        QByteArray imageData;
        if ( QgsZipUtils::extractFileFromZip( slpkPath, imagePath, imageData ) )
        {
          return imageData;
        }
        else
        {
          if ( errors )
            *errors << u"Unable to extract image '%1' from SLPK archive: %2"_s.arg( imagePath ).arg( slpkPath );
        }
      }
      else
      {
        if ( errors )
          *errors << u"Missing image path in SLPK archive: %1"_s.arg( localFilePath );
      }
    }
    else if ( QFile::exists( localFilePath ) )
    {
      QFile f( localFilePath );
      if ( f.open( QIODevice::ReadOnly ) )
      {
        return f.readAll();
      }
    }
    else
    {
      if ( errors )
        *errors << u"Unable to open image: %1"_s.arg( url.toString() );
    }
  }
  return QByteArray();
}

// Returns NULLPTR if primitive should not be rendered
static QgsMaterial *parseMaterial( tinygltf::Model &model, int materialIndex, QString baseUri, QStringList *errors, const QgsMaterialContext &context )
{
  if ( materialIndex < 0 )
  {
    // material unspecified - using default
    QgsMetalRoughMaterial *defaultMaterial = new QgsMetalRoughMaterial;
    defaultMaterial->setMetalness( 1 );
    defaultMaterial->setRoughness( 1 );
    defaultMaterial->setBaseColor( QColor::fromRgbF( 1, 1, 1 ) );
    return defaultMaterial;
  }

  tinygltf::Material &material = model.materials[materialIndex];
  tinygltf::PbrMetallicRoughness &pbr = material.pbrMetallicRoughness;

  if ( pbr.baseColorTexture.index >= 0 )
  {
    tinygltf::Texture &tex = model.textures[pbr.baseColorTexture.index];

    // Source can be undefined if texture is provided by an extension
    if ( tex.source < 0 )
    {
      QgsMetalRoughMaterial *pbrMaterial = new QgsMetalRoughMaterial;
      pbrMaterial->setMetalness( pbr.metallicFactor ); // [0..1] or texture
      pbrMaterial->setRoughness( pbr.roughnessFactor );
      pbrMaterial->setBaseColor( QColor::fromRgbF( pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3] ) );
      return pbrMaterial;
    }

    tinygltf::Image &img = model.images[tex.source];

    if ( !img.uri.empty() )
    {
      QString imgUri = QString::fromStdString( img.uri );
      QUrl url = QUrl( baseUri ).resolved( imgUri );
      QByteArray ba = fetchUri( url, errors );
      if ( !ba.isEmpty() )
      {
        if ( !QgsGltfUtils::loadImageDataWithQImage( &img, -1, nullptr, nullptr, 0, 0, ( const unsigned char * ) ba.constData(), ba.size(), nullptr ) )
        {
          if ( errors )
            *errors << u"Failed to load image: %1"_s.arg( imgUri );
        }
      }
    }

    if ( img.image.empty() )
    {
      QgsMetalRoughMaterial *pbrMaterial = new QgsMetalRoughMaterial;
      pbrMaterial->setMetalness( pbr.metallicFactor ); // [0..1] or texture
      pbrMaterial->setRoughness( pbr.roughnessFactor );
      pbrMaterial->setBaseColor( QColor::fromRgbF( pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3] ) );
      return pbrMaterial;
    }

    TinyGltfTextureImage *textureImage = new TinyGltfTextureImage( img );

    Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D;
    texture->addTextureImage( textureImage ); // textures take the ownership of textureImage if has no parant

    Qgs3DUtils::setTextureFiltering( texture, context );

    texture->setFormat( Qt3DRender::QAbstractTexture::SRGB8_Alpha8 );

    if ( tex.sampler >= 0 )
    {
      tinygltf::Sampler &sampler = model.samplers[tex.sampler];
      if ( sampler.minFilter >= 0 )
        texture->setMinificationFilter( parseTextureFilter( sampler.minFilter ) );
      if ( sampler.magFilter >= 0 )
        texture->setMagnificationFilter( parseTextureFilter( sampler.magFilter ) );
      Qt3DRender::QTextureWrapMode wrapMode;
      wrapMode.setX( parseTextureWrapMode( sampler.wrapS ) );
      wrapMode.setY( parseTextureWrapMode( sampler.wrapT ) );
      texture->setWrapMode( wrapMode );
    }

    // We should be using PBR material unless unlit material is requested using KHR_materials_unlit
    // GLTF extension, but in various datasets that extension is not used (even though it should have been).
    // In the future we may want to have a switch whether to use unlit material or PBR material...
    QgsTextureMaterial *mat = new QgsTextureMaterial;
    mat->setTexture( texture );
    return mat;
  }

  if ( qgsDoubleNear( pbr.baseColorFactor[3], 0 ) )
    return nullptr; // completely transparent primitive, just skip it

  QgsMetalRoughMaterial *pbrMaterial = new QgsMetalRoughMaterial;
  pbrMaterial->setMetalness( pbr.metallicFactor ); // [0..1] or texture
  pbrMaterial->setRoughness( pbr.roughnessFactor );
  pbrMaterial->setBaseColor( QColor::fromRgbF( pbr.baseColorFactor[0], pbr.baseColorFactor[1], pbr.baseColorFactor[2], pbr.baseColorFactor[3] ) );
  return pbrMaterial;
}


static QVector<Qt3DCore::QEntity *> parseNode(
  tinygltf::Model &model,
  int nodeIndex,
  const QgsGltf3DUtils::EntityTransform &transform,
  const QgsVector3D &tileTranslationEcef,
  QString baseUri,
  QMatrix4x4 parentTransform,
  const Qgs3DRenderContext &context,
  QStringList *errors
)
{
  QgsMaterialContext materialContext = QgsMaterialContext::fromRenderContext( context );
  tinygltf::Node &node = model.nodes[nodeIndex];

  QVector<Qt3DCore::QEntity *> entities;

  // transform
  std::unique_ptr<QMatrix4x4> matrix = QgsGltfUtils::parseNodeTransform( node );
  if ( !parentTransform.isIdentity() )
  {
    if ( matrix )
      *matrix = parentTransform * *matrix;
    else
    {
      matrix = std::make_unique<QMatrix4x4>( parentTransform );
    }
  }

  // mesh — skip nodes with EXT_mesh_gpu_instancing (handled separately by createInstancedEntities)
  if ( node.mesh >= 0 && node.extensions.find( "EXT_mesh_gpu_instancing" ) == node.extensions.end() )
  {
    tinygltf::Mesh &mesh = model.meshes[node.mesh];

    for ( const tinygltf::Primitive &primitive : mesh.primitives )
    {
      if ( primitive.mode != TINYGLTF_MODE_TRIANGLES )
      {
        if ( errors )
          *errors << u"Unsupported mesh primitive: %1"_s.arg( primitive.mode );
        continue;
      }

      auto posIt = primitive.attributes.find( "POSITION" );
      Q_ASSERT( posIt != primitive.attributes.end() );
      int positionAccessorIndex = posIt->second;

      tinygltf::Accessor &posAccessor = model.accessors[positionAccessorIndex];
      if ( posAccessor.componentType != TINYGLTF_PARAMETER_TYPE_FLOAT || posAccessor.type != TINYGLTF_TYPE_VEC3 )
      {
        if ( errors )
          *errors << u"Unsupported position accessor type: %1 / %2"_s.arg( posAccessor.componentType ).arg( posAccessor.type );
        continue;
      }

      QgsMaterial *material = parseMaterial( model, primitive.material, baseUri, errors, materialContext );
      if ( !material )
      {
        // primitive should be skipped, eg fully transparent material
        continue;
      }

      Qt3DCore::QGeometry *geom = new Qt3DCore::QGeometry;

      Qt3DCore::QAttribute *positionAttribute = reprojectPositions( model, positionAccessorIndex, transform, tileTranslationEcef, matrix.get() );
      positionAttribute->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );
      geom->addAttribute( positionAttribute );

      auto normalIt = primitive.attributes.find( "NORMAL" );
      if ( normalIt != primitive.attributes.end() )
      {
        int normalAccessorIndex = normalIt->second;
        Qt3DCore::QAttribute *normalAttribute = parseAttribute( model, normalAccessorIndex );
        normalAttribute->setName( Qt3DCore::QAttribute::defaultNormalAttributeName() );
        geom->addAttribute( normalAttribute );

        // TODO: we may need to transform normal vectors when we are altering positions
        // (but quite often normals are actually note needed - e.g. when using textured data)
      }

      auto texIt = primitive.attributes.find( "TEXCOORD_0" );
      if ( texIt != primitive.attributes.end() )
      {
        int texAccessorIndex = texIt->second;
        Qt3DCore::QAttribute *texAttribute = parseAttribute( model, texAccessorIndex );
        texAttribute->setName( Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName() );
        geom->addAttribute( texAttribute );
      }

      Qt3DCore::QAttribute *indexAttribute = nullptr;
      if ( primitive.indices != -1 )
      {
        indexAttribute = parseAttribute( model, primitive.indices );
        geom->addAttribute( indexAttribute );
      }

      Qt3DRender::QGeometryRenderer *geomRenderer = new Qt3DRender::QGeometryRenderer;
      geomRenderer->setGeometry( geom );
      geomRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Triangles ); // looks like same values as "mode"
      geomRenderer->setVertexCount( indexAttribute ? indexAttribute->count() : model.accessors[positionAccessorIndex].count );

      // if we are using PBR material, and normal vectors are not present in the data,
      // they should be auto-generated by us (according to GLTF spec)
      if ( normalIt == primitive.attributes.end() )
      {
        if ( QgsMetalRoughMaterial *pbrMat = qobject_cast<QgsMetalRoughMaterial *>( material ) )
        {
          pbrMat->setFlatShadingEnabled( true );
        }
      }

      Qt3DCore::QEntity *primitiveEntity = new Qt3DCore::QEntity;
      primitiveEntity->addComponent( geomRenderer );
      primitiveEntity->addComponent( material );
      entities << primitiveEntity;
    }
  }

  // recursively add children
  for ( int childNodeIndex : node.children )
  {
    entities << parseNode( model, childNodeIndex, transform, tileTranslationEcef, baseUri, matrix ? *matrix : QMatrix4x4(), context, errors );
  }

  return entities;
}


/**
 * Converts a float QMatrix4x4 to a double-precision QgsMatrix4x4.
 */
static QgsMatrix4x4 floatToDoubleMatrix( const QMatrix4x4 &fm )
{
  const float *f = fm.constData(); // column-major
  return QgsMatrix4x4( f[0], f[4], f[8], f[12], f[1], f[5], f[9], f[13], f[2], f[6], f[10], f[14], f[3], f[7], f[11], f[15] );
}


/**
 * Builds a QMatrix3x3 from three column vectors.
 */
static QMatrix3x3 matrixFromColumns( const QVector3D &col0, const QVector3D &col1, const QVector3D &col2 )
{
  // QMatrix3x3 constructor takes row-major data
  const float d[9] = {
    col0.x(),
    col1.x(),
    col2.x(),
    col0.y(),
    col1.y(),
    col2.y(),
    col0.z(),
    col1.z(),
    col2.z(),
  };
  return QMatrix3x3( d );
}


/**
 * Extracts the upper-left 3×3 rotation submatrix from a double-precision 4×4 matrix,
 * dividing each column by its length (removing scale). Returns the scale in \a scale.
 * Returns identity and zero scale if any column has zero length.
 */
static QMatrix3x3 extractRotation3x3( const QgsMatrix4x4 &matrix, QVector3D &scale )
{
  const double *md = matrix.constData(); // column-major
  const QVector3D col0( md[0], md[1], md[2] );
  const QVector3D col1( md[4], md[5], md[6] );
  const QVector3D col2( md[8], md[9], md[10] );

  const float sx = col0.length();
  const float sy = col1.length();
  const float sz = col2.length();
  scale = QVector3D( sx, sy, sz );

  if ( sx == 0 || sy == 0 || sz == 0 )
    return QMatrix3x3();

  return matrixFromColumns( col0 / sx, col1 / sy, col2 / sz );
}


QMatrix3x3 QgsGltf3DUtils::ecefToTargetCrsRotationCorrection( const QgsVector3D &ecefPos, const QgsVector3D &mapPos, const QgsCoordinateTransform &ecefToTargetCrs )
{
  // Local ECEF basis vectors at this point:
  //   up    = normalize(x, y, z)  — geocentric up (≤0.3° error vs ellipsoid normal)
  //   east  = normalize(-y, x, 0) — tangent to the parallel
  //   north = cross(up, east)
  const double x = ecefPos.x(), y = ecefPos.y(), z = ecefPos.z();
  const double len = std::sqrt( x * x + y * y + z * z );
  const QVector3D upEcef( x / len, y / len, z / len );

  const double eastLenXY = std::sqrt( y * y + x * x );
  const QVector3D eastEcef( -y / eastLenXY, x / eastLenXY, 0 );

  const QVector3D northEcef = QVector3D::crossProduct( upEcef, eastEcef );

  // Compute corresponding vectors in target CRS by reprojecting perturbed ECEF points.
  // Use a small delta (~1 meter) along each ECEF basis vector.
  constexpr double delta = 1.0; // meters
  double eX = x + delta * eastEcef.x(), eY = y + delta * eastEcef.y(), eZ = z + delta * eastEcef.z();
  double nX = x + delta * northEcef.x(), nY = y + delta * northEcef.y(), nZ = z + delta * northEcef.z();
  double uX = x + delta * upEcef.x(), uY = y + delta * upEcef.y(), uZ = z + delta * upEcef.z();

  try
  {
    ecefToTargetCrs.transformInPlace( eX, eY, eZ );
    ecefToTargetCrs.transformInPlace( nX, nY, nZ );
    ecefToTargetCrs.transformInPlace( uX, uY, uZ );
  }
  catch ( QgsCsException & )
  {
    return QMatrix3x3(); // identity on failure
  }

  // Target CRS basis vectors (differences from the reprojected base point, then normalized)
  QVector3D eastCrs( eX - mapPos.x(), eY - mapPos.y(), eZ - mapPos.z() );
  QVector3D northCrs( nX - mapPos.x(), nY - mapPos.y(), nZ - mapPos.z() );
  QVector3D upCrs( uX - mapPos.x(), uY - mapPos.y(), uZ - mapPos.z() );
  eastCrs.normalize();
  northCrs.normalize();
  upCrs.normalize();

  // Correction matrix C = T × Eᵀ
  // where E = [east_ecef | north_ecef | up_ecef] (columns, orthonormal)
  //       T = [east_crs  | north_crs  | up_crs]  (columns)
  // Since E is orthonormal, E⁻¹ = Eᵀ, so C = T × Eᵀ.
  const QMatrix3x3 ecefBasis = matrixFromColumns( eastEcef, northEcef, upEcef ); // E
  const QMatrix3x3 crsBasis = matrixFromColumns( eastCrs, northCrs, upCrs );     // T
  return crsBasis * ecefBasis.transposed();
}


QVector<QgsGltf3DUtils::InstanceChunkTransform> QgsGltf3DUtils::tileSpaceToChunkLocal( const QgsGltfUtils::InstancedPrimitive &primitive, const QgsGltf3DUtils::EntityTransform &transform )
{
  QVector<InstanceChunkTransform> result;
  result.resize( primitive.instanceTransforms.size() );

  if ( primitive.instanceTransforms.isEmpty() )
    return result;

  const QgsVector3D sceneOrigin = transform.chunkOriginTargetCrs;

  // ECEF-to-target-CRS rotation correction matrix, computed once from first instance.
  // All instances in a tile are close enough that one correction suffices.
  QMatrix3x3 correctionMatrix;
  bool hasCorrectionMatrix = false;

  for ( int i = 0; i < primitive.instanceTransforms.size(); ++i )
  {
    // Compose with tile transform in double precision:
    // ecefMatrix = tileTransform × instanceTransforms[i]
    const QgsMatrix4x4 instanceDouble = floatToDoubleMatrix( primitive.instanceTransforms[i] );
    const QgsMatrix4x4 ecefMatrix = transform.tileTransform * instanceDouble;

    // Extract ECEF position from column 3 (double precision)
    const double *md = ecefMatrix.constData(); // column-major
    const QgsVector3D ecefPos( md[12], md[13], md[14] );

    // Reproject ECEF → target CRS
    double mapX = ecefPos.x();
    double mapY = ecefPos.y();
    double mapZ = ecefPos.z();
    if ( transform.ecefToTargetCrs )
    {
      try
      {
        transform.ecefToTargetCrs->transformInPlace( mapX, mapY, mapZ );
      }
      catch ( QgsCsException & )
      {
        continue;
      }
    }

    // Compute the correction matrix on the first successfully reprojected instance
    if ( !hasCorrectionMatrix && transform.ecefToTargetCrs )
    {
      hasCorrectionMatrix = true;
      correctionMatrix = ecefToTargetCrsRotationCorrection( ecefPos, QgsVector3D( mapX, mapY, mapZ ), *transform.ecefToTargetCrs );
    }

    // Apply z value modifications
    mapZ = mapZ * transform.zValueScale + transform.zValueOffset;

    // Chunk-local translation
    result[i].translation = QVector3D( static_cast<float>( mapX - sceneOrigin.x() ), static_cast<float>( mapY - sceneOrigin.y() ), static_cast<float>( mapZ - sceneOrigin.z() ) );

    // Extract rotation and scale from the 3×3 part of ecefMatrix (double precision)
    QVector3D scale;
    QMatrix3x3 rotEcef = extractRotation3x3( ecefMatrix, scale );

    result[i].scale = scale;

    if ( scale.x() == 0 || scale.y() == 0 || scale.z() == 0 )
    {
      result[i].rotation = QQuaternion();
      continue;
    }

    // Apply ECEF-to-CRS rotation correction if available
    const QMatrix3x3 rotCrs = hasCorrectionMatrix ? correctionMatrix * rotEcef : rotEcef;
    result[i].rotation = QQuaternion::fromRotationMatrix( rotCrs );
  }

  return result;
}


void QgsGltf3DUtils::createInstanceBuffer( Qt3DCore::QGeometry *geometry, const QVector<InstanceChunkTransform> &instances )
{
  const int stride = 10 * sizeof( float ); // vec3 + vec4 + vec3 = 10 floats = 40 bytes
  QByteArray bufferData;
  bufferData.resize( instances.size() * stride );
  float *dst = reinterpret_cast<float *>( bufferData.data() );

  for ( const auto &inst : instances )
  {
    // translation (vec3)
    *dst++ = inst.translation.x();
    *dst++ = inst.translation.y();
    *dst++ = inst.translation.z();
    // rotation (vec4: x, y, z, w)
    *dst++ = inst.rotation.x();
    *dst++ = inst.rotation.y();
    *dst++ = inst.rotation.z();
    *dst++ = inst.rotation.scalar();
    // scale (vec3)
    *dst++ = inst.scale.x();
    *dst++ = inst.scale.y();
    *dst++ = inst.scale.z();
  }

  Qt3DCore::QBuffer *buffer = new Qt3DCore::QBuffer;
  buffer->setData( bufferData );

  // Translation attribute — matches "in vec3 instanceTranslation" in shader
  Qt3DCore::QAttribute *transAttr = new Qt3DCore::QAttribute;
  transAttr->setName( u"instanceTranslation"_s );
  transAttr->setVertexBaseType( Qt3DCore::QAttribute::Float );
  transAttr->setVertexSize( 3 );
  transAttr->setByteStride( stride );
  transAttr->setByteOffset( 0 );
  transAttr->setDivisor( 1 );
  transAttr->setCount( instances.size() );
  transAttr->setBuffer( buffer );
  geometry->addAttribute( transAttr );

  // Rotation attribute — matches "in vec4 instanceRotation" in shader
  Qt3DCore::QAttribute *rotAttr = new Qt3DCore::QAttribute;
  rotAttr->setName( u"instanceRotation"_s );
  rotAttr->setVertexBaseType( Qt3DCore::QAttribute::Float );
  rotAttr->setVertexSize( 4 );
  rotAttr->setByteStride( stride );
  rotAttr->setByteOffset( 3 * sizeof( float ) );
  rotAttr->setDivisor( 1 );
  rotAttr->setCount( instances.size() );
  rotAttr->setBuffer( buffer );
  geometry->addAttribute( rotAttr );

  // Scale attribute — matches "in vec3 instanceScale" in shader
  Qt3DCore::QAttribute *scaleAttr = new Qt3DCore::QAttribute;
  scaleAttr->setName( u"instanceScale"_s );
  scaleAttr->setVertexBaseType( Qt3DCore::QAttribute::Float );
  scaleAttr->setVertexSize( 3 );
  scaleAttr->setByteStride( stride );
  scaleAttr->setByteOffset( 7 * sizeof( float ) );
  scaleAttr->setDivisor( 1 );
  scaleAttr->setCount( instances.size() );
  scaleAttr->setBuffer( buffer );
  geometry->addAttribute( scaleAttr );
}


static Qt3DCore::QAttribute *rawPositions( tinygltf::Model &model, int accessorIndex )
{
  tinygltf::Accessor &accessor = model.accessors[accessorIndex];
  tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
  tinygltf::Buffer &b = model.buffers[bv.buffer];

  if ( accessor.componentType != TINYGLTF_PARAMETER_TYPE_FLOAT || accessor.type != TINYGLTF_TYPE_VEC3 )
    return nullptr;

  const unsigned char *ptr = b.data.data() + bv.byteOffset + accessor.byteOffset;
  const int byteStride = bv.byteStride ? bv.byteStride : 3 * sizeof( float );

  QByteArray byteArray;
  byteArray.resize( accessor.count * 3 * sizeof( float ) );
  float *out = reinterpret_cast<float *>( byteArray.data() );

  for ( std::size_t i = 0; i < accessor.count; ++i )
  {
    const float *fptr = reinterpret_cast<const float *>( ptr + i * byteStride );
    out[i * 3 + 0] = fptr[0];
    out[i * 3 + 1] = fptr[1];
    out[i * 3 + 2] = fptr[2];
  }

  Qt3DCore::QBuffer *buffer = new Qt3DCore::QBuffer;
  buffer->setData( byteArray );

  Qt3DCore::QAttribute *attribute = new Qt3DCore::QAttribute;
  attribute->setAttributeType( Qt3DCore::QAttribute::VertexAttribute );
  attribute->setBuffer( buffer );
  attribute->setByteOffset( 0 );
  attribute->setByteStride( 12 );
  attribute->setCount( accessor.count );
  attribute->setVertexBaseType( Qt3DCore::QAttribute::Float );
  attribute->setVertexSize( 3 );

  return attribute;
}


QVector<Qt3DCore::QEntity *> QgsGltf3DUtils::createInstancedEntities(
  tinygltf::Model &model, const QVector<QgsGltfUtils::InstancedPrimitive> &primitives, const QgsGltf3DUtils::EntityTransform &transform, const QString &baseUri, QStringList *errors
)
{
  QVector<Qt3DCore::QEntity *> entities;

  for ( const QgsGltfUtils::InstancedPrimitive &entry : primitives )
  {
    if ( entry.meshIndex < 0 || entry.meshIndex >= static_cast<int>( model.meshes.size() ) )
      continue;

    const tinygltf::Mesh &mesh = model.meshes[entry.meshIndex];
    if ( entry.primitiveIndex < 0 || entry.primitiveIndex >= static_cast<int>( mesh.primitives.size() ) )
      continue;

    const tinygltf::Primitive &primitive = mesh.primitives[entry.primitiveIndex];
    if ( primitive.mode != TINYGLTF_MODE_TRIANGLES )
    {
      if ( errors )
        *errors << u"Unsupported mesh primitive mode for instancing: %1"_s.arg( primitive.mode );
      continue;
    }

    auto posIt = primitive.attributes.find( "POSITION" );
    if ( posIt == primitive.attributes.end() )
      continue;

    int positionAccessorIndex = posIt->second;

    // Parse material
    QgsMaterial *material = parseMaterial( model, entry.materialIndex, baseUri, errors );
    if ( !material )
      continue;

    // Enable instancing on the material
    if ( QgsMetalRoughMaterial *pbrMat = qobject_cast<QgsMetalRoughMaterial *>( material ) )
      pbrMat->setInstancingEnabled( true );
    else if ( QgsTextureMaterial *texMat = qobject_cast<QgsTextureMaterial *>( material ) )
      texMat->setInstancingEnabled( true );

    // Build geometry with raw positions (no transform, no axis flip)
    Qt3DCore::QGeometry *geom = new Qt3DCore::QGeometry;

    Qt3DCore::QAttribute *positionAttribute = rawPositions( model, positionAccessorIndex );
    if ( !positionAttribute )
    {
      delete geom;
      delete material;
      continue;
    }
    positionAttribute->setName( Qt3DCore::QAttribute::defaultPositionAttributeName() );
    geom->addAttribute( positionAttribute );

    auto normalIt = primitive.attributes.find( "NORMAL" );
    if ( normalIt != primitive.attributes.end() )
    {
      Qt3DCore::QAttribute *normalAttribute = parseAttribute( model, normalIt->second );
      normalAttribute->setName( Qt3DCore::QAttribute::defaultNormalAttributeName() );
      geom->addAttribute( normalAttribute );
    }
    else
    {
      // Enable flat shading if no normals
      if ( QgsMetalRoughMaterial *pbrMat = qobject_cast<QgsMetalRoughMaterial *>( material ) )
        pbrMat->setFlatShadingEnabled( true );
    }

    auto texIt = primitive.attributes.find( "TEXCOORD_0" );
    if ( texIt != primitive.attributes.end() )
    {
      Qt3DCore::QAttribute *texAttribute = parseAttribute( model, texIt->second );
      texAttribute->setName( Qt3DCore::QAttribute::defaultTextureCoordinateAttributeName() );
      geom->addAttribute( texAttribute );
    }

    Qt3DCore::QAttribute *indexAttribute = nullptr;
    if ( primitive.indices != -1 )
    {
      indexAttribute = parseAttribute( model, primitive.indices );
      geom->addAttribute( indexAttribute );
    }

    // Convert tile-space matrices to chunk-local T/R/S
    const QVector<InstanceChunkTransform> chunkTransforms = tileSpaceToChunkLocal( entry, transform );
    if ( chunkTransforms.isEmpty() )
    {
      delete geom;
      delete material;
      continue;
    }

    // Add per-instance attributes
    createInstanceBuffer( geom, chunkTransforms );

    // Create geometry renderer with instancing
    Qt3DRender::QGeometryRenderer *geomRenderer = new Qt3DRender::QGeometryRenderer;
    geomRenderer->setGeometry( geom );
    geomRenderer->setPrimitiveType( Qt3DRender::QGeometryRenderer::Triangles );
    geomRenderer->setVertexCount( indexAttribute ? indexAttribute->count() : model.accessors[positionAccessorIndex].count );
    geomRenderer->setInstanceCount( chunkTransforms.size() );

    Qt3DCore::QEntity *entity = new Qt3DCore::QEntity;
    entity->addComponent( geomRenderer );
    entity->addComponent( material );
    entities << entity;
  }

  return entities;
}

Qt3DCore::QEntity *QgsGltf3DUtils::parsedGltfToEntity( tinygltf::Model &model, const QgsGltf3DUtils::EntityTransform &transform, QString baseUri, const Qgs3DRenderContext &context, QStringList *errors )
{
  bool sceneOk = false;
  const std::size_t sceneIndex = QgsGltfUtils::sourceSceneForModel( model, sceneOk );
  if ( !sceneOk )
  {
    if ( errors )
      *errors << "No scenes present in the gltf data!";
    return nullptr;
  }

  tinygltf::Scene &scene = model.scenes[sceneIndex];

  if ( scene.nodes.size() == 0 )
  {
    if ( errors )
      *errors << "No nodes present in the gltf data!";
    return nullptr;
  }

  const QgsVector3D tileTranslationEcef = QgsGltfUtils::extractTileTranslation( model );

  Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity;
  for ( const int nodeIndex : scene.nodes )
  {
    const QVector<Qt3DCore::QEntity *> entities = parseNode( model, nodeIndex, transform, tileTranslationEcef, baseUri, QMatrix4x4(), context, errors );
    for ( Qt3DCore::QEntity *e : entities )
      e->setParent( rootEntity );
  }
  return rootEntity;
}


Qt3DCore::QEntity *QgsGltf3DUtils::gltfToEntity( const QByteArray &data, const QgsGltf3DUtils::EntityTransform &transform, const QString &baseUri, const Qgs3DRenderContext &context, QStringList *errors )
{
  tinygltf::Model model;
  QString gltfErrors, gltfWarnings;

  bool res = QgsGltfUtils::loadGltfModel( data, model, &gltfErrors, &gltfWarnings );
  if ( !gltfErrors.isEmpty() )
  {
    QgsDebugError( u"Error raised reading %1: %2"_s.arg( baseUri, gltfErrors ) );
  }
  if ( !gltfWarnings.isEmpty() )
  {
    QgsDebugError( u"Warnings raised reading %1: %2"_s.arg( baseUri, gltfWarnings ) );
  }
  if ( !res )
  {
    if ( errors )
    {
      errors->append( u"GLTF load error: "_s + gltfErrors );
    }
    return nullptr;
  }

  return parsedGltfToEntity( model, transform, baseUri, context, errors );
}

// For TinyGltfTextureImage
#include "qgsgltf3dutils.moc"

///@endcond
