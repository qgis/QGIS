/***************************************************************************
  qgsabstract3dsymbol.cpp
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

#include "qgsabstract3dsymbol.h"


QgsPropertiesDefinition QgsAbstract3DSymbol::sPropertyDefinitions;


QList<QgsWkbTypes::GeometryType> QgsAbstract3DSymbol::compatibleGeometryTypes() const
{
  return QList< QgsWkbTypes::GeometryType >();
}

const QgsPropertiesDefinition &QgsAbstract3DSymbol::propertyDefinitions()
{
  initPropertyDefinitions();
  return sPropertyDefinitions;
}

void QgsAbstract3DSymbol::copyBaseSettings( QgsAbstract3DSymbol *destination ) const
{
  destination->mDataDefinedProperties = mDataDefinedProperties;
}

void QgsAbstract3DSymbol::initPropertyDefinitions()
{
  if ( !sPropertyDefinitions.isEmpty() )
    return;

  const QString origin = QStringLiteral( "symbol3d" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { PropertyHeight, QgsPropertyDefinition( "height", QObject::tr( "Height" ), QgsPropertyDefinition::Double, origin ) },
    { PropertyExtrusionHeight, QgsPropertyDefinition( "extrusionHeight", QObject::tr( "ExtrusionHeight" ), QgsPropertyDefinition::DoublePositive, origin ) },
  };

}

bool QgsAbstract3DSymbol::exportGeometries( Qgs3DSceneExporter *exporter, Qt3DCore::QEntity *entity, const QString &objectNamePrefix ) const
{
  Q_UNUSED( exporter )
  Q_UNUSED( entity )
  Q_UNUSED( objectNamePrefix )
  return false;
}

void QgsAbstract3DSymbol::setDefaultPropertiesFromLayer( const QgsVectorLayer * )
{

}
