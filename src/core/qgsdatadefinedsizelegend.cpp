/***************************************************************************
  qgsdatadefinedsizelegend.cpp
  --------------------------------------
  Date                 : June 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdatadefinedsizelegend.h"

#include "qgsproperty.h"
#include "qgspropertytransformer.h"
#include "qgssymbollayerutils.h"

QgsDataDefinedSizeLegend::QgsDataDefinedSizeLegend()
{
}

QgsDataDefinedSizeLegend::QgsDataDefinedSizeLegend( const QgsDataDefinedSizeLegend &other )
  : mType( other.mType )
  , mTitleLabel( other.mTitleLabel )
  , mSizeClasses( other.mSizeClasses )
  , mSymbol( other.mSymbol.get() ? other.mSymbol->clone() : nullptr )
{
}

QgsDataDefinedSizeLegend &QgsDataDefinedSizeLegend::operator=( const QgsDataDefinedSizeLegend &other )
{
  if ( this != &other )
  {
    mType = other.mType;
    mTitleLabel = other.mTitleLabel;
    mSizeClasses = other.mSizeClasses;
    mSymbol.reset( other.mSymbol.get() ? other.mSymbol->clone() : nullptr );
  }
  return *this;
}


void QgsDataDefinedSizeLegend::updateFromSymbolAndProperty( const QgsMarkerSymbol *symbol, const QgsProperty &ddSize )
{
  mSymbol.reset( symbol->clone() );
  mSymbol->setDataDefinedSize( QgsProperty() );  // original symbol may have had data-defined size associated

  mTitleLabel = ddSize.propertyType() == QgsProperty::ExpressionBasedProperty ? ddSize.expressionString() : ddSize.field();

  // automatically generated classes
  if ( const QgsSizeScaleTransformer *sizeTransformer = dynamic_cast< const QgsSizeScaleTransformer * >( ddSize.transformer() ) )
  {
    mSizeClasses.clear();
    Q_FOREACH ( double v, QgsSymbolLayerUtils::prettyBreaks( sizeTransformer->minValue(), sizeTransformer->maxValue(), 4 ) )
    {
      mSizeClasses << SizeClass( sizeTransformer->size( v ), QString::number( v ) );
    }
  }
}

QgsLegendSymbolList QgsDataDefinedSizeLegend::legendSymbolList() const
{
  QgsLegendSymbolList lst;
  if ( !mTitleLabel.isEmpty() )
  {
    QgsLegendSymbolItem title( nullptr, mTitleLabel, QString() );
    lst << title;
  }

  if ( mType == LegendCollapsed )
  {
    QgsLegendSymbolItem i;
    i.setDataDefinedSizeLegendSettings( new QgsDataDefinedSizeLegend( *this ) );
    lst << i;
    return lst;
  }
  else if ( mType == LegendSeparated )
  {
    Q_FOREACH ( const SizeClass &cl, mSizeClasses )
    {
      QgsLegendSymbolItem si( mSymbol.get(), cl.label, QString() );
      QgsMarkerSymbol *s = static_cast<QgsMarkerSymbol *>( si.symbol() );
      s->setSize( cl.size );
      lst << si;
    }
  }
  return lst;
}


void QgsDataDefinedSizeLegend::drawCollapsedLegend( QgsRenderContext &context, QSize *outputSize, int *labelXOffset ) const
{
  if ( mType != LegendCollapsed || mSizeClasses.isEmpty() || !mSymbol )
  {
    if ( outputSize )
      *outputSize = QSize();
    if ( labelXOffset )
      *labelXOffset = 0;
    return;
  }

  // parameters that could be configurable
  double hLengthLineMM = 2;       // extra horizontal space to be occupied by callout line
  double hSpaceLineTextMM = 1;    // horizontal space between end of the line and start of the text

  std::unique_ptr<QgsMarkerSymbol> s( mSymbol->clone() );

  // make sure we draw bigger symbols first
  QList<SizeClass> classes = mSizeClasses;
  std::sort( classes.begin(), classes.end(), []( const SizeClass & a, const SizeClass & b ) { return a.size > b.size; } );

  int hLengthLine = qRound( context.convertToPainterUnits( hLengthLineMM, QgsUnitTypes::RenderMillimeters ) );
  int hSpaceLineText = qRound( context.convertToPainterUnits( hSpaceLineTextMM, QgsUnitTypes::RenderMillimeters ) );
  int dpm = qRound( context.scaleFactor() * 1000 );  // scale factor = dots per millimeter

  // get font metrics - we need a temporary image just to get the metrics right for the given DPI
  QImage tmpImg( QSize( 1, 1 ), QImage::Format_ARGB32_Premultiplied );
  tmpImg.setDotsPerMeterX( dpm );
  tmpImg.setDotsPerMeterY( dpm );
  QFontMetrics fm( mFont, &tmpImg );
  int textHeight = fm.height();
  int leading = fm.leading();
  int minTextDistY = textHeight + leading;

  //
  // determine layout of the rendered elements
  //

  // find out how wide the text will be
  int maxTextWidth = 0;
  Q_FOREACH ( const SizeClass &c, classes )
  {
    int w = fm.width( c.label );
    if ( w > maxTextWidth )
      maxTextWidth = w;
  }

  // find out size of the largest symbol
  double largestSize = classes.at( 0 ).size;
  double outputLargestSize = context.convertToPainterUnits( largestSize, s->sizeUnit(), s->sizeMapUnitScale() );

  // find out top Y coordinate for individual symbol sizes
  QList<int> symbolTopY;
  Q_FOREACH ( const SizeClass &c, classes )
  {
    double outputSymbolSize = context.convertToPainterUnits( c.size, s->sizeUnit(), s->sizeMapUnitScale() );
    switch ( mVAlign )
    {
      case AlignCenter:
        symbolTopY << qRound( outputLargestSize / 2 - outputSymbolSize / 2 );
        break;
      case AlignBottom:
        symbolTopY << qRound( outputLargestSize - outputSymbolSize );
        break;
    }
  }

  // determine Y coordinate of texts: ideally they should be at the same level as symbolTopY
  // but we need to avoid overlapping texts, so adjust the vertical positions
  int middleIndex = 0; // classes.count() / 2;  // will get the ideal position
  QList<int> textCenterY;
  int lastY = symbolTopY[middleIndex];
  textCenterY << lastY;
  for ( int i = middleIndex + 1; i < classes.count(); ++i )
  {
    int symbolY = symbolTopY[i];
    if ( symbolY - lastY < minTextDistY )
      symbolY = lastY + minTextDistY;
    textCenterY << symbolY;
    lastY = symbolY;
  }

  int textTopY = textCenterY.first() - textHeight / 2;
  int textBottomY = textCenterY.last() + textHeight / 2;
  int totalTextHeight = textBottomY - textTopY;

  int fullWidth = outputLargestSize + hLengthLine + hSpaceLineText + maxTextWidth;
  int fullHeight = qMax( qRound( outputLargestSize ) - textTopY, totalTextHeight );

  if ( outputSize )
    *outputSize = QSize( fullWidth, fullHeight );
  if ( labelXOffset )
    *labelXOffset = outputLargestSize + hLengthLine + hSpaceLineText;

  if ( !context.painter() )
    return;  // only layout

  //
  // drawing
  //

  QPainter *p = context.painter();

  p->save();
  p->translate( 0, -textTopY );

  // draw symbols first so that they do not cover
  Q_FOREACH ( const SizeClass &c, classes )
  {
    s->setSize( c.size );

    double outputSymbolSize = context.convertToPainterUnits( c.size, s->sizeUnit(), s->sizeMapUnitScale() );
    double tx = ( outputLargestSize - outputSymbolSize ) / 2;

    p->save();
    switch ( mVAlign )
    {
      case AlignCenter:
        p->translate( tx, ( outputLargestSize - outputSymbolSize ) / 2 );
        break;
      case AlignBottom:
        p->translate( tx, outputLargestSize - outputSymbolSize );
        break;
    }
    s->drawPreviewIcon( p, QSize( outputSymbolSize, outputSymbolSize ) );
    p->restore();
  }

  p->setPen( mTextColor );
  p->setFont( mFont );

  int i = 0;
  Q_FOREACH ( const SizeClass &c, classes )
  {
    // line from symbol to the text
    p->drawLine( outputLargestSize / 2, symbolTopY[i], outputLargestSize + hLengthLine, textCenterY[i] );

    // draw label
    QRect rect( outputLargestSize + hLengthLine + hSpaceLineText, textCenterY[i] - textHeight / 2,
                maxTextWidth, textHeight );
    p->drawText( rect, mTextAlignment, c.label );
    i++;
  }

  p->restore();
}


QImage QgsDataDefinedSizeLegend::collapsedLegendImage( QgsRenderContext &context, double paddingMM ) const
{
  if ( mType != LegendCollapsed || mSizeClasses.isEmpty() || !mSymbol )
    return QImage();

  // find out the size first
  QSize contentSize;
  drawCollapsedLegend( context, &contentSize );

  int padding = qRound( context.convertToPainterUnits( paddingMM, QgsUnitTypes::RenderMillimeters ) );
  int dpm = qRound( context.scaleFactor() * 1000 );  // scale factor = dots per millimeter

  QImage img( contentSize.width() + padding * 2, contentSize.height() + padding * 2, QImage::Format_ARGB32_Premultiplied );
  img.setDotsPerMeterX( dpm );
  img.setDotsPerMeterY( dpm );
  img.fill( Qt::transparent );

  QPainter painter( &img );
  painter.setRenderHint( QPainter::Antialiasing, true );

  painter.translate( padding, padding ); // so we do not need to care about padding at all

  // now do the rendering
  QPainter *oldPainter = context.painter();
  context.setPainter( &painter );
  drawCollapsedLegend( context );
  context.setPainter( oldPainter );

  painter.end();
  return img;
}

QgsDataDefinedSizeLegend *QgsDataDefinedSizeLegend::readTypeAndAlignmentFromXml( const QDomElement &elem )
{
  if ( elem.isNull() )
    return nullptr;
  QgsDataDefinedSizeLegend *ddsLegend = new QgsDataDefinedSizeLegend;
  ddsLegend->setLegendType( elem.attribute( "type" ) == "collapsed" ? LegendCollapsed : LegendSeparated );
  ddsLegend->setVerticalAlignment( elem.attribute( "valign" ) == "center" ? AlignCenter : AlignBottom );
  return ddsLegend;
}

void QgsDataDefinedSizeLegend::writeTypeAndAlignmentToXml( const QgsDataDefinedSizeLegend &ddsLegend, QDomElement &elem )
{
  elem.setAttribute( "type", ddsLegend.legendType() == LegendCollapsed ? "collapsed" : "separated" );
  elem.setAttribute( "valign", ddsLegend.verticalAlignment() == AlignCenter ? "center" : "bottom" );
}
