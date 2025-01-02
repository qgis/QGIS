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
#include "qgsimagecache.h"
#include "qgsimagetexture.h"
#include "qgsphongmaterialsettings.h"
#include "qgsphongtexturedmaterial.h"
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QGraphicsApiFilter>
#include <QMap>


QString QgsPhongTexturedMaterialSettings::type() const
{
  return QStringLiteral( "phongtextured" );
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

void QgsPhongTexturedMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mAmbient = QgsColorUtils::colorFromString( elem.attribute( QStringLiteral( "ambient" ), QStringLiteral( "25,25,25" ) ) );
  mSpecular = QgsColorUtils::colorFromString( elem.attribute( QStringLiteral( "specular" ), QStringLiteral( "255,255,255" ) ) );
  mShininess = elem.attribute( QStringLiteral( "shininess" ) ).toDouble();
  mOpacity = elem.attribute( QStringLiteral( "opacity" ), QStringLiteral( "1.0" ) ).toDouble();
  mDiffuseTexturePath = elem.attribute( QStringLiteral( "diffuse_texture_path" ), QString() );
  mTextureScale = elem.attribute( QStringLiteral( "texture_scale" ), QString( "1.0" ) ).toDouble();
  mTextureRotation = elem.attribute( QStringLiteral( "texture-rotation" ), QString( "0.0" ) ).toDouble();

  QgsAbstractMaterialSettings::readXml( elem, context );
}

void QgsPhongTexturedMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  elem.setAttribute( QStringLiteral( "ambient" ), QgsColorUtils::colorToString( mAmbient ) );
  elem.setAttribute( QStringLiteral( "specular" ), QgsColorUtils::colorToString( mSpecular ) );
  elem.setAttribute( QStringLiteral( "shininess" ), mShininess );
  elem.setAttribute( QStringLiteral( "opacity" ), mOpacity );
  elem.setAttribute( QStringLiteral( "diffuse_texture_path" ), mDiffuseTexturePath );
  elem.setAttribute( QStringLiteral( "texture_scale" ), mTextureScale );
  elem.setAttribute( QStringLiteral( "texture-rotation" ), mTextureRotation );

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
  parameters[QStringLiteral( "Ka" )] = QStringLiteral( "%1 %2 %3" ).arg( mAmbient.redF() ).arg( mAmbient.greenF() ).arg( mAmbient.blueF() );
  parameters[QStringLiteral( "Ks" )] = QStringLiteral( "%1 %2 %3" ).arg( mSpecular.redF() ).arg( mSpecular.greenF() ).arg( mSpecular.blueF() );
  parameters[QStringLiteral( "Ns" )] = QString::number( mShininess );
  return parameters;
}

void QgsPhongTexturedMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect, const QgsMaterialContext &materialContext ) const
{
  const QColor ambientColor = materialContext.isSelected() ? materialContext.selectionColor().darker() : mAmbient;

  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ambientColor" ), ambientColor );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( QStringLiteral( "specularColor" ), mSpecular );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( QStringLiteral( "shininess" ), static_cast<float>( mShininess ) );

  effect->addParameter( ambientParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );
}
