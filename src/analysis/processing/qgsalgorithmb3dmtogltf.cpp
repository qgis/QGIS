/***************************************************************************
                         qgsalgorithmb3dmtogltf.cpp
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

#include "qgsalgorithmb3dmtogltf.h"
#include "qgscesiumutils.h"
#include "qgsgltfutils.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE       // we use QImage-based reading of images
#define TINYGLTF_NO_STB_IMAGE_WRITE // we don't need writing of images

#include "tiny_gltf.h"
#include <fstream>

///@cond PRIVATE

QString QgsB3DMToGltfAlgorithm::name() const
{
  return QStringLiteral( "b3dmtogltf" );
}

QString QgsB3DMToGltfAlgorithm::displayName() const
{
  return QObject::tr( "Convert B3DM to GLTF" );
}

QStringList QgsB3DMToGltfAlgorithm::tags() const
{
  return QObject::tr( "3d,tiles,cesium" ).split( ',' );
}

QString QgsB3DMToGltfAlgorithm::group() const
{
  return QObject::tr( "3D Tiles" );
}

QString QgsB3DMToGltfAlgorithm::groupId() const
{
  return QStringLiteral( "3dtiles" );
}

QString QgsB3DMToGltfAlgorithm::shortHelpString() const
{
  return QObject::tr( "Converts files from the legacy B3DM format to GLTF." );
}

QgsB3DMToGltfAlgorithm *QgsB3DMToGltfAlgorithm::createInstance() const
{
  return new QgsB3DMToGltfAlgorithm();
}

void QgsB3DMToGltfAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFile( QStringLiteral( "INPUT" ), QObject::tr( "Input B3DM" ), Qgis::ProcessingFileParameterBehavior::File, QStringLiteral( "b3dm" ), QVariant(), false, QStringLiteral( "B3DM (*.b3dm *.B3DM)" ) ) );

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output file" ), QStringLiteral( "GLTF (*.gltf *.GLTF);;GLB (*.glb *.GLB)" ) ) );
}

QVariantMap QgsB3DMToGltfAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString path = parameterAsFile( parameters, QStringLiteral( "INPUT" ), context );
  const QString outputPath = parameterAsFile( parameters, QStringLiteral( "OUTPUT" ), context );

  if ( !QFile::exists( path ) )
    throw QgsProcessingException( QObject::tr( "Could not load source file %1." ).arg( path ) );

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

  const QgsCesiumUtils::B3DMContents b3dmContent = QgsCesiumUtils::extractGltfFromB3dm( fileContent );

  // load gltf and then rewrite -- this allows us to both validate the B3DM GLTF content, and
  // also gives the opportunity to "upgrade" B3DM specific options (like CESIUM_RTC) to standard
  // GLTF extensions
  tinygltf::Model model;
  QString errors;
  QString warnings;
  const bool res = QgsGltfUtils::loadGltfModel( b3dmContent.gltf, model, &errors, &warnings );
  if ( !errors.isEmpty() )
  {
    feedback->reportError( errors );
  }
  if ( !warnings.isEmpty() )
  {
    feedback->pushWarning( warnings );
  }
  if ( !res )
  {
    // if we can't read the GLTF, then just write the original GLTF content to the output file
    QFile outputFile( outputPath );
    if ( !outputFile.open( QFile::WriteOnly ) )
    {
      throw QgsProcessingException( QObject::tr( "Could not create destination file %1." ).arg( outputPath ) );
    }
    outputFile.write( b3dmContent.gltf );
  }
  else
  {
    bool sceneOk = false;
    const std::size_t sceneIndex = QgsGltfUtils::sourceSceneForModel( model, sceneOk );
    if ( !sceneOk )
    {
      throw QgsProcessingException( QObject::tr( "No scenes found in model" ) );
    }

    feedback->pushDebugInfo( QObject::tr( "Found %1 scenes" ).arg( model.scenes.size() ) );

    const tinygltf::Scene &scene = model.scenes[sceneIndex];
    feedback->pushDebugInfo( QObject::tr( "Found %1 nodes in default scene [%2]" ).arg( scene.nodes.size() ).arg( sceneIndex ) );
    if ( !scene.nodes.empty() )
    {
      const int nodeIndex = scene.nodes[0];
      const tinygltf::Node &gltfNode = model.nodes[nodeIndex];
      if ( gltfNode.mesh >= 0 )
      {
        const tinygltf::Mesh &mesh = model.meshes[gltfNode.mesh];
        feedback->pushDebugInfo( QObject::tr( "Found %1 primitives in default scene node [%2]" ).arg( mesh.primitives.size() ).arg( nodeIndex ) );
      }
    }

    if ( !b3dmContent.rtcCenter.isNull() )
    {
      // transfer B3DM RTC center to GLTF CESIUM_RTC extension
      tinygltf::Value::Object cesiumRtc;
      cesiumRtc["center"] = tinygltf::Value( tinygltf::Value::Array {
        tinygltf::Value( b3dmContent.rtcCenter.x() ),
        tinygltf::Value( b3dmContent.rtcCenter.y() ),
        tinygltf::Value( b3dmContent.rtcCenter.z() )
      } );

      model.extensions["CESIUM_RTC"] = tinygltf::Value( cesiumRtc );
      model.extensionsRequired.emplace_back( "CESIUM_RTC" );
      model.extensionsUsed.emplace_back( "CESIUM_RTC" );
    }

    const QString outputExtension = QFileInfo( outputPath ).suffix();
    const bool isGlb = outputExtension.compare( QLatin1String( "glb" ), Qt::CaseInsensitive ) == 0;
    const QByteArray outputFile = QFile::encodeName( outputPath );
    std::ofstream of( outputFile.constData(), std::ios::binary | std::ios::trunc );
    if ( !of )
      throw QgsProcessingException( QObject::tr( "Could not create destination file %1." ).arg( outputPath ) );

    tinygltf::TinyGLTF writer;
    if ( !writer.WriteGltfSceneToStream( &model, of, true, isGlb ) )
    {
      of.close();
      throw QgsProcessingException( QObject::tr( "Could not write GLTF model to %1." ).arg( outputPath ) );
    }
    of.close();
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputPath );
  return outputs;
}

///@endcond
