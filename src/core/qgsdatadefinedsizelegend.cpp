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

#include "qgslayertreemodellegendnode.h"
#include "qgsproperty.h"
#include "qgspropertytransformer.h"
#include "qgssymbollayerutils.h"
#include "qgsxmlutils.h"
#include "qgslinesymbollayer.h"
#include "qgstextformat.h"
#include "qgstextrenderer.h"
#include "qgsmarkersymbol.h"
#include "qgslinesymbol.h"
#include "qgsfontutils.h"
#include "qgscolorutils.h"

QgsDataDefinedSizeLegend::QgsDataDefinedSizeLegend()
{
  std::unique_ptr< QgsSimpleLineSymbolLayer > lineSymbolLayer = std::make_unique< QgsSimpleLineSymbolLayer >( QColor( 0, 0, 0 ), 0.2 );
  mLineSymbol = std::make_unique< QgsLineSymbol >( QgsSymbolLayerList() << lineSymbolLayer.release() );
}

QgsDataDefinedSizeLegend::~QgsDataDefinedSizeLegend() = default;

QgsDataDefinedSizeLegend::QgsDataDefinedSizeLegend( const QgsDataDefinedSizeLegend &other )
  : mType( other.mType )
  , mTitleLabel( other.mTitleLabel )
  , mSizeClasses( other.mSizeClasses )
  , mSymbol( other.mSymbol.get() ? other.mSymbol->clone() : nullptr )
  , mLineSymbol( other.mLineSymbol.get() ? other.mLineSymbol->clone() : nullptr )
  , mSizeScaleTransformer( other.mSizeScaleTransformer.get() ? new QgsSizeScaleTransformer( *other.mSizeScaleTransformer ) : nullptr )
  , mVAlign( other.mVAlign )
  , mFont( other.mFont )
  , mTextColor( other.mTextColor )
  , mTextAlignment( other.mTextAlignment )
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
    mLineSymbol.reset( other.mLineSymbol.get() ? other.mLineSymbol->clone() : nullptr );
    mSizeScaleTransformer.reset( other.mSizeScaleTransformer.get() ? new QgsSizeScaleTransformer( *other.mSizeScaleTransformer ) : nullptr );
    mVAlign = other.mVAlign;
    mFont = other.mFont;
    mTextColor = other.mTextColor;
    mTextAlignment = other.mTextAlignment;
  }
  return *this;
}

void QgsDataDefinedSizeLegend::setSymbol( QgsMarkerSymbol *symbol )
{
  mSymbol.reset( symbol );
}

QgsMarkerSymbol *QgsDataDefinedSizeLegend::symbol() const
{
  return mSymbol.get();
}

void QgsDataDefinedSizeLegend::setLineSymbol( QgsLineSymbol *symbol )
{
  mLineSymbol.reset( symbol );
}

QgsLineSymbol *QgsDataDefinedSizeLegend::lineSymbol() const
{
  return mLineSymbol.get();
}

void QgsDataDefinedSizeLegend::setSizeScaleTransformer( QgsSizeScaleTransformer *transformer )
{
  mSizeScaleTransformer.reset( transformer );
}

QgsSizeScaleTransformer *QgsDataDefinedSizeLegend::sizeScaleTransformer() const
{
  return mSizeScaleTransformer.get();
}


void QgsDataDefinedSizeLegend::updateFromSymbolAndProperty( const QgsMarkerSymbol *symbol, const QgsProperty &ddSize )
{
  mSymbol.reset( symbol->clone() );
  mSymbol->setDataDefinedSize( QgsProperty() );  // original symbol may have had data-defined size associated

  const QgsSizeScaleTransformer *sizeTransformer = dynamic_cast< const QgsSizeScaleTransformer * >( ddSize.transformer() );
  mSizeScaleTransformer.reset( sizeTransformer ? sizeTransformer->clone() : nullptr );

  if ( mTitleLabel.isEmpty() )
    mTitleLabel = ddSize.propertyType() == Qgis::PropertyType::Expression ? ddSize.expressionString() : ddSize.field();

  // automatically generate classes if no classes are defined
  if ( sizeTransformer && mSizeClasses.isEmpty() )
  {
    mSizeClasses.clear();
    const auto prettyBreaks { QgsSymbolLayerUtils::prettyBreaks( sizeTransformer->minValue(), sizeTransformer->maxValue(), 4 ) };
    for ( double v : prettyBreaks )
    {
      mSizeClasses << SizeClass( v, QString::number( v ) );
    }
  }
}

