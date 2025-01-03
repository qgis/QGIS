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
#include "qgstextformat.h"
#include "qgstextdocument.h"
#include "qgstextdocumentmetrics.h"
#include "qgstextfragment.h"
#include "qgspallabeling.h"
#include "qgspainteffect.h"
#include "qgspainterswapper.h"
#include "qgsmarkersymbollayer.h"
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"
#include "qgsunittypes.h"
#include "qgstextmetrics.h"
#include "qgstextrendererutils.h"
#include "qgsgeos.h"
#include "qgspainting.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"
#include <optional>

#include <QTextBoundaryFinder>


Qgis::TextHorizontalAlignment QgsTextRenderer::convertQtHAlignment( Qt::Alignment alignment )
{
  if ( alignment & Qt::AlignLeft )
    return Qgis::TextHorizontalAlignment::Left;
  else if ( alignment & Qt::AlignRight )
    return Qgis::TextHorizontalAlignment::Right;
  else if ( alignment & Qt::AlignHCenter )
    return Qgis::TextHorizontalAlignment::Center;
  else if ( alignment & Qt::AlignJustify )
    return Qgis::TextHorizontalAlignment::Justify;

  // not supported?
  return Qgis::TextHorizontalAlignment::Left;
}

Qgis::TextVerticalAlignment QgsTextRenderer::convertQtVAlignment( Qt::Alignment alignment )
{
  if ( alignment & Qt::AlignTop )
    return Qgis::TextVerticalAlignment::Top;
  else if ( alignment & Qt::AlignBottom )
    return Qgis::TextVerticalAlignment::Bottom;
  else if ( alignment & Qt::AlignVCenter )
    return Qgis::TextVerticalAlignment::VerticalCenter;
  //not supported
  else if ( alignment & Qt::AlignBaseline )
    return Qgis::TextVerticalAlignment::Bottom;

  return Qgis::TextVerticalAlignment::Top;
}

int QgsTextRenderer::sizeToPixel( double size, const QgsRenderContext &c, Qgis::RenderUnit unit, const QgsMapUnitScale &mapUnitScale )
{
  return static_cast< int >( c.convertToPainterUnits( size, unit, mapUnitScale ) + 0.5 ); //NOLINT
}

void QgsTextRenderer::drawText( const QRectF &rect, double rotation, Qgis::TextHorizontalAlignment alignment, const QStringList &text, QgsRenderContext &context, const QgsTextFormat &_format, bool, Qgis::TextVerticalAlignment vAlignment, Qgis::TextRendererFlags flags,
                                Qgis::TextLayoutMode mode )
{
  QgsTextFormat lFormat = _format;
  if ( _format.dataDefinedProperties().hasActiveProperties() ) // note, we use format instead of tmpFormat here, it's const and potentially avoids a detach
    lFormat.updateDataDefinedProperties( context );

  // DO NOT USE _format in the following code, always use lFormat!!
  QgsTextDocumentRenderContext documentContext;
  documentContext.setFlags( flags );
  documentContext.setMaximumWidth( rect.width() );

  const QgsTextDocument document = QgsTextDocument::fromTextAndFormat( text, lFormat );

  const double fontScale = calculateScaleFactorForFormat( context, lFormat );
  const QgsTextDocumentMetrics metrics = QgsTextDocumentMetrics::calculateMetrics( document, lFormat, context, fontScale, documentContext );

  drawDocument( rect, lFormat, metrics.document(), metrics, context, alignment, vAlignment, rotation, mode, flags );
}

void QgsTextRenderer::drawDocument( const QRectF &rect, const QgsTextFormat &format, const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics, QgsRenderContext &context, Qgis::TextHorizontalAlignment horizontalAlignment, Qgis::TextVerticalAlignment verticalAlignment, double rotation, Qgis::TextLayoutMode mode, Qgis::TextRendererFlags )
{
  const QgsTextFormat tmpFormat = updateShadowPosition( format );

  Qgis::TextComponents components = Qgis::TextComponent::Text;
  if ( tmpFormat.background().enabled() )
  {
    components |= Qgis::TextComponent::Background;
  }

  if ( tmpFormat.shadow().enabled() )
  {
    components |= Qgis::TextComponent::Shadow;
  }

  if ( tmpFormat.buffer().enabled() )
  {
    components |= Qgis::TextComponent::Buffer;
  }

  drawParts( rect, rotation, horizontalAlignment, verticalAlignment, document, metrics, context, tmpFormat, components, mode );
}

void QgsTextRenderer::drawText( QPointF point, double rotation, Qgis::TextHorizontalAlignment alignment, const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &_format, bool )
{
  QgsTextFormat lFormat = _format;
  if ( _format.dataDefinedProperties().hasActiveProperties() ) // note, we use _format instead of tmpFormat here, it's const and potentially avoids a detach
    lFormat.updateDataDefinedProperties( context );
  lFormat = updateShadowPosition( lFormat );

  // DO NOT USE _format in the following code, always use lFormat!!
  const QgsTextDocument document = QgsTextDocument::fromTextAndFormat( textLines, lFormat );
  const double fontScale = calculateScaleFactorForFormat( context, lFormat );
  const QgsTextDocumentMetrics metrics = QgsTextDocumentMetrics::calculateMetrics( document, lFormat, context, fontScale );

  drawDocument( point, lFormat, metrics.document(), metrics, context, alignment, rotation );
}

void QgsTextRenderer::drawDocument( QPointF point, const QgsTextFormat &_format, const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics, QgsRenderContext &context, Qgis::TextHorizontalAlignment alignment, double rotation, Qgis::TextLayoutMode mode )
{
  const QgsTextFormat lFormat = updateShadowPosition( _format );
  // DO NOT USE _format in the following code, always use lFormat!!

  Qgis::TextComponents components = Qgis::TextComponent::Text;
  if ( lFormat.background().enabled() )
  {
    components |= Qgis::TextComponent::Background;
  }

  if ( lFormat.shadow().enabled() )
  {
    components |= Qgis::TextComponent::Shadow;
  }

  if ( lFormat.buffer().enabled() )
  {
    components |= Qgis::TextComponent::Buffer;
  }

  drawParts( point, rotation, alignment, document, metrics, context, lFormat, components, mode );
}

void QgsTextRenderer::drawTextOnLine( const QPolygonF &line, const QString &text, QgsRenderContext &context, const QgsTextFormat &_format, double offsetAlongLine, double offsetFromLine )
{
  QgsTextFormat lFormat = _format;
  if ( _format.dataDefinedProperties().hasActiveProperties() ) // note, we use _format instead of tmpFormat here, it's const and potentially avoids a detach
    lFormat.updateDataDefinedProperties( context );
  lFormat = updateShadowPosition( lFormat );

  // DO NOT USE _format in the following code, always use lFormat!!

  // todo handle newlines??
  const QgsTextDocument document = QgsTextDocument::fromTextAndFormat( {text}, lFormat );

  drawDocumentOnLine( line, lFormat, document, context, offsetAlongLine, offsetFromLine );
}

