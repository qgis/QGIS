/***************************************************************************
                         qgsalgorithmgltftovector.cpp
                         ---------------------
    begin                : August 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmgltftovector.h"
#include "qgsgltfutils.h"
#include "qgsmatrix4x4.h"
#include "qgsvector3d.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"

#include <QMatrix4x4>

#define TINYGLTF_NO_STB_IMAGE       // we use QImage-based reading of images
#define TINYGLTF_NO_STB_IMAGE_WRITE // we don't need writing of images

#include "tiny_gltf.h"

///@cond PRIVATE

QString QgsGltfToVectorFeaturesAlgorithm::name() const
{
  return QStringLiteral( "gltftovector" );
}

QString QgsGltfToVectorFeaturesAlgorithm::displayName() const
{
  return QObject::tr( "Convert GLTF to vector features" );
}

QStringList QgsGltfToVectorFeaturesAlgorithm::tags() const
{
  return QObject::tr( "3d,tiles,cesium" ).split( ',' );
}

QString QgsGltfToVectorFeaturesAlgorithm::group() const
{
  return QObject::tr( "3D Tiles" );
}

QString QgsGltfToVectorFeaturesAlgorithm::groupId() const
{
  return QStringLiteral( "3dtiles" );
}

QString QgsGltfToVectorFeaturesAlgorithm::shortHelpString() const
{
  return QObject::tr( "Converts GLTF content to standard vector layer formats." );
}

QgsGltfToVectorFeaturesAlgorithm *QgsGltfToVectorFeaturesAlgorithm::createInstance() const
{
  return new QgsGltfToVectorFeaturesAlgorithm();
}

void QgsGltfToVectorFeaturesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "INPUT" ), QObject::tr( "Input GLTF" ), Qgis::ProcessingFileParameterBehavior::File, QStringLiteral( "gltf" ), QVariant(), false, QStringLiteral( "GLTF (*.gltf *.GLTF);;GLB (*.glb *.GLB)" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_POLYGONS" ), QObject::tr( "Output polygons" ), Qgis::ProcessingSourceType::VectorPolygon, QVariant(), true, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_LINES" ), QObject::tr( "Output lines" ), Qgis::ProcessingSourceType::VectorLine, QVariant(), true, true ) );
}

std::unique_ptr<QgsAbstractGeometry> extractTriangles(
  const tinygltf::Model &model,
  const tinygltf::Primitive &primitive,
  const QgsCoordinateTransform &ecefTransform,
  const QgsVector3D &tileTranslationEcef,
  const QMatrix4x4 *gltfLocalTransform,
  QgsProcessingFeedback *feedback
)
{
  auto posIt = primitive.attributes.find( "POSITION" );
  if ( posIt == primitive.attributes.end() )
  {
    feedback->reportError( QObject::tr( "Could not find POSITION attribute for primitive" ) );
    return nullptr;
  }
  int positionAccessorIndex = posIt->second;

  QVector<double> x;
  QVector<double> y;
  QVector<double> z;
  QgsGltfUtils::accessorToMapCoordinates(
    model, positionAccessorIndex, QgsMatrix4x4(),
    &ecefTransform,
    tileTranslationEcef,
    gltfLocalTransform,
    Qgis::Axis::Y,
    x, y, z
  );

  std::unique_ptr<QgsMultiPolygon> mp = std::make_unique<QgsMultiPolygon>();

  if ( primitive.indices == -1 )
  {
    Q_ASSERT( x.size() % 3 == 0 );

    mp->reserve( x.size() );
    for ( int i = 0; i < x.size(); i += 3 )
    {
      mp->addGeometry( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( x[i], y[i], z[i] ), QgsPoint( x[i + 1], y[i + 1], z[i + 1] ), QgsPoint( x[i + 2], y[i + 2], z[i + 2] ), QgsPoint( x[i], y[i], z[i] ) } ) ) );
    }
  }
  else
  {
    const tinygltf::Accessor &primitiveAccessor = model.accessors[primitive.indices];
    const tinygltf::BufferView &bvPrimitive = model.bufferViews[primitiveAccessor.bufferView];
    const tinygltf::Buffer &bPrimitive = model.buffers[bvPrimitive.buffer];

    Q_ASSERT( ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT || primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE ) && primitiveAccessor.type == TINYGLTF_TYPE_SCALAR );

    const char *primitivePtr = reinterpret_cast<const char *>( bPrimitive.data.data() ) + bvPrimitive.byteOffset + primitiveAccessor.byteOffset;

    mp->reserve( primitiveAccessor.count / 3 );
    for ( std::size_t i = 0; i < primitiveAccessor.count / 3; i++ )
    {
      unsigned int index1 = 0;
      unsigned int index2 = 0;
      unsigned int index3 = 0;

      if ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT )
      {
        const unsigned short *usPtrPrimitive = reinterpret_cast<const unsigned short *>( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 3 * sizeof( unsigned short );

        index1 = usPtrPrimitive[0];
        index2 = usPtrPrimitive[1];
        index3 = usPtrPrimitive[2];
      }
      else if ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE )
      {
        const unsigned char *usPtrPrimitive = reinterpret_cast<const unsigned char *>( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 3 * sizeof( unsigned char );

        index1 = usPtrPrimitive[0];
        index2 = usPtrPrimitive[1];
        index3 = usPtrPrimitive[2];
      }
      else
      {
        const unsigned int *uintPtrPrimitive = reinterpret_cast<const unsigned int *>( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 3 * sizeof( unsigned int );

        index1 = uintPtrPrimitive[0];
        index2 = uintPtrPrimitive[1];
        index3 = uintPtrPrimitive[2];
      }

      mp->addGeometry( new QgsPolygon( new QgsLineString( QVector<QgsPoint> { QgsPoint( x[index1], y[index1], z[index1] ), QgsPoint( x[index2], y[index2], z[index2] ), QgsPoint( x[index3], y[index3], z[index3] ), QgsPoint( x[index1], y[index1], z[index1] ) } ) ) );
    }
  }
  return mp;
}

std::unique_ptr<QgsAbstractGeometry> extractLines(
  const tinygltf::Model &model,
  const tinygltf::Primitive &primitive,
  const QgsCoordinateTransform &ecefTransform,
  const QgsVector3D &tileTranslationEcef,
  const QMatrix4x4 *gltfLocalTransform,
  QgsProcessingFeedback *feedback
)
{
  auto posIt = primitive.attributes.find( "POSITION" );
  if ( posIt == primitive.attributes.end() )
  {
    feedback->reportError( QObject::tr( "Could not find POSITION attribute for primitive" ) );
    return nullptr;
  }
  int positionAccessorIndex = posIt->second;

  QVector<double> x;
  QVector<double> y;
  QVector<double> z;
  QgsGltfUtils::accessorToMapCoordinates(
    model, positionAccessorIndex, QgsMatrix4x4(),
    &ecefTransform,
    tileTranslationEcef,
    gltfLocalTransform,
    Qgis::Axis::Y,
    x, y, z
  );

  std::unique_ptr<QgsMultiLineString> ml = std::make_unique<QgsMultiLineString>();

  if ( primitive.indices == -1 )
  {
    Q_ASSERT( x.size() % 2 == 0 );

    ml->reserve( x.size() );
    for ( int i = 0; i < x.size(); i += 2 )
    {
      ml->addGeometry( new QgsLineString( QVector<QgsPoint> { QgsPoint( x[i], y[i], z[i] ), QgsPoint( x[i + 1], y[i + 1], z[i + 1] ) } ) );
    }
  }
  else
  {
    const tinygltf::Accessor &primitiveAccessor = model.accessors[primitive.indices];
    const tinygltf::BufferView &bvPrimitive = model.bufferViews[primitiveAccessor.bufferView];
    const tinygltf::Buffer &bPrimitive = model.buffers[bvPrimitive.buffer];

    Q_ASSERT( ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT || primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT || primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE ) && primitiveAccessor.type == TINYGLTF_TYPE_SCALAR );

    const char *primitivePtr = reinterpret_cast<const char *>( bPrimitive.data.data() ) + bvPrimitive.byteOffset + primitiveAccessor.byteOffset;

    ml->reserve( primitiveAccessor.count / 2 );
    for ( std::size_t i = 0; i < primitiveAccessor.count / 2; i++ )
    {
      unsigned int index1 = 0;
      unsigned int index2 = 0;

      if ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT )
      {
        const unsigned short *usPtrPrimitive = reinterpret_cast<const unsigned short *>( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 2 * sizeof( unsigned short );

        index1 = usPtrPrimitive[0];
        index2 = usPtrPrimitive[1];
      }
      else if ( primitiveAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE )
      {
        const unsigned char *usPtrPrimitive = reinterpret_cast<const unsigned char *>( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 2 * sizeof( unsigned char );

        index1 = usPtrPrimitive[0];
        index2 = usPtrPrimitive[1];
      }
      else
      {
        const unsigned int *uintPtrPrimitive = reinterpret_cast<const unsigned int *>( primitivePtr );
        if ( bvPrimitive.byteStride )
          primitivePtr += bvPrimitive.byteStride;
        else
          primitivePtr += 2 * sizeof( unsigned int );

        index1 = uintPtrPrimitive[0];
        index2 = uintPtrPrimitive[1];
      }

      ml->addGeometry( new QgsLineString( QVector<QgsPoint> { QgsPoint( x[index1], y[index1], z[index1] ), QgsPoint( x[index2], y[index2], z[index2] ) } ) );
    }
  }
  return ml;
}

QVariantMap QgsGltfToVectorFeaturesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString path = parameterAsFile( parameters, QStringLiteral( "INPUT" ), context );

  const QgsCoordinateReferenceSystem destCrs( QStringLiteral( "EPSG:4326" ) );
  QgsFields fields;

  QString polygonDest;
  std::unique_ptr<QgsFeatureSink> polygonSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_POLYGONS" ), context, polygonDest, fields, Qgis::WkbType::MultiPolygonZ, destCrs ) );
  if ( !polygonSink && parameters.value( QStringLiteral( "OUTPUT_POLYGONS" ) ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT_POLYGONS" ) ) );
  QString lineDest;
  std::unique_ptr<QgsFeatureSink> lineSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LINES" ), context, lineDest, fields, Qgis::WkbType::MultiLineStringZ, destCrs ) );
  if ( !lineSink && parameters.value( QStringLiteral( "OUTPUT_LINES" ) ).isValid() )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT_LINES" ) ) );

  QFile f( path );
  QByteArray fileContent;
  if ( f.open( QIODevice::ReadOnly ) )
  {
    fileContent = f.readAll();
  }
  else
  {
    throw QgsProcessingException( QObject::tr( "Could not load source file %1." ).arg( path ) );
  }

  const QgsCoordinateTransform ecefTransform( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4978" ) ), destCrs, context.transformContext() );

  tinygltf::Model model;
  QString errors;
  QString warnings;
  if ( !QgsGltfUtils::loadGltfModel( fileContent, model, &errors, &warnings ) )
  {
    throw QgsProcessingException( QObject::tr( "Error loading GLTF model: %1" ).arg( errors ) );
  }
  if ( !warnings.isEmpty() )
  {
    feedback->pushWarning( warnings );
  }
  feedback->pushDebugInfo( QObject::tr( "Found %1 scenes" ).arg( model.scenes.size() ) );

  bool sceneOk = false;
  const std::size_t sceneIndex = QgsGltfUtils::sourceSceneForModel( model, sceneOk );
  if ( !sceneOk )
  {
    throw QgsProcessingException( QObject::tr( "No scenes found in model" ) );
  }

  const tinygltf::Scene &scene = model.scenes[sceneIndex];
  feedback->pushDebugInfo( QObject::tr( "Found %1 nodes in default scene [%2]" ).arg( scene.nodes.size() ).arg( sceneIndex ) );

  QSet<int> warnedPrimitiveTypes;

  const QgsVector3D tileTranslationEcef = QgsGltfUtils::extractTileTranslation( model );
  std::function<void( int nodeIndex, const QMatrix4x4 &transform )> traverseNode;
  traverseNode = [&model, feedback, &polygonSink, &lineSink, &warnedPrimitiveTypes, &ecefTransform, &tileTranslationEcef, &traverseNode, &parameters]( int nodeIndex, const QMatrix4x4 &parentTransform ) {
    const tinygltf::Node &gltfNode = model.nodes[nodeIndex];
    std::unique_ptr<QMatrix4x4> gltfLocalTransform = QgsGltfUtils::parseNodeTransform( gltfNode );
    if ( !parentTransform.isIdentity() )
    {
      if ( gltfLocalTransform )
        *gltfLocalTransform = parentTransform * *gltfLocalTransform;
      else
      {
        gltfLocalTransform.reset( new QMatrix4x4( parentTransform ) );
      }
    }

    if ( gltfNode.mesh >= 0 )
    {
      const tinygltf::Mesh &mesh = model.meshes[gltfNode.mesh];
      feedback->pushDebugInfo( QObject::tr( "Found %1 primitives in node [%2]" ).arg( mesh.primitives.size() ).arg( nodeIndex ) );

      for ( const tinygltf::Primitive &primitive : mesh.primitives )
      {
        switch ( primitive.mode )
        {
          case TINYGLTF_MODE_TRIANGLES:
          {
            if ( polygonSink )
            {
              std::unique_ptr<QgsAbstractGeometry> geometry = extractTriangles( model, primitive, ecefTransform, tileTranslationEcef, gltfLocalTransform.get(), feedback );
              if ( geometry )
              {
                QgsFeature f;
                f.setGeometry( std::move( geometry ) );
                if ( !polygonSink->addFeature( f, QgsFeatureSink::FastInsert ) )
                  throw QgsProcessingException( writeFeatureError( polygonSink.get(), parameters, QStringLiteral( "OUTPUT_POLYGONS" ) ) );
              }
            }
            break;
          }

          case TINYGLTF_MODE_LINE:
          {
            if ( lineSink )
            {
              std::unique_ptr<QgsAbstractGeometry> geometry = extractLines( model, primitive, ecefTransform, tileTranslationEcef, gltfLocalTransform.get(), feedback );
              if ( geometry )
              {
                QgsFeature f;
                f.setGeometry( std::move( geometry ) );
                if ( !lineSink->addFeature( f, QgsFeatureSink::FastInsert ) )
                  throw QgsProcessingException( writeFeatureError( lineSink.get(), parameters, QStringLiteral( "OUTPUT_LINES" ) ) );
              }
            }
            break;
          }

          case TINYGLTF_MODE_POINTS:
            if ( !warnedPrimitiveTypes.contains( TINYGLTF_MODE_POINTS ) )
            {
              feedback->reportError( QObject::tr( "Point objects are not supported" ) );
              warnedPrimitiveTypes.insert( TINYGLTF_MODE_POINTS );
            }
            break;

          case TINYGLTF_MODE_LINE_LOOP:
            if ( !warnedPrimitiveTypes.contains( TINYGLTF_MODE_LINE_LOOP ) )
            {
              feedback->reportError( QObject::tr( "Line loops in are not supported" ) );
              warnedPrimitiveTypes.insert( TINYGLTF_MODE_LINE_LOOP );
            }
            break;

          case TINYGLTF_MODE_LINE_STRIP:
            if ( !warnedPrimitiveTypes.contains( TINYGLTF_MODE_LINE_STRIP ) )
            {
              feedback->reportError( QObject::tr( "Line strips in are not supported" ) );
              warnedPrimitiveTypes.insert( TINYGLTF_MODE_LINE_STRIP );
            }
            break;

          case TINYGLTF_MODE_TRIANGLE_STRIP:
            if ( !warnedPrimitiveTypes.contains( TINYGLTF_MODE_TRIANGLE_STRIP ) )
            {
              feedback->reportError( QObject::tr( "Triangular strips are not supported" ) );
              warnedPrimitiveTypes.insert( TINYGLTF_MODE_TRIANGLE_STRIP );
            }
            break;

          case TINYGLTF_MODE_TRIANGLE_FAN:
            if ( !warnedPrimitiveTypes.contains( TINYGLTF_MODE_TRIANGLE_FAN ) )
            {
              feedback->reportError( QObject::tr( "Triangular fans are not supported" ) );
              warnedPrimitiveTypes.insert( TINYGLTF_MODE_TRIANGLE_FAN );
            }
            break;

          default:
            if ( !warnedPrimitiveTypes.contains( primitive.mode ) )
            {
              feedback->reportError( QObject::tr( "Primitive type %1 are not supported" ).arg( primitive.mode ) );
              warnedPrimitiveTypes.insert( primitive.mode );
            }
            break;
        }
      }
    }

    for ( int childNode : gltfNode.children )
    {
      traverseNode( childNode, gltfLocalTransform ? *gltfLocalTransform : QMatrix4x4() );
    }
  };

  if ( !scene.nodes.empty() )
  {
    for ( const int nodeIndex : scene.nodes )
    {
      traverseNode( nodeIndex, QMatrix4x4() );
    }
  }

  QVariantMap outputs;
  if ( polygonSink )
  {
    polygonSink->finalize();
    outputs.insert( QStringLiteral( "OUTPUT_POLYGONS" ), polygonDest );
  }
  if ( lineSink )
  {
    lineSink->finalize();
    outputs.insert( QStringLiteral( "OUTPUT_LINES" ), lineDest );
  }
  return outputs;
}


///@endcond