QgsLegendSymbolList QgsDataDefinedSizeLegend::legendSymbolList() const
{
  QgsLegendSymbolList lst;
  QVariant isDataDefinedSize( true );
  if ( !mTitleLabel.isEmpty() )
  {
    QgsLegendSymbolItem title( nullptr, mTitleLabel, QString() );
    title.setUserData( static_cast<int>( QgsLayerTreeModelLegendNode::CustomRole::IsDataDefinedSize ), isDataDefinedSize );
    lst << title;
  }

  switch ( mType )
  {
    case LegendCollapsed:
    {
      QgsLegendSymbolItem i;
      i.setDataDefinedSizeLegendSettings( new QgsDataDefinedSizeLegend( *this ) );
      i.setUserData( static_cast<int>( QgsLayerTreeModelLegendNode::CustomRole::IsDataDefinedSize ), isDataDefinedSize );
      lst << i;
      break;
    }

    case LegendSeparated:
    {
      lst.reserve( mSizeClasses.size() );
      for ( const SizeClass &cl : mSizeClasses )
      {
        QgsLegendSymbolItem si( mSymbol.get(), cl.label, QString() );
        si.setUserData( static_cast<int>( QgsLayerTreeModelLegendNode::CustomRole::IsDataDefinedSize ), isDataDefinedSize );
        QgsMarkerSymbol *s = static_cast<QgsMarkerSymbol *>( si.symbol() );
        double size = cl.size;
        if ( mSizeScaleTransformer )
        {
          size = mSizeScaleTransformer->size( size );
        }

        s->setSize( size );
        lst << si;
      }
      break;
    }
  }
  return lst;
}


