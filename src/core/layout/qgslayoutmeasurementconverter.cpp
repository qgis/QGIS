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


QgsLayoutMeasurement QgsLayoutMeasurementConverter::convert( const QgsLayoutMeasurement measurement, const Qgis::LayoutUnit targetUnits ) const
{
  if ( measurement.units() == targetUnits )
  {
    return measurement;
  }

  switch ( targetUnits )
  {
    case Qgis::LayoutUnit::Millimeters:
      return QgsLayoutMeasurement( convertToMillimeters( measurement ), Qgis::LayoutUnit::Millimeters );
    case Qgis::LayoutUnit::Centimeters:
      return QgsLayoutMeasurement( convertToCentimeters( measurement ), Qgis::LayoutUnit::Centimeters );
    case Qgis::LayoutUnit::Meters:
      return QgsLayoutMeasurement( convertToMeters( measurement ), Qgis::LayoutUnit::Meters );
    case Qgis::LayoutUnit::Inches:
      return QgsLayoutMeasurement( convertToInches( measurement ), Qgis::LayoutUnit::Inches );
    case Qgis::LayoutUnit::Feet:
      return QgsLayoutMeasurement( convertToFeet( measurement ), Qgis::LayoutUnit::Feet );
    case Qgis::LayoutUnit::Points:
      return QgsLayoutMeasurement( convertToPoints( measurement ), Qgis::LayoutUnit::Points );
    case Qgis::LayoutUnit::Picas:
      return QgsLayoutMeasurement( convertToPicas( measurement ), Qgis::LayoutUnit::Picas );
    case Qgis::LayoutUnit::Pixels:
      return QgsLayoutMeasurement( convertToPixels( measurement ), Qgis::LayoutUnit::Pixels );
  }

  //will never be reached, but required to prevent warnings
  return QgsLayoutMeasurement( convertToMillimeters( measurement ), Qgis::LayoutUnit::Millimeters );
}

QgsLayoutSize QgsLayoutMeasurementConverter::convert( const QgsLayoutSize &size, const Qgis::LayoutUnit targetUnits ) const
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
    case Qgis::LayoutUnit::Millimeters:
      result.setSize( convertToMillimeters( width ), convertToMillimeters( height ) );
      break;
    case Qgis::LayoutUnit::Centimeters:
      result.setSize( convertToCentimeters( width ), convertToCentimeters( height ) );
      break;
    case Qgis::LayoutUnit::Meters:
      result.setSize( convertToMeters( width ), convertToMeters( height ) );
      break;
    case Qgis::LayoutUnit::Inches:
      result.setSize( convertToInches( width ), convertToInches( height ) );
      break;
    case Qgis::LayoutUnit::Feet:
      result.setSize( convertToFeet( width ), convertToFeet( height ) );
      break;
    case Qgis::LayoutUnit::Points:
      result.setSize( convertToPoints( width ), convertToPoints( height ) );
      break;
    case Qgis::LayoutUnit::Picas:
      result.setSize( convertToPicas( width ), convertToPicas( height ) );
      break;
    case Qgis::LayoutUnit::Pixels:
      result.setSize( convertToPixels( width ), convertToPixels( height ) );
      break;
  }
  return result;
}

QgsLayoutPoint QgsLayoutMeasurementConverter::convert( const QgsLayoutPoint &point, const Qgis::LayoutUnit targetUnits ) const
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
    case Qgis::LayoutUnit::Millimeters:
      result.setPoint( convertToMillimeters( x ), convertToMillimeters( y ) );
      break;
    case Qgis::LayoutUnit::Centimeters:
      result.setPoint( convertToCentimeters( x ), convertToCentimeters( y ) );
      break;
    case Qgis::LayoutUnit::Meters:
      result.setPoint( convertToMeters( x ), convertToMeters( y ) );
      break;
    case Qgis::LayoutUnit::Inches:
      result.setPoint( convertToInches( x ), convertToInches( y ) );
      break;
    case Qgis::LayoutUnit::Feet:
      result.setPoint( convertToFeet( x ), convertToFeet( y ) );
      break;
    case Qgis::LayoutUnit::Points:
      result.setPoint( convertToPoints( x ), convertToPoints( y ) );
      break;
    case Qgis::LayoutUnit::Picas:
      result.setPoint( convertToPicas( x ), convertToPicas( y ) );
      break;
    case Qgis::LayoutUnit::Pixels:
      result.setPoint( convertToPixels( x ), convertToPixels( y ) );
      break;
  }
  return result;
}

double QgsLayoutMeasurementConverter::convertToMillimeters( const QgsLayoutMeasurement measurement ) const
{
  switch ( measurement.units() )
  {
    case Qgis::LayoutUnit::Millimeters:
      return measurement.length();
    case Qgis::LayoutUnit::Centimeters:
      return measurement.length() * 10.0;
    case Qgis::LayoutUnit::Meters:
      return measurement.length() * 1000.0;
    case Qgis::LayoutUnit::Inches:
      return measurement.length() * 25.4;
    case Qgis::LayoutUnit::Feet:
      return measurement.length() * 304.8;
    case Qgis::LayoutUnit::Points:
      return measurement.length() * 0.352777778;
    case Qgis::LayoutUnit::Picas:
      return measurement.length() * 4.23333333;
    case Qgis::LayoutUnit::Pixels:
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
