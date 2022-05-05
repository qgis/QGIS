/***************************************************************************
                         qgsabstractprofilegenerator.cpp
                         ---------------
    begin                : March 2022
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
#include "qgsabstractprofilegenerator.h"
#include "qgsprofilesnapping.h"


QgsProfileRenderContext::QgsProfileRenderContext( QgsRenderContext &context )
  : mRenderContext( context )
{

}

const QTransform &QgsProfileRenderContext::worldTransform() const
{
  return mWorldTransform;
}

void QgsProfileRenderContext::setWorldTransform( const QTransform &transform )
{
  mWorldTransform = transform;
}

QgsDoubleRange QgsProfileRenderContext::distanceRange() const
{
  return mDistanceRange;
}

void QgsProfileRenderContext::setDistanceRange( const QgsDoubleRange &range )
{
  mDistanceRange = range;
}

QgsDoubleRange QgsProfileRenderContext::elevationRange() const
{
  return mElevationRange;
}

void QgsProfileRenderContext::setElevationRange( const QgsDoubleRange &range )
{
  mElevationRange = range;
}


QgsAbstractProfileGenerator::~QgsAbstractProfileGenerator() = default;

QgsAbstractProfileResults::~QgsAbstractProfileResults() = default;

QgsProfileSnapResult QgsAbstractProfileResults::snapPoint( const QgsProfilePoint &, const QgsProfileSnapContext & )
{
  return QgsProfileSnapResult();
}

QVector<QgsProfileIdentifyResults> QgsAbstractProfileResults::identify( const QgsProfilePoint &, const QgsProfileIdentifyContext & )
{
  return {};
}

QVector<QgsProfileIdentifyResults> QgsAbstractProfileResults::identify( const QgsDoubleRange &, const QgsDoubleRange &, const QgsProfileIdentifyContext & )
{
  return {};
}

void QgsAbstractProfileResults::copyPropertiesFromGenerator( const QgsAbstractProfileGenerator * )
{

}

//
// QgsProfileGenerationContext
//

#define POINTS_TO_MM 2.83464567
#define INCH_TO_MM 25.4

double QgsProfileGenerationContext::convertDistanceToPixels( double size, QgsUnitTypes::RenderUnit unit ) const
{
  double conversionFactor = 1.0;
  const double pixelsPerMillimeter = mDpi / 25.4;
  switch ( unit )
  {
    case QgsUnitTypes::RenderMillimeters:
      conversionFactor = pixelsPerMillimeter;
      break;

    case QgsUnitTypes::RenderPoints:
      conversionFactor = pixelsPerMillimeter / POINTS_TO_MM;
      break;

    case QgsUnitTypes::RenderInches:
      conversionFactor = pixelsPerMillimeter * INCH_TO_MM;
      break;

    case QgsUnitTypes::RenderMapUnits:
    {
      conversionFactor = 1.0 / mMapUnitsPerDistancePixel;
      break;
    }
    case QgsUnitTypes::RenderPixels:
      conversionFactor = 1.0;
      break;

    case QgsUnitTypes::RenderUnknownUnit:
    case QgsUnitTypes::RenderPercentage:
    case QgsUnitTypes::RenderMetersInMapUnits:
      //not supported
      conversionFactor = 1.0;
      break;
  }

  return size * conversionFactor;
}

bool QgsProfileGenerationContext::operator==( const QgsProfileGenerationContext &other ) const
{
  return qgsDoubleNear( mMaxErrorMapUnits, other.mMaxErrorMapUnits )
         && qgsDoubleNear( mMapUnitsPerDistancePixel, other.mMapUnitsPerDistancePixel )
         && qgsDoubleNear( mDpi, other.mDpi )
         && mDistanceRange == other.mDistanceRange
         && mElevationRange == other.mElevationRange;
}

bool QgsProfileGenerationContext::operator!=( const QgsProfileGenerationContext &other ) const
{
  return !( *this == other );
}

Qgis::ProfileGeneratorFlags QgsAbstractProfileGenerator::flags() const
{
  return Qgis::ProfileGeneratorFlags();
}

QgsProfileIdentifyResults::QgsProfileIdentifyResults( QgsMapLayer *layer, const QVector<QVariantMap> &results )
  : mLayer( layer )
  , mResults( results )
{

}
