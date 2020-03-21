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
#include "qgsfillsymbollayer.h"

QgsScaleBarSettings::QgsScaleBarSettings()
{
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

  mFillSymbol = qgis::make_unique< QgsFillSymbol >();
  mFillSymbol->setColor( QColor( 0, 0, 0 ) );
  if ( QgsSimpleFillSymbolLayer *fill = dynamic_cast< QgsSimpleFillSymbolLayer * >( mFillSymbol->symbolLayer( 0 ) ) )
  {
    fill->setStrokeStyle( Qt::NoPen );
  }
  mFillSymbol2 = qgis::make_unique< QgsFillSymbol >();
  mFillSymbol2->setColor( QColor( 255, 255, 255 ) );
  if ( QgsSimpleFillSymbolLayer *fill = dynamic_cast< QgsSimpleFillSymbolLayer * >( mFillSymbol2->symbolLayer( 0 ) ) )
  {
    fill->setStrokeStyle( Qt::NoPen );
  }
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
  , mHeight( other.mHeight )
  , mLineSymbol( other.mLineSymbol->clone() )
  , mFillSymbol( other.mFillSymbol->clone() )
  , mFillSymbol2( other.mFillSymbol2->clone() )
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
  mLineSymbol.reset( other.mLineSymbol->clone() );
  mFillSymbol.reset( other.mFillSymbol->clone() );
  mFillSymbol2.reset( other.mFillSymbol2->clone() );
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

QColor QgsScaleBarSettings::fillColor() const
{
  return mFillSymbol->color();
}

void QgsScaleBarSettings::setFillColor( const QColor &color )
{
  mFillSymbol->setColor( color );
}

QColor QgsScaleBarSettings::fillColor2() const
{
  return mFillSymbol2->color();
}

void QgsScaleBarSettings::setFillColor2( const QColor &color )
{
  mFillSymbol2->setColor( color );
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

QgsFillSymbol *QgsScaleBarSettings::fillSymbol1() const
{
  return mFillSymbol.get();
}

void QgsScaleBarSettings::setFillSymbol1( QgsFillSymbol *symbol )
{
  mFillSymbol.reset( symbol );
}

QgsFillSymbol *QgsScaleBarSettings::fillSymbol2() const
{
  return mFillSymbol2.get();
}

void QgsScaleBarSettings::setFillSymbol2( QgsFillSymbol *symbol )
{
  mFillSymbol2.reset( symbol );
}

QBrush QgsScaleBarSettings::brush() const
{
  QBrush b;
  b.setColor( mFillSymbol->color() );
  if ( QgsSimpleFillSymbolLayer *fill = dynamic_cast< QgsSimpleFillSymbolLayer * >( mFillSymbol->symbolLayer( 0 ) ) )
  {
    b.setStyle( fill->brushStyle() );
  }

  return b;
}

void QgsScaleBarSettings::setBrush( const QBrush &brush )
{
  mFillSymbol->setColor( brush.color() );
  if ( QgsSimpleFillSymbolLayer *fill = dynamic_cast< QgsSimpleFillSymbolLayer * >( mFillSymbol->symbolLayer( 0 ) ) )
  {
    fill->setBrushStyle( brush.style() );
  }
}

QBrush QgsScaleBarSettings::brush2() const
{
  QBrush b;
  b.setColor( mFillSymbol2->color() );
  if ( QgsSimpleFillSymbolLayer *fill = dynamic_cast< QgsSimpleFillSymbolLayer * >( mFillSymbol2->symbolLayer( 0 ) ) )
  {
    b.setStyle( fill->brushStyle() );
  }

  return b;
}

void QgsScaleBarSettings::setBrush2( const QBrush &brush )
{
  mFillSymbol2->setColor( brush.color() );
  if ( QgsSimpleFillSymbolLayer *fill = dynamic_cast< QgsSimpleFillSymbolLayer * >( mFillSymbol2->symbolLayer( 0 ) ) )
  {
    fill->setBrushStyle( brush.style() );
  }
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

