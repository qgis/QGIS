/***************************************************************************
  qgsmetalroughmaterial.cpp
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetalroughmaterial.h"

#include "qgs3dutils.h"

#include <QString>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgramBuilder>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QTexture>

#include "moc_qgsmetalroughmaterial.cpp"

using namespace Qt::StringLiterals;

///@cond PRIVATE
QgsMetalRoughMaterial::QgsMetalRoughMaterial( QNode *parent )
  : QgsPBRMaterial( parent )
  , mMetalnessParameter( new Qt3DRender::QParameter( u"metalness"_s, 0.0f, this ) )
  , mReflectanceParameter( new Qt3DRender::QParameter( u"reflectance"_s, 0.5f, this ) )
  , mAnisotropyParameter( new Qt3DRender::QParameter( u"anisotropy"_s, 0.0f, this ) )
  , mAnisotropyRotationParameter( new Qt3DRender::QParameter( u"anisotropyRotation"_s, 0.0f, this ) )
  , mMetalnessMapParameter( new Qt3DRender::QParameter( u"metalnessMap"_s, QVariant(), this ) )
  , mEmissionMapParameter( new Qt3DRender::QParameter( u"emissionMap"_s, QVariant(), this ) )
  , mEmissiveColorParameter( new Qt3DRender::QParameter( u"emissiveColor"_s, Qgs3DUtils::srgbToLinear( QColor( 0, 0, 0 ) ), this ) )
  , mEmissionFactorParameter( new Qt3DRender::QParameter( u"emissiveFactor"_s, 1.0f, this ) )
  , mClearCoatFactorParameter( new Qt3DRender::QParameter( u"clearCoatFactor"_s, 0.0f, this ) )
  , mClearCoatRoughnessParameter( new Qt3DRender::QParameter( u"clearCoatRoughness"_s, 0.0f, this ) )
{
  init();
}

QgsMetalRoughMaterial::~QgsMetalRoughMaterial() = default;

void QgsMetalRoughMaterial::setMetalness( float metalness )
{
  mMetalnessParameter->setValue( metalness );
  bool oldUsingMetalnessMap = mUsingMetalnessMap;

  mUsingMetalnessMap = false;
  if ( mEffect->parameters().contains( mMetalnessMapParameter ) )
    mEffect->removeParameter( mMetalnessMapParameter );
  mEffect->addParameter( mMetalnessParameter );

  if ( oldUsingMetalnessMap != mUsingMetalnessMap )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setMetalnessTexture( Qt3DRender::QAbstractTexture *metalness )
{
  mMetalnessMapParameter->setValue( QVariant::fromValue( metalness ) );
  bool oldUsingMetalnessMap = mUsingMetalnessMap;

  mUsingMetalnessMap = true;
  mEffect->addParameter( mMetalnessMapParameter );
  if ( mEffect->parameters().contains( mMetalnessParameter ) )
    mEffect->removeParameter( mMetalnessParameter );

  if ( oldUsingMetalnessMap != mUsingMetalnessMap )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setReflectance( float reflectance )
{
  mReflectanceParameter->setValue( QVariant::fromValue( reflectance ) );
}

void QgsMetalRoughMaterial::setAnisotropy( float anisotropy )
{
  const bool oldUsingAnisotropy = mEffect->parameters().contains( mAnisotropyParameter );
  mAnisotropyParameter->setValue( anisotropy );
  const bool newUsingAnisotropy = anisotropy > 0;
  if ( newUsingAnisotropy )
  {
    if ( !oldUsingAnisotropy )
    {
      mEffect->addParameter( mAnisotropyParameter );
      mEffect->addParameter( mAnisotropyRotationParameter );
    }
  }
  else if ( oldUsingAnisotropy )
  {
    mEffect->removeParameter( mAnisotropyParameter );
    mEffect->removeParameter( mAnisotropyRotationParameter );
  }

  if ( oldUsingAnisotropy != newUsingAnisotropy )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setAnisotropyRotation( float rotation )
{
  mAnisotropyRotationParameter->setValue( M_PI * rotation / 180.0 );
}

void QgsMetalRoughMaterial::setEmissionColor( const QColor &color )
{
  mEmissiveColorParameter->setValue( Qgs3DUtils::srgbToLinear( color ) );
  const bool oldUsingEmissionMap = mUsingEmissionMap;

  mUsingEmissionMap = false;
  if ( mEffect->parameters().contains( mEmissionMapParameter ) )
    mEffect->removeParameter( mEmissionMapParameter );
  mEffect->addParameter( mEmissiveColorParameter );

  if ( oldUsingEmissionMap != mUsingEmissionMap )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setEmissionTexture( Qt3DRender::QAbstractTexture *emission )
{
  const bool oldUsingEmissionMap = mUsingEmissionMap;

  if ( emission )
  {
    mEmissionMapParameter->setValue( QVariant::fromValue( emission ) );
    mUsingEmissionMap = true;
    mEffect->addParameter( mEmissionMapParameter );
    if ( mEffect->parameters().contains( mEmissiveColorParameter ) )
      mEffect->removeParameter( mEmissiveColorParameter );
  }
  else
  {
    mEmissionMapParameter->setValue( QVariant() );
    mUsingEmissionMap = false;
    if ( mEffect->parameters().contains( mEmissionMapParameter ) )
      mEffect->removeParameter( mEmissionMapParameter );
    mEffect->addParameter( mEmissiveColorParameter );
  }

  if ( oldUsingEmissionMap != mUsingEmissionMap )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setEmissionFactor( double factor )
{
  mEmissionFactorParameter->setValue( factor );
}

void QgsMetalRoughMaterial::setClearCoatFactor( float factor )
{
  mClearCoatFactorParameter->setValue( factor );
  const bool oldUsingClearCoat = mEffect->parameters().contains( mClearCoatFactorParameter );
  const bool newUsingClearCoat = factor > 0;
  if ( newUsingClearCoat )
  {
    if ( !oldUsingClearCoat )
    {
      mEffect->addParameter( mClearCoatFactorParameter );
      mEffect->addParameter( mClearCoatRoughnessParameter );
    }
  }
  else if ( oldUsingClearCoat )
  {
    mEffect->removeParameter( mClearCoatFactorParameter );
    mEffect->removeParameter( mClearCoatRoughnessParameter );
  }

  if ( oldUsingClearCoat != newUsingClearCoat )
  {
    updateShaders();
  }
}

void QgsMetalRoughMaterial::setClearCoatRoughness( float roughness )
{
  mClearCoatRoughnessParameter->setValue( roughness );
}

void QgsMetalRoughMaterial::init()
{
  initMaterial();

  // give parameters a parent
  mMetalnessMapParameter->setParent( mEffect );
  mEmissionMapParameter->setParent( mEffect );

  mEffect->addParameter( mMetalnessParameter );
  mEffect->addParameter( mReflectanceParameter );
  mEffect->addParameter( mEmissiveColorParameter );
  mEffect->addParameter( mEmissionFactorParameter );
}

QStringList QgsMetalRoughMaterial::fragmentShaderDefines() const
{
  QStringList defines = QgsPBRMaterial::fragmentShaderDefines();

  if ( mUsingMetalnessMap )
    defines << "METALNESS_MAP";
  else
    defines << "METALNESS";
  if ( mUsingEmissionMap )
    defines << "EMISSION_MAP";
  if ( mEffect->parameters().contains( mAnisotropyParameter ) )
    defines << "ANISOTROPY";
  if ( mEffect->parameters().contains( mClearCoatFactorParameter ) )
    defines << "CLEAR_COAT";

  return defines;
}

///@endcond PRIVATE
