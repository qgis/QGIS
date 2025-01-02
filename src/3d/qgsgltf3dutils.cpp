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

#include "qgsgltfutils.h"
#include "qgsblockingnetworkrequest.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmetalroughmaterial.h"
#include "qgstexturematerial.h"

#include <Qt3DCore/QEntity>

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
#include <Qt3DRender/QAttribute>
#include <Qt3DRender/QBuffer>
#include <Qt3DRender/QGeometry>
typedef Qt3DRender::QAttribute Qt3DQAttribute;
typedef Qt3DRender::QBuffer Qt3DQBuffer;
typedef Qt3DRender::QGeometry Qt3DQGeometry;
#else
#include <Qt3DCore/QAttribute>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
typedef Qt3DCore::QAttribute Qt3DQAttribute;
typedef Qt3DCore::QBuffer Qt3DQBuffer;
typedef Qt3DCore::QGeometry Qt3DQGeometry;
#endif

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QTexture>

#include <QFile>
#include <QFileInfo>
#include <QMatrix4x4>

///@cond PRIVATE

static Qt3DQAttribute::VertexBaseType parseVertexBaseType( int componentType )
{
  switch ( componentType )
  {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
      return Qt3DQAttribute::Byte;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
      return Qt3DQAttribute::UnsignedByte;
    case TINYGLTF_COMPONENT_TYPE_SHORT:
      return Qt3DQAttribute::Short;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
      return Qt3DQAttribute::UnsignedShort;
    case TINYGLTF_COMPONENT_TYPE_INT:
      return Qt3DQAttribute::Int;
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
      return Qt3DQAttribute::UnsignedInt;
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
      return Qt3DQAttribute::Float;
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
      return Qt3DQAttribute::Double;
  }
  Q_ASSERT( false );
  return Qt3DQAttribute::UnsignedInt;
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


static Qt3DQAttribute *parseAttribute( tinygltf::Model &model, int accessorIndex )
{
  tinygltf::Accessor &accessor = model.accessors[accessorIndex];
  tinygltf::BufferView &bv = model.bufferViews[accessor.bufferView];
  tinygltf::Buffer &b = model.buffers[bv.buffer];

  // TODO: only ever create one QBuffer for a buffer even if it is used multiple times
  QByteArray byteArray( reinterpret_cast<const char *>( b.data.data() ),
                        static_cast<int>( b.data.size() ) ); // makes a deep copy
  Qt3DQBuffer *buffer = new Qt3DQBuffer();
  buffer->setData( byteArray );

  Qt3DQAttribute *attribute = new Qt3DQAttribute();

  // "target" is optional, can be zero
  if ( bv.target == TINYGLTF_TARGET_ARRAY_BUFFER )
    attribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  else if ( bv.target == TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER )
    attribute->setAttributeType( Qt3DQAttribute::IndexAttribute );

  attribute->setBuffer( buffer );
  attribute->setByteOffset( bv.byteOffset + accessor.byteOffset );
  attribute->setByteStride( bv.byteStride ); // could be zero, it seems that's fine (assuming packed)
  attribute->setCount( accessor.count );
  attribute->setVertexBaseType( parseVertexBaseType( accessor.componentType ) );
  attribute->setVertexSize( tinygltf::GetNumComponentsInType( accessor.type ) );

  return attribute;
}


static Qt3DQAttribute *reprojectPositions( tinygltf::Model &model, int accessorIndex, const QgsGltf3DUtils::EntityTransform &transform, const QgsVector3D &tileTranslationEcef, QMatrix4x4 *matrix )
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

  Qt3DQBuffer *buffer = new Qt3DQBuffer();
  buffer->setData( byteArray );

  Qt3DQAttribute *attribute = new Qt3DQAttribute();
  attribute->setAttributeType( Qt3DQAttribute::VertexAttribute );
  attribute->setBuffer( buffer );
  attribute->setByteOffset( 0 );
  attribute->setByteStride( 12 );
  attribute->setCount( accessor.count );
  attribute->setVertexBaseType( Qt3DQAttribute::Float );
  attribute->setVertexSize( 3 );

  return attribute;
}


// QAbstractFunctor marked as deprecated in 5.15, but undeprecated for Qt 6.0. TODO -- remove when we require 6.0
Q_NOWARN_DEPRECATED_PUSH

class TinyGltfTextureImageDataGenerator : public Qt3DRender::QTextureImageDataGenerator
{
  public:
    TinyGltfTextureImageDataGenerator( Qt3DRender::QTextureImageDataPtr imagePtr )
      : mImagePtr( imagePtr ) {}

    QT3D_FUNCTOR( TinyGltfTextureImageDataGenerator )

    Qt3DRender::QTextureImageDataPtr operator()() override
    {
      return mImagePtr;
    }

    bool operator==( const QTextureImageDataGenerator &other ) const override
    {
      const TinyGltfTextureImageDataGenerator *otherFunctor = functor_cast<TinyGltfTextureImageDataGenerator>( &other );
      return mImagePtr.get() == otherFunctor->mImagePtr.get();
    }

    Qt3DRender::QTextureImageDataPtr mImagePtr;
};

Q_NOWARN_DEPRECATED_POP

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

    Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override
    {
      return Qt3DRender::QTextureImageDataGeneratorPtr( new TinyGltfTextureImageDataGenerator( imgDataPtr ) );
    }

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
        *errors << QStringLiteral( "Failed to download image: %1" ).arg( url.toString() );
    }
    else
    {
      const QgsNetworkReplyContent content = networkRequest.reply();
      return content.content();
    }
  }
  else if ( url.isLocalFile() && QFile::exists( url.toLocalFile() ) )
  {
    QFile f( url.toLocalFile() );
    if ( f.open( QIODevice::ReadOnly ) )
    {
      return f.readAll();
    }
    else
    {
      if ( errors )
        *errors << QStringLiteral( "Unable to open image: %1" ).arg( url.toString() );
    }
  }
  return QByteArray();
}