void QgsDataDefinedSizeLegend::drawCollapsedLegend( QgsRenderContext &context, QSizeF *outputSize, double *labelXOffset ) const
{
  // this assumes the context's painter has been scaled to pixels in advance!

  if ( mType != LegendCollapsed || mSizeClasses.isEmpty() || !mSymbol )
  {
    if ( outputSize )
      *outputSize = QSizeF();
    if ( labelXOffset )
      *labelXOffset = 0;
    return;
  }

  // parameters that could be configurable
  double hLengthLineMM = 2;       // extra horizontal space to be occupied by callout line
  double hSpaceLineTextMM = 1;    // horizontal space between end of the line and start of the text

  std::unique_ptr<QgsMarkerSymbol> s( mSymbol->clone() );

  QList<SizeClass> classes = mSizeClasses;

  // optionally scale size values if transformer is defined
  if ( mSizeScaleTransformer )
  {
    for ( SizeClass &cls : classes )
      cls.size = mSizeScaleTransformer->size( cls.size );
  }

  // make sure we draw bigger symbols first
  std::sort( classes.begin(), classes.end(), []( const SizeClass & a, const SizeClass & b ) { return a.size > b.size; } );

  double hLengthLine = context.convertToPainterUnits( hLengthLineMM, Qgis::RenderUnit::Millimeters );
  double hSpaceLineText = context.convertToPainterUnits( hSpaceLineTextMM, Qgis::RenderUnit::Millimeters );
  int dpm = std::round( context.scaleFactor() * 1000 );  // scale factor = dots per millimeter

  // get font metrics - we need a temporary image just to get the metrics right for the given DPI
  QImage tmpImg( QSize( 1, 1 ), QImage::Format_ARGB32_Premultiplied );
  tmpImg.setDotsPerMeterX( dpm );
  tmpImg.setDotsPerMeterY( dpm );
  QFontMetricsF fm( mFont, &tmpImg );
  double textHeight = fm.height();
  double leading = fm.leading();
  double minTextDistY = textHeight + leading;

  //
  // determine layout of the rendered elements
  //

  // find out how wide the text will be
  double maxTextWidth = 0;
  for ( const SizeClass &c : std::as_const( classes ) )
  {
    maxTextWidth = std::max( maxTextWidth, fm.boundingRect( c.label ).width() );
  }
  // add extra width needed to handle varying rendering of font weight
  maxTextWidth += 1;

  // find out size of the largest symbol
  double largestSize = classes.at( 0 ).size;
  double outputLargestSize = context.convertToPainterUnits( largestSize, s->sizeUnit(), s->sizeMapUnitScale() );

  // find out top Y coordinate for individual symbol sizes
  QList<double> symbolTopY;
  for ( const SizeClass &c : std::as_const( classes ) )
  {
    double outputSymbolSize = context.convertToPainterUnits( c.size, s->sizeUnit(), s->sizeMapUnitScale() );
    switch ( mVAlign )
    {
      case AlignCenter:
        symbolTopY << outputLargestSize / 2 - outputSymbolSize / 2;
        break;
      case AlignBottom:
        symbolTopY <<  outputLargestSize - outputSymbolSize;
        break;
    }
  }

  // determine Y coordinate of texts: ideally they should be at the same level as symbolTopY
  // but we need to avoid overlapping texts, so adjust the vertical positions
  double middleIndex = 0; // classes.count() / 2;  // will get the ideal position
  QList<double> textCenterY;
  double lastY = middleIndex < symbolTopY.size() ? symbolTopY[middleIndex] : 0;
  textCenterY << lastY;
  for ( int i = middleIndex + 1; i < classes.count(); ++i )
  {
    double symbolY = symbolTopY[i];
    if ( symbolY - lastY < minTextDistY )
      symbolY = lastY + minTextDistY;
    textCenterY << symbolY;
    lastY = symbolY;
  }

  double textTopY = textCenterY.first() - textHeight / 2;
  double textBottomY = textCenterY.last() + textHeight / 2;
  double totalTextHeight = textBottomY - textTopY;

  double fullWidth = outputLargestSize + hLengthLine + hSpaceLineText + maxTextWidth;
  double fullHeight = std::max( outputLargestSize - textTopY, totalTextHeight );

  if ( outputSize )
    *outputSize = QSizeF( fullWidth, fullHeight );
  if ( labelXOffset )
    *labelXOffset = outputLargestSize + hLengthLine + hSpaceLineText;

  if ( !context.painter() )
    return;  // only layout

  //
  // drawing
  //

  QPainter *p = context.painter();
  QgsScopedQPainterState painterState( p );
  p->translate( 0, -textTopY );

  // draw symbols first so that they do not cover
  for ( const SizeClass &c : std::as_const( classes ) )
  {
    s->setSize( c.size );

    double outputSymbolSize = context.convertToPainterUnits( c.size, s->sizeUnit(), s->sizeMapUnitScale() );
    double tx = ( outputLargestSize - outputSymbolSize ) / 2;

    QgsScopedQPainterState symbolPainterState( p );
    switch ( mVAlign )
    {
      case AlignCenter:
        p->translate( tx, ( outputLargestSize - outputSymbolSize ) / 2 );
        break;
      case AlignBottom:
        p->translate( tx, outputLargestSize - outputSymbolSize );
        break;
    }
    s->drawPreviewIcon( nullptr, QSize( outputSymbolSize, outputSymbolSize ), &context );
  }

  QgsTextFormat format = QgsTextFormat::fromQFont( mFont );
  format.setColor( mTextColor );

  if ( mLineSymbol )
  {
    mLineSymbol->startRender( context );
  }

  int i = 0;
  for ( const SizeClass &c : std::as_const( classes ) )
  {
    // line from symbol to the text
    if ( mLineSymbol )
    {
      mLineSymbol->renderPolyline( QPolygonF() << QPointF( outputLargestSize / 2, symbolTopY[i] )
                                   << QPointF( outputLargestSize + hLengthLine, textCenterY[i] ), nullptr, context );
    }

    // draw label
    QRect rect( outputLargestSize + hLengthLine + hSpaceLineText, textCenterY[i] - textHeight / 2,
                maxTextWidth, textHeight );

    QgsTextRenderer::drawText( rect, 0, QgsTextRenderer::convertQtHAlignment( mTextAlignment ),
                               QStringList() << c.label, context, format );
    i++;
  }

  if ( mLineSymbol )
    mLineSymbol->stopRender( context );
}