void QgsTextRenderer::drawDocumentOnLine( const QPolygonF &line, const QgsTextFormat &format, const QgsTextDocument &document, QgsRenderContext &context, double offsetAlongLine, double offsetFromLine )
{
  QPolygonF labelBaselineCurve = line;
  if ( !qgsDoubleNear( offsetFromLine, 0 ) )
  {
    std::unique_ptr < QgsLineString > ring( QgsLineString::fromQPolygonF( line ) );
    QgsGeos geos( ring.get() );
    std::unique_ptr < QgsLineString > offsetCurve( dynamic_cast< QgsLineString * >( geos.offsetCurve( offsetFromLine, 4, Qgis::JoinStyle::Round, 2 ) ) );
    if ( !offsetCurve )
      return;

#if GEOS_VERSION_MAJOR==3 && GEOS_VERSION_MINOR<11
    if ( offsetFromLine < 0 )
    {
      // geos < 3.11 reverses the direction of offset curves with negative distances -- we don't want that!
      std::unique_ptr < QgsLineString > reversed( offsetCurve->reversed() );
      if ( !reversed )
        return;

      offsetCurve = std::move( reversed );
    }
#endif

    labelBaselineCurve = offsetCurve->asQPolygonF();
  }

  const double fontScale = calculateScaleFactorForFormat( context, format );

  const QFont baseFont = format.scaledFont( context, fontScale );
  const double letterSpacing = baseFont.letterSpacing() / fontScale;
  const double wordSpacing = baseFont.wordSpacing() / fontScale;

  QStringList graphemes;
  QVector< QgsTextCharacterFormat > graphemeFormats;
  QVector< QgsTextDocumentMetrics > graphemeMetrics;

  for ( const QgsTextBlock &block : std::as_const( document ) )
  {
    for ( const QgsTextFragment &fragment : block )
    {
      const QStringList fragmentGraphemes = QgsPalLabeling::splitToGraphemes( fragment.text() );
      for ( const QString &grapheme : fragmentGraphemes )
      {
        graphemes.append( grapheme );
        graphemeFormats.append( fragment.characterFormat() );

        QgsTextDocument document;
        document.append( QgsTextBlock( QgsTextFragment( grapheme, fragment.characterFormat() ) ) );

        graphemeMetrics.append( QgsTextDocumentMetrics::calculateMetrics( document, format, context, fontScale ) );
      }
    }
  }

  QVector< double > characterWidths( graphemes.count() );
  QVector< double > characterHeights( graphemes.count() );
  QVector< double > characterDescents( graphemes.count() );
  QFont previousNonSuperSubScriptFont;

  for ( int i = 0; i < graphemes.count(); i++ )
  {
    // reconstruct how Qt creates word spacing, then adjust per individual stored character
    // this will allow the text renderer to create each candidate width = character width + correct spacing

    double graphemeFirstCharHorizontalAdvanceWithLetterSpacing = 0;
    double graphemeFirstCharHorizontalAdvance = 0;
    double graphemeHorizontalAdvance = 0;
    double characterDescent = 0;
    double characterHeight = 0;
    const QgsTextCharacterFormat *graphemeFormat = &graphemeFormats[i];

    QFont graphemeFont = baseFont;
    graphemeFormat->updateFontForFormat( graphemeFont, context, fontScale );

    if ( i == 0 )
      previousNonSuperSubScriptFont = graphemeFont;

    if ( graphemeFormat->hasVerticalAlignmentSet() )
    {
      switch ( graphemeFormat->verticalAlignment() )
      {
        case Qgis::TextCharacterVerticalAlignment::Normal:
          previousNonSuperSubScriptFont = graphemeFont;
          break;

        case Qgis::TextCharacterVerticalAlignment::SuperScript:
        case Qgis::TextCharacterVerticalAlignment::SubScript:
        {
          if ( graphemeFormat->fontPointSize() < 0 )
          {
            // if fragment has no explicit font size set, then we scale the inherited font size to 60% of base font size
            // this allows for easier use of super/subscript in labels as "my text<sup>2</sup>" will automatically render
            // the superscript in a smaller font size. BUT if the fragment format HAS a non -1 font size then it indicates
            // that the document has an explicit font size for the super/subscript element, eg "my text<sup style="font-size: 6pt">2</sup>"
            // which we should respect
            graphemeFont.setPixelSize( static_cast< int >( std::round( graphemeFont.pixelSize() * SUPERSCRIPT_SUBSCRIPT_FONT_SIZE_SCALING_FACTOR ) ) );
          }
          break;
        }
      }
    }
    else
    {
      previousNonSuperSubScriptFont = graphemeFont;
    }

    const QFontMetricsF graphemeFontMetrics( graphemeFont );
    graphemeFirstCharHorizontalAdvance = graphemeFontMetrics.horizontalAdvance( QString( graphemes[i].at( 0 ) ) ) / fontScale;
    graphemeFirstCharHorizontalAdvanceWithLetterSpacing = graphemeFontMetrics.horizontalAdvance( graphemes[i].at( 0 ) )  / fontScale + letterSpacing;
    graphemeHorizontalAdvance = graphemeFontMetrics.horizontalAdvance( QString( graphemes[i] ) )  / fontScale;
    characterDescent = graphemeFontMetrics.descent() / fontScale;
    characterHeight = graphemeFontMetrics.height() / fontScale;

    qreal wordSpaceFix = qreal( 0.0 );
    if ( graphemes[i] == QLatin1String( " " ) )
    {
      // word spacing only gets added once at end of consecutive run of spaces, see QTextEngine::shapeText()
      int nxt = i + 1;
      wordSpaceFix = ( nxt < graphemes.count() && graphemes[nxt] != QLatin1String( " " ) ) ? wordSpacing : qreal( 0.0 );
    }

    // this workaround only works for clusters with a single character. Not sure how it should be handled
    // with multi-character clusters.
    if ( graphemes[i].length() == 1 &&
         !qgsDoubleNear( graphemeFirstCharHorizontalAdvance, graphemeFirstCharHorizontalAdvanceWithLetterSpacing ) )
    {
      // word spacing applied when it shouldn't be
      wordSpaceFix -= wordSpacing;
    }

    const double charWidth = graphemeHorizontalAdvance + wordSpaceFix;
    characterWidths[i] = charWidth;
    characterHeights[i] = characterHeight;
    characterDescents[i] = characterDescent;
  }

  QgsPrecalculatedTextMetrics metrics( graphemes, std::move( characterWidths ), std::move( characterHeights ), std::move( characterDescents ) );
  metrics.setGraphemeFormats( graphemeFormats );

  std::unique_ptr< QgsTextRendererUtils::CurvePlacementProperties > placement = QgsTextRendererUtils::generateCurvedTextPlacement(
        metrics, labelBaselineCurve, offsetAlongLine,
        QgsTextRendererUtils::RespectPainterOrientation,
        -1, -1,
        QgsTextRendererUtils::CurvedTextFlag::UseBaselinePlacement
        | QgsTextRendererUtils::CurvedTextFlag::TruncateStringWhenLineIsTooShort
      );

  if ( placement->graphemePlacement.empty() )
    return;

  // We may have deliberately skipped over some graphemes during curved text placement (such as zero-width graphemes).
  // So we need to use a hash of the original grapheme index to place generated components in, as there may accordingly
  // be graphemes which don't result in components, and we can't just blindly assume the component array position
  // will match the original grapheme index
  QHash< int, QgsTextRenderer::Component > components;
  components.reserve( placement->graphemePlacement.size() );
  for ( const QgsTextRendererUtils::CurvedGraphemePlacement &grapheme : std::as_const( placement->graphemePlacement ) )
  {
    QgsTextRenderer::Component component;
    component.origin = QPointF( grapheme.x, grapheme.y );
    component.rotation = -grapheme.angle;

    QgsTextDocumentMetrics &metrics = graphemeMetrics[ grapheme.graphemeIndex ];
    const double verticalOffset = metrics.fragmentVerticalOffset( 0, 0, Qgis::TextLayoutMode::Point );
    if ( !qgsDoubleNear( verticalOffset, 0 ) )
    {
      component.origin.rx() += verticalOffset * std::cos( grapheme.angle + M_PI_2 );
      component.origin.ry() += verticalOffset * std::sin( grapheme.angle + M_PI_2 );
    }

    components.insert( grapheme.graphemeIndex, component );
  }

  if ( format.background().enabled() )
  {
    for ( const QgsTextRendererUtils::CurvedGraphemePlacement &grapheme : std::as_const( placement->graphemePlacement ) )
    {
      const QgsTextDocumentMetrics &metrics = graphemeMetrics.at( grapheme.graphemeIndex );
      const QgsTextRenderer::Component &component = components[grapheme.graphemeIndex ];
      drawBackground( context, component, format, metrics, Qgis::TextLayoutMode::Point );
    }
  }

  if ( format.buffer().enabled() )
  {
    for ( const QgsTextRendererUtils::CurvedGraphemePlacement &grapheme : std::as_const( placement->graphemePlacement ) )
    {
      const QgsTextDocumentMetrics &metrics = graphemeMetrics.at( grapheme.graphemeIndex );
      const QgsTextRenderer::Component &component = components[grapheme.graphemeIndex ];

      drawTextInternal( Qgis::TextComponent::Buffer,
                        context,
                        format,
                        component,
                        metrics.document(),
                        metrics,
                        Qgis::TextHorizontalAlignment::Left,
                        Qgis::TextVerticalAlignment::Top,
                        Qgis::TextLayoutMode::Point );
    }
  }

  for ( const QgsTextRendererUtils::CurvedGraphemePlacement &grapheme : std::as_const( placement->graphemePlacement ) )
  {
    const QgsTextDocumentMetrics &metrics = graphemeMetrics.at( grapheme.graphemeIndex );
    const QgsTextRenderer::Component &component = components[grapheme.graphemeIndex ];

    drawTextInternal( Qgis::TextComponent::Text,
                      context,
                      format,
                      component,
                      metrics.document(),
                      metrics,
                      Qgis::TextHorizontalAlignment::Left,
                      Qgis::TextVerticalAlignment::Top,
                      Qgis::TextLayoutMode::Point );
  }
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

void QgsTextRenderer::drawPart( const QRectF &rect, double rotation, Qgis::TextHorizontalAlignment alignment,
                                const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, Qgis::TextComponent part, bool )
{
  const QgsTextDocument document = QgsTextDocument::fromTextAndFormat( textLines, format );
  const double fontScale = calculateScaleFactorForFormat( context, format );
  const QgsTextDocumentMetrics metrics = QgsTextDocumentMetrics::calculateMetrics( document, format, context, fontScale );

  drawParts( rect, rotation, alignment, Qgis::TextVerticalAlignment::Top, metrics.document(), metrics, context, format, part, Qgis::TextLayoutMode::Rectangle );
}

void QgsTextRenderer::drawParts( const QRectF &rect, double rotation, Qgis::TextHorizontalAlignment alignment, Qgis::TextVerticalAlignment vAlignment, const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics, QgsRenderContext &context, const QgsTextFormat &format, Qgis::TextComponents parts, Qgis::TextLayoutMode mode )
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

  if ( ( parts & Qgis::TextComponent::Background ) && format.background().enabled() )
  {
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

    switch ( vAlignment )
    {
      case Qgis::TextVerticalAlignment::Top:
        break;
      case Qgis::TextVerticalAlignment::VerticalCenter:
        component.origin.ry() += ( rect.height() - metrics.documentSize( mode, format.orientation() ).height() ) / 2;
        break;
      case Qgis::TextVerticalAlignment::Bottom:
        component.origin.ry() += ( rect.height() - metrics.documentSize( mode, format.orientation() ).height() );
        break;
    }

    QgsTextRenderer::drawBackground( context, component, format, metrics, Qgis::TextLayoutMode::Rectangle );
  }

  if ( parts == Qgis::TextComponents( Qgis::TextComponent::Buffer ) && !format.buffer().enabled() )
  {
    return;
  }

  if ( parts & Qgis::TextComponent::Buffer || parts & Qgis::TextComponent::Text || parts & Qgis::TextComponent::Shadow )
  {
    drawTextInternal( parts, context, format, component,
                      document, metrics,
                      alignment, vAlignment, mode );
  }
}

