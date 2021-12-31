/***************************************************************************
  qgstextrenderer.cpp
  -------------------
   begin                : September 2015
   copyright            : (C) Nyall Dawson
   email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextrenderer.h"
#include "qgsvectorlayer.h"
#include "qgstextformat.h"
#include "qgstextdocument.h"
#include "qgstextfragment.h"
#include "qgspallabeling.h"
#include "qgspainteffect.h"
#include "qgspainterswapper.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

#include <optional>

#include <QTextBoundaryFinder>

Q_GUI_EXPORT extern int qt_defaultDpiX();
Q_GUI_EXPORT extern int qt_defaultDpiY();

static void _fixQPictureDPI( QPainter *p )
{
  // QPicture makes an assumption that we drawing to it with system DPI.
  // Then when being drawn, it scales the painter. The following call
  // negates the effect. There is no way of setting QPicture's DPI.
  // See QTBUG-20361
  p->scale( static_cast< double >( qt_defaultDpiX() ) / p->device()->logicalDpiX(),
            static_cast< double >( qt_defaultDpiY() ) / p->device()->logicalDpiY() );
}

QgsTextRenderer::HAlignment QgsTextRenderer::convertQtHAlignment( Qt::Alignment alignment )
{
  if ( alignment & Qt::AlignLeft )
    return AlignLeft;
  else if ( alignment & Qt::AlignRight )
    return AlignRight;
  else if ( alignment & Qt::AlignHCenter )
    return AlignCenter;
  else if ( alignment & Qt::AlignJustify )
    return AlignJustify;

  // not supported?
  return AlignLeft;
}

QgsTextRenderer::VAlignment QgsTextRenderer::convertQtVAlignment( Qt::Alignment alignment )
{
  if ( alignment & Qt::AlignTop )
    return AlignTop;
  else if ( alignment & Qt::AlignBottom )
    return AlignBottom;
  else if ( alignment & Qt::AlignVCenter )
    return AlignVCenter;
  //not supported
  else if ( alignment & Qt::AlignBaseline )
    return AlignBottom;

  return AlignTop;
}

int QgsTextRenderer::sizeToPixel( double size, const QgsRenderContext &c, QgsUnitTypes::RenderUnit unit, const QgsMapUnitScale &mapUnitScale )
{
  return static_cast< int >( c.convertToPainterUnits( size, unit, mapUnitScale ) + 0.5 ); //NOLINT
}

void QgsTextRenderer::drawText( const QRectF &rect, double rotation, QgsTextRenderer::HAlignment alignment, const QStringList &text, QgsRenderContext &context, const QgsTextFormat &format, bool, VAlignment vAlignment, Qgis::TextRendererFlags flags )
{
  QgsTextFormat tmpFormat = format;
  if ( format.dataDefinedProperties().hasActiveProperties() ) // note, we use format instead of tmpFormat here, it's const and potentially avoids a detach
    tmpFormat.updateDataDefinedProperties( context );
  tmpFormat = updateShadowPosition( tmpFormat );

  QStringList textLines;
  for ( const QString &line : text )
  {
    if ( flags & Qgis::TextRendererFlag::WrapLines && textRequiresWrapping( context, line, rect.width(), format ) )
    {
      textLines.append( wrappedText( context, line, rect.width(), format ) );
    }
    else
    {
      textLines.append( line );
    }
  }

  QgsTextDocument document = format.allowHtmlFormatting() ? QgsTextDocument::fromHtml( textLines ) : QgsTextDocument::fromPlainText( textLines );
  document.applyCapitalization( format.capitalization() );

  if ( tmpFormat.background().enabled() )
  {
    drawPart( rect, rotation, alignment, vAlignment, document, context, tmpFormat, Background );
  }

  if ( tmpFormat.buffer().enabled() )
  {
    drawPart( rect, rotation, alignment, vAlignment, document, context, tmpFormat, Buffer );
  }

  drawPart( rect, rotation, alignment, vAlignment, document, context, tmpFormat, Text );
}

void QgsTextRenderer::drawText( QPointF point, double rotation, QgsTextRenderer::HAlignment alignment, const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, bool )
{
  QgsTextFormat tmpFormat = format;
  if ( format.dataDefinedProperties().hasActiveProperties() ) // note, we use format instead of tmpFormat here, it's const and potentially avoids a detach
    tmpFormat.updateDataDefinedProperties( context );
  tmpFormat = updateShadowPosition( tmpFormat );

  QgsTextDocument document = format.allowHtmlFormatting() ? QgsTextDocument::fromHtml( textLines ) : QgsTextDocument::fromPlainText( textLines );
  document.applyCapitalization( format.capitalization() );

  if ( tmpFormat.background().enabled() )
  {
    drawPart( point, rotation, alignment, document, context, tmpFormat, Background );
  }

  if ( tmpFormat.buffer().enabled() )
  {
    drawPart( point, rotation, alignment, document, context, tmpFormat, Buffer );
  }

  drawPart( point, rotation, alignment, document, context, tmpFormat, Text );
}

QgsTextFormat QgsTextRenderer::updateShadowPosition( const QgsTextFormat &format )
{
  if ( !format.shadow().enabled() || format.shadow().shadowPlacement() != QgsTextShadowSettings::ShadowLowest )
    return format;

  QgsTextFormat tmpFormat = format;
  if ( tmpFormat.background().enabled() && tmpFormat.background().type() != QgsTextBackgroundSettings::ShapeMarkerSymbol ) // background shadow not compatible with marker symbol backgrounds
  {
    tmpFormat.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowShape );
  }
  else if ( tmpFormat.buffer().enabled() )
  {
    tmpFormat.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowBuffer );
  }
  else
  {
    tmpFormat.shadow().setShadowPlacement( QgsTextShadowSettings::ShadowText );
  }
  return tmpFormat;
}

void QgsTextRenderer::drawPart( const QRectF &rect, double rotation, HAlignment alignment,
                                const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, QgsTextRenderer::TextPart part, bool )
{
  const QgsTextDocument document = format.allowHtmlFormatting() ? QgsTextDocument::fromHtml( textLines ) : QgsTextDocument::fromPlainText( textLines );

  drawPart( rect, rotation, alignment, AlignTop, document, context, format, part );
}

void QgsTextRenderer::drawPart( const QRectF &rect, double rotation, QgsTextRenderer::HAlignment alignment, VAlignment vAlignment, const QgsTextDocument &document, QgsRenderContext &context, const QgsTextFormat &format, QgsTextRenderer::TextPart part )
{
  if ( !context.painter() )
  {
    return;
  }

  Component component;
  component.dpiRatio = 1.0;
  component.origin = rect.topLeft();
  component.rotation = rotation;
  component.size = rect.size();
  component.hAlign = alignment;

  switch ( part )
  {
    case Background:
    {
      if ( !format.background().enabled() )
        return;

      if ( !qgsDoubleNear( rotation, 0.0 ) )
      {
        // get rotated label's center point

        double xc = rect.width() / 2.0;
        double yc = rect.height() / 2.0;

        double angle = -rotation;
        double xd = xc * std::cos( angle ) - yc * std::sin( angle );
        double yd = xc * std::sin( angle ) + yc * std::cos( angle );

        component.center = QPointF( component.origin.x() + xd, component.origin.y() + yd );
      }
      else
      {
        component.center = rect.center();
      }

      QgsTextRenderer::drawBackground( context, component, format, document, Rect );

      break;
    }

    case Buffer:
    {
      if ( !format.buffer().enabled() )
        break;
    }
    FALLTHROUGH
    case Text:
    case Shadow:
    {
      drawTextInternal( part, context, format, component,
                        document,
                        nullptr,
                        alignment, vAlignment );
      break;
    }
  }
}

void QgsTextRenderer::drawPart( QPointF origin, double rotation, QgsTextRenderer::HAlignment alignment, const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, QgsTextRenderer::TextPart part, bool )
{
  const QgsTextDocument document = format.allowHtmlFormatting() ? QgsTextDocument::fromHtml( textLines ) : QgsTextDocument::fromPlainText( textLines );
  drawPart( origin, rotation, alignment, document, context, format, part );
}

void QgsTextRenderer::drawPart( QPointF origin, double rotation, QgsTextRenderer::HAlignment alignment, const QgsTextDocument &document, QgsRenderContext &context, const QgsTextFormat &format, QgsTextRenderer::TextPart part )
{
  if ( !context.painter() )
  {
    return;
  }

  Component component;
  component.dpiRatio = 1.0;
  component.origin = origin;
  component.rotation = rotation;
  component.hAlign = alignment;

  switch ( part )
  {
    case Background:
    {
      if ( !format.background().enabled() )
        return;

      QgsTextRenderer::drawBackground( context, component, format, document, Point );
      break;
    }

    case Buffer:
    {
      if ( !format.buffer().enabled() )
        break;
    }
    FALLTHROUGH
    case Text:
    case Shadow:
    {
      drawTextInternal( part, context, format, component,
                        document,
                        nullptr,
                        alignment, AlignTop,
                        Point );
      break;
    }
  }
}

QFontMetricsF QgsTextRenderer::fontMetrics( QgsRenderContext &context, const QgsTextFormat &format, const double scaleFactor )
{
  return QFontMetricsF( format.scaledFont( context, scaleFactor ), context.painter() ? context.painter()->device() : nullptr );
}

double QgsTextRenderer::drawBuffer( QgsRenderContext &context, const QgsTextRenderer::Component &component, const QgsTextFormat &format,
                                    DrawMode mode )
{
  QPainter *p = context.painter();

  QgsTextFormat::TextOrientation orientation = format.orientation();
  if ( format.orientation() == QgsTextFormat::RotationBasedOrientation )
  {
    if ( component.rotation >= -315 && component.rotation < -90 )
    {
      orientation = QgsTextFormat::VerticalOrientation;
    }
    else if ( component.rotation >= -90 && component.rotation < -45 )
    {
      orientation = QgsTextFormat::VerticalOrientation;
    }
    else
    {
      orientation = QgsTextFormat::HorizontalOrientation;
    }
  }

  QgsTextBufferSettings buffer = format.buffer();

  const double penSize =  buffer.sizeUnit() == QgsUnitTypes::RenderPercentage
                          ? context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() ) * buffer.size() / 100
                          : context.convertToPainterUnits( buffer.size(), buffer.sizeUnit(), buffer.sizeMapUnitScale() );

  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
  if ( mode == Label )
  {
    // label size has already been calculated using any symbology reference scale factor -- we need
    // to temporarily remove the reference scale here or we'll be applying the scaling twice
    referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
  }

  bool isNullSize = false;
  const QFont font = format.scaledFont( context, scaleFactor, &isNullSize );
  if ( isNullSize )
    return 0;

  referenceScaleOverride.reset();

  QPainterPath path;
  path.setFillRule( Qt::WindingFill );
  double advance = 0;
  switch ( orientation )
  {
    case QgsTextFormat::HorizontalOrientation:
    {
      double xOffset = 0;
      for ( const QgsTextFragment &fragment : component.block )
      {
        QFont fragmentFont = font;
        fragment.characterFormat().updateFontForFormat( fragmentFont, scaleFactor );

        if ( component.extraWordSpacing || component.extraLetterSpacing )
          applyExtraSpacingForLineJustification( fragmentFont, component.extraWordSpacing, component.extraLetterSpacing );

        path.addText( xOffset, 0, fragmentFont, fragment.text() );

        xOffset += fragment.horizontalAdvance( fragmentFont, true, scaleFactor );
      }
      advance = xOffset;
      break;
    }

    case QgsTextFormat::VerticalOrientation:
    case QgsTextFormat::RotationBasedOrientation:
    {
      double letterSpacing = font.letterSpacing();
      double partYOffset = component.offset.y() * scaleFactor;
      for ( const QgsTextFragment &fragment : component.block )
      {
        QFont fragmentFont = font;
        fragment.characterFormat().updateFontForFormat( fragmentFont, scaleFactor );

        QFontMetricsF fragmentMetrics( fragmentFont );
        const double labelWidth = fragmentMetrics.maxWidth();

        const QStringList parts = QgsPalLabeling::splitToGraphemes( fragment.text() );
        for ( const QString &part : parts )
        {
          double partXOffset = ( labelWidth - ( fragmentMetrics.horizontalAdvance( part ) - letterSpacing ) ) / 2;
          path.addText( partXOffset, partYOffset, fragmentFont, part );
          partYOffset += fragmentMetrics.ascent() + letterSpacing;
        }
      }
      advance = partYOffset - component.offset.y() * scaleFactor;
      break;
    }
  }

  QColor bufferColor = buffer.color();
  bufferColor.setAlphaF( buffer.opacity() );
  QPen pen( bufferColor );
  pen.setWidthF( penSize * scaleFactor );
  pen.setJoinStyle( buffer.joinStyle() );
  QColor tmpColor( bufferColor );
  // honor pref for whether to fill buffer interior
  if ( !buffer.fillBufferInterior() )
  {
    tmpColor.setAlpha( 0 );
  }

  // store buffer's drawing in QPicture for drop shadow call
  QPicture buffPict;
  QPainter buffp;
  buffp.begin( &buffPict );
  if ( buffer.paintEffect() && buffer.paintEffect()->enabled() )
  {
    context.setPainter( &buffp );
    std::unique_ptr< QgsPaintEffect > tmpEffect( buffer.paintEffect()->clone() );

    tmpEffect->begin( context );
    context.painter()->setPen( pen );
    context.painter()->setBrush( tmpColor );
    if ( scaleFactor != 1.0 )
      context.painter()->scale( 1 / scaleFactor, 1 / scaleFactor );
    context.painter()->drawPath( path );
    if ( scaleFactor != 1.0 )
      context.painter()->scale( scaleFactor, scaleFactor );
    tmpEffect->end( context );

    context.setPainter( p );
  }
  else
  {
    if ( scaleFactor != 1.0 )
      buffp.scale( 1 / scaleFactor, 1 / scaleFactor );
    buffp.setPen( pen );
    buffp.setBrush( tmpColor );
    buffp.drawPath( path );
  }
  buffp.end();

  if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowBuffer )
  {
    QgsTextRenderer::Component bufferComponent = component;
    bufferComponent.origin = QPointF( 0.0, 0.0 );
    bufferComponent.picture = buffPict;
    bufferComponent.pictureBuffer = penSize / 2.0;

    if ( format.orientation() == QgsTextFormat::VerticalOrientation || format.orientation() == QgsTextFormat::RotationBasedOrientation )
    {
      bufferComponent.offset.setY( bufferComponent.offset.y() - bufferComponent.size.height() );
    }
    drawShadow( context, bufferComponent, format );
  }

  QgsScopedQPainterState painterState( p );
  context.setPainterFlagsUsingContext( p );

  if ( context.useAdvancedEffects() )
  {
    p->setCompositionMode( buffer.blendMode() );
  }

  // scale for any print output or image saving @ specific dpi
  p->scale( component.dpiRatio, component.dpiRatio );
  _fixQPictureDPI( p );
  p->drawPicture( 0, 0, buffPict );

  return advance / scaleFactor;
}

void QgsTextRenderer::drawMask( QgsRenderContext &context, const QgsTextRenderer::Component &component, const QgsTextFormat &format,
                                DrawMode mode )
{
  QgsTextMaskSettings mask = format.mask();

  // the mask is drawn to a side painter
  // or to the main painter for preview
  QPainter *p = context.isGuiPreview() ? context.painter() : context.maskPainter( context.currentMaskId() );
  if ( ! p )
    return;

  double penSize = mask.sizeUnit() == QgsUnitTypes::RenderPercentage
                   ? context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() ) * mask.size() / 100
                   : context.convertToPainterUnits( mask.size(), mask.sizeUnit(), mask.sizeMapUnitScale() );

  // buffer: draw the text with a big pen
  QPainterPath path;
  path.setFillRule( Qt::WindingFill );

  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  // TODO: vertical text mode was ignored when masking feature was added.
  // Hopefully Oslandia come back and fix this? Hint hint...

  std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
  if ( mode == Label )
  {
    // label size has already been calculated using any symbology reference scale factor -- we need
    // to temporarily remove the reference scale here or we'll be applying the scaling twice
    referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
  }

  bool isNullSize = false;
  const QFont font = format.scaledFont( context, scaleFactor, &isNullSize );
  if ( isNullSize )
    return;

  referenceScaleOverride.reset();

  double xOffset = 0;
  for ( const QgsTextFragment &fragment : component.block )
  {
    QFont fragmentFont = font;
    fragment.characterFormat().updateFontForFormat( fragmentFont, scaleFactor );

    path.addText( xOffset, 0, fragmentFont, fragment.text() );

    xOffset += fragment.horizontalAdvance( fragmentFont, true );
  }

  QColor bufferColor( Qt::gray );
  bufferColor.setAlphaF( mask.opacity() );

  QPen pen;
  QBrush brush;
  brush.setColor( bufferColor );
  pen.setColor( bufferColor );
  pen.setWidthF( penSize * scaleFactor );
  pen.setJoinStyle( mask.joinStyle() );

  QgsScopedQPainterState painterState( p );
  context.setPainterFlagsUsingContext( p );

  // scale for any print output or image saving @ specific dpi
  p->scale( component.dpiRatio, component.dpiRatio );
  if ( mask.paintEffect() && mask.paintEffect()->enabled() )
  {
    QgsPainterSwapper swapper( context, p );
    {
      QgsEffectPainter effectPainter( context, mask.paintEffect() );
      if ( scaleFactor != 1.0 )
        context.painter()->scale( 1 / scaleFactor, 1 / scaleFactor );
      context.painter()->setPen( pen );
      context.painter()->setBrush( brush );
      context.painter()->drawPath( path );
      if ( scaleFactor != 1.0 )
        context.painter()->scale( scaleFactor, scaleFactor );
    }
  }
  else
  {
    if ( scaleFactor != 1.0 )
      p->scale( 1 / scaleFactor, 1 / scaleFactor );
    p->setPen( pen );
    p->setBrush( brush );
    p->drawPath( path );
    if ( scaleFactor != 1.0 )
      p->scale( scaleFactor, scaleFactor );

  }
}

double QgsTextRenderer::textWidth( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines, QFontMetricsF * )
{
  QgsTextDocument doc;
  if ( !format.allowHtmlFormatting() )
  {
    doc = QgsTextDocument::fromPlainText( textLines );
  }
  else
  {
    doc = QgsTextDocument::fromHtml( textLines );
  }
  doc.applyCapitalization( format.capitalization() );
  return textWidth( context, format, doc );
}

double QgsTextRenderer::textWidth( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &document )
{
  //calculate max width of text lines
  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  bool isNullSize = false;
  const QFont baseFont = format.scaledFont( context, scaleFactor, &isNullSize );
  if ( isNullSize )
    return 0;

  double width = 0;
  switch ( format.orientation() )
  {
    case QgsTextFormat::HorizontalOrientation:
    {
      double maxLineWidth = 0;
      for ( const QgsTextBlock &block : document )
      {
        double blockWidth = 0;
        for ( const QgsTextFragment &fragment : block )
        {
          blockWidth += fragment.horizontalAdvance( baseFont, scaleFactor );
        }
        maxLineWidth = std::max( maxLineWidth, blockWidth );
      }
      width = maxLineWidth;
      break;
    }

    case QgsTextFormat::VerticalOrientation:
    {
      double totalLineWidth = 0;
      int blockIndex = 0;
      for ( const QgsTextBlock &block : document )
      {
        double blockWidth = 0;
        for ( const QgsTextFragment &fragment : block )
        {
          QFont fragmentFont = baseFont;
          fragment.characterFormat().updateFontForFormat( fragmentFont, scaleFactor );
          blockWidth = std::max( QFontMetricsF( fragmentFont ).maxWidth(), blockWidth );
        }

        totalLineWidth += blockIndex == 0 ? blockWidth : blockWidth * format.lineHeight();
        blockIndex++;
      }
      width = totalLineWidth;
      break;
    }

    case QgsTextFormat::RotationBasedOrientation:
    {
      // label mode only
      break;
    }
  }

  return width / scaleFactor;
}

double QgsTextRenderer::textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines, DrawMode mode, QFontMetricsF *, Qgis::TextRendererFlags flags, double maxLineWidth )
{
  QStringList lines;
  for ( const QString &line : textLines )
  {
    if ( flags & Qgis::TextRendererFlag::WrapLines && maxLineWidth > 0 && textRequiresWrapping( context, line, maxLineWidth, format ) )
    {
      lines.append( wrappedText( context, line, maxLineWidth, format ) );
    }
    else
    {
      lines.append( line );
    }
  }

  if ( !format.allowHtmlFormatting() )
  {
    return textHeight( context, format, QgsTextDocument::fromPlainText( lines ), mode );
  }
  else
  {
    return textHeight( context, format, QgsTextDocument::fromHtml( lines ), mode );
  }
}

double QgsTextRenderer::textHeight( const QgsRenderContext &context, const QgsTextFormat &format, QChar character, bool includeEffects )
{
  const double scaleFactor = calculateScaleFactorForFormat( context, format );
  bool isNullSize = false;
  const QFont baseFont = format.scaledFont( context, scaleFactor, &isNullSize );
  if ( isNullSize )
    return 0;

  const QFontMetrics fm( baseFont );
  const double height = ( character.isNull() ? fm.height() : fm.boundingRect( character ).height() ) / scaleFactor;

  if ( !includeEffects )
    return height;

  double maxExtension = 0;
  const double fontSize = context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() );
  if ( format.buffer().enabled() )
  {
    maxExtension += format.buffer().sizeUnit() == QgsUnitTypes::RenderPercentage
                    ? fontSize * format.buffer().size() / 100
                    : context.convertToPainterUnits( format.buffer().size(), format.buffer().sizeUnit(), format.buffer().sizeMapUnitScale() );
  }
  if ( format.shadow().enabled() )
  {
    maxExtension += ( format.shadow().offsetUnit() == QgsUnitTypes::RenderPercentage
                      ? fontSize * format.shadow().offsetDistance() / 100
                      : context.convertToPainterUnits( format.shadow().offsetDistance(), format.shadow().offsetUnit(), format.shadow().offsetMapUnitScale() )
                    )
                    + ( format.shadow().blurRadiusUnit() == QgsUnitTypes::RenderPercentage
                        ? fontSize * format.shadow().blurRadius() / 100
                        : context.convertToPainterUnits( format.shadow().blurRadius(), format.shadow().blurRadiusUnit(), format.shadow().blurRadiusMapUnitScale() )
                      );
  }
  if ( format.background().enabled() )
  {
    maxExtension += context.convertToPainterUnits( std::fabs( format.background().offset().y() ), format.background().offsetUnit(), format.background().offsetMapUnitScale() )
                    + context.convertToPainterUnits( format.background().strokeWidth(), format.background().strokeWidthUnit(), format.background().strokeWidthMapUnitScale() ) / 2.0;
    if ( format.background().sizeType() == QgsTextBackgroundSettings::SizeBuffer && format.background().size().height() > 0 )
    {
      maxExtension += context.convertToPainterUnits( format.background().size().height(), format.background().sizeUnit(), format.background().sizeMapUnitScale() );
    }
  }

  return height + maxExtension;
}

bool QgsTextRenderer::textRequiresWrapping( const QgsRenderContext &context, const QString &text, double width, const QgsTextFormat &format )
{
  if ( qgsDoubleNear( width, 0.0 ) )
    return false;

  const QStringList multiLineSplit = text.split( '\n' );
  const double currentTextWidth = QgsTextRenderer::textWidth( context, format, multiLineSplit );
  return currentTextWidth > width;
}

QStringList QgsTextRenderer::wrappedText( const QgsRenderContext &context, const QString &text, double width, const QgsTextFormat &format )
{
  const QStringList lines = text.split( '\n' );
  QStringList outLines;
  for ( const QString &line : lines )
  {
    if ( textRequiresWrapping( context, line, width, format ) )
    {
      //first step is to identify words which must be on their own line (too long to fit)
      const QStringList words = line.split( ' ' );
      QStringList linesToProcess;
      QString wordsInCurrentLine;
      for ( const QString &word : words )
      {
        if ( textRequiresWrapping( context, word, width, format ) )
        {
          //too long to fit
          if ( !wordsInCurrentLine.isEmpty() )
            linesToProcess << wordsInCurrentLine;
          wordsInCurrentLine.clear();
          linesToProcess << word;
        }
        else
        {
          if ( !wordsInCurrentLine.isEmpty() )
            wordsInCurrentLine.append( ' ' );
          wordsInCurrentLine.append( word );
        }
      }
      if ( !wordsInCurrentLine.isEmpty() )
        linesToProcess << wordsInCurrentLine;

      for ( const QString &line : std::as_const( linesToProcess ) )
      {
        QString remainingText = line;
        int lastPos = remainingText.lastIndexOf( ' ' );
        while ( lastPos > -1 )
        {
          //check if remaining text is short enough to go in one line
          if ( !textRequiresWrapping( context, remainingText, width, format ) )
          {
            break;
          }

          if ( !textRequiresWrapping( context, remainingText.left( lastPos ), width, format ) )
          {
            outLines << remainingText.left( lastPos );
            remainingText = remainingText.mid( lastPos + 1 );
            lastPos = 0;
          }
          lastPos = remainingText.lastIndexOf( ' ', lastPos - 1 );
        }
        outLines << remainingText;
      }
    }
    else
    {
      outLines << line;
    }
  }

  return outLines;
}

double QgsTextRenderer::textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &doc, DrawMode mode )
{
  QgsTextDocument document = doc;
  document.applyCapitalization( format.capitalization() );

  //calculate max height of text lines
  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  bool isNullSize = false;
  const QFont baseFont = format.scaledFont( context, scaleFactor, &isNullSize );
  if ( isNullSize )
    return 0;

  switch ( format.orientation() )
  {
    case QgsTextFormat::HorizontalOrientation:
    {
      int blockIndex = 0;
      double totalHeight = 0;
      double lastLineLeading = 0;
      for ( const QgsTextBlock &block : document )
      {
        double maxBlockHeight = 0;
        double maxBlockLineSpacing = 0;
        double maxBlockLeading = 0;
        for ( const QgsTextFragment &fragment : block )
        {
          QFont fragmentFont = baseFont;
          fragment.characterFormat().updateFontForFormat( fragmentFont, scaleFactor );
          const QFontMetricsF fm( fragmentFont );

          const double fragmentHeight = fm.ascent() + fm.descent(); // ignore +1 for baseline

          maxBlockHeight = std::max( maxBlockHeight, fragmentHeight );
          if ( fm.lineSpacing() > maxBlockLineSpacing )
          {
            maxBlockLineSpacing = fm.lineSpacing();
            maxBlockLeading = fm.leading();
          }
        }

        switch ( mode )
        {
          case Label:
            // rendering labels needs special handling - in this case text should be
            // drawn with the bottom left corner coinciding with origin, vs top left
            // for standard text rendering. Line height is also slightly different.
            totalHeight += blockIndex == 0 ? maxBlockHeight : maxBlockHeight * format.lineHeight();
            break;

          case Rect:
          case Point:
            // standard rendering - designed to exactly replicate QPainter's drawText method
            totalHeight += blockIndex == 0 ? maxBlockHeight : maxBlockLineSpacing * format.lineHeight();
            if ( blockIndex > 0 )
              lastLineLeading = maxBlockLeading;
            break;
        }

        blockIndex++;
      }

      return ( totalHeight - lastLineLeading ) / scaleFactor;
    }

    case QgsTextFormat::VerticalOrientation:
    {
      double maxBlockHeight = 0;
      for ( const QgsTextBlock &block : document )
      {
        double blockHeight = 0;
        int fragmentIndex = 0;
        for ( const QgsTextFragment &fragment : block )
        {
          QFont fragmentFont = baseFont;
          fragment.characterFormat().updateFontForFormat( fragmentFont, scaleFactor );
          const QFontMetricsF fm( fragmentFont );

          const double labelHeight = fm.ascent();
          const double letterSpacing = fragmentFont.letterSpacing();

          blockHeight += fragmentIndex = 0 ? labelHeight * fragment.text().size() + ( fragment.text().size() - 1 ) * letterSpacing
                                         : fragment.text().size() * ( labelHeight + letterSpacing );
          fragmentIndex++;
        }
        maxBlockHeight = std::max( maxBlockHeight, blockHeight );
      }

      return maxBlockHeight / scaleFactor;
    }

    case QgsTextFormat::RotationBasedOrientation:
    {
      // label mode only
      break;
    }
  }

  return 0;
}

void QgsTextRenderer::drawBackground( QgsRenderContext &context, QgsTextRenderer::Component component, const QgsTextFormat &format, const QgsTextDocument &document, QgsTextRenderer::DrawMode mode )
{
  QgsTextBackgroundSettings background = format.background();

  QPainter *prevP = context.painter();
  QPainter *p = context.painter();
  std::unique_ptr< QgsPaintEffect > tmpEffect;
  if ( background.paintEffect() && background.paintEffect()->enabled() )
  {
    tmpEffect.reset( background.paintEffect()->clone() );
    tmpEffect->begin( context );
    p = context.painter();
  }

  //QgsDebugMsgLevel( QStringLiteral( "Background label rotation: %1" ).arg( component.rotation() ), 4 );

  // shared calculations between shapes and SVG

  // configure angles, set component rotation and rotationOffset
  const double originAdjustRotationRadians = -component.rotation;
  if ( background.rotationType() != QgsTextBackgroundSettings::RotationFixed )
  {
    component.rotation = -( component.rotation * 180 / M_PI ); // RotationSync
    component.rotationOffset =
      background.rotationType() == QgsTextBackgroundSettings::RotationOffset ? background.rotation() : 0.0;
  }
  else // RotationFixed
  {
    component.rotation = 0.0; // don't use label's rotation
    component.rotationOffset = background.rotation();
  }

  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  if ( mode != Label )
  {
    // need to calculate size of text
    double width = textWidth( context, format, document );
    double height = textHeight( context, format, document, mode );

    switch ( mode )
    {
      case Rect:
        switch ( component.hAlign )
        {
          case AlignLeft:
          case AlignJustify:
            component.center = QPointF( component.origin.x() + width / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;

          case AlignCenter:
            component.center = QPointF( component.origin.x() + component.size.width() / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;

          case AlignRight:
            component.center = QPointF( component.origin.x() + component.size.width() - width / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;
        }
        break;

      case Point:
      {
        bool isNullSize = false;
        QFontMetricsF fm( format.scaledFont( context, scaleFactor, &isNullSize ) );
        double originAdjust = isNullSize ? 0 : ( fm.ascent() / scaleFactor / 2.0 - fm.leading() / scaleFactor / 2.0 );
        switch ( component.hAlign )
        {
          case AlignLeft:
          case AlignJustify:
            component.center = QPointF( component.origin.x() + width / 2.0,
                                        component.origin.y() - height / 2.0 + originAdjust );
            break;

          case AlignCenter:
            component.center = QPointF( component.origin.x(),
                                        component.origin.y() - height / 2.0 + originAdjust );
            break;

          case AlignRight:
            component.center = QPointF( component.origin.x() - width / 2.0,
                                        component.origin.y() - height / 2.0 + originAdjust );
            break;
        }

        // apply rotation to center point
        if ( !qgsDoubleNear( originAdjustRotationRadians, 0 ) )
        {
          const double dx = component.center.x() - component.origin.x();
          const double dy = component.center.y() - component.origin.y();
          component.center.setX( component.origin.x() + ( std::cos( originAdjustRotationRadians ) * dx - std::sin( originAdjustRotationRadians ) * dy ) );
          component.center.setY( component.origin.y() + ( std::sin( originAdjustRotationRadians ) * dx + std::cos( originAdjustRotationRadians ) * dy ) );
        }
        break;
      }

      case Label:
        break;
    }

    if ( format.background().sizeType() != QgsTextBackgroundSettings::SizeFixed )
      component.size = QSizeF( width, height );
  }

  // TODO: the following label-buffered generated shapes and SVG symbols should be moved into marker symbology classes

  switch ( background.type() )
  {
    case QgsTextBackgroundSettings::ShapeSVG:
    case QgsTextBackgroundSettings::ShapeMarkerSymbol:
    {
      // all calculations done in shapeSizeUnits, which are then passed to symbology class for painting

      if ( background.type() == QgsTextBackgroundSettings::ShapeSVG && background.svgFile().isEmpty() )
        return;

      if ( background.type() == QgsTextBackgroundSettings::ShapeMarkerSymbol && !background.markerSymbol() )
        return;

      double sizeOut = 0.0;
      // only one size used for SVG/marker symbol sizing/scaling (no use of shapeSize.y() or Y field in gui)
      if ( background.sizeType() == QgsTextBackgroundSettings::SizeFixed )
      {
        sizeOut = context.convertToPainterUnits( background.size().width(), background.sizeUnit(), background.sizeMapUnitScale() );
      }
      else if ( background.sizeType() == QgsTextBackgroundSettings::SizeBuffer )
      {
        sizeOut = std::max( component.size.width(), component.size.height() );
        double bufferSize = context.convertToPainterUnits( background.size().width(), background.sizeUnit(), background.sizeMapUnitScale() );

        // add buffer
        sizeOut += bufferSize * 2;
      }

      // don't bother rendering symbols smaller than 1x1 pixels in size
      // TODO: add option to not show any svgs under/over a certain size
      if ( sizeOut < 1.0 )
        return;

      std::unique_ptr< QgsMarkerSymbol > renderedSymbol;
      if ( background.type() == QgsTextBackgroundSettings::ShapeSVG )
      {
        QVariantMap map; // for SVG symbology marker
        map[QStringLiteral( "name" )] = background.svgFile().trimmed();
        map[QStringLiteral( "size" )] = QString::number( sizeOut );
        map[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( QgsUnitTypes::RenderPixels );
        map[QStringLiteral( "angle" )] = QString::number( 0.0 ); // angle is handled by this local painter

        // offset is handled by this local painter
        // TODO: see why the marker renderer doesn't seem to translate offset *after* applying rotation
        //map["offset"] = QgsSymbolLayerUtils::encodePoint( tmpLyr.shapeOffset );
        //map["offset_unit"] = QgsUnitTypes::encodeUnit(
        //                       tmpLyr.shapeOffsetUnits == QgsPalLayerSettings::MapUnits ? QgsUnitTypes::MapUnit : QgsUnitTypes::MM );

        map[QStringLiteral( "fill" )] = background.fillColor().name();
        map[QStringLiteral( "outline" )] = background.strokeColor().name();
        map[QStringLiteral( "outline-width" )] = QString::number( background.strokeWidth() );
        map[QStringLiteral( "outline_width_unit" )] = QgsUnitTypes::encodeUnit( background.strokeWidthUnit() );

        if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowShape )
        {
          QgsTextShadowSettings shadow = format.shadow();
          // configure SVG shadow specs
          QVariantMap shdwmap( map );
          shdwmap[QStringLiteral( "fill" )] = shadow.color().name();
          shdwmap[QStringLiteral( "outline" )] = shadow.color().name();
          shdwmap[QStringLiteral( "size" )] = QString::number( sizeOut );

          // store SVG's drawing in QPicture for drop shadow call
          QPicture svgPict;
          QPainter svgp;
          svgp.begin( &svgPict );

          // draw shadow symbol

          // clone current render context map unit/mm conversion factors, but not
          // other map canvas parameters, then substitute this painter for use in symbology painting
          // NOTE: this is because the shadow needs to be scaled correctly for output to map canvas,
          //       but will be created relative to the SVG's computed size, not the current map canvas
          QgsRenderContext shdwContext;
          shdwContext.setMapToPixel( context.mapToPixel() );
          shdwContext.setScaleFactor( context.scaleFactor() );
          shdwContext.setPainter( &svgp );

          std::unique_ptr< QgsSymbolLayer > symShdwL( QgsSvgMarkerSymbolLayer::create( shdwmap ) );
          QgsSvgMarkerSymbolLayer *svgShdwM = static_cast<QgsSvgMarkerSymbolLayer *>( symShdwL.get() );
          QgsSymbolRenderContext svgShdwContext( shdwContext, QgsUnitTypes::RenderUnknownUnit, background.opacity() );

          svgShdwM->renderPoint( QPointF( sizeOut / 2, -sizeOut / 2 ), svgShdwContext );
          svgp.end();

          component.picture = svgPict;
          // TODO: when SVG symbol's stroke width/units is fixed in QgsSvgCache, adjust for it here
          component.pictureBuffer = 0.0;

          component.size = QSizeF( sizeOut, sizeOut );
          component.offset = QPointF( 0.0, 0.0 );

          // rotate about origin center of SVG
          QgsScopedQPainterState painterState( p );
          context.setPainterFlagsUsingContext( p );

          p->translate( component.center.x(), component.center.y() );
          p->rotate( component.rotation );
          double xoff = context.convertToPainterUnits( background.offset().x(), background.offsetUnit(), background.offsetMapUnitScale() );
          double yoff = context.convertToPainterUnits( background.offset().y(), background.offsetUnit(), background.offsetMapUnitScale() );
          p->translate( QPointF( xoff, yoff ) );
          p->rotate( component.rotationOffset );
          p->translate( -sizeOut / 2, sizeOut / 2 );

          drawShadow( context, component, format );
        }
        renderedSymbol.reset( );

        QgsSymbolLayer *symL = QgsSvgMarkerSymbolLayer::create( map );
        renderedSymbol.reset( new QgsMarkerSymbol( QgsSymbolLayerList() << symL ) );
      }
      else
      {
        renderedSymbol.reset( background.markerSymbol()->clone() );
        renderedSymbol->setSize( sizeOut );
        renderedSymbol->setSizeUnit( QgsUnitTypes::RenderPixels );
      }

      renderedSymbol->setOpacity( renderedSymbol->opacity() * background.opacity() );

      // draw the actual symbol
      QgsScopedQPainterState painterState( p );
      context.setPainterFlagsUsingContext( p );

      if ( context.useAdvancedEffects() )
      {
        p->setCompositionMode( background.blendMode() );
      }
      p->translate( component.center.x(), component.center.y() );
      p->rotate( component.rotation );
      double xoff = context.convertToPainterUnits( background.offset().x(), background.offsetUnit(), background.offsetMapUnitScale() );
      double yoff = context.convertToPainterUnits( background.offset().y(), background.offsetUnit(), background.offsetMapUnitScale() );
      p->translate( QPointF( xoff, yoff ) );
      p->rotate( component.rotationOffset );

      const QgsFeature f = context.expressionContext().feature();
      renderedSymbol->startRender( context, context.expressionContext().fields() );
      renderedSymbol->renderPoint( QPointF( 0, 0 ), &f, context );
      renderedSymbol->stopRender( context );
      p->setCompositionMode( QPainter::CompositionMode_SourceOver ); // just to be sure

      break;
    }

    case QgsTextBackgroundSettings::ShapeRectangle:
    case QgsTextBackgroundSettings::ShapeCircle:
    case QgsTextBackgroundSettings::ShapeSquare:
    case QgsTextBackgroundSettings::ShapeEllipse:
    {
      double w = component.size.width();
      double h = component.size.height();

      if ( background.sizeType() == QgsTextBackgroundSettings::SizeFixed )
      {
        w = context.convertToPainterUnits( background.size().width(), background.sizeUnit(),
                                           background.sizeMapUnitScale() );
        h = context.convertToPainterUnits( background.size().height(), background.sizeUnit(),
                                           background.sizeMapUnitScale() );
      }
      else if ( background.sizeType() == QgsTextBackgroundSettings::SizeBuffer )
      {
        if ( background.type() == QgsTextBackgroundSettings::ShapeSquare )
        {
          if ( w > h )
            h = w;
          else if ( h > w )
            w = h;
        }
        else if ( background.type() == QgsTextBackgroundSettings::ShapeCircle )
        {
          // start with label bound by circle
          h = std::sqrt( std::pow( w, 2 ) + std::pow( h, 2 ) );
          w = h;
        }
        else if ( background.type() == QgsTextBackgroundSettings::ShapeEllipse )
        {
          // start with label bound by ellipse
          h = h * M_SQRT1_2 * 2;
          w = w * M_SQRT1_2 * 2;
        }

        double bufferWidth = context.convertToPainterUnits( background.size().width(), background.sizeUnit(),
                             background.sizeMapUnitScale() );
        double bufferHeight = context.convertToPainterUnits( background.size().height(), background.sizeUnit(),
                              background.sizeMapUnitScale() );

        w += bufferWidth * 2;
        h += bufferHeight * 2;
      }

      // offsets match those of symbology: -x = left, -y = up
      QRectF rect( -w / 2.0, - h / 2.0, w, h );

      if ( rect.isNull() )
        return;

      QgsScopedQPainterState painterState( p );
      context.setPainterFlagsUsingContext( p );

      p->translate( QPointF( component.center.x(), component.center.y() ) );
      p->rotate( component.rotation );
      double xoff = context.convertToPainterUnits( background.offset().x(), background.offsetUnit(), background.offsetMapUnitScale() );
      double yoff = context.convertToPainterUnits( background.offset().y(), background.offsetUnit(), background.offsetMapUnitScale() );
      p->translate( QPointF( xoff, yoff ) );
      p->rotate( component.rotationOffset );

      QPainterPath path;

      // Paths with curves must be enlarged before conversion to QPolygonF, or
      // the curves are approximated too much and appear jaggy
      QTransform t = QTransform::fromScale( 10, 10 );
      // inverse transform used to scale created polygons back to expected size
      QTransform ti = t.inverted();

      if ( background.type() == QgsTextBackgroundSettings::ShapeRectangle
           || background.type() == QgsTextBackgroundSettings::ShapeSquare )
      {
        if ( background.radiiUnit() == QgsUnitTypes::RenderPercentage )
        {
          path.addRoundedRect( rect, background.radii().width(), background.radii().height(), Qt::RelativeSize );
        }
        else
        {
          const double xRadius = context.convertToPainterUnits( background.radii().width(), background.radiiUnit(), background.radiiMapUnitScale() );
          const double yRadius = context.convertToPainterUnits( background.radii().height(), background.radiiUnit(), background.radiiMapUnitScale() );
          path.addRoundedRect( rect, xRadius, yRadius );
        }
      }
      else if ( background.type() == QgsTextBackgroundSettings::ShapeEllipse
                || background.type() == QgsTextBackgroundSettings::ShapeCircle )
      {
        path.addEllipse( rect );
      }
      QPolygonF tempPolygon = path.toFillPolygon( t );
      QPolygonF polygon = ti.map( tempPolygon );
      QPicture shapePict;
      QPainter *oldp = context.painter();
      QPainter shapep;

      shapep.begin( &shapePict );
      context.setPainter( &shapep );

      std::unique_ptr< QgsFillSymbol > renderedSymbol;
      renderedSymbol.reset( background.fillSymbol()->clone() );
      renderedSymbol->setOpacity( renderedSymbol->opacity() * background.opacity() );

      const QgsFeature f = context.expressionContext().feature();
      renderedSymbol->startRender( context, context.expressionContext().fields() );
      renderedSymbol->renderPolygon( polygon, nullptr, &f, context );
      renderedSymbol->stopRender( context );

      shapep.end();
      context.setPainter( oldp );

      if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowShape )
      {
        component.picture = shapePict;
        component.pictureBuffer = QgsSymbolLayerUtils::estimateMaxSymbolBleed( renderedSymbol.get(), context ) * 2;

        component.size = rect.size();
        component.offset = QPointF( rect.width() / 2, -rect.height() / 2 );
        drawShadow( context, component, format );
      }

      if ( context.useAdvancedEffects() )
      {
        p->setCompositionMode( background.blendMode() );
      }

      // scale for any print output or image saving @ specific dpi
      p->scale( component.dpiRatio, component.dpiRatio );
      _fixQPictureDPI( p );
      p->drawPicture( 0, 0, shapePict );
      p->setCompositionMode( QPainter::CompositionMode_SourceOver ); // just to be sure
      break;
    }
  }

  if ( tmpEffect )
  {
    tmpEffect->end( context );
    context.setPainter( prevP );
  }
}

void QgsTextRenderer::drawShadow( QgsRenderContext &context, const QgsTextRenderer::Component &component, const QgsTextFormat &format )
{
  QgsTextShadowSettings shadow = format.shadow();

  // incoming component sizes should be multiplied by rasterCompressFactor, as
  // this allows shadows to be created at paint device dpi (e.g. high resolution),
  // then scale device painter by 1.0 / rasterCompressFactor for output

  QPainter *p = context.painter();
  double componentWidth = component.size.width(), componentHeight = component.size.height();
  double xOffset = component.offset.x(), yOffset = component.offset.y();
  double pictbuffer = component.pictureBuffer;

  // generate pixmap representation of label component drawing
  bool mapUnits = shadow.blurRadiusUnit() == QgsUnitTypes::RenderMapUnits;

  const double fontSize = context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() );
  double radius = shadow.blurRadiusUnit() == QgsUnitTypes::RenderPercentage
                  ? fontSize * shadow.blurRadius() / 100
                  : context.convertToPainterUnits( shadow.blurRadius(), shadow.blurRadiusUnit(), shadow.blurRadiusMapUnitScale() );
  radius /= ( mapUnits ? context.scaleFactor() / component.dpiRatio : 1 );
  radius = static_cast< int >( radius + 0.5 ); //NOLINT

  // TODO: add labeling gui option to adjust blurBufferClippingScale to minimize pixels, or
  //       to ensure shadow isn't clipped too tight. (Or, find a better method of buffering)
  double blurBufferClippingScale = 3.75;
  int blurbuffer = ( radius > 17 ? 16 : radius ) * blurBufferClippingScale;

  QImage blurImg( componentWidth + ( pictbuffer * 2.0 ) + ( blurbuffer * 2.0 ),
                  componentHeight + ( pictbuffer * 2.0 ) + ( blurbuffer * 2.0 ),
                  QImage::Format_ARGB32_Premultiplied );

  // TODO: add labeling gui option to not show any shadows under/over a certain size
  // keep very small QImages from causing paint device issues, i.e. must be at least > 1
  int minBlurImgSize = 1;
  // max limitation on QgsSvgCache is 10,000 for screen, which will probably be reasonable for future caching here, too
  // 4 x QgsSvgCache limit for output to print/image at higher dpi
  // TODO: should it be higher, scale with dpi, or have no limit? Needs testing with very large labels rendered at high dpi output
  int maxBlurImgSize = 40000;
  if ( blurImg.isNull()
       || ( blurImg.width() < minBlurImgSize || blurImg.height() < minBlurImgSize )
       || ( blurImg.width() > maxBlurImgSize || blurImg.height() > maxBlurImgSize ) )
    return;

  blurImg.fill( QColor( Qt::transparent ).rgba() );
  QPainter pictp;
  if ( !pictp.begin( &blurImg ) )
    return;
  pictp.setRenderHints( QPainter::Antialiasing | QPainter::SmoothPixmapTransform );
  QPointF imgOffset( blurbuffer + pictbuffer + xOffset,
                     blurbuffer + pictbuffer + componentHeight + yOffset );

  pictp.drawPicture( imgOffset,
                     component.picture );

  // overlay shadow color
  pictp.setCompositionMode( QPainter::CompositionMode_SourceIn );
  pictp.fillRect( blurImg.rect(), shadow.color() );
  pictp.end();

  // blur the QImage in-place
  if ( shadow.blurRadius() > 0.0 && radius > 0 )
  {
    QgsSymbolLayerUtils::blurImageInPlace( blurImg, blurImg.rect(), radius, shadow.blurAlphaOnly() );
  }

#if 0
  // debug rect for QImage shadow registration and clipping visualization
  QPainter picti;
  picti.begin( &blurImg );
  picti.setBrush( Qt::Dense7Pattern );
  QPen imgPen( QColor( 0, 0, 255, 255 ) );
  imgPen.setWidth( 1 );
  picti.setPen( imgPen );
  picti.setOpacity( 0.1 );
  picti.drawRect( 0, 0, blurImg.width(), blurImg.height() );
  picti.end();
#endif

  const double offsetDist = shadow.offsetUnit() == QgsUnitTypes::RenderPercentage
                            ? fontSize * shadow.offsetDistance() / 100
                            : context.convertToPainterUnits( shadow.offsetDistance(), shadow.offsetUnit(), shadow.offsetMapUnitScale() );
  double angleRad = shadow.offsetAngle() * M_PI / 180; // to radians
  if ( shadow.offsetGlobal() )
  {
    // TODO: check for differences in rotation origin and cw/ccw direction,
    //       when this shadow function is used for something other than labels

    // it's 0-->cw-->360 for labels
    //QgsDebugMsgLevel( QStringLiteral( "Shadow aggregated label rotation (degrees): %1" ).arg( component.rotation() + component.rotationOffset() ), 4 );
    angleRad -= ( component.rotation * M_PI / 180 + component.rotationOffset * M_PI / 180 );
  }

  QPointF transPt( -offsetDist * std::cos( angleRad + M_PI_2 ),
                   -offsetDist * std::sin( angleRad + M_PI_2 ) );

  p->save();
  context.setPainterFlagsUsingContext( p );
  // this was historically ALWAYS set for text renderer. We may want to consider getting it to respect the
  // corresponding flag in the render context instead...
  p->setRenderHint( QPainter::SmoothPixmapTransform );
  if ( context.useAdvancedEffects() )
  {
    p->setCompositionMode( shadow.blendMode() );
  }
  p->setOpacity( shadow.opacity() );

  double scale = shadow.scale() / 100.0;
  // TODO: scale from center/center, left/center or left/top, instead of default left/bottom?
  p->scale( scale, scale );
  if ( component.useOrigin )
  {
    p->translate( component.origin.x(), component.origin.y() );
  }
  p->translate( transPt );
  p->translate( -imgOffset.x(),
                -imgOffset.y() );
  p->drawImage( 0, 0, blurImg );
  p->restore();

  // debug rects
#if 0
  // draw debug rect for QImage painting registration
  p->save();
  p->setBrush( Qt::NoBrush );
  QPen imgPen( QColor( 255, 0, 0, 10 ) );
  imgPen.setWidth( 2 );
  imgPen.setStyle( Qt::DashLine );
  p->setPen( imgPen );
  p->scale( scale, scale );
  if ( component.useOrigin() )
  {
    p->translate( component.origin().x(), component.origin().y() );
  }
  p->translate( transPt );
  p->translate( -imgOffset.x(),
                -imgOffset.y() );
  p->drawRect( 0, 0, blurImg.width(), blurImg.height() );
  p->restore();

  // draw debug rect for passed in component dimensions
  p->save();
  p->setBrush( Qt::NoBrush );
  QPen componentRectPen( QColor( 0, 255, 0, 70 ) );
  componentRectPen.setWidth( 1 );
  if ( component.useOrigin() )
  {
    p->translate( component.origin().x(), component.origin().y() );
  }
  p->setPen( componentRectPen );
  p->drawRect( QRect( -xOffset, -componentHeight - yOffset, componentWidth, componentHeight ) );
  p->restore();
#endif
}


void QgsTextRenderer::drawTextInternal( TextPart drawType,
                                        QgsRenderContext &context,
                                        const QgsTextFormat &format,
                                        const Component &component,
                                        const QgsTextDocument &document,
                                        const QFontMetricsF *fontMetrics,
                                        HAlignment alignment, VAlignment vAlignment, DrawMode mode )
{
  if ( !context.painter() )
  {
    return;
  }

  double fontScale = 1.0;
  std::unique_ptr< QFontMetricsF > tmpMetrics;
  if ( !fontMetrics )
  {
    fontScale = calculateScaleFactorForFormat( context, format );

    std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
    if ( mode == Label )
    {
      // label size has already been calculated using any symbology reference scale factor -- we need
      // to temporarily remove the reference scale here or we'll be applying the scaling twice
      referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
    }

    bool isNullSize = false;
    const QFont f = format.scaledFont( context, fontScale, &isNullSize );
    if ( isNullSize )
      return;

    tmpMetrics = std::make_unique< QFontMetricsF >( f );
    fontMetrics = tmpMetrics.get();

    referenceScaleOverride.reset();
  }

  double rotation = 0;
  const QgsTextFormat::TextOrientation orientation = calculateRotationAndOrientationForComponent( format, component, rotation );
  switch ( orientation )
  {
    case QgsTextFormat::HorizontalOrientation:
    {
      drawTextInternalHorizontal( context, format, drawType, mode, component, document, fontScale, fontMetrics, alignment, vAlignment, rotation );
      break;
    }

    case QgsTextFormat::VerticalOrientation:
    case QgsTextFormat::RotationBasedOrientation:
    {
      drawTextInternalVertical( context, format, drawType, mode, component, document, fontScale, fontMetrics, alignment, vAlignment, rotation );
      break;
    }
  }
}

QgsTextFormat::TextOrientation QgsTextRenderer::calculateRotationAndOrientationForComponent( const QgsTextFormat &format, const QgsTextRenderer::Component &component, double &rotation )
{
  rotation = -component.rotation * 180 / M_PI;

  switch ( format.orientation() )
  {
    case QgsTextFormat::RotationBasedOrientation:
    {
      // Between 45 to 135 and 235 to 315 degrees, rely on vertical orientation
      if ( rotation >= -315 && rotation < -90 )
      {
        rotation -= 90;
        return QgsTextFormat::VerticalOrientation;
      }
      else if ( rotation >= -90 && rotation < -45 )
      {
        rotation += 90;
        return QgsTextFormat::VerticalOrientation;
      }

      return QgsTextFormat::HorizontalOrientation;
    }

    case QgsTextFormat::HorizontalOrientation:
    case QgsTextFormat::VerticalOrientation:
      return format.orientation();
  }
  return QgsTextFormat::HorizontalOrientation;
}

void QgsTextRenderer::calculateExtraSpacingForLineJustification( const double spaceToDistribute, const QgsTextBlock &block, double &extraWordSpace, double &extraLetterSpace )
{
  const QString blockText = block.toPlainText();
  QTextBoundaryFinder finder( QTextBoundaryFinder::Word, blockText );
  finder.toStart();
  int wordBoundaries = 0;
  while ( finder.toNextBoundary() != -1 )
  {
    if ( finder.boundaryReasons() & QTextBoundaryFinder::StartOfItem )
      wordBoundaries++;
  }

  if ( wordBoundaries > 0 )
  {
    // word boundaries found => justify by padding word spacing
    extraWordSpace = spaceToDistribute / wordBoundaries;
  }
  else
  {
    // no word boundaries found => justify by letter spacing
    QTextBoundaryFinder finder( QTextBoundaryFinder::Grapheme, blockText );
    finder.toStart();

    int graphemeBoundaries = 0;
    while ( finder.toNextBoundary() != -1 )
    {
      if ( finder.boundaryReasons() & QTextBoundaryFinder::StartOfItem )
        graphemeBoundaries++;
    }

    if ( graphemeBoundaries > 0 )
    {
      extraLetterSpace = spaceToDistribute / graphemeBoundaries;
    }
  }
}

void QgsTextRenderer::applyExtraSpacingForLineJustification( QFont &font, double extraWordSpace, double extraLetterSpace )
{
  const double prevWordSpace = font.wordSpacing();
  font.setWordSpacing( prevWordSpace + extraWordSpace );
  const double prevLetterSpace = font.letterSpacing();
  font.setLetterSpacing( QFont::AbsoluteSpacing, prevLetterSpace + extraLetterSpace );
}

void QgsTextRenderer::drawTextInternalHorizontal( QgsRenderContext &context, const QgsTextFormat &format, TextPart drawType, DrawMode mode, const Component &component, const QgsTextDocument &document, double fontScale, const QFontMetricsF *fontMetrics, HAlignment hAlignment,
    VAlignment vAlignment, double rotation )
{
  QPainter *maskPainter = context.maskPainter( context.currentMaskId() );
  const QStringList textLines = document.toPlainText();

  double labelWidest = 0.0;
  switch ( mode )
  {
    case Label:
    case Point:
      for ( const QString &line : textLines )
      {
        double labelWidth = fontMetrics->horizontalAdvance( line ) / fontScale;
        if ( labelWidth > labelWidest )
        {
          labelWidest = labelWidth;
        }
      }
      break;

    case Rect:
      labelWidest = component.size.width();
      break;
  }

  double labelHeight = ( fontMetrics->ascent() + fontMetrics->descent() ) / fontScale; // ignore +1 for baseline
  //  double labelHighest = labelfm->height() + ( double )(( lines - 1 ) * labelHeight * tmpLyr.multilineHeight );

  // needed to move bottom of text's descender to within bottom edge of label
  double ascentOffset = 0.25 * fontMetrics->ascent() / fontScale; // labelfm->descent() is not enough

  int i = 0;

  bool adjustForAlignment = hAlignment != AlignLeft && ( mode != Label || textLines.size() > 1 );

  if ( mode == Rect && vAlignment != AlignTop )
  {
    std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;

    const double overallHeight = textHeight( context, format, textLines, Rect );
    switch ( vAlignment )
    {
      case AlignTop:
        break;

      case AlignVCenter:
        ascentOffset = -( component.size.height() - overallHeight ) * 0.5 + ascentOffset;
        break;

      case AlignBottom:
        ascentOffset = -( component.size.height() - overallHeight ) + ascentOffset;
        break;
    }
    referenceScaleOverride.reset();
  }

  for ( const QString &line : std::as_const( textLines ) )
  {
    const QgsTextBlock block = document.at( i );

    const bool isFinalLineInParagraph = ( i == document.size() - 1 )
                                        || document.at( i + 1 ).toPlainText().trimmed().isEmpty();

    QgsScopedQPainterState painterState( context.painter() );
    context.setPainterFlagsUsingContext();
    context.painter()->translate( component.origin );
    if ( !qgsDoubleNear( rotation, 0.0 ) )
      context.painter()->rotate( rotation );

    // apply to the mask painter the same transformations
    if ( maskPainter )
    {
      maskPainter->save();
      maskPainter->translate( component.origin );
      if ( !qgsDoubleNear( rotation, 0.0 ) )
        maskPainter->rotate( rotation );
    }

    // figure x offset for horizontal alignment of multiple lines
    double xMultiLineOffset = 0.0;
    double labelWidth = fontMetrics->horizontalAdvance( line ) / fontScale;
    double extraWordSpace = 0;
    double extraLetterSpace = 0;
    if ( adjustForAlignment )
    {
      double labelWidthDiff = 0;
      switch ( hAlignment )
      {
        case AlignCenter:
          labelWidthDiff = ( labelWidest - labelWidth ) * 0.5;
          break;

        case AlignRight:
          labelWidthDiff = labelWidest - labelWidth;
          break;

        case AlignJustify:
          if ( !isFinalLineInParagraph && labelWidest > labelWidth )
          {
            calculateExtraSpacingForLineJustification( labelWidest - labelWidth, block, extraWordSpace, extraLetterSpace );
            labelWidth = labelWidest;
          }
          break;

        case AlignLeft:
          break;
      }

      switch ( mode )
      {
        case Label:
        case Rect:
          xMultiLineOffset = labelWidthDiff;
          break;

        case Point:
        {
          switch ( hAlignment )
          {
            case AlignRight:
              xMultiLineOffset = labelWidthDiff - labelWidest;
              break;

            case AlignCenter:
              xMultiLineOffset = labelWidthDiff - labelWidest / 2.0;
              break;

            case AlignLeft:
            case AlignJustify:
              break;
          }
        }
        break;
      }
    }

    double yMultiLineOffset = ascentOffset;
    switch ( mode )
    {
      case Label:
        // rendering labels needs special handling - in this case text should be
        // drawn with the bottom left corner coinciding with origin, vs top left
        // for standard text rendering. Line height is also slightly different.
        yMultiLineOffset = - ascentOffset - ( textLines.size() - 1 - i ) * labelHeight * format.lineHeight();
        break;

      case Rect:
        // standard rendering - designed to exactly replicate QPainter's drawText method
        yMultiLineOffset = - ascentOffset + labelHeight - 1 /*baseline*/ + format.lineHeight() * fontMetrics->lineSpacing() * i / fontScale;
        break;

      case Point:
        // standard rendering - designed to exactly replicate QPainter's drawText rect method
        yMultiLineOffset = 0 - ( textLines.size() - 1 - i ) * fontMetrics->lineSpacing() * format.lineHeight() / fontScale;
        break;

    }

    context.painter()->translate( QPointF( xMultiLineOffset, yMultiLineOffset ) );
    if ( maskPainter )
      maskPainter->translate( QPointF( xMultiLineOffset, yMultiLineOffset ) );

    Component subComponent;
    subComponent.block = block;
    subComponent.size = QSizeF( labelWidth, labelHeight );
    subComponent.offset = QPointF( 0.0, -ascentOffset );
    subComponent.rotation = -component.rotation * 180 / M_PI;
    subComponent.rotationOffset = 0.0;
    subComponent.extraWordSpacing = extraWordSpace * fontScale;
    subComponent.extraLetterSpacing = extraLetterSpace * fontScale;

    // draw the mask below the text (for preview)
    if ( format.mask().enabled() )
    {
      QgsTextRenderer::drawMask( context, subComponent, format, mode );
    }

    if ( drawType == QgsTextRenderer::Buffer )
    {
      QgsTextRenderer::drawBuffer( context, subComponent, format, mode );
    }
    else
    {
      // store text's drawing in QPicture for drop shadow call
      QPicture textPict;
      QPainter textp;
      textp.begin( &textPict );
      textp.setPen( Qt::NoPen );

      std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
      if ( mode == Label )
      {
        // label size has already been calculated using any symbology reference scale factor -- we need
        // to temporarily remove the reference scale here or we'll be applying the scaling twice
        referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
      }
      bool isNullSize = false;
      const QFont font = format.scaledFont( context, fontScale, &isNullSize );
      referenceScaleOverride.reset();

      if ( !isNullSize )
      {
        textp.scale( 1 / fontScale, 1 / fontScale );

        double xOffset = 0;
        for ( const QgsTextFragment &fragment : block )
        {
          // draw text, QPainterPath method
          QPainterPath path;
          path.setFillRule( Qt::WindingFill );

          QFont fragmentFont = font;
          fragment.characterFormat().updateFontForFormat( fragmentFont, fontScale );

          if ( extraWordSpace || extraLetterSpace )
            applyExtraSpacingForLineJustification( fragmentFont, extraWordSpace * fontScale, extraLetterSpace * fontScale );

          path.addText( xOffset, 0, fragmentFont, fragment.text() );

          QColor textColor = fragment.characterFormat().textColor().isValid() ? fragment.characterFormat().textColor() : format.color();
          textColor.setAlphaF( fragment.characterFormat().textColor().isValid() ? textColor.alphaF() * format.opacity() : format.opacity() );
          textp.setBrush( textColor );
          textp.drawPath( path );

          xOffset += fragment.horizontalAdvance( fragmentFont, true );
        }
        textp.end();
      }

      if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowText )
      {
        subComponent.picture = textPict;
        subComponent.pictureBuffer = 0.0; // no pen width to deal with
        subComponent.origin = QPointF( 0.0, 0.0 );

        QgsTextRenderer::drawShadow( context, subComponent, format );
      }

      // paint the text
      if ( context.useAdvancedEffects() )
      {
        context.painter()->setCompositionMode( format.blendMode() );
      }

      // scale for any print output or image saving @ specific dpi
      context.painter()->scale( subComponent.dpiRatio, subComponent.dpiRatio );

      switch ( context.textRenderFormat() )
      {
        case Qgis::TextRenderFormat::AlwaysOutlines:
        {
          // draw outlined text
          _fixQPictureDPI( context.painter() );
          context.painter()->drawPicture( 0, 0, textPict );
          break;
        }

        case Qgis::TextRenderFormat::AlwaysText:
        {
          double xOffset = 0;
          for ( const QgsTextFragment &fragment : block )
          {
            QFont fragmentFont = font;
            fragment.characterFormat().updateFontForFormat( fragmentFont, fontScale );

            if ( extraWordSpace || extraLetterSpace )
              applyExtraSpacingForLineJustification( fragmentFont, extraWordSpace * fontScale, extraLetterSpace * fontScale );

            QColor textColor = fragment.characterFormat().textColor().isValid() ? fragment.characterFormat().textColor() : format.color();
            textColor.setAlphaF( fragment.characterFormat().textColor().isValid() ? textColor.alphaF() * format.opacity() : format.opacity() );

            context.painter()->setPen( textColor );
            context.painter()->setFont( fragmentFont );
            context.painter()->setRenderHint( QPainter::TextAntialiasing );

            context.painter()->scale( 1 / fontScale, 1 / fontScale );
            context.painter()->drawText( xOffset, 0, fragment.text() );
            context.painter()->scale( fontScale, fontScale );

            xOffset += fragment.horizontalAdvance( fragmentFont, true, fontScale );
          }
        }
      }
    }
    if ( maskPainter )
      maskPainter->restore();
    i++;
  }
}