QImage QgsDataDefinedSizeLegend::collapsedLegendImage( QgsRenderContext &context, const QColor &backgroundColor, double paddingMM ) const
{
  if ( mType != LegendCollapsed || mSizeClasses.isEmpty() || !mSymbol )
    return QImage();

  // find out the size first
  QSizeF contentSize;
  drawCollapsedLegend( context, &contentSize );

  double padding = context.convertToPainterUnits( paddingMM, Qgis::RenderUnit::Millimeters );
  int dpm = std::round( context.scaleFactor() * 1000 );  // scale factor = dots per millimeter

  QImage img( contentSize.width() + padding * 2, contentSize.height() + padding * 2, QImage::Format_ARGB32_Premultiplied );
  img.setDotsPerMeterX( dpm );
  img.setDotsPerMeterY( dpm );
  img.fill( backgroundColor );

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

QgsDataDefinedSizeLegend *QgsDataDefinedSizeLegend::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  if ( elem.isNull() )
    return nullptr;
  QgsDataDefinedSizeLegend *ddsLegend = new QgsDataDefinedSizeLegend;
  ddsLegend->setLegendType( elem.attribute( QStringLiteral( "type" ) ) == QLatin1String( "collapsed" ) ? LegendCollapsed : LegendSeparated );
  ddsLegend->setVerticalAlignment( elem.attribute( QStringLiteral( "valign" ) ) == QLatin1String( "center" ) ? AlignCenter : AlignBottom );
  ddsLegend->setTitle( elem.attribute( QStringLiteral( "title" ) ) );

  QDomElement elemSymbol = elem.firstChildElement( QStringLiteral( "symbol" ) );
  if ( !elemSymbol.isNull() )
  {
    ddsLegend->setSymbol( QgsSymbolLayerUtils::loadSymbol<QgsMarkerSymbol>( elemSymbol, context ) );
  }

  const QDomElement lineSymbolElem = elem.firstChildElement( QStringLiteral( "lineSymbol" ) );
  if ( !lineSymbolElem.isNull() )
  {
    ddsLegend->setLineSymbol( QgsSymbolLayerUtils::loadSymbol<QgsLineSymbol>( lineSymbolElem.firstChildElement(), context ) );
  }

  QgsSizeScaleTransformer *transformer = nullptr;
  QDomElement elemTransformer = elem.firstChildElement( QStringLiteral( "transformer" ) );
  if ( !elemTransformer.isNull() )
  {
    transformer = new QgsSizeScaleTransformer;
    transformer->loadVariant( QgsXmlUtils::readVariant( elemTransformer ) );
  }
  ddsLegend->setSizeScaleTransformer( transformer );

  QDomElement elemTextStyle = elem.firstChildElement( QStringLiteral( "text-style" ) );
  if ( !elemTextStyle.isNull() )
  {
    QDomElement elemFont = elemTextStyle.firstChildElement( QStringLiteral( "font" ) );
    if ( !elemFont.isNull() )
    {
      ddsLegend->setFont( QgsFontUtils::createFont( elemFont.attribute( QStringLiteral( "family" ) ), elemFont.attribute( QStringLiteral( "size" ) ).toInt(),
                          elemFont.attribute( QStringLiteral( "weight" ) ).toInt(), elemFont.attribute( QStringLiteral( "italic" ) ).toInt() ) );
    }
    ddsLegend->setTextColor( QgsColorUtils::colorFromString( elemTextStyle.attribute( QStringLiteral( "color" ) ) ) );
    ddsLegend->setTextAlignment( static_cast<Qt::AlignmentFlag>( elemTextStyle.attribute( QStringLiteral( "align" ) ).toInt() ) );
  }

  QDomElement elemClasses = elem.firstChildElement( QStringLiteral( "classes" ) );
  if ( !elemClasses.isNull() )
  {
    QList<SizeClass> classes;
    QDomElement elemClass = elemClasses.firstChildElement( QStringLiteral( "class" ) );
    while ( !elemClass.isNull() )
    {
      classes << SizeClass( elemClass.attribute( QStringLiteral( "size" ) ).toDouble(), elemClass.attribute( QStringLiteral( "label" ) ) );
      elemClass = elemClass.nextSiblingElement();
    }
    ddsLegend->setClasses( classes );
  }

  return ddsLegend;
}

