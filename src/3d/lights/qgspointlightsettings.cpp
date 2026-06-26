/***************************************************************************
  qgspointlightsettings.cpp
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointlightsettings.h"

#include "qgs3dmapsettings.h"
#include "qgs3dutils.h"
#include "qgscolorutils.h"
#include "qgsgeotransform.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <QString>
#include <Qt3DCore/QEntity>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSphereMesh>
#include <Qt3DRender/QPointLight>

using namespace Qt::StringLiterals;

Qgis::LightSourceType QgsPointLightSettings::type() const
{
  return Qgis::LightSourceType::Point;
}

QgsPointLightSettings *QgsPointLightSettings::clone() const
{
  auto res = std::make_unique< QgsPointLightSettings >( *this );
  res->mId = mId;
  return res.release();
}

Qt3DCore::QEntity *QgsPointLightSettings::createEntity( const Qgs3DMapSettings &map, Qt3DCore::QEntity *parent ) const
{
  Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity();
  QgsGeoTransform *lightTransform = new QgsGeoTransform;
  lightTransform->setOrigin( map.origin() );
  lightTransform->setGeoTranslation( position().toVector3D() );

  Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight;
  light->setColor( Qgs3DUtils::srgbToLinear( color() ) );
  light->setIntensity( static_cast< float >( intensity() ) );

  light->setConstantAttenuation( static_cast< float >( constantAttenuation() ) );
  light->setLinearAttenuation( static_cast< float >( linearAttenuation() ) );
  light->setQuadraticAttenuation( static_cast< float >( quadraticAttenuation() ) );

  lightEntity->addComponent( light );
  lightEntity->addComponent( lightTransform );

  if ( !map.debugFlags().testFlag( Qgis::Map3DDebugFlag::ShowLightSourceOrigins ) )
  {
    lightEntity->setParent( parent );
    return lightEntity;
  }
  else
  {
    Qt3DCore::QEntity *originEntity = new Qt3DCore::QEntity();

    QgsGeoTransform *originTransform = new QgsGeoTransform;
    originTransform->setOrigin( map.origin() );
    originTransform->setGeoTranslation( position().toVector3D() );
    originEntity->addComponent( originTransform );

    Qt3DExtras::QPhongMaterial *materialLightOriginCenter = new Qt3DExtras::QPhongMaterial;
    materialLightOriginCenter->setAmbient( color() );
    originEntity->addComponent( materialLightOriginCenter );

    Qt3DExtras::QSphereMesh *rendererLightOriginCenter = new Qt3DExtras::QSphereMesh;
    rendererLightOriginCenter->setRadius( 20 );
    originEntity->addComponent( rendererLightOriginCenter );

    originEntity->setEnabled( true );

    Qt3DCore::QEntity *groupEntity = new Qt3DCore::QEntity( parent );
    lightEntity->setParent( groupEntity );
    originEntity->setParent( groupEntity );
    groupEntity->setEnabled( true );
    return groupEntity;
  }
}

QDomElement QgsPointLightSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement elemLight = doc.createElement( u"point-light"_s );
  elemLight.setAttribute( u"id"_s, mId );
  elemLight.setAttribute( u"x"_s, mPosition.x() );
  elemLight.setAttribute( u"y"_s, mPosition.y() );
  elemLight.setAttribute( u"z"_s, mPosition.z() );
  elemLight.setAttribute( u"color"_s, QgsColorUtils::colorToString( mColor ) );
  elemLight.setAttribute( u"intensity"_s, mIntensity );
  elemLight.setAttribute( u"attenuation-0"_s, mConstantAttenuation );
  elemLight.setAttribute( u"attenuation-1"_s, mLinearAttenuation );
  elemLight.setAttribute( u"attenuation-2"_s, mQuadraticAttenuation );
  return elemLight;
}

void QgsPointLightSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  if ( elem.hasAttribute( u"id"_s ) )
    mId = elem.attribute( u"id"_s );

  mPosition.set( elem.attribute( u"x"_s ).toDouble(), elem.attribute( u"y"_s ).toDouble(), elem.attribute( u"z"_s ).toDouble() );
  mColor = QgsColorUtils::colorFromString( elem.attribute( u"color"_s ) );
  mIntensity = elem.attribute( u"intensity"_s ).toDouble();
  mConstantAttenuation = elem.attribute( u"attenuation-0"_s ).toDouble();
  mLinearAttenuation = elem.attribute( u"attenuation-1"_s ).toDouble();
  mQuadraticAttenuation = elem.attribute( u"attenuation-2"_s ).toDouble();
}

bool QgsPointLightSettings::operator==( const QgsPointLightSettings &other ) const
{
  return mId == other.mId
         && mPosition == other.mPosition
         && mColor == other.mColor
         && qgsDoubleNear( mIntensity, other.mIntensity )
         && qgsDoubleNear( mConstantAttenuation, other.mConstantAttenuation )
         && qgsDoubleNear( mLinearAttenuation, other.mLinearAttenuation )
         && qgsDoubleNear( mQuadraticAttenuation, other.mQuadraticAttenuation );
}
