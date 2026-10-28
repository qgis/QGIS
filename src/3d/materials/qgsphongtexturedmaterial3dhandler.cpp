/***************************************************************************
  qgsphongtexturedmaterial3dhandler.cpp
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsphongtexturedmaterial3dhandler.h"

#include "qgs3d.h"
#include "qgs3dutils.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"
#include "qgsimagetexture.h"
#include "qgsphongmaterial3dhandler.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterial.h"
#include "qgsphongtexturedmaterialsettings.h"
#include "qgsunlitmaterial.h"

#include <QMap>
#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

using namespace Qt::StringLiterals;

QgsMaterial *QgsPhongTexturedMaterial3DHandler::toMaterial( const QgsAbstractMaterialSettings *settings, Qgis::MaterialRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  const QgsPhongTexturedMaterialSettings *phongSettings = dynamic_cast< const QgsPhongTexturedMaterialSettings * >( settings );
  Q_ASSERT( phongSettings );

  switch ( technique )
  {
    case Qgis::MaterialRenderingTechnique::Triangles:
    case Qgis::MaterialRenderingTechnique::InstancedPoints:
    case Qgis::MaterialRenderingTechnique::Points:
    case Qgis::MaterialRenderingTechnique::TrianglesWithFixedTexture:
    case Qgis::MaterialRenderingTechnique::TrianglesFromModel:
    case Qgis::MaterialRenderingTechnique::TrianglesDataDefined:
    {
      if ( context.isHighlighted() )
      {
        return Qgs3D::createHighlightMaterial();
      }

      bool fitsInCache = false;
      QImage textureSourceImage = QgsApplication::imageCache()->pathAsImage( phongSettings->diffuseTexturePath(), QSize(), true, 1.0, fitsInCache );
      ( void ) fitsInCache;

      // No texture image was provided.
      // Fallback to QgsPhongMaterialSettings.
      if ( textureSourceImage.isNull() )
      {
        QgsPhongMaterialSettings phongSettings = QgsPhongMaterialSettings();
        phongSettings.setAmbient( phongSettings.ambient() );
        phongSettings.setDiffuse( QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) ); // default diffuse color from QDiffuseSpecularMaterial
        phongSettings.setOpacity( phongSettings.opacity() );
        phongSettings.setShininess( phongSettings.shininess() );
        phongSettings.setSpecular( phongSettings.specular() );
        QgsMaterial *material = QgsPhongMaterial3DHandler().toMaterial( &phongSettings, technique, context );
        return material;
      }

      QgsPhongTexturedMaterial *material = new QgsPhongTexturedMaterial();
      material->setObjectName( u"phongTexturedMaterial"_s );

      const float opacity = static_cast<float>( phongSettings->opacity() );
      QColor ambient = context.isSelected() ? context.selectionColor().darker() : phongSettings->ambient();
      material->setAmbient( QColor::fromRgbF( ambient.redF(), ambient.greenF(), ambient.blueF(), opacity ) );
      const QColor specular = phongSettings->specular();
      material->setSpecular( QColor::fromRgbF( specular.redF(), specular.greenF(), specular.blueF(), opacity ) );
      material->setShininess( static_cast<float>( phongSettings->shininess() ) );
      material->setOpacity( static_cast<float>( phongSettings->opacity() ) );

      // TODO : if ( context.isSelected() ) dampen the color of diffuse texture
      // with context.map().selectionColor()
      Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D();

      bool requiresConversionToRgb = false;
      Qt3DRender::QAbstractTexture::TextureFormat textureFormat = Qgs3DUtils::determineTextureFormat( textureSourceImage.format(), true, requiresConversionToRgb );
      if ( requiresConversionToRgb )
      {
        textureSourceImage.convertTo( QImage::Format::Format_ARGB32_Premultiplied );
      }
      texture->setFormat( textureFormat );
      texture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::Repeat );
      texture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::Repeat );
      Qgs3DUtils::setTextureFiltering( texture, context );

      texture->addTextureImage( new QgsImageTexture( textureSourceImage ) );

      material->setDiffuseTexture( texture );
      material->setDiffuseTextureScale( static_cast<float>( phongSettings->textureScale() ) );
      material->setDiffuseTextureRotation( static_cast<float>( phongSettings->textureRotation() ) );
      material->setDiffuseTextureOffset( static_cast<float>( phongSettings->textureOffset().x() ), static_cast<float>( phongSettings->textureOffset().y() ) );

      const QgsPropertyCollection ddProps = phongSettings->dataDefinedProperties();
      const bool hasDDTextureTransform = ddProps.isActive( QgsAbstractMaterialSettings::Property::TextureOffset )
                                         || ddProps.isActive( QgsAbstractMaterialSettings::Property::TextureScale )
                                         || ddProps.isActive( QgsAbstractMaterialSettings::Property::TextureRotation );
      material->setDataDefinedTextureTransformEnabled( hasDDTextureTransform );

      return material;
    }

    case Qgis::MaterialRenderingTechnique::Lines:
    case Qgis::MaterialRenderingTechnique::Billboards:
      return nullptr;
  }
  return nullptr;
}

QgsMaterial *QgsPhongTexturedMaterial3DHandler::toInstancedMaterial(
  const QgsAbstractMaterialSettings *settings, const QgsMaterialContext &context, Qgis::InstancedMaterialFlags flags, const QMatrix4x4 &transform
) const
{
  const QgsPhongTexturedMaterialSettings *phongSettings = dynamic_cast< const QgsPhongTexturedMaterialSettings * >( settings );
  Q_ASSERT( phongSettings );

  QgsPhongTexturedMaterial *material = new QgsPhongTexturedMaterial();
  material->setObjectName( u"phongTexturedMaterial"_s );
  material->setInstancingEnabled( true, flags );
  material->setInstancingMeshTransform( transform );

  const float opacity = static_cast<float>( phongSettings->opacity() );
  QColor ambient = context.isSelected() ? context.selectionColor().darker() : phongSettings->ambient();
  material->setAmbient( QColor::fromRgbF( ambient.redF(), ambient.greenF(), ambient.blueF(), opacity ) );
  const QColor specular = phongSettings->specular();
  material->setSpecular( QColor::fromRgbF( specular.redF(), specular.greenF(), specular.blueF(), opacity ) );
  material->setShininess( static_cast<float>( phongSettings->shininess() ) );
  material->setOpacity( static_cast<float>( phongSettings->opacity() ) );

  bool fitsInCache = false;
  QImage textureSourceImage = QgsApplication::imageCache()->pathAsImage( phongSettings->diffuseTexturePath(), QSize(), true, 1.0, fitsInCache );
  ( void ) fitsInCache;

  if ( !textureSourceImage.isNull() )
  {
    Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D();
    texture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::Repeat );
    texture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::Repeat );

    bool requiresConversionToRgb = false;
    Qt3DRender::QAbstractTexture::TextureFormat textureFormat = Qgs3DUtils::determineTextureFormat( textureSourceImage.format(), true, requiresConversionToRgb );
    if ( requiresConversionToRgb )
    {
      textureSourceImage.convertTo( QImage::Format::Format_ARGB32_Premultiplied );
    }
    texture->setFormat( textureFormat );

    Qgs3DUtils::setTextureFiltering( texture, context );
    texture->addTextureImage( new QgsImageTexture( textureSourceImage ) );
    material->setDiffuseTexture( texture );
    material->setDiffuseTextureScale( static_cast<float>( phongSettings->textureScale() ) );
    material->setDiffuseTextureRotation( static_cast<float>( phongSettings->textureRotation() ) );
  }

  return material;
}

QMap<QString, QString> QgsPhongTexturedMaterial3DHandler::toExportParameters( const QgsAbstractMaterialSettings *settings ) const
{
  const QgsPhongTexturedMaterialSettings *phongSettings = dynamic_cast< const QgsPhongTexturedMaterialSettings * >( settings );
  Q_ASSERT( phongSettings );

  QMap<QString, QString> parameters;
  parameters[u"Ka"_s] = u"%1 %2 %3"_s.arg( phongSettings->ambient().redF() ).arg( phongSettings->ambient().greenF() ).arg( phongSettings->ambient().blueF() );
  parameters[u"Ks"_s] = u"%1 %2 %3"_s.arg( phongSettings->specular().redF() ).arg( phongSettings->specular().greenF() ).arg( phongSettings->specular().blueF() );
  parameters[u"Ns"_s] = QString::number( phongSettings->shininess() );
  return parameters;
}

bool QgsPhongTexturedMaterial3DHandler::updatePreviewScene( Qt3DCore::QEntity *sceneRoot, const QgsAbstractMaterialSettings *settings, const QgsMaterialContext & ) const
{
  const QgsPhongTexturedMaterialSettings *phongSettings = qgis::down_cast< const QgsPhongTexturedMaterialSettings * >( settings );

  QgsMaterial *material = sceneRoot->findChild<QgsMaterial *>();
  if ( material->objectName() != "phongTexturedMaterial"_L1 )
    return false;

  Qt3DRender::QEffect *effect = material->effect();

  if ( Qt3DRender::QParameter *p = findParameter( effect, u"ambientColor"_s ) )
    p->setValue( Qgs3DUtils::srgbToLinear( phongSettings->ambient() ) );

  Qt3DRender::QTexture2D *texture = material->findChild<Qt3DRender::QTexture2D *>();
  bool fitsInCache;
  texture->addTextureImage( new QgsImageTexture( QgsApplication::imageCache()->pathAsImage( phongSettings->diffuseTexturePath(), QSize(), true, 1.0, fitsInCache ) ) );
  texture->removeTextureImage( texture->textureImages().at( 0 ) );

  if ( Qt3DRender::QParameter *p = findParameter( effect, u"texCoordScale"_s ) )
    p->setValue( phongSettings->textureScale() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"texCoordRotation"_s ) )
    p->setValue( phongSettings->textureRotation() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"texCoordOffset"_s ) )
    p->setValue( QVariant::fromValue( QVector2D( static_cast< float>( phongSettings->textureOffset().x() ), static_cast< float >( phongSettings->textureOffset().y() ) ) ) );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"specularColor"_s ) )
    p->setValue( Qgs3DUtils::srgbToLinear( phongSettings->specular() ) );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"shininess"_s ) )
    p->setValue( phongSettings->shininess() );
  if ( Qt3DRender::QParameter *p = findParameter( effect, u"opacity"_s ) )
    p->setValue( phongSettings->opacity() );
  return true;
}
