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

#include "qgscolorutils.h"
#include "qgssymbollayerutils.h"

#include <QDomDocument>
#include <Qt3DCore/QEntity>
#include <Qt3DRender/QDirectionalLight>

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

  Qt3DRender::QDirectionalLight *light = new Qt3DRender::QDirectionalLight;
  light->setColor( color() );
  light->setIntensity( intensity() );
  QgsVector3D direction = QgsDirectionalLightSettings::direction();
  light->setWorldDirection( QVector3D( direction.x(), direction.y(), direction.z() ) );

  lightEntity->addComponent( light );

  return lightEntity;
}

QDomElement QgsDirectionalLightSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext & ) const
{
  QDomElement elemLight = doc.createElement( u"directional-light"_s );
  elemLight.setAttribute( u"x"_s, mDirection.x() );
  elemLight.setAttribute( u"y"_s, mDirection.y() );
  elemLight.setAttribute( u"z"_s, mDirection.z() );
  elemLight.setAttribute( u"color"_s, QgsColorUtils::colorToString( mColor ) );
  elemLight.setAttribute( u"intensity"_s, mIntensity );
  return elemLight;
}

void QgsDirectionalLightSettings::readXml( const QDomElement &elem, const QgsReadWriteContext & )
{
  mDirection.set( elem.attribute( u"x"_s ).toFloat(), elem.attribute( u"y"_s ).toFloat(), elem.attribute( u"z"_s ).toFloat() );
  mColor = QgsColorUtils::colorFromString( elem.attribute( u"color"_s ) );
  mIntensity = elem.attribute( u"intensity"_s ).toFloat();
}

bool QgsDirectionalLightSettings::operator==( const QgsDirectionalLightSettings &other ) const
{
  return mDirection == other.mDirection && mColor == other.mColor && mIntensity == other.mIntensity;
}
