/***************************************************************************
  qgsdirectionallightsettings.cpp
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdirectionallightsettings.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <Qt3DRender/QDirectionalLight>
#include <Qt3DCore/QEntity>

Qgis::LightSourceType QgsDirectionalLightSettings::type() const
{
  return Qgis::LightSourceType::Directional;
}

QgsDirectionalLightSettings *QgsDirectionalLightSettings::clone() const
{
  return new QgsDirectionalLightSettings( *this );
}

Qt3DCore::QEntity *QgsDirectionalLightSettings::createEntity( const Qgs3DMapSettings &, Qt3DCore::QEntity *parent ) const
{
  Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity( parent );
  Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform;

  Qt3DRender::QDirectionalLight *light = new Qt3DRender::QDirectionalLight;
  light->setColor( color() );
  light->setIntensity( intensity() );
  QgsVector3D direction = QgsDirectionalLightSettings::direction();
  light->setWorldDirection( QVector3D( direction.x(), direction.y(), direction.z() ) );

  lightEntity->addComponent( light );
  lightEntity->addComponent( lightTransform );

  return lightEntity;
}

QDomElement QgsDirectionalLightSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement elemLight = doc.createElement( QStringLiteral( "directional-light" ) );
  elemLight.setAttribute( QStringLiteral( "x" ), mDirection.x() );
  elemLight.setAttribute( QStringLiteral( "y" ), mDirection.y() );
  elemLight.setAttribute( QStringLiteral( "z" ), mDirection.z() );
  elemLight.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  elemLight.setAttribute( QStringLiteral( "intensity" ), mIntensity );
  return elemLight;
}

void QgsDirectionalLightSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mDirection.set( elem.attribute( QStringLiteral( "x" ) ).toFloat(),
                  elem.attribute( QStringLiteral( "y" ) ).toFloat(),
                  elem.attribute( QStringLiteral( "z" ) ).toFloat() );
  mColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "color" ) ) );
  mIntensity = elem.attribute( QStringLiteral( "intensity" ) ).toFloat();
}

bool QgsDirectionalLightSettings::operator==( const QgsDirectionalLightSettings &other )
{
  return mDirection == other.mDirection && mColor == other.mColor && mIntensity == other.mIntensity;
}
