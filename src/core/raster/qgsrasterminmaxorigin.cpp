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
#include "qgssettings.h"

#include <QDomDocument>
#include <QDomElement>
#include <cmath>

QgsRasterMinMaxOrigin::QgsRasterMinMaxOrigin()
  : mCumulativeCutLower( CUMULATIVE_CUT_LOWER )
  , mCumulativeCutUpper( CUMULATIVE_CUT_UPPER )
  , mStdDevFactor( DEFAULT_STDDEV_FACTOR )
{
  const QgsSettings mySettings;
  mCumulativeCutLower = mySettings.value( QStringLiteral( "Raster/cumulativeCutLower" ), CUMULATIVE_CUT_LOWER ).toDouble();
  mCumulativeCutUpper = mySettings.value( QStringLiteral( "Raster/cumulativeCutUpper" ), CUMULATIVE_CUT_UPPER ).toDouble();
  mStdDevFactor = mySettings.value( QStringLiteral( "Raster/defaultStandardDeviation" ), DEFAULT_STDDEV_FACTOR ).toDouble();
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

QString QgsRasterMinMaxOrigin::limitsString( Limits limits )
{
  switch ( limits )
  {
    case MinMax:
      return QStringLiteral( "MinMax" );
    case StdDev:
      return QStringLiteral( "StdDev" );
    case CumulativeCut:
      return QStringLiteral( "CumulativeCut" );
    default:
      break;
  }
  return QStringLiteral( "None" );
}

QgsRasterMinMaxOrigin::Limits QgsRasterMinMaxOrigin::limitsFromString( const QString &limits )
{
  if ( limits == QLatin1String( "MinMax" ) )
  {
    return MinMax;
  }
  else if ( limits == QLatin1String( "StdDev" ) )
  {
    return StdDev;
  }
  else if ( limits == QLatin1String( "CumulativeCut" ) )
  {
    return CumulativeCut;
  }
  return None;
}

QString QgsRasterMinMaxOrigin::extentString( Extent minMaxExtent )
{
  switch ( minMaxExtent )
  {
    case WholeRaster:
      return QStringLiteral( "WholeRaster" );
    case CurrentCanvas:
      return QStringLiteral( "CurrentCanvas" );
    case UpdatedCanvas:
      return QStringLiteral( "UpdatedCanvas" );
  }
  return QStringLiteral( "WholeRaster" );
}

QgsRasterMinMaxOrigin::Extent QgsRasterMinMaxOrigin::extentFromString( const QString &extent )
{
  if ( extent == QLatin1String( "WholeRaster" ) )
  {
    return WholeRaster;
  }
  else if ( extent == QLatin1String( "CurrentCanvas" ) )
  {
    return CurrentCanvas;
  }
  else if ( extent == QLatin1String( "UpdatedCanvas" ) )
  {
    return UpdatedCanvas;
  }
  else
  {
    return WholeRaster;
  }
}

QString QgsRasterMinMaxOrigin::statAccuracyString( StatAccuracy accuracy )
{
  if ( accuracy == Exact )
    return QStringLiteral( "Exact" );
  return QStringLiteral( "Estimated" );
}

QgsRasterMinMaxOrigin::StatAccuracy QgsRasterMinMaxOrigin::statAccuracyFromString( const QString &accuracy )
{
  if ( accuracy == QLatin1String( "Exact" ) )
    return Exact;
  return Estimated;
}

void QgsRasterMinMaxOrigin::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  // limits
  QDomElement limitsElem = doc.createElement( QStringLiteral( "limits" ) );
  const QDomText limitsText = doc.createTextNode( limitsString( mLimits ) );
  limitsElem.appendChild( limitsText );
  parentElem.appendChild( limitsElem );

  // extent
  QDomElement extentElem = doc.createElement( QStringLiteral( "extent" ) );
  const QDomText extentText = doc.createTextNode( extentString( mExtent ) );
  extentElem.appendChild( extentText );
  parentElem.appendChild( extentElem );

  // statAccuracy
  QDomElement statAccuracyElem = doc.createElement( QStringLiteral( "statAccuracy" ) );
  const QDomText statAccuracyText = doc.createTextNode( statAccuracyString( mAccuracy ) );
  statAccuracyElem.appendChild( statAccuracyText );
  parentElem.appendChild( statAccuracyElem );

  // mCumulativeCutLower
  QDomElement cumulativeCutLowerElem = doc.createElement( QStringLiteral( "cumulativeCutLower" ) );
  const QDomText cumulativeCutLowerText = doc.createTextNode( QString::number( mCumulativeCutLower ) );
  cumulativeCutLowerElem.appendChild( cumulativeCutLowerText );
  parentElem.appendChild( cumulativeCutLowerElem );

  // mCumulativeCutUpper
  QDomElement cumulativeCutUpperElem = doc.createElement( QStringLiteral( "cumulativeCutUpper" ) );
  const QDomText cumulativeCutUpperText = doc.createTextNode( QString::number( mCumulativeCutUpper ) );
  cumulativeCutUpperElem.appendChild( cumulativeCutUpperText );
  parentElem.appendChild( cumulativeCutUpperElem );

  // mCumulativeCutUpper
  QDomElement stdDevFactorElem = doc.createElement( QStringLiteral( "stdDevFactor" ) );
  const QDomText stdDevFactorText = doc.createTextNode( QString::number( mStdDevFactor ) );
  stdDevFactorElem.appendChild( stdDevFactorText );
  parentElem.appendChild( stdDevFactorElem );
}

void QgsRasterMinMaxOrigin::readXml( const QDomElement &elem )
{
  const QDomElement limitsElem = elem.firstChildElement( QStringLiteral( "limits" ) );
  if ( !limitsElem.isNull() )
  {
    mLimits = limitsFromString( limitsElem.text() );
  }

  const QDomElement extentElem = elem.firstChildElement( QStringLiteral( "extent" ) );
  if ( !extentElem.isNull() )
  {
    mExtent = extentFromString( extentElem.text() );
  }

  const QDomElement statAccuracyElem = elem.firstChildElement( QStringLiteral( "statAccuracy" ) );
  if ( !statAccuracyElem.isNull() )
  {
    mAccuracy = statAccuracyFromString( statAccuracyElem.text() );
  }

  const QDomElement cumulativeCutLowerElem = elem.firstChildElement( QStringLiteral( "cumulativeCutLower" ) );
  if ( !cumulativeCutLowerElem.isNull() )
  {
    mCumulativeCutLower = cumulativeCutLowerElem.text().toDouble();
  }

  const QDomElement cumulativeCutUpperElem = elem.firstChildElement( QStringLiteral( "cumulativeCutUpper" ) );
  if ( !cumulativeCutUpperElem.isNull() )
  {
    mCumulativeCutUpper = cumulativeCutUpperElem.text().toDouble();
  }

  const QDomElement stdDevFactorElem = elem.firstChildElement( QStringLiteral( "stdDevFactor" ) );
  if ( !stdDevFactorElem.isNull() )
  {
    mStdDevFactor = stdDevFactorElem.text().toDouble();
  }
}
