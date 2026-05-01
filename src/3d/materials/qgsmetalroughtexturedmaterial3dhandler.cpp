/***************************************************************************
  qgsmetalroughtexturedmaterial3dhandler.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetalroughtexturedmaterial3dhandler.h"

#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgshighlightmaterial.h"
#include "qgsimagecache.h"
#include "qgsimagetexture.h"
#include "qgsmetalroughmaterial.h"
#include "qgsmetalroughtexturedmaterialsettings.h"

#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

using namespace Qt::StringLiterals;

QgsMaterial *QgsMetalRoughTexturedMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  const QgsMetalRoughTexturedMaterialSettings *texturedSettings = dynamic_cast< const QgsMetalRoughTexturedMaterialSettings * >( settings );
  Q_ASSERT( texturedSettings );

  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    {
      if ( context.isHighlighted() )
      {
        return new QgsHighlightMaterial( technique );
      }

      QgsMetalRoughMaterial *material = new QgsMetalRoughMaterial();
      material->setObjectName( u"metalRoughTexturedMaterial"_s );
      applySettingsToMaterial( texturedSettings, material );

      return material;
    }

    default:
      return nullptr;
  }
}

QMap<QString, QString> QgsMetalRoughTexturedMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings * ) const
{
  QMap<QString, QString> parameters;
  return parameters;
}

void QgsMetalRoughTexturedMaterial3DHandler::addParametersToEffect( Qt3DRender::QEffect *, const QgsAbstractMaterialSettings *, const QgsMaterialContext & ) const
{}

bool QgsMetalRoughTexturedMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext & ) const
{
  const QgsMetalRoughTexturedMaterialSettings *metalRoughTexturedSettings = qgis::down_cast< const QgsMetalRoughTexturedMaterialSettings * >( settings );

  QgsMetalRoughMaterial *material = sceneRoot->findChild<QgsMetalRoughMaterial *>();
  if ( !material || material->objectName() != "metalRoughTexturedMaterial"_L1 )
    return false;

  applySettingsToMaterial( metalRoughTexturedSettings, material );
  return true;
}

Qt3DRender::QTexture2D *QgsMetalRoughTexturedMaterial3DHandler::loadTexture( const QString &path, bool isSrgb )
{
  if ( path.isEmpty() )
    return nullptr;

  bool fitsInCache = false;
  QImage image = QgsApplication::imageCache()->pathAsImage( path, QSize(), true, 1.0, fitsInCache );
  if ( image.isNull() )
    return nullptr;

  Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D();

  if ( isSrgb )
  {
    texture->setFormat( Qt3DRender::QAbstractTexture::SRGB8_Alpha8 );
  }
  else
  {
    texture->setFormat( Qt3DRender::QAbstractTexture::RGBA8_UNorm );
  }

  texture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::Repeat );
  texture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::Repeat );
  Qgs3DUtils::setTextureFiltering( texture );

  // texture takes ownership of textureImage
  texture->addTextureImage( new QgsImageTexture( image ) );

  return texture;
}

void QgsMetalRoughTexturedMaterial3DHandler::applySettingsToMaterial( const QgsMetalRoughTexturedMaterialSettings *texturedSettings, QgsMetalRoughMaterial *material )
{
  material->setTextureScale( static_cast<float>( texturedSettings->textureScale() ) );
  material->setTextureRotation( static_cast<float>( texturedSettings->textureRotation() ) );

  // base color
  if ( Qt3DRender::QTexture2D *baseTex = loadTexture( texturedSettings->baseColorTexturePath(), true ) )
  {
    // takes ownership of texture
    material->setBaseColorTexture( baseTex );
  }
  else
  {
    // fallback to default solid color if no texture specified
    material->setBaseColor( QColor( "grey" ) );
  }

  // metalness
  if ( Qt3DRender::QTexture2D *metalTex = loadTexture( texturedSettings->metalnessTexturePath(), false ) )
  {
    // takes ownership of texture
    material->setMetalnessTexture( metalTex );
  }
  else
  {
    // fallback to constant default value if no texture specified
    material->setMetalness( 0.0 );
  }

  // roughness
  if ( Qt3DRender::QTexture2D *roughTex = loadTexture( texturedSettings->roughnessTexturePath(), false ) )
  {
    // takes ownership of texture
    material->setRoughnessTexture( roughTex );
  }
  else
  {
    // fallback to constant default value if no texture specified
    material->setRoughness( 0.5 );
  }

  if ( Qt3DRender::QTexture2D *normalTex = loadTexture( texturedSettings->normalTexturePath(), false ) )
  {
    // takes ownership of texture
    material->setNormalTexture( normalTex );
  }
  else
  {
    // default to none
    material->setNormalTexture( nullptr );
  }

  if ( Qt3DRender::QTexture2D *heightTex = loadTexture( texturedSettings->heightTexturePath(), false ) )
  {
    material->setHeightTexture( heightTex );
  }
  else
  {
    // default to none
    material->setHeightTexture( nullptr );
  }
  material->setParallaxScale( texturedSettings->parallaxScale() );

  // ambient occlusion
  if ( Qt3DRender::QTexture2D *aoTex = loadTexture( texturedSettings->ambientOcclusionTexturePath(), false ) )
  {
    // takes ownership of texture
    material->setAmbientOcclusionTexture( aoTex );
  }
  else
  {
    // default to none
    material->setAmbientOcclusionTexture( nullptr );
  }

  if ( Qt3DRender::QTexture2D *emissionTex = loadTexture( texturedSettings->emissionTexturePath(), true ) )
  {
    material->setEmissionTexture( emissionTex );
  }
  else
  {
    // default to none
    material->setEmissionTexture( nullptr );
  }

  material->setEmissionFactor( texturedSettings->emissionFactor() );
};
