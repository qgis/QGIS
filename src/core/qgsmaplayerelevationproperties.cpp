/***************************************************************************
                         qgsmaplayerelevationproperties.cpp
                         ---------------
    begin                : November 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerelevationproperties.h"
#include <mutex>


QgsPropertiesDefinition QgsMapLayerElevationProperties::sPropertyDefinitions;

QgsMapLayerElevationProperties::QgsMapLayerElevationProperties( QObject *parent )
  : QObject( parent )
{
}

bool QgsMapLayerElevationProperties::hasElevation() const
{
  return false;
}

void QgsMapLayerElevationProperties::setDefaultsFromLayer( QgsMapLayer * )
{

}

QString QgsMapLayerElevationProperties::htmlSummary() const
{
  return QString();
}

void QgsMapLayerElevationProperties::writeCommonProperties( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext & )
{
  QDomElement elemDataDefinedProperties = doc.createElement( QStringLiteral( "data-defined-properties" ) );
  mDataDefinedProperties.writeXml( elemDataDefinedProperties, propertyDefinitions() );
  element.appendChild( elemDataDefinedProperties );

  element.setAttribute( QStringLiteral( "zoffset" ), qgsDoubleToString( mZOffset ) );
  element.setAttribute( QStringLiteral( "zscale" ), qgsDoubleToString( mZScale ) );
}

void QgsMapLayerElevationProperties::readCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  const QDomElement elemDataDefinedProperties = element.firstChildElement( QStringLiteral( "data-defined-properties" ) );
  if ( !elemDataDefinedProperties.isNull() )
    mDataDefinedProperties.readXml( elemDataDefinedProperties, propertyDefinitions() );

  mZOffset = element.attribute( QStringLiteral( "zoffset" ), QStringLiteral( "0" ) ).toDouble();
  mZScale = element.attribute( QStringLiteral( "zscale" ), QStringLiteral( "1" ) ).toDouble();
}

void QgsMapLayerElevationProperties::copyCommonProperties( const QgsMapLayerElevationProperties *other )
{
  mDataDefinedProperties = other->dataDefinedProperties();
  mZScale = other->zScale();
  mZOffset = other->zOffset();
}

bool QgsMapLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange & ) const
{
  return true;
}

QgsDoubleRange QgsMapLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  return QgsDoubleRange();
}

bool QgsMapLayerElevationProperties::showByDefaultInElevationProfilePlots() const
{
  return false;
}

void QgsMapLayerElevationProperties::setZOffset( double offset )
{
  if ( qgsDoubleNear( offset, mZOffset ) )
    return;

  mZOffset = offset;
  emit changed();
  emit zOffsetChanged();
  emit profileGenerationPropertyChanged();
}

void QgsMapLayerElevationProperties::setZScale( double scale )
{
  if ( qgsDoubleNear( scale, mZScale ) )
    return;

  mZScale = scale;
  emit changed();
  emit zScaleChanged();
  emit profileGenerationPropertyChanged();
}

void QgsMapLayerElevationProperties::setDataDefinedProperties( const QgsPropertyCollection &collection )
{
  if ( mDataDefinedProperties == collection )
    return;

  mDataDefinedProperties = collection;
  emit changed();
  emit profileGenerationPropertyChanged();
}

QgsPropertiesDefinition QgsMapLayerElevationProperties::propertyDefinitions()
{
  static std::once_flag initialized;
  std::call_once( initialized, [ = ]( )
  {
    initPropertyDefinitions();
  } );
  return sPropertyDefinitions;
}

void QgsMapLayerElevationProperties::initPropertyDefinitions()
{
  const QString origin = QStringLiteral( "elevation" );

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { QgsMapLayerElevationProperties::ZOffset, QgsPropertyDefinition( "ZOffset", QObject::tr( "Offset" ), QgsPropertyDefinition::Double, origin ) },
    { QgsMapLayerElevationProperties::ExtrusionHeight, QgsPropertyDefinition( "ExtrusionHeight", QObject::tr( "Extrusion height" ), QgsPropertyDefinition::DoublePositive, origin ) },
  };
}
