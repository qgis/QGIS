/***************************************************************************
                         qgsprojectelevationproperties.cpp
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsprojectelevationproperties.h"

#include "qgis.h"
#include "qgsterrainprovider.h"

#include <QDomElement>

#include "moc_qgsprojectelevationproperties.cpp"

QgsProjectElevationProperties::QgsProjectElevationProperties( QObject *parent )
  : QObject( parent )
  , mTerrainProvider( std::make_unique< QgsFlatTerrainProvider >() )
{

}

QgsProjectElevationProperties::~QgsProjectElevationProperties() = default;

void QgsProjectElevationProperties::reset()
{
  mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();
  mElevationRange = QgsDoubleRange();
  emit changed();
  emit elevationRangeChanged( mElevationRange );
}

void QgsProjectElevationProperties::resolveReferences( const QgsProject *project )
{
  if ( mTerrainProvider )
    mTerrainProvider->resolveReferences( project );
}

bool QgsProjectElevationProperties::readXml( const QDomElement &element, const QgsReadWriteContext &context )
{
  const QDomElement providerElement = element.firstChildElement( u"terrainProvider"_s );
  if ( !providerElement.isNull() )
  {
    const QString type = providerElement.attribute( u"type"_s );
    if ( type.compare( "flat"_L1 ) == 0 )
      mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();
    else if ( type.compare( "raster"_L1 ) == 0 )
      mTerrainProvider = std::make_unique< QgsRasterDemTerrainProvider >();
    else if ( type.compare( "mesh"_L1 ) == 0 )
      mTerrainProvider = std::make_unique< QgsMeshTerrainProvider >();
    else
      mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();

    mTerrainProvider->readXml( providerElement, context );
  }
  else
  {
    mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();
  }

  bool ok = false;
  double rangeLower = std::numeric_limits< double >::lowest();
  const double storedRangeLower = element.attribute( u"RangeLower"_s ).toDouble( &ok );
  if ( ok )
    rangeLower = storedRangeLower;
  double rangeUpper = std::numeric_limits< double >::max();
  const double storedRangeUpper = element.attribute( u"RangeUpper"_s ).toDouble( &ok );
  if ( ok )
    rangeUpper = storedRangeUpper;
  mElevationRange = QgsDoubleRange( rangeLower, rangeUpper );

  mElevationFilterRangeSize = element.attribute( u"FilterRangeSize"_s ).toDouble( &ok );
  if ( !ok )
    mElevationFilterRangeSize = -1;

  mInvertElevationFilter = element.attribute( u"FilterInvertSlider"_s, u"0"_s ).toInt();

  emit changed();
  emit elevationRangeChanged( mElevationRange );
  return true;
}

QDomElement QgsProjectElevationProperties::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( u"ElevationProperties"_s );

  if ( mTerrainProvider )
  {
    QDomElement providerElement = document.createElement( u"terrainProvider"_s );
    providerElement.setAttribute( u"type"_s, mTerrainProvider->type() );
    providerElement.appendChild( mTerrainProvider->writeXml( document, context ) );
    element.appendChild( providerElement );
  }

  if ( mElevationRange.lower() != std::numeric_limits< double >::lowest() )
    element.setAttribute( u"RangeLower"_s, qgsDoubleToString( mElevationRange.lower() ) );
  if ( mElevationRange.upper() != std::numeric_limits< double >::max() )
    element.setAttribute( u"RangeUpper"_s, qgsDoubleToString( mElevationRange.upper() ) );

  if ( mElevationFilterRangeSize >= 0 )
  {
    element.setAttribute( u"FilterRangeSize"_s, mElevationFilterRangeSize );
  }
  element.setAttribute( u"FilterInvertSlider"_s, mInvertElevationFilter ? "1" : "0" );

  return element;
}

QgsAbstractTerrainProvider *QgsProjectElevationProperties::terrainProvider()
{
  return mTerrainProvider.get();
}

void QgsProjectElevationProperties::setTerrainProvider( QgsAbstractTerrainProvider *provider )
{
  if ( mTerrainProvider.get() == provider )
    return;

  const bool hasChanged = ( provider && mTerrainProvider ) ? !mTerrainProvider->equals( provider ) : ( static_cast< bool >( provider ) != static_cast< bool >( mTerrainProvider.get() ) );

  mTerrainProvider.reset( provider );
  if ( hasChanged )
    emit changed();
}

void QgsProjectElevationProperties::setElevationFilterRangeSize( double size )
{
  if ( mElevationFilterRangeSize == size )
    return;

  mElevationFilterRangeSize = size;
  emit changed();
}

void QgsProjectElevationProperties::setInvertElevationFilter( bool invert )
{
  if ( mInvertElevationFilter == invert )
    return;

  mInvertElevationFilter = invert;
  emit changed();
}

void QgsProjectElevationProperties::setElevationRange( const QgsDoubleRange &range )
{
  if ( mElevationRange == range )
    return;

  mElevationRange = range;
  emit changed();
  emit elevationRangeChanged( mElevationRange );
}