void QgsTextRenderer::drawPart( QPointF origin, double rotation, Qgis::TextHorizontalAlignment alignment, const QStringList &textLines, QgsRenderContext &context, const QgsTextFormat &format, Qgis::TextComponent part, bool )
{
  const QgsTextDocument document = QgsTextDocument::fromTextAndFormat( textLines, format );
  const double fontScale = calculateScaleFactorForFormat( context, format );
  const QgsTextDocumentMetrics metrics = QgsTextDocumentMetrics::calculateMetrics( document, format, context, fontScale );

  drawParts( origin, rotation, alignment, metrics.document(), metrics, context, format, part, Qgis::TextLayoutMode::Point );
}

void QgsTextRenderer::drawParts( QPointF origin, double rotation, Qgis::TextHorizontalAlignment alignment, const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics, QgsRenderContext &context, const QgsTextFormat &format, Qgis::TextComponents parts, Qgis::TextLayoutMode mode )
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

  if ( ( parts & Qgis::TextComponent::Background ) && format.background().enabled() )
  {
    QgsTextRenderer::drawBackground( context, component, format, metrics, mode );
  }

  if ( parts == Qgis::TextComponents( Qgis::TextComponent::Buffer ) && !format.buffer().enabled() )
  {
    return;
  }

  if ( parts & Qgis::TextComponent::Buffer || parts & Qgis::TextComponent::Text || parts & Qgis::TextComponent::Shadow )
  {
    drawTextInternal( parts, context, format, component,
                      document,
                      metrics,
                      alignment, Qgis::TextVerticalAlignment::Top,
                      mode );
  }
}

QFontMetricsF QgsTextRenderer::fontMetrics( QgsRenderContext &context, const QgsTextFormat &format, const double scaleFactor )
{
  return QFontMetricsF( format.scaledFont( context, scaleFactor ), context.painter() ? context.painter()->device() : nullptr );
}

