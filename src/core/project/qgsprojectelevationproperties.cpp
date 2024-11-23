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
#include "moc_qgsprojectelevationproperties.cpp"
#include "qgis.h"
#include "qgsterrainprovider.h"

#include <QDomElement>

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
  const QDomElement providerElement = element.firstChildElement( QStringLiteral( "terrainProvider" ) );
  if ( !providerElement.isNull() )
  {
    const QString type = providerElement.attribute( QStringLiteral( "type" ) );
    if ( type.compare( QLatin1String( "flat" ) ) == 0 )
      mTerrainProvider = std::make_unique< QgsFlatTerrainProvider >();
    else if ( type.compare( QLatin1String( "raster" ) ) == 0 )
      mTerrainProvider = std::make_unique< QgsRasterDemTerrainProvider >();
    else if ( type.compare( QLatin1String( "mesh" ) ) == 0 )
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
  const double storedRangeLower = element.attribute( QStringLiteral( "RangeLower" ) ).toDouble( &ok );
  if ( ok )
    rangeLower = storedRangeLower;
  double rangeUpper = std::numeric_limits< double >::max();
  const double storedRangeUpper = element.attribute( QStringLiteral( "RangeUpper" ) ).toDouble( &ok );
  if ( ok )
    rangeUpper = storedRangeUpper;
  mElevationRange = QgsDoubleRange( rangeLower, rangeUpper );

  mElevationFilterRangeSize = element.attribute( QStringLiteral( "FilterRangeSize" ) ).toDouble( &ok );
  if ( !ok )
    mElevationFilterRangeSize = -1;

  mInvertElevationFilter = element.attribute( QStringLiteral( "FilterInvertSlider" ), QStringLiteral( "0" ) ).toInt();

  emit changed();
  emit elevationRangeChanged( mElevationRange );
  return true;
}

QDomElement QgsProjectElevationProperties::writeXml( QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QDomElement element = document.createElement( QStringLiteral( "ElevationProperties" ) );

  if ( mTerrainProvider )
  {
    QDomElement providerElement = document.createElement( QStringLiteral( "terrainProvider" ) );
    providerElement.setAttribute( QStringLiteral( "type" ), mTerrainProvider->type() );
    providerElement.appendChild( mTerrainProvider->writeXml( document, context ) );
    element.appendChild( providerElement );
  }

  if ( mElevationRange.lower() != std::numeric_limits< double >::lowest() )
    element.setAttribute( QStringLiteral( "RangeLower" ), qgsDoubleToString( mElevationRange.lower() ) );
  if ( mElevationRange.upper() != std::numeric_limits< double >::max() )
    element.setAttribute( QStringLiteral( "RangeUpper" ), qgsDoubleToString( mElevationRange.upper() ) );

  if ( mElevationFilterRangeSize >= 0 )
  {
    element.setAttribute( QStringLiteral( "FilterRangeSize" ), mElevationFilterRangeSize );
  }
  element.setAttribute( QStringLiteral( "FilterInvertSlider" ), mInvertElevationFilter ? "1" : "0" );

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