// Returns NULLPTR if primitive should not be rendered
static QgsMaterial *parseMaterial( tinygltf::Model &model, int materialIndex, QString baseUri, QStringList *errors )
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
            *errors << QStringLiteral( "Failed to load image: %1" ).arg( imgUri );
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

    // let's use linear (rather than nearest) filtering by default to avoid blocky look of textures
    texture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );
    texture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );

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


static QVector<Qt3DCore::QEntity *> parseNode( tinygltf::Model &model, int nodeIndex, const QgsGltf3DUtils::EntityTransform &transform, const QgsVector3D &tileTranslationEcef, QString baseUri, QMatrix4x4 parentTransform, QStringList *errors )
{
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
      matrix.reset( new QMatrix4x4( parentTransform ) );
    }
  }

  // mesh
  if ( node.mesh >= 0 )
  {
    tinygltf::Mesh &mesh = model.meshes[node.mesh];

    for ( const tinygltf::Primitive &primitive : mesh.primitives )
    {
      if ( primitive.mode != TINYGLTF_MODE_TRIANGLES )
      {
        if ( errors )
          *errors << QStringLiteral( "Unsupported mesh primitive: %1" ).arg( primitive.mode );
        continue;
      }

      auto posIt = primitive.attributes.find( "POSITION" );
      Q_ASSERT( posIt != primitive.attributes.end() );
      int positionAccessorIndex = posIt->second;

      tinygltf::Accessor &posAccessor = model.accessors[positionAccessorIndex];
      if ( posAccessor.componentType != TINYGLTF_PARAMETER_TYPE_FLOAT || posAccessor.type != TINYGLTF_TYPE_VEC3 )
      {
        if ( errors )
          *errors << QStringLiteral( "Unsupported position accessor type: %1 / %2" ).arg( posAccessor.componentType ).arg( posAccessor.type );
        continue;
      }

      QgsMaterial *material = parseMaterial( model, primitive.material, baseUri, errors );
      if ( !material )
      {
        // primitive should be skipped, eg fully transparent material
        continue;
      }

      Qt3DQGeometry *geom = new Qt3DQGeometry;

      Qt3DQAttribute *positionAttribute = reprojectPositions( model, positionAccessorIndex, transform, tileTranslationEcef, matrix.get() );
      positionAttribute->setName( Qt3DQAttribute::defaultPositionAttributeName() );
      geom->addAttribute( positionAttribute );

      auto normalIt = primitive.attributes.find( "NORMAL" );
      if ( normalIt != primitive.attributes.end() )
      {
        int normalAccessorIndex = normalIt->second;
        Qt3DQAttribute *normalAttribute = parseAttribute( model, normalAccessorIndex );
        normalAttribute->setName( Qt3DQAttribute::defaultNormalAttributeName() );
        geom->addAttribute( normalAttribute );

        // TODO: we may need to transform normal vectors when we are altering positions
        // (but quite often normals are actually note needed - e.g. when using textured data)
      }

      auto texIt = primitive.attributes.find( "TEXCOORD_0" );
      if ( texIt != primitive.attributes.end() )
      {
        int texAccessorIndex = texIt->second;
        Qt3DQAttribute *texAttribute = parseAttribute( model, texAccessorIndex );
        texAttribute->setName( Qt3DQAttribute::defaultTextureCoordinateAttributeName() );
        geom->addAttribute( texAttribute );
      }

      Qt3DQAttribute *indexAttribute = nullptr;
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
    entities << parseNode( model, childNodeIndex, transform, tileTranslationEcef, baseUri, matrix ? *matrix : QMatrix4x4(), errors );
  }

  return entities;
}


Qt3DCore::QEntity *QgsGltf3DUtils::parsedGltfToEntity( tinygltf::Model &model, const QgsGltf3DUtils::EntityTransform &transform, QString baseUri, QStringList *errors )
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
    const QVector<Qt3DCore::QEntity *> entities = parseNode( model, nodeIndex, transform, tileTranslationEcef, baseUri, QMatrix4x4(), errors );
    for ( Qt3DCore::QEntity *e : entities )
      e->setParent( rootEntity );
  }
  return rootEntity;
}


Qt3DCore::QEntity *QgsGltf3DUtils::gltfToEntity( const QByteArray &data, const QgsGltf3DUtils::EntityTransform &transform, const QString &baseUri, QStringList *errors )
{
  tinygltf::Model model;
  QString gltfErrors, gltfWarnings;

  bool res = QgsGltfUtils::loadGltfModel( data, model, &gltfErrors, &gltfWarnings );
  if ( !gltfErrors.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Error raised reading %1: %2" ).arg( baseUri, gltfErrors ) );
  }
  if ( !gltfWarnings.isEmpty() )
  {
    QgsDebugError( QStringLiteral( "Warnings raised reading %1: %2" ).arg( baseUri, gltfWarnings ) );
  }
  if ( !res )
  {
    if ( errors )
    {
      errors->append( QStringLiteral( "GLTF load error: " ) + gltfErrors );
    }
    return nullptr;
  }

  return parsedGltfToEntity( model, transform, baseUri, errors );
}

// For TinyGltfTextureImage
#include "qgsgltf3dutils.moc"

///@endcond
