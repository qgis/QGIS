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

#include "qgssymbollayerutils.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"
#include <Qt3DExtras/QDiffuseMapMaterial>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DRender/QPaintedTextureImage>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QEffect>
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
      return true;

    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
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

float QgsPhongTexturedMaterialSettings::textureRotation() const
{
  return mTextureRotation;
}

void QgsPhongTexturedMaterialSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mAmbient = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "ambient" ), QStringLiteral( "25,25,25" ) ) );
  mSpecular = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "specular" ), QStringLiteral( "255,255,255" ) ) );
  mShininess = elem.attribute( QStringLiteral( "shininess" ) ).toFloat();
  mDiffuseTexturePath = elem.attribute( QStringLiteral( "diffuse_texture_path" ), QString() );
  mTextureScale = elem.attribute( QStringLiteral( "texture_scale" ), QString( "1.0" ) ).toFloat();
  mTextureRotation = elem.attribute( QStringLiteral( "texture-rotation" ), QString( "0.0" ) ).toFloat();
}

void QgsPhongTexturedMaterialSettings::writeXml( QDomElement &elem, const QgsReadWriteContext & ) const
{
  elem.setAttribute( QStringLiteral( "ambient" ), QgsSymbolLayerUtils::encodeColor( mAmbient ) );
  elem.setAttribute( QStringLiteral( "specular" ), QgsSymbolLayerUtils::encodeColor( mSpecular ) );
  elem.setAttribute( QStringLiteral( "shininess" ), mShininess );
  elem.setAttribute( QStringLiteral( "diffuse_texture_path" ), mDiffuseTexturePath );
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
Qt3DRender::QMaterial *QgsPhongTexturedMaterialSettings::toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const
{
  switch ( technique )
  {
    case QgsMaterialSettingsRenderingTechnique::Triangles:
    case QgsMaterialSettingsRenderingTechnique::InstancedPoints:
    case QgsMaterialSettingsRenderingTechnique::Points:
    case QgsMaterialSettingsRenderingTechnique::TrianglesWithFixedTexture:
    {
      bool fitsInCache = false;
      QImage textureSourceImage = QgsApplication::imageCache()->pathAsImage( mDiffuseTexturePath, QSize(), true, 1.0, fitsInCache );
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
        material->setAmbient( mAmbient );
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

    case QgsMaterialSettingsRenderingTechnique::Lines:
      return nullptr;

  }
  return nullptr;
}

QMap<QString, QString> QgsPhongTexturedMaterialSettings::toExportParameters() const
{
  QMap<QString, QString> parameters;
  parameters[ QStringLiteral( "Ka" ) ] = QStringLiteral( "%1 %2 %3" ).arg( mAmbient.redF() ).arg( mAmbient.greenF() ).arg( mAmbient.blueF() );
  parameters[ QStringLiteral( "Ks" ) ] = QStringLiteral( "%1 %2 %3" ).arg( mSpecular.redF() ).arg( mSpecular.greenF() ).arg( mSpecular.blueF() );
  parameters[ QStringLiteral( "Ns" ) ] = QString::number( mShininess );
  return parameters;
}

void QgsPhongTexturedMaterialSettings::addParametersToEffect( Qt3DRender::QEffect *effect ) const
{
  Qt3DRender::QParameter *ambientParameter = new Qt3DRender::QParameter( QStringLiteral( "ka" ), QColor::fromRgbF( 0.05f, 0.05f, 0.05f, 1.0f ) );
  Qt3DRender::QParameter *specularParameter = new Qt3DRender::QParameter( QStringLiteral( "ks" ), QColor::fromRgbF( 0.01f, 0.01f, 0.01f, 1.0f ) );
  Qt3DRender::QParameter *shininessParameter = new Qt3DRender::QParameter( QStringLiteral( "shininess" ), 150.0f );

  ambientParameter->setValue( mAmbient );
  specularParameter->setValue( mSpecular );
  shininessParameter->setValue( mShininess );

  effect->addParameter( ambientParameter );
  effect->addParameter( specularParameter );
  effect->addParameter( shininessParameter );
}
