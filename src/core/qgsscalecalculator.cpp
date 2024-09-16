/***************************************************************************
                              qgsscalecalculator.h
                 Calculates scale based on map extent and units
                              -------------------
  begin                : May 18, 2004
  copyright            : (C) 2004 by Gary E.Sherman
  email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <cmath>
#include "qgslogger.h"
#include "qgsscalecalculator.h"
#include "qgsrectangle.h"
#include "qgsunittypes.h"
#include <QSizeF>

QgsScaleCalculator::QgsScaleCalculator( double dpi, Qgis::DistanceUnit mapUnits )
  : mDpi( dpi )
  , mMapUnits( mapUnits )
{}

void QgsScaleCalculator::setDpi( double dpi )
{
  mDpi = dpi;
}
double QgsScaleCalculator::dpi() const
{
  return mDpi;
}

void QgsScaleCalculator::setMapUnits( Qgis::DistanceUnit mapUnits )
{
  QgsDebugMsgLevel( QStringLiteral( "Map units set to %1" ).arg( qgsEnumValueToKey( mapUnits ) ), 3 );
  mMapUnits = mapUnits;
}

Qgis::DistanceUnit QgsScaleCalculator::mapUnits() const
{
  QgsDebugMsgLevel( QStringLiteral( "Map units returned as %1" ).arg( qgsEnumValueToKey( mMapUnits ) ), 4 );
  return mMapUnits;
}

double QgsScaleCalculator::calculate( const QgsRectangle &mapExtent, double canvasWidth )  const
{
  if ( qgsDoubleNear( canvasWidth, 0. ) || qgsDoubleNear( mDpi, 0.0 ) )
  {
    QgsDebugError( QStringLiteral( "Can't calculate scale from the input values" ) );
    return 0;
  }

  double conversionFactor = 0;
  double delta = 0;
  calculateMetrics( mapExtent, delta, conversionFactor );

  const double scale = ( delta * conversionFactor ) / ( static_cast< double >( canvasWidth ) / mDpi );
  QgsDebugMsgLevel( QStringLiteral( "scale = %1 conversionFactor = %2" ).arg( scale ).arg( conversionFactor ), 4 );
  return scale;
}

QSizeF QgsScaleCalculator::calculateImageSize( const QgsRectangle &mapExtent, double scale )  const
{
  if ( qgsDoubleNear( scale, 0.0 ) || qgsDoubleNear( mDpi, 0.0 ) )
  {
    QgsDebugError( QStringLiteral( "Can't calculate image size from the input values" ) );
    return QSizeF();
  }
  double conversionFactor = 0;
  double delta = 0;

  calculateMetrics( mapExtent, delta, conversionFactor );
  const double imageWidth = ( delta * conversionFactor ) / ( static_cast< double >( scale ) ) * mDpi;
  const double deltaHeight = ( mapExtent.yMaximum() - mapExtent.yMinimum() ) * delta / ( mapExtent.xMaximum() - mapExtent.xMinimum() );
  const double imageHeight = ( deltaHeight * conversionFactor ) / ( static_cast< double >( scale ) ) * mDpi;

  QgsDebugMsgLevel( QStringLiteral( "imageWidth = %1 imageHeight = %2 conversionFactor = %3" )
                    .arg( imageWidth ).arg( imageHeight ).arg( conversionFactor ), 4 );

  return QSizeF( imageWidth, imageHeight );
}

void QgsScaleCalculator::calculateMetrics( const QgsRectangle &mapExtent, double &delta, double &conversionFactor ) const
{
  delta = mapExtent.xMaximum() - mapExtent.xMinimum();

  switch ( mMapUnits )
  {
    case Qgis::DistanceUnit::Inches:
      conversionFactor = 1;
      break;

    case Qgis::DistanceUnit::Meters:
    case Qgis::DistanceUnit::Kilometers:
    case Qgis::DistanceUnit::Feet:
    case Qgis::DistanceUnit::Yards:
    case Qgis::DistanceUnit::Millimeters:
    case Qgis::DistanceUnit::Centimeters:
    case Qgis::DistanceUnit::Miles:
    case Qgis::DistanceUnit::NauticalMiles:
    case Qgis::DistanceUnit::ChainsInternational:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895A:
    case Qgis::DistanceUnit::ChainsBritishBenoit1895B:
    case Qgis::DistanceUnit::ChainsBritishSears1922Truncated:
    case Qgis::DistanceUnit::ChainsBritishSears1922:
    case Qgis::DistanceUnit::ChainsClarkes:
    case Qgis::DistanceUnit::ChainsUSSurvey:
    case Qgis::DistanceUnit::FeetBritish1865:
    case Qgis::DistanceUnit::FeetBritish1936:
    case Qgis::DistanceUnit::FeetBritishBenoit1895A:
    case Qgis::DistanceUnit::FeetBritishBenoit1895B:
    case Qgis::DistanceUnit::FeetBritishSears1922Truncated:
    case Qgis::DistanceUnit::FeetBritishSears1922:
    case Qgis::DistanceUnit::FeetClarkes:
    case Qgis::DistanceUnit::FeetGoldCoast:
    case Qgis::DistanceUnit::FeetIndian:
    case Qgis::DistanceUnit::FeetIndian1937:
    case Qgis::DistanceUnit::FeetIndian1962:
    case Qgis::DistanceUnit::FeetIndian1975:
    case Qgis::DistanceUnit::FeetUSSurvey:
    case Qgis::DistanceUnit::LinksInternational:
    case Qgis::DistanceUnit::LinksBritishBenoit1895A:
    case Qgis::DistanceUnit::LinksBritishBenoit1895B:
    case Qgis::DistanceUnit::LinksBritishSears1922Truncated:
    case Qgis::DistanceUnit::LinksBritishSears1922:
    case Qgis::DistanceUnit::LinksClarkes:
    case Qgis::DistanceUnit::LinksUSSurvey:
    case Qgis::DistanceUnit::YardsBritishBenoit1895A:
    case Qgis::DistanceUnit::YardsBritishBenoit1895B:
    case Qgis::DistanceUnit::YardsBritishSears1922Truncated:
    case Qgis::DistanceUnit::YardsBritishSears1922:
    case Qgis::DistanceUnit::YardsClarkes:
    case Qgis::DistanceUnit::YardsIndian:
    case Qgis::DistanceUnit::YardsIndian1937:
    case Qgis::DistanceUnit::YardsIndian1962:
    case Qgis::DistanceUnit::YardsIndian1975:
    case Qgis::DistanceUnit::MilesUSSurvey:
    case Qgis::DistanceUnit::Fathoms:
    case Qgis::DistanceUnit::MetersGermanLegal:
      // convert to inches
      conversionFactor = QgsUnitTypes::fromUnitToUnitFactor( mMapUnits, Qgis::DistanceUnit::Inches );
      break;

    case Qgis::DistanceUnit::Unknown:
      // assume degrees to maintain old API
      [[fallthrough]];

    case Qgis::DistanceUnit::Degrees:
      // degrees require conversion to meters first
      conversionFactor = 39.3700787;
      delta = calculateGeographicDistance( mapExtent );
      break;
  }
}

double QgsScaleCalculator::calculateGeographicDistance( const QgsRectangle &mapExtent ) const
{
  // need to calculate the x distance in meters
  // We'll use the middle latitude for the calculation
  // Note this is an approximation (although very close) but calculating scale
  // for geographic data over large extents is quasi-meaningless

  // The distance between two points on a sphere can be estimated
  // using the Haversine formula. This gives the shortest distance
  // between two points on the sphere. However, what we're after is
  // the distance from the left of the given extent and the right of
  // it. This is not necessarily the shortest distance between two
  // points on a sphere.
  //
  // The code below uses the Haversine formula, but with some changes
  // to cope with the above problem, and also to deal with the extent
  // possibly extending beyond +/-180 degrees:
  //
  // - Use the Halversine formula to calculate the distance from -90 to
  //   +90 degrees at the mean latitude.
  // - Scale this distance by the number of degrees between
  //   mapExtent.xMinimum() and mapExtent.xMaximum();
  // - For a slight improvemnt, allow for the ellipsoid shape of earth.


  // For a longitude change of 180 degrees
  const double lat = ( mapExtent.yMaximum() + mapExtent.yMinimum() ) * 0.5;
  static const double RADS = ( 4.0 * std::atan( 1.0 ) ) / 180.0;
  const double a = std::pow( std::cos( lat * RADS ), 2 );
  const double c = 2.0 * std::atan2( std::sqrt( a ), std::sqrt( 1.0 - a ) );
  static const double RA = 6378000; // [m]
  // The eccentricity. This comes from sqrt(1.0 - rb*rb/(ra*ra)) with rb set
  // to 6357000 m.
  static const double E = 0.0810820288;
  const double radius = RA * ( 1.0 - E * E ) /
                        std::pow( 1.0 - E * E * std::sin( lat * RADS ) * std::sin( lat * RADS ), 1.5 );
  const double meters = ( mapExtent.xMaximum() - mapExtent.xMinimum() ) / 180.0 * radius * c;

  QgsDebugMsgLevel( "Distance across map extent (m): " + QString::number( meters ), 4 );

  return meters;
}