double QgsTextRenderer::drawBuffer( QgsRenderContext &context, const QgsTextRenderer::Component &component, const QgsTextFormat &format,
                                    const QgsTextDocumentMetrics &metrics,
                                    Qgis::TextLayoutMode mode )
{
  QPainter *p = context.painter();

  Qgis::TextOrientation orientation = format.orientation();
  if ( format.orientation() == Qgis::TextOrientation::RotationBased )
  {
    if ( component.rotation >= -315 && component.rotation < -90 )
    {
      orientation = Qgis::TextOrientation::Vertical;
    }
    else if ( component.rotation >= -90 && component.rotation < -45 )
    {
      orientation = Qgis::TextOrientation::Vertical;
    }
    else
    {
      orientation = Qgis::TextOrientation::Horizontal;
    }
  }

  QgsTextBufferSettings buffer = format.buffer();

  const double penSize =  buffer.sizeUnit() == Qgis::RenderUnit::Percentage
                          ? context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() ) * buffer.size() / 100
                          : context.convertToPainterUnits( buffer.size(), buffer.sizeUnit(), buffer.sizeMapUnitScale() );

  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
  if ( mode == Qgis::TextLayoutMode::Labeling )
  {
    // label size has already been calculated using any symbology reference scale factor -- we need
    // to temporarily remove the reference scale here or we'll be applying the scaling twice
    referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
  }

  if ( metrics.isNullFontSize() )
    return 0;

  referenceScaleOverride.reset();

  QPainterPath path;
  path.setFillRule( Qt::WindingFill );
  double advance = 0;
  double height = component.size.height();
  switch ( orientation )
  {
    case Qgis::TextOrientation::Horizontal:
    {
      // NOT SUPPORTED BY THIS METHOD ANYMORE -- buffer drawing is handled in drawTextInternalHorizontal since QGIS 3.42
      break;
    }

    case Qgis::TextOrientation::Vertical:
    case Qgis::TextOrientation::RotationBased:
    {
      double partYOffset = component.offset.y() * scaleFactor;

      const double blockMaximumCharacterWidth = metrics.blockMaximumCharacterWidth( component.blockIndex );
      double partLastDescent = 0;

      int fragmentIndex = 0;
      for ( const QgsTextFragment &fragment : component.block )
      {
        const QFont fragmentFont = metrics.fragmentFont( component.blockIndex, component.firstFragmentIndex + fragmentIndex );
        const double letterSpacing = fragmentFont.letterSpacing() / scaleFactor;

        const QFontMetricsF fragmentMetrics( fragmentFont );

        const double fragmentYOffset = metrics.fragmentVerticalOffset( component.blockIndex, fragmentIndex, mode );

        const QStringList parts = QgsPalLabeling::splitToGraphemes( fragment.text() );
        for ( const QString &part : parts )
        {
          double partXOffset = ( blockMaximumCharacterWidth - ( fragmentMetrics.horizontalAdvance( part ) / scaleFactor - letterSpacing ) ) / 2;
          partYOffset += fragmentMetrics.ascent() / scaleFactor;
          path.addText( partXOffset, partYOffset + fragmentYOffset, fragmentFont, part );
          partYOffset += letterSpacing;
        }
        partLastDescent = fragmentMetrics.descent() / scaleFactor;

        fragmentIndex++;
      }
      height = partYOffset + partLastDescent;
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
    bufferComponent.size.setHeight( height );

    if ( format.orientation() == Qgis::TextOrientation::Vertical || format.orientation() == Qgis::TextOrientation::RotationBased )
    {
      bufferComponent.offset.setY( - bufferComponent.size.height() );
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
  QgsPainting::applyScaleFixForQPictureDpi( p );
  p->drawPicture( 0, 0, buffPict );

  return advance / scaleFactor;
}

void QgsTextRenderer::drawMask( QgsRenderContext &context, const QgsTextRenderer::Component &component, const QgsTextFormat &format, const QgsTextDocumentMetrics &metrics,
                                Qgis::TextLayoutMode mode )
{
  QgsTextMaskSettings mask = format.mask();

  // the mask is drawn to a side painter
  // or to the main painter for preview
  QPainter *p = context.isGuiPreview() ? context.painter() : context.maskPainter( context.currentMaskId() );
  if ( ! p )
    return;

  double penSize = mask.sizeUnit() == Qgis::RenderUnit::Percentage
                   ? context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() ) * mask.size() / 100
                   : context.convertToPainterUnits( mask.size(), mask.sizeUnit(), mask.sizeMapUnitScale() );

  // buffer: draw the text with a big pen
  QPainterPath path;
  path.setFillRule( Qt::WindingFill );

  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  // TODO: vertical text mode was ignored when masking feature was added.
  // Hopefully Oslandia come back and fix this? Hint hint...

  std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
  if ( mode == Qgis::TextLayoutMode::Labeling )
  {
    // label size has already been calculated using any symbology reference scale factor -- we need
    // to temporarily remove the reference scale here or we'll be applying the scaling twice
    referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
  }

  if ( metrics.isNullFontSize() )
    return;

  referenceScaleOverride.reset();

  double xOffset = 0;
  int fragmentIndex = 0;
  for ( const QgsTextFragment &fragment : component.block )
  {
    if ( !fragment.isWhitespace() && !fragment.isImage() )
    {
      const QFont fragmentFont = metrics.fragmentFont( component.blockIndex, fragmentIndex );

      const double fragmentYOffset = metrics.fragmentVerticalOffset( component.blockIndex, fragmentIndex, mode );
      path.addText( xOffset, fragmentYOffset, fragmentFont, fragment.text() );
    }

    xOffset += metrics.fragmentHorizontalAdvance( component.blockIndex, fragmentIndex, mode ) * scaleFactor;
    fragmentIndex++;
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
  const QgsTextDocument doc = QgsTextDocument::fromTextAndFormat( textLines, format );
  if ( doc.size() == 0 )
    return 0;

  return textWidth( context, format, doc );
}

double QgsTextRenderer::textWidth( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &document )
{
  //calculate max width of text lines
  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  const QgsTextDocumentMetrics metrics = QgsTextDocumentMetrics::calculateMetrics( document, format, context, scaleFactor );

  // width doesn't change depending on layout mode, we can use anything here
  return metrics.documentSize( Qgis::TextLayoutMode::Point, format.orientation() ).width();
}

double QgsTextRenderer::textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QStringList &textLines, Qgis::TextLayoutMode mode, QFontMetricsF *, Qgis::TextRendererFlags flags, double maxLineWidth )
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

  const QgsTextDocument doc = QgsTextDocument::fromTextAndFormat( lines, format );
  return textHeight( context, format, doc, mode );
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
    maxExtension += format.buffer().sizeUnit() == Qgis::RenderUnit::Percentage
                    ? fontSize * format.buffer().size() / 100
                    : context.convertToPainterUnits( format.buffer().size(), format.buffer().sizeUnit(), format.buffer().sizeMapUnitScale() );
  }
  if ( format.shadow().enabled() )
  {
    maxExtension += ( format.shadow().offsetUnit() == Qgis::RenderUnit::Percentage
                      ? fontSize * format.shadow().offsetDistance() / 100
                      : context.convertToPainterUnits( format.shadow().offsetDistance(), format.shadow().offsetUnit(), format.shadow().offsetMapUnitScale() )
                    )
                    + ( format.shadow().blurRadiusUnit() == Qgis::RenderUnit::Percentage
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

double QgsTextRenderer::textHeight( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &doc, Qgis::TextLayoutMode mode )
{
  QgsTextDocument document = doc;
  document.applyCapitalization( format.capitalization() );

  //calculate max height of text lines
  const double scaleFactor = calculateScaleFactorForFormat( context, format );

  const QgsTextDocumentMetrics metrics = QgsTextDocumentMetrics::calculateMetrics( document, format, context, scaleFactor );
  if ( metrics.isNullFontSize() )
    return 0;

  return metrics.documentSize( mode, format.orientation() ).height();
}

void QgsTextRenderer::drawBackground( QgsRenderContext &context, const QgsTextRenderer::Component &c, const QgsTextFormat &format, const QgsTextDocumentMetrics &metrics, Qgis::TextLayoutMode mode )
{
  Component component = c;
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

  if ( mode != Qgis::TextLayoutMode::Labeling )
  {
    // need to calculate size of text
    const QSizeF documentSize = metrics.documentSize( mode, format.orientation() );
    double width = documentSize.width();
    double height = documentSize.height();

    switch ( mode )
    {
      case Qgis::TextLayoutMode::Rectangle:
      case Qgis::TextLayoutMode::RectangleCapHeightBased:
      case Qgis::TextLayoutMode::RectangleAscentBased:
        switch ( component.hAlign )
        {
          case Qgis::TextHorizontalAlignment::Left:
          case Qgis::TextHorizontalAlignment::Justify:
            component.center = QPointF( component.origin.x() + width / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;

          case Qgis::TextHorizontalAlignment::Center:
            component.center = QPointF( component.origin.x() + component.size.width() / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;

          case Qgis::TextHorizontalAlignment::Right:
            component.center = QPointF( component.origin.x() + component.size.width() - width / 2.0,
                                        component.origin.y() + height / 2.0 );
            break;
        }
        break;

      case Qgis::TextLayoutMode::Point:
      {
        bool isNullSize = false;
        QFontMetricsF fm( format.scaledFont( context, scaleFactor, &isNullSize ) );
        double originAdjust = isNullSize ? 0 : ( fm.ascent() / scaleFactor / 2.0 - fm.leading() / scaleFactor / 2.0 );
        switch ( component.hAlign )
        {
          case Qgis::TextHorizontalAlignment::Left:
          case Qgis::TextHorizontalAlignment::Justify:
            component.center = QPointF( component.origin.x() + width / 2.0,
                                        component.origin.y() - height / 2.0 + originAdjust );
            break;

          case Qgis::TextHorizontalAlignment::Center:
            component.center = QPointF( component.origin.x(),
                                        component.origin.y() - height / 2.0 + originAdjust );
            break;

          case Qgis::TextHorizontalAlignment::Right:
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

      case Qgis::TextLayoutMode::Labeling:
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
      {
        QgsScopedRenderContextReferenceScaleOverride referenceScaleOverride( context, -1 );

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
        map[QStringLiteral( "size_unit" )] = QgsUnitTypes::encodeUnit( Qgis::RenderUnit::Pixels );
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
          QgsSymbolRenderContext svgShdwContext( shdwContext, Qgis::RenderUnit::Unknown, background.opacity() );

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
        renderedSymbol->setSizeUnit( Qgis::RenderUnit::Pixels );
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
        if ( background.radiiUnit() == Qgis::RenderUnit::Percentage )
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
      QgsPainting::applyScaleFixForQPictureDpi( p );
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

  QPainter *p = context.painter();
  const double componentWidth = component.size.width();
  const double componentHeight = component.size.height();
  const double xOffset = component.offset.x();
  const double yOffset = component.offset.y();
  double pictbuffer = component.pictureBuffer;

  // generate pixmap representation of label component drawing
  bool mapUnits = shadow.blurRadiusUnit() == Qgis::RenderUnit::MapUnits;

  const double fontSize = context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() );
  double radius = shadow.blurRadiusUnit() == Qgis::RenderUnit::Percentage
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

  const double offsetDist = shadow.offsetUnit() == Qgis::RenderUnit::Percentage
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


void QgsTextRenderer::drawTextInternal( Qgis::TextComponents components,
                                        QgsRenderContext &context,
                                        const QgsTextFormat &format,
                                        const Component &component,
                                        const QgsTextDocument &document,
                                        const QgsTextDocumentMetrics &metrics,
                                        Qgis::TextHorizontalAlignment alignment, Qgis::TextVerticalAlignment vAlignment, Qgis::TextLayoutMode mode )
{
  if ( !context.painter() )
  {
    return;
  }

  const double fontScale = calculateScaleFactorForFormat( context, format );

  std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
  if ( mode == Qgis::TextLayoutMode::Labeling )
  {
    // label size has already been calculated using any symbology reference scale factor -- we need
    // to temporarily remove the reference scale here or we'll be applying the scaling twice
    referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
  }

  if ( metrics.isNullFontSize() )
    return;

  referenceScaleOverride.reset();

  double rotation = 0;
  const Qgis::TextOrientation orientation = calculateRotationAndOrientationForComponent( format, component, rotation );
  switch ( orientation )
  {
    case Qgis::TextOrientation::Horizontal:
    {
      drawTextInternalHorizontal( context, format, components, mode, component, document, metrics, fontScale, alignment, vAlignment, rotation );
      break;
    }

    case Qgis::TextOrientation::Vertical:
    case Qgis::TextOrientation::RotationBased:
    {
      // TODO: vertical text renderer currently doesn't handle one-pass buffer + text drawing
      if ( components & Qgis::TextComponent::Buffer )
        drawTextInternalVertical( context, format, Qgis::TextComponent::Buffer, mode, component, document, metrics, fontScale, alignment, vAlignment, rotation );
      if ( components & Qgis::TextComponent::Text )
        drawTextInternalVertical( context, format, Qgis::TextComponent::Text, mode, component, document, metrics, fontScale, alignment, vAlignment, rotation );
      break;
    }
  }
}

Qgis::TextOrientation QgsTextRenderer::calculateRotationAndOrientationForComponent( const QgsTextFormat &format, const QgsTextRenderer::Component &component, double &rotation )
{
  rotation = -component.rotation * 180 / M_PI;

  switch ( format.orientation() )
  {
    case Qgis::TextOrientation::RotationBased:
    {
      // Between 45 to 135 and 235 to 315 degrees, rely on vertical orientation
      if ( rotation >= -315 && rotation < -90 )
      {
        rotation -= 90;
        return Qgis::TextOrientation::Vertical;
      }
      else if ( rotation >= -90 && rotation < -45 )
      {
        rotation += 90;
        return Qgis::TextOrientation::Vertical;
      }

      return Qgis::TextOrientation::Horizontal;
    }

    case Qgis::TextOrientation::Horizontal:
    case Qgis::TextOrientation::Vertical:
      return format.orientation();
  }
  return Qgis::TextOrientation::Horizontal;
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


void QgsTextRenderer::renderBlockHorizontal( const QgsTextBlock &block, int blockIndex,
    const QgsTextDocumentMetrics &metrics, QgsRenderContext &context,
    const QgsTextFormat &format,
    QPainter *painter, bool forceRenderAsPaths,
    double fontScale, double extraWordSpace, double extraLetterSpace,
    Qgis::TextLayoutMode mode, DeferredRenderBlock *deferredRenderBlock )
{
  if ( !metrics.isNullFontSize() )
  {
    double xOffset = 0;
    int fragmentIndex = 0;
    for ( const QgsTextFragment &fragment : block )
    {
      // draw text, QPainterPath method
      if ( !fragment.isWhitespace() && !fragment.isImage() )
      {
        QFont fragmentFont = metrics.fragmentFont( blockIndex, fragmentIndex );

        if ( !qgsDoubleNear( extraWordSpace, 0 ) || !qgsDoubleNear( extraLetterSpace, 0 ) )
          applyExtraSpacingForLineJustification( fragmentFont, extraWordSpace * fontScale, extraLetterSpace * fontScale );

        const double yOffset = metrics.fragmentVerticalOffset( blockIndex, fragmentIndex, mode );

        QColor textColor = fragment.characterFormat().textColor().isValid() ? fragment.characterFormat().textColor() : format.color();
        textColor.setAlphaF( fragment.characterFormat().textColor().isValid() ? textColor.alphaF() * format.opacity() : format.opacity() );

        if ( deferredRenderBlock )
        {
          DeferredRenderFragment renderFragment;
          renderFragment.color = textColor;
          if ( forceRenderAsPaths )
          {
            renderFragment.path.setFillRule( Qt::WindingFill );
            renderFragment.path.addText( xOffset, yOffset, fragmentFont, fragment.text() );
          }
          renderFragment.font = fragmentFont;
          renderFragment.point = QPointF( xOffset, yOffset );
          renderFragment.text = fragment.text();
          deferredRenderBlock->fragments.append( renderFragment );
        }
        else if ( forceRenderAsPaths )
        {
          painter->setBrush( textColor );
          QPainterPath path;
          path.setFillRule( Qt::WindingFill );
          path.addText( xOffset, yOffset, fragmentFont, fragment.text() );
          painter->drawPath( path );
        }
        else
        {
          painter->setPen( textColor );
          painter->setFont( fragmentFont );
          painter->drawText( QPointF( xOffset, yOffset ), fragment.text() );
        }
      }
      else if ( fragment.isImage() )
      {
        bool fitsInCache = false;
        const double imageWidth = metrics.fragmentHorizontalAdvance( blockIndex, fragmentIndex, mode ) * fontScale;
        const double imageHeight = metrics.fragmentFixedHeight( blockIndex, fragmentIndex, mode ) * fontScale;

        const QImage image = QgsApplication::imageCache()->pathAsImage( fragment.characterFormat().imagePath(),
                             QSize( static_cast< int >( std::round( imageWidth ) ),
                                    static_cast< int >( std::round( imageHeight ) ) ),
                             false,
                             1, fitsInCache, context.flags() & Qgis::RenderContextFlag::RenderBlocking );
        const double imageBaseline = metrics.fragmentVerticalOffset( blockIndex, fragmentIndex, mode );
        const double yOffset = imageBaseline - image.height();
        if ( !image.isNull() )
          painter->drawImage( QPointF( xOffset, yOffset ), image );
      }

      xOffset += metrics.fragmentHorizontalAdvance( blockIndex, fragmentIndex, mode ) * fontScale;
      fragmentIndex ++;
    }
  }
};

bool QgsTextRenderer::usePathsToRender( const QgsRenderContext &context, const QgsTextFormat &format, const QgsTextDocument &document )
{
  switch ( context.textRenderFormat() )
  {
    case Qgis::TextRenderFormat::AlwaysOutlines:
      return true;
    case Qgis::TextRenderFormat::AlwaysText:
      return false;
    case Qgis::TextRenderFormat::PreferText:
    {
      // Prefer not to use paths -- but certain conditions will require us to use them
      if ( format.buffer().enabled() )
      {
        // text buffer requires use of paths
        // TODO: this was the original cause of use switching from text to paths by default,
        // but that was way back in the 2.0 days and maybe the Qt issues have now been fixed?
        return true;
      }

      // underline/overline/strikethrough looks different between path/non-path renders.
      // TODO: validate which is correct. For now, maintain default appearance from before this code
      // was introduced
      if ( format.font().underline()
           || format.font().overline()
           || format.font().strikeOut()
           || std::any_of( document.begin(), document.end(), []( const QgsTextBlock & block )
    {
      return std::any_of( block.begin(), block.end(), []( const QgsTextFragment & fragment )
      {
        return fragment.characterFormat().underline() == QgsTextCharacterFormat::BooleanValue::SetTrue
                 || fragment.characterFormat().overline() == QgsTextCharacterFormat::BooleanValue::SetTrue
                 || fragment.characterFormat().strikeOut() == QgsTextCharacterFormat::BooleanValue::SetTrue;
        } );
      } ) )
      return true;

      return false;
    }
  }
  BUILTIN_UNREACHABLE
}

bool QgsTextRenderer::usePictureToRender( const QgsRenderContext &, const QgsTextFormat &, const QgsTextDocument &document )
{
  return std::any_of( document.begin(), document.end(), []( const QgsTextBlock & block )
  {
    return std::any_of( block.begin(), block.end(), []( const QgsTextFragment & fragment )
    {
      return fragment.isImage();
    } );
  } );
}

QVector< QgsTextRenderer::BlockMetrics > QgsTextRenderer::calculateBlockMetrics( const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics, Qgis::TextLayoutMode mode, double targetWidth, const Qgis::TextHorizontalAlignment hAlignment )
{
  QVector< BlockMetrics > blockMetrics;
  blockMetrics.reserve( document.size() );

  int blockIndex = 0;
  for ( const QgsTextBlock &block : document )
  {
    Qgis::TextHorizontalAlignment blockAlignment = hAlignment;
    if ( block.blockFormat().hasHorizontalAlignmentSet() )
      blockAlignment = block.blockFormat().horizontalAlignment();
    const bool adjustForAlignment = blockAlignment != Qgis::TextHorizontalAlignment::Left &&
                                    ( mode != Qgis::TextLayoutMode::Labeling
                                      || document.size() > 1 );

    const bool isFinalLineInParagraph = ( blockIndex == document.size() - 1 )
                                        || document.at( blockIndex + 1 ).toPlainText().trimmed().isEmpty();

    BlockMetrics thisBlockMetrics;
    // figure x offset for horizontal alignment of multiple lines
    thisBlockMetrics.width = metrics.blockWidth( blockIndex );

    if ( adjustForAlignment )
    {
      double blockWidthDiff = 0;
      switch ( blockAlignment )
      {
        case Qgis::TextHorizontalAlignment::Center:
          blockWidthDiff = ( targetWidth - thisBlockMetrics.width - metrics.blockLeftMargin( blockIndex ) - metrics.blockRightMargin( blockIndex ) ) * 0.5 + metrics.blockLeftMargin( blockIndex );
          break;

        case Qgis::TextHorizontalAlignment::Right:
          blockWidthDiff = targetWidth - thisBlockMetrics.width - metrics.blockRightMargin( blockIndex );
          break;

        case Qgis::TextHorizontalAlignment::Justify:
          if ( !isFinalLineInParagraph && targetWidth > thisBlockMetrics.width )
          {
            calculateExtraSpacingForLineJustification( targetWidth - thisBlockMetrics.width, block, thisBlockMetrics.extraWordSpace, thisBlockMetrics.extraLetterSpace );
            thisBlockMetrics.width = targetWidth;
          }
          blockWidthDiff = metrics.blockLeftMargin( blockIndex );
          break;

        case Qgis::TextHorizontalAlignment::Left:
          blockWidthDiff = metrics.blockLeftMargin( blockIndex );
          break;
      }

      switch ( mode )
      {
        case Qgis::TextLayoutMode::Labeling:
        case Qgis::TextLayoutMode::Rectangle:
        case Qgis::TextLayoutMode::RectangleCapHeightBased:
        case Qgis::TextLayoutMode::RectangleAscentBased:
          thisBlockMetrics.xOffset = blockWidthDiff;
          break;

        case Qgis::TextLayoutMode::Point:
        {
          switch ( blockAlignment )
          {
            case Qgis::TextHorizontalAlignment::Right:
              thisBlockMetrics.xOffset = blockWidthDiff - targetWidth;
              break;

            case Qgis::TextHorizontalAlignment::Center:
              thisBlockMetrics.xOffset = blockWidthDiff - targetWidth / 2.0;
              break;

            case Qgis::TextHorizontalAlignment::Left:
            case Qgis::TextHorizontalAlignment::Justify:
              thisBlockMetrics.xOffset = metrics.blockLeftMargin( blockIndex );
              break;
          }
        }
        break;
      }
    }
    else if ( blockAlignment == Qgis::TextHorizontalAlignment::Left || blockAlignment == Qgis::TextHorizontalAlignment::Justify )
    {
      thisBlockMetrics.xOffset = metrics.blockLeftMargin( blockIndex );
    }

    switch ( mode )
    {
      case Qgis::TextLayoutMode::Rectangle:
      case Qgis::TextLayoutMode::RectangleCapHeightBased:
      case Qgis::TextLayoutMode::RectangleAscentBased:
        thisBlockMetrics.backgroundWidth = targetWidth;
        thisBlockMetrics.backgroundXOffset = 0;
        break;
      case Qgis::TextLayoutMode::Point:
      case Qgis::TextLayoutMode::Labeling:
        thisBlockMetrics.backgroundWidth = thisBlockMetrics.width;
        thisBlockMetrics.backgroundXOffset = thisBlockMetrics.xOffset;
        break;
    }

    blockMetrics << thisBlockMetrics;
    blockIndex++;
  }
  return blockMetrics;
}

QBrush QgsTextRenderer::createBrushForPath( QgsRenderContext &context, const QString &path )
{
  bool fitsInCache = false;
  // use original image size
  const QSize imageSize = QgsApplication::imageCache()->originalSize( path, context.flags() & Qgis::RenderContextFlag::RenderBlocking );
  // TODO: maybe there's more optimal logic we could use here, but for now we assume 96dpi image resolution...
  const QSizeF originalSizeMmAt96Dpi = imageSize / 3.7795275590551185;
  const double pixelsPerMm = context.scaleFactor();
  const double imageWidth = originalSizeMmAt96Dpi.width() * pixelsPerMm;
  const double imageHeight = originalSizeMmAt96Dpi.height() * pixelsPerMm;
  QBrush res;
  if ( imageWidth == 0 || imageHeight == 0 )
    return res;
  const QImage image = QgsApplication::imageCache()->pathAsImage( path,
                       QSize( static_cast< int >( std::round( imageWidth ) ),
                              static_cast< int >( std::round( imageHeight ) ) ),
                       false,
                       1, fitsInCache, context.flags() & Qgis::RenderContextFlag::RenderBlocking );

  if ( !image.isNull() )
  {

    res.setTextureImage( image );
  }
  return res;
}

void QgsTextRenderer::renderDocumentBackgrounds( QgsRenderContext &context, const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics, const Component &component,  const QVector< QgsTextRenderer::BlockMetrics > &blockMetrics, Qgis::TextLayoutMode mode, double verticalAlignOffset, double rotation )
{
  int blockIndex = 0;
  context.painter()->translate( component.origin );
  if ( !qgsDoubleNear( rotation, 0.0 ) )
    context.painter()->rotate( rotation );

  context.painter()->setPen( Qt::NoPen );
  context.painter()->setBrush( Qt::NoBrush );
  for ( const QgsTextBlock &block : document )
  {
    const double baseLineOffset = metrics.baselineOffset( blockIndex, mode );
    const double blockMaximumDescent = metrics.blockMaximumDescent( blockIndex );
    const double blockMaximumAscent = metrics.blockMaximumAscent( blockIndex );

    if ( block.blockFormat().hasBackground() )
    {
      QBrush backgroundBrush = block.blockFormat().backgroundBrush();
      if ( !block.blockFormat().backgroundImagePath().isEmpty() )
      {
        const QBrush backgroundImageBrush = createBrushForPath( context, block.blockFormat().backgroundImagePath() );
        if ( backgroundImageBrush.style() == Qt::BrushStyle::TexturePattern )
          backgroundBrush = backgroundImageBrush;
      }

      context.painter()->setBrush( backgroundBrush );
      context.painter()->drawRect( QRectF( blockMetrics[ blockIndex ].backgroundXOffset, baseLineOffset - blockMaximumAscent, blockMetrics[ blockIndex ].backgroundWidth, blockMaximumDescent + blockMaximumAscent ) );
    }

    double xOffset = 0;
    int fragmentIndex = 0;

    for ( const QgsTextFragment &fragment : block )
    {
      const double horizontalAdvance = metrics.fragmentHorizontalAdvance( blockIndex, fragmentIndex, mode );
      const double ascent = metrics.fragmentAscent( blockIndex, fragmentIndex, mode );
      const double descent = metrics.fragmentDescent( blockIndex, fragmentIndex, mode );

      if ( fragment.characterFormat().hasBackground() )
      {
        const double yOffset = metrics.fragmentVerticalOffset( blockIndex, fragmentIndex, mode );

        QBrush backgroundBrush = fragment.characterFormat().backgroundBrush();
        if ( !fragment.characterFormat().backgroundImagePath().isEmpty() )
        {
          const QBrush backgroundImageBrush = createBrushForPath( context, fragment.characterFormat().backgroundImagePath() );
          if ( backgroundImageBrush.style() == Qt::BrushStyle::TexturePattern )
            backgroundBrush = backgroundImageBrush;
        }

        context.painter()->setBrush( backgroundBrush );
        context.painter()->drawRect( QRectF( blockMetrics[ blockIndex ].xOffset + xOffset,
                                             baseLineOffset + verticalAlignOffset + yOffset - ascent, horizontalAdvance, ascent + descent ) );
      }

      xOffset += horizontalAdvance;
      fragmentIndex ++;
    }

    blockIndex++;
  }

  context.painter()->setBrush( Qt::NoBrush );

  if ( !qgsDoubleNear( rotation, 0.0 ) )
    context.painter()->rotate( -rotation );
  context.painter()->translate( -component.origin );
}

void QgsTextRenderer::drawTextInternalHorizontal( QgsRenderContext &context, const QgsTextFormat &format, Qgis::TextComponents components, Qgis::TextLayoutMode mode, const Component &component, const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics, double fontScale, const Qgis::TextHorizontalAlignment hAlignment,
    Qgis::TextVerticalAlignment vAlignment, double rotation )
{
  QPainter *maskPainter = context.maskPainter( context.currentMaskId() );

  const QSizeF documentSize = metrics.documentSize( mode, Qgis::TextOrientation::Horizontal );

  double targetWidth = 0.0;
  switch ( mode )
  {
    case Qgis::TextLayoutMode::Labeling:
    case Qgis::TextLayoutMode::Point:
      targetWidth = documentSize.width();
      break;

    case Qgis::TextLayoutMode::Rectangle:
    case Qgis::TextLayoutMode::RectangleCapHeightBased:
    case Qgis::TextLayoutMode::RectangleAscentBased:
      targetWidth = component.size.width();
      break;
  }

  double verticalAlignOffset = 0;

  if ( mode == Qgis::TextLayoutMode::Rectangle )
  {
    const double overallHeight = documentSize.height();
    switch ( vAlignment )
    {
      case Qgis::TextVerticalAlignment::Top:
        verticalAlignOffset = metrics.blockVerticalMargin( - 1 );
        break;

      case Qgis::TextVerticalAlignment::VerticalCenter:
        verticalAlignOffset = ( component.size.height() - overallHeight ) * 0.5 + metrics.blockVerticalMargin( - 1 );
        break;

      case Qgis::TextVerticalAlignment::Bottom:
        verticalAlignOffset = ( component.size.height() - overallHeight ) + metrics.blockVerticalMargin( - 1 );
        break;
    }
  }
  else if ( mode == Qgis::TextLayoutMode::Point )
  {
    verticalAlignOffset = - metrics.blockVerticalMargin( document.size() - 1 );
  }

  // should we use text or paths for this render?
  const bool usePathsForText = usePathsToRender( context, format, document );

  // TODO -- maybe we can avoid the nested vector? Need to confirm whether painter rotation & translation can be
  // done ONCE only, upfront
  std::unique_ptr< std::vector< DeferredRenderBlock > > deferredBlocks;

  // Depending on format settings, we may need to render in multiple passes. Eg buffer than text, or shadow than text.
  // We try to avoid this if possible as it requires more work, and just do a single pass, rendering text directly as we go.
  // If we need to do multi-pass rendering then we'll calculate paths ONCE upfront and defer actually renderring these.
  const bool requiresMultiPassRendering = ( components & Qgis::TextComponent::Buffer && format.buffer().enabled() )
                                          || ( components & Qgis::TextComponent::Shadow && format.shadow().enabled() && ( format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowText || format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowBuffer ) );
  if ( requiresMultiPassRendering )
  {
    deferredBlocks = std::make_unique< std::vector< DeferredRenderBlock > >();
    deferredBlocks->reserve( document.size() );
  }

  if ( ( components & Qgis::TextComponent::Buffer )
       || ( components & Qgis::TextComponent::Text )
       || ( components & Qgis::TextComponent::Shadow ) )
  {
    const QVector< BlockMetrics > blockMetrics = calculateBlockMetrics( document, metrics, mode, targetWidth, hAlignment );

    if ( document.hasBackgrounds() )
    {
      renderDocumentBackgrounds( context, document, metrics, component, blockMetrics, mode, verticalAlignOffset, rotation );
    }

    int blockIndex = 0;
    for ( const QgsTextBlock &block : document )
    {
      const double blockHeight = metrics.blockHeight( blockIndex );

      DeferredRenderBlock *deferredBlock = nullptr;
      if ( requiresMultiPassRendering && deferredBlocks )
      {
        deferredBlocks->emplace_back( DeferredRenderBlock() );
        deferredBlock = &deferredBlocks->back();
        deferredBlock->fragments.reserve( block.size() );
      }

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

      const BlockMetrics thisBlockMetrics = blockMetrics[ blockIndex ];
      const double baseLineOffset = metrics.baselineOffset( blockIndex, mode );

      const QPointF blockOrigin( thisBlockMetrics.xOffset, baseLineOffset + verticalAlignOffset );
      if ( deferredBlock )
        deferredBlock->origin = blockOrigin;
      else
        context.painter()->translate( blockOrigin );
      if ( maskPainter )
        maskPainter->translate( blockOrigin );

      Component subComponent;
      subComponent.block = block;
      subComponent.blockIndex = blockIndex;
      subComponent.size = QSizeF( thisBlockMetrics.width, blockHeight );
      subComponent.offset = QPointF( 0.0, -metrics.ascentOffset() );
      subComponent.rotation = -component.rotation * 180 / M_PI;
      subComponent.rotationOffset = 0.0;
      subComponent.extraWordSpacing = thisBlockMetrics.extraWordSpace * fontScale;
      subComponent.extraLetterSpacing = thisBlockMetrics.extraLetterSpace * fontScale;
      if ( deferredBlock )
        deferredBlock->component = subComponent;

      // draw the mask below the text (for preview)
      if ( format.mask().enabled() )
      {
        QgsTextRenderer::drawMask( context, subComponent, format, metrics, mode );
      }

      // if we are drawing both text + buffer, we'll need a path, as we HAVE to render buffers using paths
      const bool needsPaths = usePathsForText
                              || ( ( components & Qgis::TextComponent::Buffer ) && format.buffer().enabled() )
                              || ( ( components & Qgis::TextComponent::Shadow ) && format.shadow().enabled() );

      std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
      if ( mode == Qgis::TextLayoutMode::Labeling )
      {
        // label size has already been calculated using any symbology reference scale factor -- we need
        // to temporarily remove the reference scale here or we'll be applying the scaling twice
        referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
      }

      referenceScaleOverride.reset();

      // now render the actual text
      if ( context.useAdvancedEffects() )
      {
        context.painter()->setCompositionMode( format.blendMode() );
      }

      // scale for any print output or image saving @ specific dpi
      context.painter()->scale( subComponent.dpiRatio, subComponent.dpiRatio );

      context.painter()->scale( 1 / fontScale, 1 / fontScale );
      context.painter()->setPen( Qt::NoPen );
      context.painter()->setBrush( Qt::NoBrush );

      renderBlockHorizontal( block, blockIndex, metrics, context, format, context.painter(), needsPaths,
                             fontScale, thisBlockMetrics.extraWordSpace, thisBlockMetrics.extraLetterSpace, mode, deferredBlock );

      if ( maskPainter )
        maskPainter->restore();

      blockIndex++;
    }
  }

  if ( deferredBlocks )
  {
    renderDeferredBlocks(
      context, format, components, *deferredBlocks, usePathsForText, fontScale, component, rotation
    );
  }
}

void QgsTextRenderer::renderDeferredBlocks( QgsRenderContext &context,
    const QgsTextFormat &format,
    Qgis::TextComponents components,
    const std::vector< DeferredRenderBlock > &deferredBlocks,
    bool usePathsForText,
    double fontScale,
    const Component &component,
    double rotation )
{
  if ( format.buffer().enabled() && ( components & Qgis::TextComponent::Buffer ) )
  {
    renderDeferredBuffer( context, format, components, deferredBlocks, fontScale, component, rotation );
  }

  if ( ( components & Qgis::TextComponent::Shadow )
       && format.shadow().enabled()
       && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowText )
  {
    renderDeferredShadowForText( context, format, deferredBlocks, fontScale, component, rotation );
    // TODO: there's an optimisation opportunity here -- if we are ALSO rendering the text component,
    // we could move the actual text rendering into renderDeferredShadowForText and use the same
    // QPicture as we used for the shadow. But we'd need to ensure that all the settings
    // which control whether text is rendered as text or paths also also considered.
  }

  if ( components & Qgis::TextComponent::Text )
  {
    renderDeferredText( context, deferredBlocks, usePathsForText, fontScale, component, rotation );
  }
}

void QgsTextRenderer::renderDeferredShadowForText( QgsRenderContext &context,
    const QgsTextFormat &format,
    const std::vector< DeferredRenderBlock > &deferredBlocks,
    double fontScale,
    const Component &component,
    double rotation )
{
  QgsScopedQPainterState painterState( context.painter() );
  context.setPainterFlagsUsingContext();
  context.painter()->translate( component.origin );
  if ( !qgsDoubleNear( rotation, 0.0 ) )
    context.painter()->rotate( rotation );

  context.painter()->setPen( Qt::NoPen );
  context.painter()->setBrush( Qt::NoBrush );

  for ( const DeferredRenderBlock &block : deferredBlocks )
  {
    Component subComponent = block.component;

    QPainter painter( &subComponent.picture );
    painter.setPen( Qt::NoPen );
    painter.setBrush( Qt::NoBrush );
    painter.scale( 1 / fontScale, 1 / fontScale );

    for ( const DeferredRenderFragment &fragment : std::as_const( block.fragments ) )
    {
      if ( !fragment.path.isEmpty() )
      {
        painter.setBrush( fragment.color );
        painter.drawPath( fragment.path );
      }
      else
      {
        painter.setPen( fragment.color );
        painter.setFont( fragment.font );
        painter.drawText( fragment.point, fragment.text );
      }
    }
    painter.end();

    subComponent.pictureBuffer = 1.0; // no pen width to deal with, but we'll add 1 px for antialiasing
    subComponent.origin = QPointF( 0.0, 0.0 );
    const QRectF pictureBoundingRect = subComponent.picture.boundingRect();
    subComponent.size = pictureBoundingRect.size();
    subComponent.offset = QPointF( -pictureBoundingRect.left(), -pictureBoundingRect.height() - pictureBoundingRect.top() );

    context.painter()->translate( block.origin );
    drawShadow( context, subComponent, format );
    context.painter()->translate( -block.origin );
  }
}

void QgsTextRenderer::renderDeferredBuffer( QgsRenderContext &context,
    const QgsTextFormat &format,
    Qgis::TextComponents components,
    const std::vector< DeferredRenderBlock > &deferredBlocks,
    double fontScale,
    const Component &component,
    double rotation )
{
  QgsScopedQPainterState painterState( context.painter() );
  context.setPainterFlagsUsingContext();

  // do we need a drop shadow effect on the buffer component? If so, we'll render the buffer to a QPicture first and then use this
  // to generate the shadow, and then render the QPicture as the buffer on top. If not, avoid the unwanted expense of the temporary QPicture
  // and render directly.
  const bool needsShadowOnBuffer = ( ( components & Qgis::TextComponent::Shadow ) && format.shadow().enabled() && format.shadow().shadowPlacement() == QgsTextShadowSettings::ShadowBuffer );
  std::unique_ptr< QPicture > bufferPicture;
  std::unique_ptr< QPainter > bufferPainter;
  QPainter *prevPainter = context.painter();
  if ( needsShadowOnBuffer )
  {
    bufferPicture = std::make_unique< QPicture >();
    bufferPainter = std::make_unique< QPainter >( bufferPicture.get() );
    context.setPainter( bufferPainter.get() );
  }

  std::unique_ptr< QgsPaintEffect > tmpEffect;
  if ( format.buffer().paintEffect() && format.buffer().paintEffect()->enabled() )
  {
    tmpEffect.reset( format.buffer().paintEffect()->clone() );
    tmpEffect->begin( context );
  }

  QColor bufferColor = format.buffer().color();
  bufferColor.setAlphaF( format.buffer().opacity() );
  QPen pen( bufferColor );
  const QgsTextBufferSettings &buffer = format.buffer();
  const double penSize = buffer.sizeUnit() == Qgis::RenderUnit::Percentage
                         ? context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() ) * buffer.size() / 100
                         : context.convertToPainterUnits( buffer.size(), buffer.sizeUnit(), buffer.sizeMapUnitScale() );
  pen.setWidthF( penSize * fontScale );
  pen.setJoinStyle( buffer.joinStyle() );
  context.painter()->setPen( pen );

  // honor pref for whether to fill buffer interior
  if ( !buffer.fillBufferInterior() )
  {
    bufferColor.setAlpha( 0 );
  }
  context.painter()->setBrush( bufferColor );

  context.painter()->translate( component.origin );
  if ( !qgsDoubleNear( rotation, 0.0 ) )
    context.painter()->rotate( rotation );

  if ( context.useAdvancedEffects() )
  {
    context.painter()->setCompositionMode( format.buffer().blendMode() );
  }

  for ( const DeferredRenderBlock &block : deferredBlocks )
  {
    context.painter()->translate( block.origin );
    context.painter()->scale( 1 / fontScale, 1 / fontScale );
    for ( const DeferredRenderFragment &fragment : std::as_const( block.fragments ) )
    {
      context.painter()->drawPath( fragment.path );
    }
    context.painter()->scale( fontScale, fontScale );
    context.painter()->translate( -block.origin );
  }

  if ( tmpEffect )
  {
    tmpEffect->end( context );
  }

  if ( needsShadowOnBuffer && bufferPicture )
  {
    bufferPainter->end();
    bufferPainter.reset();
    context.setPainter( prevPainter );

    QgsTextRenderer::Component bufferComponent = component;
    bufferComponent.origin = QPointF( 0.0, 0.0 );
    bufferComponent.picture = *bufferPicture;
    bufferComponent.pictureBuffer = penSize / 2.0;
    const QRectF bufferBoundingBox = bufferPicture->boundingRect();
    bufferComponent.size = bufferBoundingBox.size();
    bufferComponent.offset = QPointF( -bufferBoundingBox.left(), -bufferBoundingBox.height() - bufferBoundingBox.top() );

    drawShadow( context, bufferComponent, format );

    // also draw buffer
    if ( context.useAdvancedEffects() )
    {
      context.painter()->setCompositionMode( buffer.blendMode() );
    }

    // scale for any print output or image saving @ specific dpi
    context.painter()->scale( component.dpiRatio, component.dpiRatio );
    QgsPainting::drawPicture( context.painter(), QPointF( 0, 0 ), *bufferPicture );
  }
}

void QgsTextRenderer::renderDeferredText( QgsRenderContext &context,
    const std::vector< DeferredRenderBlock > &deferredBlocks,
    bool usePathsForText,
    double fontScale,
    const Component &component,
    double rotation )
{
  QgsScopedQPainterState painterState( context.painter() );
  context.setPainterFlagsUsingContext();
  context.painter()->translate( component.origin );
  if ( !qgsDoubleNear( rotation, 0.0 ) )
    context.painter()->rotate( rotation );

  context.painter()->setPen( Qt::NoPen );
  context.painter()->setBrush( Qt::NoBrush );

  // draw the text
  for ( const DeferredRenderBlock &block : deferredBlocks )
  {
    context.painter()->translate( block.origin );
    context.painter()->scale( 1 / fontScale, 1 / fontScale );

    for ( const DeferredRenderFragment &fragment : std::as_const( block.fragments ) )
    {
      if ( usePathsForText )
      {
        context.painter()->setBrush( fragment.color );
        context.painter()->drawPath( fragment.path );
      }
      else
      {
        context.painter()->setPen( fragment.color );
        context.painter()->setFont( fragment.font );
        context.painter()->drawText( fragment.point, fragment.text );
      }
    }

    context.painter()->scale( fontScale, fontScale );
    context.painter()->translate( -block.origin );
  }
}

void QgsTextRenderer::drawTextInternalVertical( QgsRenderContext &context, const QgsTextFormat &format, Qgis::TextComponents components, Qgis::TextLayoutMode mode, const QgsTextRenderer::Component &component, const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics, double fontScale, Qgis::TextHorizontalAlignment hAlignment, Qgis::TextVerticalAlignment, double rotation )
{
  QPainter *maskPainter = context.maskPainter( context.currentMaskId() );
  const QStringList textLines = document.toPlainText();

  std::optional< QgsScopedRenderContextReferenceScaleOverride > referenceScaleOverride;
  if ( mode == Qgis::TextLayoutMode::Labeling )
  {
    // label size has already been calculated using any symbology reference scale factor -- we need
    // to temporarily remove the reference scale here or we'll be applying the scaling twice
    referenceScaleOverride.emplace( QgsScopedRenderContextReferenceScaleOverride( context, -1.0 ) );
  }

  if ( metrics.isNullFontSize() )
    return;

  referenceScaleOverride.reset();

  const QSizeF documentSize = metrics.documentSize( mode, Qgis::TextOrientation::Vertical );
  const double actualTextWidth = documentSize.width();
  double textRectWidth = 0.0;

  switch ( mode )
  {
    case Qgis::TextLayoutMode::Labeling:
    case Qgis::TextLayoutMode::Point:
      textRectWidth = actualTextWidth;
      break;

    case Qgis::TextLayoutMode::Rectangle:
    case Qgis::TextLayoutMode::RectangleCapHeightBased:
    case Qgis::TextLayoutMode::RectangleAscentBased:
      textRectWidth = component.size.width();
      break;
  }

  int maxLineLength = 0;
  for ( const QString &line : std::as_const( textLines ) )
  {
    maxLineLength = std::max( maxLineLength, static_cast<int>( line.length() ) );
  }

  const double actualLabelHeight = documentSize.height();
  int blockIndex = 0;

  bool adjustForAlignment = hAlignment != Qgis::TextHorizontalAlignment::Left && ( mode != Qgis::TextLayoutMode::Labeling || textLines.size() > 1 );

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

    const double blockMaximumCharacterWidth = metrics.blockMaximumCharacterWidth( blockIndex );

    // figure x offset of multiple lines
    double xOffset = metrics.verticalOrientationXOffset( blockIndex );
    if ( adjustForAlignment )
    {
      double hAlignmentOffset = 0;
      switch ( hAlignment )
      {
        case Qgis::TextHorizontalAlignment::Center:
          hAlignmentOffset = ( textRectWidth - actualTextWidth ) * 0.5;
          break;

        case Qgis::TextHorizontalAlignment::Right:
          hAlignmentOffset = textRectWidth - actualTextWidth;
          break;

        case Qgis::TextHorizontalAlignment::Left:
        case Qgis::TextHorizontalAlignment::Justify:
          break;
      }

      switch ( mode )
      {
        case Qgis::TextLayoutMode::Labeling:
        case Qgis::TextLayoutMode::Rectangle:
        case Qgis::TextLayoutMode::RectangleCapHeightBased:
        case Qgis::TextLayoutMode::RectangleAscentBased:
          xOffset += hAlignmentOffset;
          break;

        case Qgis::TextLayoutMode::Point:
          break;
      }
    }

    double yOffset = 0.0;
    switch ( mode )
    {
      case Qgis::TextLayoutMode::Labeling:
        if ( format.orientation() == Qgis::TextOrientation::RotationBased )
        {
          if ( rotation >= -405 && rotation < -180 )
          {
            yOffset = 0;
          }
          else if ( rotation >= 0 && rotation < 45 )
          {
            xOffset -= actualTextWidth;
            yOffset = -actualLabelHeight + metrics.blockMaximumDescent( blockIndex );
          }
        }
        else
        {
          yOffset = -actualLabelHeight;
        }
        break;

      case Qgis::TextLayoutMode::Point:
        yOffset = -actualLabelHeight;
        break;

      case Qgis::TextLayoutMode::Rectangle:
      case Qgis::TextLayoutMode::RectangleCapHeightBased:
      case Qgis::TextLayoutMode::RectangleAscentBased:
        yOffset = 0;
        break;
    }

    context.painter()->translate( QPointF( xOffset, yOffset ) );

    double currentBlockYOffset = 0;
    int fragmentIndex = 0;
    for ( const QgsTextFragment &fragment : block )
    {
      QgsScopedQPainterState fragmentPainterState( context.painter() );

      // apply some character replacement to draw symbols in vertical presentation
      const QString line = QgsStringUtils::substituteVerticalCharacters( fragment.text() );

      const QFont fragmentFont = metrics.fragmentFont( blockIndex, fragmentIndex );

      QFontMetricsF fragmentMetrics( fragmentFont );

      const double letterSpacing = fragmentFont.letterSpacing() / fontScale;
      const double labelHeight = fragmentMetrics.ascent() / fontScale + ( fragmentMetrics.ascent() / fontScale + letterSpacing ) * ( line.length() - 1 );

      Component subComponent;
      subComponent.block = QgsTextBlock( fragment );
      subComponent.blockIndex = blockIndex;
      subComponent.firstFragmentIndex = fragmentIndex;
      subComponent.size = QSizeF( blockMaximumCharacterWidth, labelHeight + fragmentMetrics.descent() / fontScale );
      subComponent.offset = QPointF( 0.0, currentBlockYOffset );
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

      if ( components & Qgis::TextComponent::Buffer )
      {
        currentBlockYOffset += QgsTextRenderer::drawBuffer( context, subComponent, format, metrics, mode );
      }
      if ( ( components & Qgis::TextComponent::Text ) || ( components & Qgis::TextComponent::Shadow ) )
      {
        // draw text, QPainterPath method
        QPainterPath path;
        path.setFillRule( Qt::WindingFill );
        const QStringList parts = QgsPalLabeling::splitToGraphemes( fragment.text() );
        double partYOffset = 0.0;
        for ( const QString &part : parts )
        {
          double partXOffset = ( blockMaximumCharacterWidth - ( fragmentMetrics.horizontalAdvance( part ) / fontScale - letterSpacing ) ) / 2;
          partYOffset += fragmentMetrics.ascent() / fontScale;
          path.addText( partXOffset * fontScale, partYOffset * fontScale, fragmentFont, part );
          partYOffset += letterSpacing;
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
          subComponent.origin = QPointF( 0.0, currentBlockYOffset );
          const double prevY = subComponent.offset.y();
          subComponent.offset = QPointF( 0, -subComponent.size.height() );
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

        // TODO -- this should respect the context's TextRenderFormat
        // draw outlined text
        context.painter()->translate( 0, currentBlockYOffset );
        QgsPainting::applyScaleFixForQPictureDpi( context.painter() );
        context.painter()->drawPicture( 0, 0, textPict );
        currentBlockYOffset += partYOffset;
      }
      fragmentIndex++;
    }

    if ( maskPainter )
      maskPainter->restore();
    blockIndex++;
  }
}

double QgsTextRenderer::calculateScaleFactorForFormat( const QgsRenderContext &context, const QgsTextFormat &format )
{
  if ( !( context.flags() & Qgis::RenderContextFlag::ApplyScalingWorkaroundForTextRendering ) )
    return 1.0;

  const double pixelSize = context.convertToPainterUnits( format.size(), format.sizeUnit(), format.sizeMapUnitScale() );

  // THESE THRESHOLDS MAY NEED TWEAKING!

  // NOLINTBEGIN(bugprone-branch-clone)

  // for small font sizes we need to apply a growth scaling workaround designed to stablise the rendering of small font sizes
  // we scale the painter up so that we render small text at 200 pixel size and let the painter scaling handle making it the correct size
  if ( pixelSize < 50 )
    return 200 / pixelSize;
  //... but for large font sizes we might run into https://bugreports.qt.io/browse/QTBUG-98778, which messes up the spacing between words for large fonts!
  // so instead we scale down the painter so that we render the text at 200 pixel size and let painter scaling handle making it the correct size
  else if ( pixelSize > 200 )
    return 200 / pixelSize;
  else
    return 1.0;

  // NOLINTEND(bugprone-branch-clone)
}

