/***************************************************************************
  qgsphongtexturedmaterialsettings.cpp
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

#include "qgsphongtexturedmaterialsettings.h"

#include "qgsapplication.h"
#include "qgscolorutils.h"
#include "qgshighlightmaterial.h"
#include "qgsimagecache.h"
#include "qgsimagetexture.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterial.h"

#include <QMap>
#include <QString>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

using namespace Qt::StringLiterals;

QString QgsPhongTexturedMaterialSettings::type() const
{
  return u"phongtextured"_s;
}

bool QgsPhongTexturedMaterialSettings::supportsTechnique( QgsMaterialSettingsRenderingTechnique technique )
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined: //technique is supported but color can't be datadefined
      return true;

    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Lines:
      return false;
  }
  return false;
}

QgsAbstractMaterialSettings *QgsPhongTexturedMaterialSettings::create()
{
  return new QgsPhongTexturedMaterialSettings();
}

QgsPhongTexturedMaterialSettings *QgsPhongTexturedMaterialSettings::clone() const
{
  return new QgsPhongTexturedMaterialSettings( *this );
}

bool QgsPhongTexturedMaterialSettings::equals( const QgsAbstractMaterialSettings *other ) const
{
  const QgsPhongTexturedMaterialSettings *otherPhong = dynamic_cast<const QgsPhongTexturedMaterialSettings *>( other );
  if ( !otherPhong )
    return false;

  return *this == *otherPhong;
}

double QgsPhongTexturedMaterialSettings::textureRotation() const
{
  return mTextureRotation;
}

QColor QgsPhongTexturedMaterialSettings::averageColor() const
{
  const double avgDiffuseFactor = 0.3;
  const double avgSpecularFactor = 0.2;

  const double kAmbient = 0.2;
  const double kDiffuse = 0.6;
  const double kSpecular = 0.2;

  const QColor diffuse = textureAverageColor();

  double red = kAmbient * mAmbient.redF()
               + kDiffuse * avgDiffuseFactor * diffuse.redF()
               + kSpecular * avgSpecularFactor * mSpecular.redF();

  double green = kAmbient * mAmbient.greenF()
                 + kDiffuse * avgDiffuseFactor * diffuse.greenF()
                 + kSpecular * avgSpecularFactor * mSpecular.greenF();

  double blue = kAmbient * mAmbient.blueF()
                + kDiffuse * avgDiffuseFactor * diffuse.blueF()
                + kSpecular * avgSpecularFactor * mSpecular.blueF();

  red = std::clamp( red, 0.0, 1.0 );
  green = std::clamp( green, 0.0, 1.0 );
  blue = std::clamp( blue, 0.0, 1.0 );

  return QColor::fromRgbF( static_cast<float>( red ), static_cast<float>( green ), static_cast<float>( blue ), static_cast<float>( mOpacity ) );
}


QColor QgsPhongTexturedMaterialSettings::textureAverageColor() const
{
  if ( mTextureAverageColor.has_value() )
  {
    return *mTextureAverageColor;
  }

  QImage texture( mDiffuseTexturePath );
  if ( texture.isNull() )
  {
    mTextureAverageColor = QColor( 127, 127, 127 );
    return *mTextureAverageColor;
  }

  if ( texture.format() != QImage::Format_ARGB32 )
  {
    texture = texture.convertToFormat( QImage::Format_ARGB32 );
  }

  int red = 0;
  int green = 0;
  int blue = 0;
  int pixelCount = 0;

  // downsampling to ensure a fast computation
  const int sampleStep = 8;
  const int width = texture.width();
  const int height = texture.height();
  for ( int y = 0; y < height; y += sampleStep )
  {
    const QRgb *line = reinterpret_cast< const QRgb * >( texture.constScanLine( y ) );
    for ( int x = 0; x < width; x += sampleStep )
    {
      const QRgb pixel = line[x];
      red += qRed( pixel );
      green += qGreen( pixel );
      blue += qBlue( pixel );
      pixelCount++;
    }
  }

  mTextureAverageColor = QColor( red / pixelCount, green / pixelCount, blue / pixelCount );
  return *mTextureAverageColor;
}

void QgsPhongTexturedMaterialSettings::setColorsFromBase( const QColor &baseColor, float metallic )
{
  metallic = std::clamp( metallic, 0.0f, 1.0f );

  const float baseR = baseColor.redF();
  const float baseG = baseColor.greenF();
  const float baseB = baseColor.blueF();

  // ambient: stable, non-directional lighting
  constexpr float AMBIENT_FACTOR = 0.2f;
  mAmbient = QColor::fromRgbF( baseR * AMBIENT_FACTOR, baseG * AMBIENT_FACTOR, baseB * AMBIENT_FACTOR );

  // F0: Fresnel reflectance at normal incidence
  constexpr float F0_DIELECTRIC = 0.04f;

  // specular
  // * Non-metallic surfaces: Independent of base color
  // * Metallic surfaces:
  //   - Reflect their own color
  //   - Linear interpolation from white to base color as metallic increases
  mSpecular = QColor::fromRgbF(
    ( 1.0f - metallic ) * F0_DIELECTRIC + metallic * baseR,
    ( 1.0f - metallic ) * F0_DIELECTRIC + metallic * baseG,
    ( 1.0f - metallic ) * F0_DIELECTRIC + metallic * baseB
  );

  constexpr float MIN_SHININESS = 32.0f;
  constexpr float MAX_SHININESS = 200.0f;
  mShininess = MIN_SHININESS + metallic * ( MAX_SHININESS - MIN_SHININESS );
}

void QgsPhongTexturedMaterialSettings::setColorsFromBase( const QColor &baseColor )
{
  setColorsFromBase( baseColor, 0.0f );
}

void QgsPhongTexturedMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( u"ambient"_s, u"25,25,25"_s ) );
  mSpecular = QgsColorUtils::colorFromString( elem.attribute( u"specular"_s, u"255,255,255"_s ) );
  mShininess = elem.attribute( u"shininess"_s ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s, u"1.0"_s ).toDouble();
  mDiffuseTexturePath = elem.attribute( u"diffuse_texture_path"_s, QString() );
  mTextureScale = elem.attribute( u"texture_scale"_s, QString( "1.0" ) ).toDouble();
  mTextureRotation = elem.attribute( u"texture-rotation"_s, QString( "0.0" ) ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsPhongTexturedMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( u"ambient"_s, QgsColorUtils::colorToString( mAmbient ) );
  elem.setAttribute( u"specular"_s, QgsColorUtils::colorToString( mSpecular ) );
  elem.setAttribute( u"shininess"_s, mShininess );
  elem.setAttribute( u"opacity"_s, mOpacity );
  elem.setAttribute( u"diffuse_texture_path"_s, mDiffuseTexturePath );
  elem.setAttribute( u"texture_scale"_s, mTextureScale );
  elem.setAttribute( u"texture-rotation"_s, mTextureRotation );

  QgsAbstractMaterialSettings::writeXml( elem, context );
}

QgsMaterial *QgsPhongTexturedMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    case QgsMaterialSettingsRenderingTechnique::TrianglesFromModel:
    case QgsMaterialSettingsRenderingTechnique::TrianglesDataDefined:
    {
      if ( context.isHighlighted() )
      {
        return new QgsHighlightMaterial( technique );
      }

      bool fitsInCache = false;
      const QImage textureSourceImage = QgsApplication::imageCache()->pathAsImage( mDiffuseTexturePath, QSize(), true, 1.0, fitsInCache );
      ( void ) fitsInCache;

      // No texture image was provided.
      // Fallback to QgsPhongMaterialSettings.
      if ( textureSourceImage.isNull() )
      {
        QgsPhongMaterialSettings phongSettings = QgsPhongMaterialSettings();
        phongSettings.setAmbient( mAmbient );
        phongSettings.setDiffuse( QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) ); // default diffuse color from QDiffuseSpecularMaterial
        phongSettings.setOpacity( mOpacity );
        phongSettings.setShininess( mShininess );
        phongSettings.setSpecular( mSpecular );
        QgsMaterial *material = phongSettings.toMaterial( technique, context );
        return material;
      }

      QgsPhongTexturedMaterial *material = new QgsPhongTexturedMaterial();

      int opacity = static_cast<int>( mOpacity * 255.0 );
      QColor ambient = context.isSelected() ? context.selectionColor().darker() : mAmbient;
      material->setAmbient( QColor( ambient.red(), ambient.green(), ambient.blue(), opacity ) );
      material->setSpecular( QColor( mSpecular.red(), mSpecular.green(), mSpecular.blue(), opacity ) );
      material->setShininess( static_cast<float>( mShininess ) );
      material->setOpacity( static_cast<float>( mOpacity ) );

      // TODO : if ( context.isSelected() ) dampen the color of diffuse texture
      // with context.map().selectionColor()
      QgsImageTexture *textureImage = new QgsImageTexture( textureSourceImage );
      Qt3DRender::QTexture2D *texture = new Qt3DRender::QTexture2D();
      texture->addTextureImage( textureImage );

      texture->wrapMode()->setX( Qt3DRender::QTextureWrapMode::Repeat );
      texture->wrapMode()->setY( Qt3DRender::QTextureWrapMode::Repeat );
      texture->wrapMode()->setZ( Qt3DRender::QTextureWrapMode::Repeat );

      texture->setSamples( 4 );

      texture->setGenerateMipMaps( true );
      texture->setMagnificationFilter( Qt3DRender::QTexture2D::Linear );
      texture->setMinificationFilter( Qt3DRender::QTexture2D::Linear );

      material->setDiffuseTexture( texture );
      material->setDiffuseTextureScale( static_cast<float>( mTextureScale ) );

      return material;
    }

    case QgsMaterialSettingsRenderingTechnique::Lines:
      return nullptr;
  }
  return nullptr;
}

QMap<QString, QString> QgsPhongTexturedMaterialSettings::toExportParameters() const
{
  QMap<QString, QString> parameters;
  parameters[u"Ka"_s] = u"%1 %2 %3"_s.arg( mAmbient.redF() ).arg( mAmbient.greenF() ).arg( mAmbient.blueF() );
  parameters[u"Ks"_s] = u"%1 %2 %3"_s.arg( mSpecular.redF() ).arg( mSpecular.greenF() ).arg( mSpecular.blueF() );
  parameters[u"Ns"_s] = QString::number( mShininess );
  return parameters;
}

void QgsPhongTexturedMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect, const QgsMaterialContext &materialContext ) const
{
  const QColor ambientColor = materialContext.isSelected() ? materialContext.selectionColor().darker() : mAmbient;

  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( u"ambientColor"_s, ambientColor );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( u"specularColor"_s, mSpecular );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( u"shininess"_s, static_cast<float>( mShininess ) );

  effect->addParameter( ambientParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );
}
