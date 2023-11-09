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
#include "qgssymbollayerutils.h"
#include "qgs3dmapsettings.h"

#include <QDomDocument>

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QPointLight>
#include <Qt3DExtras/QPhongMaterial>
#include <Qt3DExtras/QSphereMesh>

Qgis::LightSourceType QgsPointLightSettings::type() const
{
  return Qgis::LightSourceType::Point;
}

QgsPointLightSettings *QgsPointLightSettings::clone() const
{
  return new QgsPointLightSettings( *this );
}

Qt3DCore::QEntity *QgsPointLightSettings::createEntity( const Qgs3DMapSettings &map, Qt3DCore::QEntity *parent ) const
{
  Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity();
  Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform;
  lightTransform->setTranslation( QVector3D( position().x(),
                                  position().y(),
                                  position().z() ) );

  Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight;
  light->setColor( color() );
  light->setIntensity( intensity() );

  light->setConstantAttenuation( constantAttenuation() );
  light->setLinearAttenuation( linearAttenuation() );
  light->setQuadraticAttenuation( quadraticAttenuation() );

  lightEntity->addComponent( light );
  lightEntity->addComponent( lightTransform );

  if ( !map.showLightSourceOrigins() )
  {
    lightEntity->setParent( parent );
    return lightEntity;
  }
  else
  {
    Qt3DCore::QEntity *originEntity = new Qt3DCore::QEntity();

    Qt3DCore::QTransform *trLightOriginCenter = new Qt3DCore::QTransform;
    trLightOriginCenter->setTranslation( lightTransform->translation() );
    originEntity->addComponent( trLightOriginCenter );

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
  QDomElement elemLight = doc.createElement( QStringLiteral( "point-light" ) );
  elemLight.setAttribute( QStringLiteral( "x" ), mPosition.x() );
  elemLight.setAttribute( QStringLiteral( "y" ), mPosition.y() );
  elemLight.setAttribute( QStringLiteral( "z" ), mPosition.z() );
  elemLight.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  elemLight.setAttribute( QStringLiteral( "intensity" ), mIntensity );
  elemLight.setAttribute( QStringLiteral( "attenuation-0" ), mConstantAttenuation );
  elemLight.setAttribute( QStringLiteral( "attenuation-1" ), mLinearAttenuation );
  elemLight.setAttribute( QStringLiteral( "attenuation-2" ), mQuadraticAttenuation );
  return elemLight;
}

void QgsPointLightSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mPosition.set( elem.attribute( QStringLiteral( "x" ) ).toDouble(),
                 elem.attribute( QStringLiteral( "y" ) ).toDouble(),
                 elem.attribute( QStringLiteral( "z" ) ).toDouble() );
  mColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "color" ) ) );
  mIntensity = elem.attribute( QStringLiteral( "intensity" ) ).toFloat();
  mConstantAttenuation = elem.attribute( QStringLiteral( "attenuation-0" ) ).toDouble();
  mLinearAttenuation = elem.attribute( QStringLiteral( "attenuation-1" ) ).toDouble();
  mQuadraticAttenuation = elem.attribute( QStringLiteral( "attenuation-2" ) ).toDouble();
}

bool QgsPointLightSettings::operator==( const QgsPointLightSettings &other )
{
  return mPosition == other.mPosition && mColor == other.mColor && mIntensity == other.mIntensity &&
         mConstantAttenuation == other.mConstantAttenuation && mLinearAttenuation == other.mLinearAttenuation &&
         mQuadraticAttenuation == other.mQuadraticAttenuation;
}