void QgsDataDefinedSizeLegend::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( QStringLiteral( "type" ), mType == LegendCollapsed ? "collapsed" : "separated" );
  elem.setAttribute( QStringLiteral( "valign" ), mVAlign == AlignCenter ? "center" : "bottom" );
  elem.setAttribute( QStringLiteral( "title" ), mTitleLabel );

  if ( mSymbol )
  {
    QDomElement elemSymbol = QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "source" ), mSymbol.get(), doc, context );
    elem.appendChild( elemSymbol );
  }

  if ( mLineSymbol )
  {
    QDomElement lineSymbolElem = doc.createElement( QStringLiteral( "lineSymbol" ) );
    lineSymbolElem.appendChild( QgsSymbolLayerUtils::saveSymbol( QStringLiteral( "lineSymbol" ), mLineSymbol.get(), doc, context ) );
    elem.appendChild( lineSymbolElem );
  }

  if ( mSizeScaleTransformer )
  {
    QDomElement elemTransformer = QgsXmlUtils::writeVariant( mSizeScaleTransformer->toVariant(), doc );
    elemTransformer.setTagName( QStringLiteral( "transformer" ) );
    elem.appendChild( elemTransformer );
  }

  QDomElement elemFont = doc.createElement( QStringLiteral( "font" ) );
  elemFont.setAttribute( QStringLiteral( "family" ), mFont.family() );
  elemFont.setAttribute( QStringLiteral( "size" ), mFont.pointSize() );
  elemFont.setAttribute( QStringLiteral( "weight" ), mFont.weight() );
  elemFont.setAttribute( QStringLiteral( "italic" ), mFont.italic() );

  QDomElement elemTextStyle = doc.createElement( QStringLiteral( "text-style" ) );
  elemTextStyle.setAttribute( QStringLiteral( "color" ), QgsColorUtils::colorToString( mTextColor ) );
  elemTextStyle.setAttribute( QStringLiteral( "align" ), static_cast<int>( mTextAlignment ) );
  elemTextStyle.appendChild( elemFont );
  elem.appendChild( elemTextStyle );

  if ( !mSizeClasses.isEmpty() )
  {
    QDomElement elemClasses = doc.createElement( QStringLiteral( "classes" ) );
    for ( const SizeClass &sc : std::as_const( mSizeClasses ) )
    {
      QDomElement elemClass = doc.createElement( QStringLiteral( "class" ) );
      elemClass.setAttribute( QStringLiteral( "size" ), sc.size );
      elemClass.setAttribute( QStringLiteral( "label" ), sc.label );
      elemClasses.appendChild( elemClass );
    }
    elem.appendChild( elemClasses );
  }
}
