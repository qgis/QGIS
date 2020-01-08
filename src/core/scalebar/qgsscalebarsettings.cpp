/***************************************************************************
                            qgsscalebarsettings.cpp
                            -----------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsscalebarsettings.h"
#include "qgsapplication.h"
#include "qgsnumericformat.h"
#include "qgsbasicnumericformat.h"

QgsScaleBarSettings::QgsScaleBarSettings()
{
  mPen = QPen( mLineColor );
  mPen.setJoinStyle( mLineJoinStyle );
  mPen.setCapStyle( mLineCapStyle );
  mPen.setWidthF( mLineWidth );

  mBrush.setColor( mFillColor );
  mBrush.setStyle( Qt::SolidPattern );

  mBrush2.setColor( mFillColor2 );
  mBrush2.setStyle( Qt::SolidPattern );

  mTextFormat.setSize( 12.0 );
  mTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
  mTextFormat.setColor( QColor( 0, 0, 0 ) );

  mNumericFormat = qgis::make_unique< QgsBasicNumericFormat >();
}

QgsScaleBarSettings::QgsScaleBarSettings( const QgsScaleBarSettings &other )
  : mNumSegments( other.mNumSegments )
  , mNumSegmentsLeft( other.mNumSegmentsLeft )
  , mNumUnitsPerSegment( other.mNumUnitsPerSegment )
  , mNumMapUnitsPerScaleBarUnit( other.mNumMapUnitsPerScaleBarUnit )
  , mSegmentSizeMode( other.mSegmentSizeMode )
  , mMinBarWidth( other.mMinBarWidth )
  , mMaxBarWidth( other.mMaxBarWidth )
  , mUnitLabeling( other.mUnitLabeling )
  , mTextFormat( other.mTextFormat )
  , mFillColor( other.mFillColor )
  , mFillColor2( other.mFillColor2 )
  , mLineColor( other.mLineColor )
  , mLineWidth( other.mLineWidth )
  , mPen( other.mPen )
  , mBrush( other.mBrush )
  , mBrush2( other.mBrush2 )
  , mHeight( other.mHeight )
  , mLabelBarSpace( other.mLabelBarSpace )
  , mLabelVerticalPlacement( other.mLabelVerticalPlacement )
  , mLabelHorizontalPlacement( other.mLabelHorizontalPlacement )
  , mBoxContentSpace( other.mBoxContentSpace )
  , mAlignment( other.mAlignment )
  , mUnits( other.mUnits )
  , mLineJoinStyle( other.mLineJoinStyle )
  , mLineCapStyle( other.mLineCapStyle )
  , mNumericFormat( other.mNumericFormat->clone() )
{

}

QgsScaleBarSettings &QgsScaleBarSettings::operator=( const QgsScaleBarSettings &other )
{
  mNumSegments = other.mNumSegments;
  mNumSegmentsLeft = other.mNumSegmentsLeft;
  mNumUnitsPerSegment = other.mNumUnitsPerSegment;
  mNumMapUnitsPerScaleBarUnit = other.mNumMapUnitsPerScaleBarUnit;
  mSegmentSizeMode = other.mSegmentSizeMode;
  mMinBarWidth = other.mMinBarWidth;
  mMaxBarWidth = other.mMaxBarWidth;
  mUnitLabeling = other.mUnitLabeling;
  mTextFormat = other.mTextFormat;
  mFillColor = other.mFillColor;
  mFillColor2 = other.mFillColor2;
  mLineColor = other.mLineColor;
  mLineWidth = other.mLineWidth;
  mPen = other.mPen;
  mBrush = other.mBrush;
  mBrush2 = other.mBrush2;
  mHeight = other.mHeight;
  mLabelBarSpace = other.mLabelBarSpace;
  mLabelVerticalPlacement = other.mLabelVerticalPlacement;
  mLabelHorizontalPlacement = other.mLabelHorizontalPlacement;
  mBoxContentSpace = other.mBoxContentSpace;
  mAlignment = other.mAlignment;
  mUnits = other.mUnits;
  mLineJoinStyle = other.mLineJoinStyle;
  mLineCapStyle = other.mLineCapStyle;
  mNumericFormat.reset( other.mNumericFormat->clone() );
  return *this;
}

const QgsNumericFormat *QgsScaleBarSettings::numericFormat() const
{
  return mNumericFormat.get();
}

void QgsScaleBarSettings::setNumericFormat( QgsNumericFormat *format )
{
  mNumericFormat.reset( format );
}

QgsScaleBarSettings::~QgsScaleBarSettings() = default;

