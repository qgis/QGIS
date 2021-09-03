/***************************************************************************
                         qgslayoutmeasurementconverter.cpp
                         ---------------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutmeasurementconverter.h"


QgsLayoutMeasurement QgsLayoutMeasurementConverter::convert( const QgsLayoutMeasurement measurement, const QgsUnitTypes::LayoutUnit targetUnits ) const
{
  if ( measurement.units() == targetUnits )
  {
    return measurement;
  }

  switch ( targetUnits )
  {
    case QgsUnitTypes::LayoutMillimeters:
      return QgsLayoutMeasurement( convertToMillimeters( measurement ), QgsUnitTypes::LayoutMillimeters );
    case QgsUnitTypes::LayoutCentimeters:
      return QgsLayoutMeasurement( convertToCentimeters( measurement ), QgsUnitTypes::LayoutCentimeters );
    case QgsUnitTypes::LayoutMeters:
      return QgsLayoutMeasurement( convertToMeters( measurement ), QgsUnitTypes::LayoutMeters );
    case QgsUnitTypes::LayoutInches:
      return QgsLayoutMeasurement( convertToInches( measurement ), QgsUnitTypes::LayoutInches );
    case QgsUnitTypes::LayoutFeet:
      return QgsLayoutMeasurement( convertToFeet( measurement ), QgsUnitTypes::LayoutFeet );
    case QgsUnitTypes::LayoutPoints:
      return QgsLayoutMeasurement( convertToPoints( measurement ), QgsUnitTypes::LayoutPoints );
    case QgsUnitTypes::LayoutPicas:
      return QgsLayoutMeasurement( convertToPicas( measurement ), QgsUnitTypes::LayoutPicas );
    case QgsUnitTypes::LayoutPixels:
      return QgsLayoutMeasurement( convertToPixels( measurement ), QgsUnitTypes::LayoutPixels );
  }

  //will never be reached, but required to prevent warnings
  return QgsLayoutMeasurement( convertToMillimeters( measurement ), QgsUnitTypes::LayoutMillimeters );
}

QgsLayoutSize QgsLayoutMeasurementConverter::convert( const QgsLayoutSize &size, const QgsUnitTypes::LayoutUnit targetUnits ) const
{
  if ( size.units() == targetUnits )
  {
    return size;
  }

  QgsLayoutSize result( size );
  result.setUnits( targetUnits );
  const QgsLayoutMeasurement width = QgsLayoutMeasurement( size.width(), size.units() );
  const QgsLayoutMeasurement height = QgsLayoutMeasurement( size.height(), size.units() );
  switch ( targetUnits )
  {
    case QgsUnitTypes::LayoutMillimeters:
      result.setSize( convertToMillimeters( width ), convertToMillimeters( height ) );
      break;
    case QgsUnitTypes::LayoutCentimeters:
      result.setSize( convertToCentimeters( width ), convertToCentimeters( height ) );
      break;
    case QgsUnitTypes::LayoutMeters:
      result.setSize( convertToMeters( width ), convertToMeters( height ) );
      break;
    case QgsUnitTypes::LayoutInches:
      result.setSize( convertToInches( width ), convertToInches( height ) );
      break;
    case QgsUnitTypes::LayoutFeet:
      result.setSize( convertToFeet( width ), convertToFeet( height ) );
      break;
    case QgsUnitTypes::LayoutPoints:
      result.setSize( convertToPoints( width ), convertToPoints( height ) );
      break;
    case QgsUnitTypes::LayoutPicas:
      result.setSize( convertToPicas( width ), convertToPicas( height ) );
      break;
    case QgsUnitTypes::LayoutPixels:
      result.setSize( convertToPixels( width ), convertToPixels( height ) );
      break;
  }
  return result;
}

QgsLayoutPoint QgsLayoutMeasurementConverter::convert( const QgsLayoutPoint &point, const QgsUnitTypes::LayoutUnit targetUnits ) const
{
  if ( point.units() == targetUnits )
  {
    return point;
  }

  QgsLayoutPoint result( point );
  result.setUnits( targetUnits );
  const QgsLayoutMeasurement x = QgsLayoutMeasurement( point.x(), point.units() );
  const QgsLayoutMeasurement y = QgsLayoutMeasurement( point.y(), point.units() );
  switch ( targetUnits )
  {
    case QgsUnitTypes::LayoutMillimeters:
      result.setPoint( convertToMillimeters( x ), convertToMillimeters( y ) );
      break;
    case QgsUnitTypes::LayoutCentimeters:
      result.setPoint( convertToCentimeters( x ), convertToCentimeters( y ) );
      break;
    case QgsUnitTypes::LayoutMeters:
      result.setPoint( convertToMeters( x ), convertToMeters( y ) );
      break;
    case QgsUnitTypes::LayoutInches:
      result.setPoint( convertToInches( x ), convertToInches( y ) );
      break;
    case QgsUnitTypes::LayoutFeet:
      result.setPoint( convertToFeet( x ), convertToFeet( y ) );
      break;
    case QgsUnitTypes::LayoutPoints:
      result.setPoint( convertToPoints( x ), convertToPoints( y ) );
      break;
    case QgsUnitTypes::LayoutPicas:
      result.setPoint( convertToPicas( x ), convertToPicas( y ) );
      break;
    case QgsUnitTypes::LayoutPixels:
      result.setPoint( convertToPixels( x ), convertToPixels( y ) );
      break;
  }
  return result;
}

double QgsLayoutMeasurementConverter::convertToMillimeters( const QgsLayoutMeasurement measurement ) const
{
  switch ( measurement.units() )
  {
    case QgsUnitTypes::LayoutMillimeters:
      return measurement.length();
    case QgsUnitTypes::LayoutCentimeters:
      return measurement.length() * 10.0;
    case QgsUnitTypes::LayoutMeters:
      return measurement.length() * 1000.0;
    case QgsUnitTypes::LayoutInches:
      return measurement.length() * 25.4;
    case QgsUnitTypes::LayoutFeet:
      return measurement.length() * 304.8;
    case QgsUnitTypes::LayoutPoints:
      return measurement.length() * 0.352777778;
    case QgsUnitTypes::LayoutPicas:
      return measurement.length() * 4.23333333;
    case QgsUnitTypes::LayoutPixels:
      return measurement.length() * 25.4 / mDpi;
  }

  //will never be reached, but required to prevent warnings
  return measurement.length();
}

double QgsLayoutMeasurementConverter::convertToCentimeters( const QgsLayoutMeasurement measurement ) const
{
  return convertToMillimeters( measurement ) / 10.0;
}

double QgsLayoutMeasurementConverter::convertToMeters( const QgsLayoutMeasurement measurement ) const
{
  return convertToMillimeters( measurement ) / 1000.0;
}

double QgsLayoutMeasurementConverter::convertToInches( const QgsLayoutMeasurement measurement ) const
{
  return convertToMillimeters( measurement ) / 25.4;
}

double QgsLayoutMeasurementConverter::convertToFeet( const QgsLayoutMeasurement measurement ) const
{
  return convertToMillimeters( measurement ) / 304.8;
}

double QgsLayoutMeasurementConverter::convertToPoints( const QgsLayoutMeasurement measurement ) const
{
  return convertToMillimeters( measurement ) * 2.83464567;
}

double QgsLayoutMeasurementConverter::convertToPicas( const QgsLayoutMeasurement measurement ) const
{
  return convertToMillimeters( measurement ) * 0.236220472;
}

double QgsLayoutMeasurementConverter::convertToPixels( const QgsLayoutMeasurement measurement ) const
{
  return convertToMillimeters( measurement ) * mDpi / 25.4;
}
