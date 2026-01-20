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

#include "moc_qgsmaplayerelevationproperties.cpp"

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
  QDomElement elemDataDefinedProperties = doc.createElement( u"data-defined-properties"_s );
  mDataDefinedProperties.writeXml( elemDataDefinedProperties, propertyDefinitions() );
  element.appendChild( elemDataDefinedProperties );

  element.setAttribute( u"zoffset"_s, qgsDoubleToString( mZOffset ) );
  element.setAttribute( u"zscale"_s, qgsDoubleToString( mZScale ) );
}

void QgsMapLayerElevationProperties::readCommonProperties( const QDomElement &element, const QgsReadWriteContext & )
{
  const QDomElement elemDataDefinedProperties = element.firstChildElement( u"data-defined-properties"_s );
  if ( !elemDataDefinedProperties.isNull() )
    mDataDefinedProperties.readXml( elemDataDefinedProperties, propertyDefinitions() );

  mZOffset = element.attribute( u"zoffset"_s, u"0"_s ).toDouble();
  mZScale = element.attribute( u"zscale"_s, u"1"_s ).toDouble();
}

void QgsMapLayerElevationProperties::copyCommonProperties( const QgsMapLayerElevationProperties *other )
{
  mDataDefinedProperties = other->dataDefinedProperties();
  mZScale = other->zScale();
  mZOffset = other->zOffset();
}

bool QgsMapLayerElevationProperties::isVisibleInZRange( const QgsDoubleRange &, QgsMapLayer * ) const
{
  return true;
}

QgsDoubleRange QgsMapLayerElevationProperties::calculateZRange( QgsMapLayer * ) const
{
  return QgsDoubleRange();
}

QList<double> QgsMapLayerElevationProperties::significantZValues( QgsMapLayer * ) const
{
  return {};
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
  std::call_once( initialized, initPropertyDefinitions );
  return sPropertyDefinitions;
}

void QgsMapLayerElevationProperties::initPropertyDefinitions()
{
  const QString origin = u"elevation"_s;

  sPropertyDefinitions = QgsPropertiesDefinition
  {
    { static_cast< int >( QgsMapLayerElevationProperties::Property::ZOffset ), QgsPropertyDefinition( "ZOffset", QObject::tr( "Offset" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsMapLayerElevationProperties::Property::ExtrusionHeight ), QgsPropertyDefinition( "ExtrusionHeight", QObject::tr( "Extrusion height" ), QgsPropertyDefinition::DoublePositive, origin ) },
    { static_cast< int >( QgsMapLayerElevationProperties::Property::RasterPerBandLowerElevation ), QgsPropertyDefinition( "RasterPerBandLowerElevation", QObject::tr( "Lower elevation for band" ), QgsPropertyDefinition::Double, origin ) },
    { static_cast< int >( QgsMapLayerElevationProperties::Property::RasterPerBandUpperElevation ), QgsPropertyDefinition( "RasterPerBandUpperElevation", QObject::tr( "Upper elevation for band" ), QgsPropertyDefinition::Double, origin ) },
  };
}
