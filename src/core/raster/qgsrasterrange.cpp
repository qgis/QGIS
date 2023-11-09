/***************************************************************************
          qgsrasterrange.h
     --------------------------------------
    Date                 : Oct 9, 2012
    Copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrange.h"

QgsRasterRange::QgsRasterRange( double min, double max, BoundsType bounds )
  : mMin( min )
  , mMax( max )
  , mType( bounds )
{
}

bool QgsRasterRange::overlaps( const QgsRasterRange &other ) const
{
  const bool thisIncludesLower = mType == IncludeMinAndMax || mType == IncludeMin;
  const bool thisIncludesUpper = mType == IncludeMinAndMax || mType == IncludeMax;
  const bool thisLowerInfinite = !std::isfinite( mMin );
  const bool thisUpperInfinite = !std::isfinite( mMax );
  const bool otherIncludesLower = other.mType == IncludeMinAndMax || other.mType == IncludeMin;
  const bool otherIncludesUpper = other.mType == IncludeMinAndMax || other.mType == IncludeMax;
  const bool otherLowerInfinite = !std::isfinite( other.mMin );
  const bool otherUpperInfinite = !std::isfinite( other.mMax );

  if ( ( ( thisIncludesLower && otherIncludesLower && ( mMin <= other.mMin || thisLowerInfinite ) ) ||
         ( ( !thisIncludesLower || !otherIncludesLower ) && ( mMin < other.mMin || thisLowerInfinite ) ) )
       && ( ( thisIncludesUpper && otherIncludesUpper && ( mMax >= other.mMax || thisUpperInfinite ) ) ||
            ( ( !thisIncludesUpper || !otherIncludesUpper ) && ( mMax > other.mMax || thisUpperInfinite ) ) ) )
    return true;

  if ( ( ( otherIncludesLower && ( mMin <= other.mMin || thisLowerInfinite ) ) ||
         ( !otherIncludesLower && ( mMin < other.mMin || thisLowerInfinite ) ) )
       && ( ( thisIncludesUpper && otherIncludesLower && ( mMax >= other.mMin || thisUpperInfinite ) ) ||
            ( ( !thisIncludesUpper || !otherIncludesLower ) && ( mMax > other.mMin || thisUpperInfinite ) ) ) )
    return true;

  if ( ( ( thisIncludesLower && otherIncludesUpper && ( mMin <= other.mMax || thisLowerInfinite ) ) ||
         ( ( !thisIncludesLower || !otherIncludesUpper ) && ( mMin < other.mMax || thisLowerInfinite ) ) )
       && ( ( thisIncludesUpper && otherIncludesUpper && ( mMax >= other.mMax || thisUpperInfinite ) ) ||
            ( ( !thisIncludesUpper || !otherIncludesUpper ) && ( mMax > other.mMax || thisUpperInfinite ) ) ) )
    return true;

  if ( ( ( thisIncludesLower && otherIncludesLower && ( mMin >= other.mMin || otherLowerInfinite ) ) ||
         ( ( !thisIncludesLower || !otherIncludesLower ) && ( mMin > other.mMin || otherLowerInfinite ) ) )
       && ( ( thisIncludesLower && otherIncludesUpper && ( mMin <= other.mMax || thisLowerInfinite || otherUpperInfinite ) ) ||
            ( ( !thisIncludesLower || !otherIncludesUpper ) && ( mMin < other.mMax || thisLowerInfinite || otherUpperInfinite ) ) ) )
    return true;

  if ( qgsDoubleNear( mMin, other.mMin ) && qgsDoubleNear( mMax, other.mMax ) )
    return true;

  return false;
}

QString QgsRasterRange::asText() const
{
  const QString minText = std::isnan( mMin ) ? QStringLiteral( "-%1" ).arg( QChar( 0x221e ) ) : QString::number( mMin );
  const QString maxText = std::isnan( mMax ) ? QChar( 0x221e ) : QString::number( mMax );
  const QString operator1 = ( mType == IncludeMinAndMax || mType == IncludeMin ) ? QChar( 0x2264 ) : '<';
  const QString operator2 = ( mType == IncludeMinAndMax || mType == IncludeMax ) ? QChar( 0x2264 ) : '<';

  return QStringLiteral( "%1 %2 x %3 %4" ).arg( minText, operator1, operator2, maxText );
}





