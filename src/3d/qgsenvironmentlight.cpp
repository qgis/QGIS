/***************************************************************************
  qgsenvironmentlight.cpp
  --------------------------------------
  Date                 : May 2026
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

#include "qgsenvironmentlight.h"

#include "qgsframegraph.h"

#include <QString>
#include <QVector3D>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

#include "moc_qgsenvironmentlight.cpp"

using namespace Qt::StringLiterals;

QgsEnvironmentLight::QgsEnvironmentLight( QgsFrameGraph *frameGraph, QNode *parent )
  : Qt3DCore::QEntity( parent )
{
  const QVariantList sh( 9, QVector3D( 0, 0, 0 ) );
  mShParam = new Qt3DRender::QParameter( u"envLightSh[0]"_s, QVariant::fromValue( sh ), this );

  mDummyCubeMap = new Qt3DRender::QTextureCubeMap( this );
  mDummyCubeMap->setFormat( Qt3DRender::QAbstractTexture::RGBA8_UNorm );
  mDummyCubeMap->setWidth( 1 );
  mDummyCubeMap->setHeight( 1 );
  mDummyCubeMap->setGenerateMipMaps( false );
  mDummyCubeMap->setMagnificationFilter( Qt3DRender::QAbstractTexture::Nearest );
  mDummyCubeMap->setMinificationFilter( Qt3DRender::QAbstractTexture::Nearest );

  mSpecularMapParam = new Qt3DRender::QParameter( u"globalSpecularMap"_s, QVariant::fromValue( mDummyCubeMap ), this );
  mMipLevelsParam = new Qt3DRender::QParameter( u"globalSpecularMipLevels"_s, 1, this );

  mEnvironmentLightModeParam = new Qt3DRender::QParameter( u"envLightMode"_s, 0, this );
  mEnvironmentLightStrengthParam = new Qt3DRender::QParameter( u"envLightStrength"_s, 1, this );
  frameGraph->addGlobalParameters( { mShParam, mSpecularMapParam, mMipLevelsParam, mEnvironmentLightModeParam, mEnvironmentLightStrengthParam } );
}

void QgsEnvironmentLight::setMode( Mode mode )
{
  switch ( mode )
  {
    case Mode::Disabled:
      mEnvironmentLightModeParam->setValue( 0 );
      break;
    case Mode::SpecularMapWithSphericalHarmonics:
      mEnvironmentLightModeParam->setValue( 1 );
      break;
  }
}

void QgsEnvironmentLight::setStrength( float strength )
{
  mEnvironmentLightStrengthParam->setValue( strength );
}

void QgsEnvironmentLight::setSphericalHarmonics( const QVector<QVector3D> &harmonics )
{
  Q_ASSERT( harmonics.size() == 9 );
  QVariantList value;
  value.reserve( 9 );
  for ( const QVector3D &vector : harmonics )
  {
    value << vector;
  }
  mShParam->setValue( QVariant::fromValue( value ) );
}

void QgsEnvironmentLight::setSpecularMap( Qt3DRender::QTextureCubeMap *specularTexture, int mipLevels )
{
  mSpecularMapParam->setValue( specularTexture ? QVariant::fromValue( specularTexture ) : QVariant::fromValue( mDummyCubeMap ) );
  mMipLevelsParam->setValue( mipLevels );
}
