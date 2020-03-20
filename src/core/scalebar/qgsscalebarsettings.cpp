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
#include "qgslinesymbollayer.h"
#include "qgssymbol.h"

QgsScaleBarSettings::QgsScaleBarSettings()
{
  mBrush.setColor( mFillColor );
  mBrush.setStyle( Qt::SolidPattern );

  mBrush2.setColor( mFillColor2 );
  mBrush2.setStyle( Qt::SolidPattern );

  mTextFormat.setSize( 12.0 );
  mTextFormat.setSizeUnit( QgsUnitTypes::RenderPoints );
  mTextFormat.setColor( QColor( 0, 0, 0 ) );

  mNumericFormat = qgis::make_unique< QgsBasicNumericFormat >();

  mLineSymbol = qgis::make_unique< QgsLineSymbol >();
  mLineSymbol->setColor( QColor( 0, 0, 0 ) );
  mLineSymbol->setWidth( 0.3 );
  if ( QgsSimpleLineSymbolLayer *line = dynamic_cast< QgsSimpleLineSymbolLayer * >( mLineSymbol->symbolLayer( 0 ) ) )
  {
    line->setPenJoinStyle( Qt::MiterJoin );
    line->setPenCapStyle( Qt::SquareCap );
  }
  mLineSymbol->setOutputUnit( QgsUnitTypes::RenderMillimeters );
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
  , mBrush( other.mBrush )
  , mBrush2( other.mBrush2 )
  , mHeight( other.mHeight )
  , mLineSymbol( other.mLineSymbol->clone() )
  , mLabelBarSpace( other.mLabelBarSpace )
  , mLabelVerticalPlacement( other.mLabelVerticalPlacement )
  , mLabelHorizontalPlacement( other.mLabelHorizontalPlacement )
  , mBoxContentSpace( other.mBoxContentSpace )
  , mAlignment( other.mAlignment )
  , mUnits( other.mUnits )
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
  mBrush = other.mBrush;
  mBrush2 = other.mBrush2;
  mLineSymbol.reset( other.mLineSymbol->clone() );
  mHeight = other.mHeight;
  mLabelBarSpace = other.mLabelBarSpace;
  mLabelVerticalPlacement = other.mLabelVerticalPlacement;
  mLabelHorizontalPlacement = other.mLabelHorizontalPlacement;
  mBoxContentSpace = other.mBoxContentSpace;
  mAlignment = other.mAlignment;
  mUnits = other.mUnits;
  mNumericFormat.reset( other.mNumericFormat->clone() );
  return *this;
}

QColor QgsScaleBarSettings::lineColor() const
{
  return mLineSymbol->color();
}

void QgsScaleBarSettings::setLineColor( const QColor &color )
{
  mLineSymbol->setColor( color );
}

double QgsScaleBarSettings::lineWidth() const
{
  return mLineSymbol->width();
}

void QgsScaleBarSettings::setLineWidth( double width )
{
  mLineSymbol->setWidth( width );
  mLineSymbol->setOutputUnit( QgsUnitTypes::RenderMillimeters );
}

QPen QgsScaleBarSettings::pen() const
{
  QPen pen( mLineSymbol->color() );
  if ( QgsSimpleLineSymbolLayer *line = dynamic_cast< QgsSimpleLineSymbolLayer * >( mLineSymbol->symbolLayer( 0 ) ) )
  {
    pen.setJoinStyle( line->penJoinStyle() );
    pen.setCapStyle( line->penCapStyle() );
  }
  pen.setWidthF( mLineSymbol->width() );
  return pen;
}

void QgsScaleBarSettings::setPen( const QPen &pen )
{
  mLineSymbol->setColor( pen.color() );
  mLineSymbol->setWidth( pen.widthF() );
  mLineSymbol->setOutputUnit( QgsUnitTypes::RenderMillimeters );
  if ( QgsSimpleLineSymbolLayer *line = dynamic_cast< QgsSimpleLineSymbolLayer * >( mLineSymbol->symbolLayer( 0 ) ) )
  {
    line->setPenJoinStyle( pen.joinStyle() );
    line->setPenCapStyle( pen.capStyle() );
  }
}

QgsLineSymbol *QgsScaleBarSettings::lineSymbol() const
{
  return mLineSymbol.get();
}

void QgsScaleBarSettings::setLineSymbol( QgsLineSymbol *symbol )
{
  mLineSymbol.reset( symbol );
}

Qt::PenJoinStyle QgsScaleBarSettings::lineJoinStyle() const
{
  if ( QgsSimpleLineSymbolLayer *line = dynamic_cast< QgsSimpleLineSymbolLayer * >( mLineSymbol->symbolLayer( 0 ) ) )
  {
    return line->penJoinStyle();
  }
  return Qt::MiterJoin;
}

void QgsScaleBarSettings::setLineJoinStyle( Qt::PenJoinStyle style )
{
  if ( QgsSimpleLineSymbolLayer *line = dynamic_cast< QgsSimpleLineSymbolLayer * >( mLineSymbol->symbolLayer( 0 ) ) )
  {
    line->setPenJoinStyle( style );
  }
}

Qt::PenCapStyle QgsScaleBarSettings::lineCapStyle() const
{
  if ( QgsSimpleLineSymbolLayer *line = dynamic_cast< QgsSimpleLineSymbolLayer * >( mLineSymbol->symbolLayer( 0 ) ) )
  {
    return line->penCapStyle();
  }
  return Qt::FlatCap;
}

void QgsScaleBarSettings::setLineCapStyle( Qt::PenCapStyle style )
{
  if ( QgsSimpleLineSymbolLayer *line = dynamic_cast< QgsSimpleLineSymbolLayer * >( mLineSymbol->symbolLayer( 0 ) ) )
  {
    line->setPenCapStyle( style );
  }
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

