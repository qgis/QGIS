/***************************************************************************
  qgsphongmaterialsettings.cpp
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsphongmaterialsettings.h"

#include "qgssymbollayerutils.h"
#include "qgslinematerial_p.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>


QString QgsPhongMaterialSettings::type() const
{
  return QStringLiteral( "phong" );
}

QgsAbstractMaterialSettings *QgsPhongMaterialSettings::create()
{
  return new QgsPhongMaterialSettings();
}

QgsPhongMaterialSettings *QgsPhongMaterialSettings::clone() const
{
  return new QgsPhongMaterialSettings( *this );
}

void QgsPhongMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mAmbient = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "ambient" ) ) );
  mDiffuse = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "diffuse" ) ) );
  mSpecular = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "specular" ) ) );
  mShininess = elem.attribute( QStringLiteral( "shininess" ) ).toFloat();
  mDiffuseTextureEnabled = elem.attribute( QStringLiteral( "is_using_diffuse_texture" ), QStringLiteral( "0" ) ).toInt();
  mTexturePath = elem.attribute( QStringLiteral( "diffuse_texture_path" ), QString() );
  mTextureScale = elem.attribute( QStringLiteral( "texture_scale" ), QString( "1.0" ) ).toFloat();
  mTextureRotation = elem.attribute( QStringLiteral( "texture-rotation" ), QString( "0.0" ) ).toFloat();
}

void QgsPhongMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext & ) const
{
  elem.setAttribute( QStringLiteral( "ambient" ), QgsSymbolLayerUtils::encodeColor( mAmbient ) );
  elem.setAttribute( QStringLiteral( "diffuse" ), QgsSymbolLayerUtils::encodeColor( mDiffuse ) );
  elem.setAttribute( QStringLiteral( "specular" ), QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( QStringLiteral( "shininess" ), mShininess );
  elem.setAttribute( QStringLiteral( "is_using_diffuse_texture" ), mDiffuseTextureEnabled );
  elem.setAttribute( QStringLiteral( "diffuse_texture_path" ), mTexturePath );
  elem.setAttribute( QStringLiteral( "texture_scale" ), mTextureScale );
  elem.setAttribute( QStringLiteral( "texture-rotation" ), mTextureRotation );
}

///@cond PRIVATE
class QgsQImageTextureImage : public Qt3DRender::QPaintedTextureImage
{
  public:
    QgsQImageTextureImage( const QImage &image, Qt3DCore::QNode *parent = nullptr )
      : Qt3DRender::QPaintedTextureImage( parent )
      , mImage( image )
    {
      setSize( mImage.size() );
    }

    void paint( QPainter *painter ) override
    {
      painter->drawImage( mImage.rect(), mImage, mImage.rect() );
    }

  private:

    QImage mImage;

};
///@endcond
Qt3DRender::QMaterial *QgsPhongMaterialSettings::toMaterial( const QgsMaterialContext &context ) const
{
  bool fitsInCache = false;
  QImage textureSourceImage;

  if ( shouldUseDiffuseTexture() )
    textureSourceImage = QgsApplication::imageCache()->pathAsImage( mTexturePath, QSize(), true, 1.0, fitsInCache );
  ( void )fitsInCache;

  if ( !textureSourceImage.isNull() )
  {
    Qt3DExtras::QDiffuseMapMaterial *material = new Qt3DExtras::QDiffuseMapMaterial;

    QgsQImageTextureImage *textureImage = new QgsQImageTextureImage( textureSourceImage );
    material->diffuse()->addTextureImage( textureImage );

    material->diffuse()->wrapMode()->setX( Qt3DRender::QTextureWrapMode::Repeat );
    material->diffuse()->wrapMode()->setY( Qt3DRender::QTextureWrapMode::Repeat );
    material->diffuse()->wrapMode()->setZ( Qt3DRender::QTextureWrapMode::Repeat );
    material->setSpecular( mSpecular );
    material->setShininess( mShininess );
    material->setTextureScale( mTextureScale );

    if ( context.isSelected() )
    {
      // update the material with selection colors
      // TODO : dampen the color of diffuse texture
//      mat->setDiffuse( context.map().selectionColor() );
      material->setAmbient( context.selectionColor().darker() );
    }

    return material;
  }
  else
  {
    Qt3DExtras::QPhongMaterial *material  = new Qt3DExtras::QPhongMaterial;
    material->setDiffuse( mDiffuse );
    material->setAmbient( mAmbient );
    material->setSpecular( mSpecular );
    material->setShininess( mShininess );

    if ( context.isSelected() )
    {
      // update the material with selection colors
      material->setDiffuse( context.selectionColor() );
      material->setAmbient( context.selectionColor().darker() );
    }
    return material;
  }
}

QgsLineMaterial *QgsPhongMaterialSettings::toLineMaterial( const QgsMaterialContext &context ) const
{
  QgsLineMaterial *mat = new QgsLineMaterial;
  mat->setLineColor( mAmbient );
  if ( context.isSelected() )
  {
    // update the material with selection colors
    mat->setLineColor( context.selectionColor() );
  }
  return mat;
}

void QgsPhongMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect ) const
{
  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ka" ), QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) );
  Qt3DRender::QParameter *diffuseParameter = new Qt3DRender::QParameter( QStringLiteral( "kd" ), QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( QStringLiteral( "ks" ), QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( QStringLiteral( "shininess" ), 150.0f );

  diffuseParameter->setValue( mDiffuse );
  ambientParameter->setValue( mAmbient );
  specularParameter->setValue( mSpecular );
  shininessParameter->setValue( mShininess );

  effect->addParameter( ambientParameter );
  effect->addParameter( diffuseParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );
}
