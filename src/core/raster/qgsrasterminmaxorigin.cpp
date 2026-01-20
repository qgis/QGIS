/***************************************************************************
    qgsrasterminmaxorigin.h - Origin of min/max values
     --------------------------------------
    Date                 : Dec 2016
    Copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterminmaxorigin.h"

#include <cmath>

#include "qgssettings.h"

#include <QDomDocument>
#include <QDomElement>

QgsRasterMinMaxOrigin::QgsRasterMinMaxOrigin()
  : mCumulativeCutLower( CUMULATIVE_CUT_LOWER )
  , mCumulativeCutUpper( CUMULATIVE_CUT_UPPER )
  , mStdDevFactor( DEFAULT_STDDEV_FACTOR )
{
  const QgsSettings mySettings;
  mCumulativeCutLower = mySettings.value( u"Raster/cumulativeCutLower"_s, CUMULATIVE_CUT_LOWER ).toDouble();
  mCumulativeCutUpper = mySettings.value( u"Raster/cumulativeCutUpper"_s, CUMULATIVE_CUT_UPPER ).toDouble();
  mStdDevFactor = mySettings.value( u"Raster/defaultStandardDeviation"_s, DEFAULT_STDDEV_FACTOR ).toDouble();
}

bool QgsRasterMinMaxOrigin::operator ==( const QgsRasterMinMaxOrigin &other ) const
{
  return mLimits == other.mLimits &&
         mExtent == other.mExtent &&
         mAccuracy == other.mAccuracy &&
         std::fabs( mCumulativeCutLower - other.mCumulativeCutLower ) < 1e-5 &&
         std::fabs( mCumulativeCutUpper - other.mCumulativeCutUpper ) < 1e-5 &&
         std::fabs( mStdDevFactor - other.mStdDevFactor ) < 1e-5;
}

QString QgsRasterMinMaxOrigin::limitsString( Qgis::RasterRangeLimit limits )
{
  switch ( limits )
  {
    case Qgis::RasterRangeLimit::MinimumMaximum:
      return u"MinMax"_s;
    case Qgis::RasterRangeLimit::StdDev:
      return u"StdDev"_s;
    case Qgis::RasterRangeLimit::CumulativeCut:
      return u"CumulativeCut"_s;
    default:
      break;
  }
  return u"None"_s;
}

Qgis::RasterRangeLimit QgsRasterMinMaxOrigin::limitsFromString( const QString &limits )
{
  if ( limits == "MinMax"_L1 )
  {
    return Qgis::RasterRangeLimit::MinimumMaximum;
  }
  else if ( limits == "StdDev"_L1 )
  {
    return Qgis::RasterRangeLimit::StdDev;
  }
  else if ( limits == "CumulativeCut"_L1 )
  {
    return Qgis::RasterRangeLimit::CumulativeCut;
  }
  return Qgis::RasterRangeLimit::NotSet;
}

QString QgsRasterMinMaxOrigin::extentString( Qgis::RasterRangeExtent minMaxExtent )
{
  switch ( minMaxExtent )
  {
    case Qgis::RasterRangeExtent::WholeRaster:
      return u"WholeRaster"_s;
    case Qgis::RasterRangeExtent::FixedCanvas:
      return u"CurrentCanvas"_s;
    case Qgis::RasterRangeExtent::UpdatedCanvas:
      return u"UpdatedCanvas"_s;
  }
  return u"WholeRaster"_s;
}

Qgis::RasterRangeExtent QgsRasterMinMaxOrigin::extentFromString( const QString &extent )
{
  if ( extent == "WholeRaster"_L1 )
  {
    return Qgis::RasterRangeExtent::WholeRaster;
  }
  else if ( extent == "CurrentCanvas"_L1 )
  {
    return Qgis::RasterRangeExtent::FixedCanvas;
  }
  else if ( extent == "UpdatedCanvas"_L1 )
  {
    return Qgis::RasterRangeExtent::UpdatedCanvas;
  }
  else
  {
    return Qgis::RasterRangeExtent::WholeRaster;
  }
}

QString QgsRasterMinMaxOrigin::statAccuracyString( Qgis::RasterRangeAccuracy accuracy )
{
  switch ( accuracy )
  {
    case Qgis::RasterRangeAccuracy::Exact:
      return u"Exact"_s;
    case Qgis::RasterRangeAccuracy::Estimated:
      return u"Estimated"_s;
  }
  BUILTIN_UNREACHABLE
}

Qgis::RasterRangeAccuracy QgsRasterMinMaxOrigin::statAccuracyFromString( const QString &accuracy )
{
  if ( accuracy == "Exact"_L1 )
    return Qgis::RasterRangeAccuracy::Exact;
  return Qgis::RasterRangeAccuracy::Estimated;
}

void QgsRasterMinMaxOrigin::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  // limits
  QDomElement limitsElem = doc.createElement( u"limits"_s );
  const QDomText limitsText = doc.createTextNode( limitsString( mLimits ) );
  limitsElem.appendChild( limitsText );
  parentElem.appendChild( limitsElem );

  // extent
  QDomElement extentElem = doc.createElement( u"extent"_s );
  const QDomText extentText = doc.createTextNode( extentString( mExtent ) );
  extentElem.appendChild( extentText );
  parentElem.appendChild( extentElem );

  // statAccuracy
  QDomElement statAccuracyElem = doc.createElement( u"statAccuracy"_s );
  const QDomText statAccuracyText = doc.createTextNode( statAccuracyString( mAccuracy ) );
  statAccuracyElem.appendChild( statAccuracyText );
  parentElem.appendChild( statAccuracyElem );

  // mCumulativeCutLower
  QDomElement cumulativeCutLowerElem = doc.createElement( u"cumulativeCutLower"_s );
  const QDomText cumulativeCutLowerText = doc.createTextNode( QString::number( mCumulativeCutLower ) );
  cumulativeCutLowerElem.appendChild( cumulativeCutLowerText );
  parentElem.appendChild( cumulativeCutLowerElem );

  // mCumulativeCutUpper
  QDomElement cumulativeCutUpperElem = doc.createElement( u"cumulativeCutUpper"_s );
  const QDomText cumulativeCutUpperText = doc.createTextNode( QString::number( mCumulativeCutUpper ) );
  cumulativeCutUpperElem.appendChild( cumulativeCutUpperText );
  parentElem.appendChild( cumulativeCutUpperElem );

  // mCumulativeCutUpper
  QDomElement stdDevFactorElem = doc.createElement( u"stdDevFactor"_s );
  const QDomText stdDevFactorText = doc.createTextNode( QString::number( mStdDevFactor ) );
  stdDevFactorElem.appendChild( stdDevFactorText );
  parentElem.appendChild( stdDevFactorElem );
}

void QgsRasterMinMaxOrigin::readXml( const QDomElement &elem )
{
  const QDomElement limitsElem = elem.firstChildElement( u"limits"_s );
  if ( !limitsElem.isNull() )
  {
    mLimits = limitsFromString( limitsElem.text() );
  }

  const QDomElement extentElem = elem.firstChildElement( u"extent"_s );
  if ( !extentElem.isNull() )
  {
    mExtent = extentFromString( extentElem.text() );
  }

  const QDomElement statAccuracyElem = elem.firstChildElement( u"statAccuracy"_s );
  if ( !statAccuracyElem.isNull() )
  {
    mAccuracy = statAccuracyFromString( statAccuracyElem.text() );
  }

  const QDomElement cumulativeCutLowerElem = elem.firstChildElement( u"cumulativeCutLower"_s );
  if ( !cumulativeCutLowerElem.isNull() )
  {
    mCumulativeCutLower = cumulativeCutLowerElem.text().toDouble();
  }

  const QDomElement cumulativeCutUpperElem = elem.firstChildElement( u"cumulativeCutUpper"_s );
  if ( !cumulativeCutUpperElem.isNull() )
  {
    mCumulativeCutUpper = cumulativeCutUpperElem.text().toDouble();
  }

  const QDomElement stdDevFactorElem = elem.firstChildElement( u"stdDevFactor"_s );
  if ( !stdDevFactorElem.isNull() )
  {
    mStdDevFactor = stdDevFactorElem.text().toDouble();
  }
}
