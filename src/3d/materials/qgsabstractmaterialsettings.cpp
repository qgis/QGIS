/***************************************************************************
  qgsabstractmaterialsettings.cpp
  --------------------------------------
  Date                 : July 2020
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

#include <qgsabstractmaterialsettings.h>

QgsPropertiesDefinition QgsAbstractMaterialSettings::sPropertyDefinitions;

void QgsAbstractMaterialSettings::readXml( const QDomElement &element, const QgsReadWriteContext & )
{
  const QDomElement elemDataDefinedProperties = element.firstChildElement( QStringLiteral( "data-defined-properties" ) );
  if ( !elemDataDefinedProperties.isNull() )
    mDataDefinedProperties.readXml( elemDataDefinedProperties, propertyDefinitions() );
}

void QgsAbstractMaterialSettings::writeXml( QDomElement &element, const QgsReadWriteContext & ) const
{
  QDomElement elemDataDefinedProperties = element.ownerDocument().createElement( QStringLiteral( "data-defined-properties" ) );
  mDataDefinedProperties.writeXml( elemDataDefinedProperties, propertyDefinitions() );
  element.appendChild( elemDataDefinedProperties );
}

void QgsAbstractMaterialSettings::setDataDefinedProperties( const QgsPropertyCollection &collection )
{
  mDataDefinedProperties = collection;
}

QgsPropertyCollection QgsAbstractMaterialSettings::dataDefinedProperties() const { return mDataDefinedProperties; }

const QgsPropertiesDefinition &QgsAbstractMaterialSettings::propertyDefinitions() const
{
  if ( sPropertyDefinitions.isEmpty() )
    initPropertyDefinitions();
  return sPropertyDefinitions;
}

QByteArray QgsAbstractMaterialSettings::dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const
{
  Q_UNUSED( expressionContext )
  return QByteArray();
}

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
void QgsAbstractMaterialSettings::applyDataDefinedToGeometry( Qt3DRender::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes ) const
#else
void QgsAbstractMaterialSettings::applyDataDefinedToGeometry( Qt3DCore::QGeometry *geometry, int vertexCount, const QByteArray &dataDefinedBytes ) const
#endif
{
  Q_UNUSED( geometry )
  Q_UNUSED( vertexCount )
  Q_UNUSED( dataDefinedBytes )
}

void QgsAbstractMaterialSettings::initPropertyDefinitions() const
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  const QString origin = QStringLiteral( "material3d" );

  sPropertyDefinitions = QgsPropertiesDefinition {
    { static_cast<int>( Property::Diffuse ), QgsPropertyDefinition( "diffuse", QObject::tr( "Diffuse" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { static_cast<int>( Property::Ambient ), QgsPropertyDefinition( "ambient", QObject::tr( "Ambient" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { static_cast<int>( Property::Warm ), QgsPropertyDefinition( "warm", QObject::tr( "Warm" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { static_cast<int>( Property::Cool ), QgsPropertyDefinition( "cool", QObject::tr( "Cool" ), QgsPropertyDefinition::ColorNoAlpha, origin ) },
    { static_cast<int>( Property::Specular ), QgsPropertyDefinition( "specular", QObject::tr( "Specular" ), QgsPropertyDefinition::ColorNoAlpha, origin ) }
  };
}