void QgsTextRenderer::drawTextInternalVertical( QgsRenderContext &context, const QgsTextFormat &format, QgsTextRenderer::TextPart drawType, QgsTextRenderer::DrawMode mode, const QgsTextRenderer::Component &component, const QgsTextDocument &document, double fontScale, const QFontMetricsF *fontMetrics, QgsTextRenderer::HAlignment hAlignment, QgsTextRenderer::VAlignment, double rotation )
{
  QPainter *maskPainter = context.maskPainter( context.currentMaskId() );
  const QStringList textLines = document.toPlainText();

  std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
  if ( mode == Label )
  {
    // label size has already been calculated using any symbology reference scale factor -- we need
    // to temporarily remove the reference scale here or we'll be applying the scaling twice
    referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
  }

  bool isNullSize = false;
  const QFont font = format.scaledFont( context, fontScale, &isNullSize );
  if ( isNullSize )
    return;

  referenceScaleOverride.reset();

  double letterSpacing = font.letterSpacing() / fontScale;

  double labelWidth = fontMetrics->maxWidth() / fontScale; // label width represents the width of one line of a multi-line label
  double actualLabelWidest = labelWidth + ( textLines.size() - 1 ) * labelWidth * format.lineHeight();
  double labelWidest = 0.0;
  switch ( mode )
  {
    case Label:
    case Point:
      labelWidest = actualLabelWidest;
      break;

    case Rect:
      labelWidest = component.size.width();
      break;
  }

  int maxLineLength = 0;
  for ( const QString &line : std::as_const( textLines ) )
  {
    maxLineLength = std::max( maxLineLength, static_cast<int>( line.length() ) );
  }
  double actualLabelHeight = fontMetrics->ascent() / fontScale + ( fontMetrics->ascent() / fontScale + letterSpacing ) * ( maxLineLength - 1 );
  double ascentOffset = fontMetrics->ascent() / fontScale;

  int i = 0;

  bool adjustForAlignment = hAlignment != AlignLeft && ( mode != Label || textLines.size() > 1 );

  for ( const QgsTextBlock &block : document )
  {
    QgsScopedQPainterState painterState( context.painter() );
    context.setPainterFlagsUsingContext();

    context.painter()->translate( component.origin );
    if ( !qgsDoubleNear( rotation, 0.0 ) )
      context.painter()->rotate( rotation );

    // apply to the mask painter the same transformations
    if ( maskPainter )
    {
      maskPainter->save();
      maskPainter->translate( component.origin );
      if ( !qgsDoubleNear( rotation, 0.0 ) )
        maskPainter->rotate( rotation );
    }

    // figure x offset of multiple lines
    double xOffset = actualLabelWidest - labelWidth - ( i * labelWidth * format.lineHeight() );
    if ( adjustForAlignment )
    {
      double labelWidthDiff = 0;
      switch ( hAlignment )
      {
        case AlignCenter:
          labelWidthDiff = ( labelWidest - actualLabelWidest ) * 0.5;
          break;

        case AlignRight:
          labelWidthDiff = labelWidest - actualLabelWidest;
          break;

        case AlignLeft:
        case AlignJustify:
          break;
      }

      switch ( mode )
      {
        case Label:
        case Rect:
          xOffset += labelWidthDiff;
          break;

        case Point:
          break;
      }
    }

    double yOffset = 0.0;
    switch ( mode )
    {
      case Label:
        if ( format.orientation() == QgsTextFormat::RotationBasedOrientation )
        {
          if ( rotation >= -405 && rotation < -180 )
          {
            yOffset = ascentOffset;
          }
          else if ( rotation >= 0 && rotation < 45 )
          {
            xOffset -= actualLabelWidest;
            yOffset = -actualLabelHeight + ascentOffset + fontMetrics->descent() / fontScale;
          }
        }
        else
        {
          yOffset = -actualLabelHeight + ascentOffset;
        }
        break;

      case Point:
        yOffset = -actualLabelHeight + ascentOffset;
        break;

      case Rect:
        yOffset = ascentOffset;
        break;
    }

    context.painter()->translate( QPointF( xOffset, yOffset ) );

    double fragmentYOffset = 0;
    for ( const QgsTextFragment &fragment : block )
    {
      // apply some character replacement to draw symbols in vertical presentation
      const QString line = QgsStringUtils::substituteVerticalCharacters( fragment.text() );

      QFont fragmentFont( font );
      fragment.characterFormat().updateFontForFormat( fragmentFont, fontScale );

      QFontMetricsF fragmentMetrics( fragmentFont );

      double labelHeight = fragmentMetrics.ascent() / fontScale + ( fragmentMetrics.ascent() / fontScale + letterSpacing ) * ( line.length() - 1 );

      Component subComponent;
      subComponent.block = QgsTextBlock( fragment );
      subComponent.size = QSizeF( labelWidth, labelHeight );
      subComponent.offset = QPointF( 0.0, fragmentYOffset );
      subComponent.rotation = -component.rotation * 180 / M_PI;
      subComponent.rotationOffset = 0.0;

      // draw the mask below the text (for preview)
      if ( format.mask().enabled() )
      {
        // WARNING: totally broken! (has been since mask was introduced)
#if 0
        QgsTextRenderer::drawMask( context, subComponent, format );
#endif
      }

      if ( drawType == QgsTextRenderer::Buffer )
      {
        fragmentYOffset += QgsTextRenderer::drawBuffer( context, subComponent, format, mode );
      }
      else
      {
        // draw text, QPainterPath method
        QPainterPath path;
        path.setFillRule( Qt::WindingFill );
        const QStringList parts = QgsPalLabeling::splitToGraphemes( fragment.text() );
        double partYOffset = 0.0;
        for ( const auto &part : parts )
        {
          double partXOffset = ( labelWidth - ( fragmentMetrics.horizontalAdvance( part ) / fontScale - letterSpacing ) ) / 2;
          path.addText( partXOffset * fontScale, partYOffset * fontScale, fragmentFont, part );
          partYOffset += fragmentMetrics.ascent() / fontScale + letterSpacing;
        }

        // store text's drawing in QPicture for drop shadow call
        QPicture textPict;
        QPainter textp;
        textp.begin( &textPict );
        textp.setPen( Qt::NoPen );
        QColor textColor = fragment.characterFormat().textColor().isValid() ? fragment.characterFormat().textColor() : format.color();
        textColor.setAlphaF( fragment.characterFormat().textColor().isValid() ? textColor.alphaF() * format.opacity() : format.opacity() );
        textp.setBrush( textColor );
        textp.scale( 1 / fontScale, 1 / fontScale );
        textp.drawPath( path );
        // TODO: why are some font settings lost on drawPicture() when using drawText() inside QPicture?
        //       e.g. some capitalization options, but not others
        //textp.setFont( tmpLyr.textFont );
        //textp.setPen( tmpLyr.textColor );
        //textp.drawText( 0, 0, component.text() );
        textp.end();

        if ( format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowText )
        {
          subComponent.picture = textPict;
          subComponent.pictureBuffer = 0.0; // no pen width to deal with
          subComponent.origin = QPointF( 0.0, fragmentYOffset );
          const double prevY = subComponent.offset.y();
          subComponent.offset = QPointF( 0, -labelHeight );
          subComponent.useOrigin = true;
          QgsTextRenderer::drawShadow( context, subComponent, format );
          subComponent.useOrigin = false;
          subComponent.offset = QPointF( 0, prevY );
        }

        // paint the text
        if ( context.useAdvancedEffects() )
        {
          context.painter()->setCompositionMode( format.blendMode() );
        }

        // scale for any print output or image saving @ specific dpi
        context.painter()->scale( subComponent.dpiRatio, subComponent.dpiRatio );

        switch ( context.textRenderFormat() )
        {
          case Qgis::TextRenderFormat::AlwaysOutlines:
          {
            // draw outlined text
            _fixQPictureDPI( context.painter() );
            context.painter()->drawPicture( 0, fragmentYOffset, textPict );
            fragmentYOffset += partYOffset;
            break;
          }

          case Qgis::TextRenderFormat::AlwaysText:
          {
            context.painter()->setFont( fragmentFont );
            context.painter()->setPen( textColor );
            context.painter()->setRenderHint( QPainter::TextAntialiasing );

            double partYOffset = 0.0;
            for ( const QString &part : parts )
            {
              double partXOffset = ( labelWidth - ( fragmentMetrics.horizontalAdvance( part ) / fontScale - letterSpacing ) ) / 2;
              context.painter()->scale( 1 / fontScale, 1 / fontScale );
              context.painter()->drawText( partXOffset * fontScale, ( fragmentYOffset + partYOffset ) * fontScale, part );
              context.painter()->scale( fontScale, fontScale );
              partYOffset += fragmentMetrics.ascent() / fontScale + letterSpacing;
            }
            fragmentYOffset += partYOffset;
          }
        }
      }
    }

    if ( maskPainter )
      maskPainter->restore();
    i++;
  }
}

double QgsTextRenderer::calculateScaleFactorForFormat( const QgsRenderContext &context, const QgsTextFormat &format )
{
  if ( !( context.flags() & Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering ) )
    return 1.0;

  const double pixelSize = context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() );

  // THESE THRESHOLD MAY NEED TWEAKING!

  // for small font sizes we need to apply a growth scaling workaround designed to stablise the rendering of small font sizes
  if ( pixelSize < 50 )
    return FONT_WORKAROUND_SCALE;
  //... but for font sizes we might run into https://bugreports.qt.io/browse/QTBUG-98778, which messes up the spacing between words for large fonts!
  // so instead we scale down the painter so that we render the text at 200 pixel size and let painter scaling handle making it the correct size
  else if ( pixelSize > 200 )
    return 200 / pixelSize;
  else
    return 1.0;
}

