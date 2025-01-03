/***************************************************************************
    qgstextlabelfeature.cpp
    ---------------------
    begin                : December 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstextlabelfeature.h"

#include "qgspallabeling.h"
#include "qgsmaptopixel.h"
#include "qgstextcharacterformat.h"
#include "qgstextfragment.h"
#include "qgstextblock.h"
#include "qgstextrenderer.h"
#include "qgsrendercontext.h"

QgsTextLabelFeature::QgsTextLabelFeature( QgsFeatureId id, geos::unique_ptr geometry, QSizeF size )
  : QgsLabelFeature( id, std::move( geometry ), size )
{
  mDefinedFont = QFont();
}

QgsTextLabelFeature::~QgsTextLabelFeature() = default;

QString QgsTextLabelFeature::text( int partId ) const
{
  if ( partId == -1 )
    return mLabelText;
  else
    return mTextMetrics->grapheme( partId );
}

QgsTextCharacterFormat QgsTextLabelFeature::characterFormat( int partId ) const
{
  return mTextMetrics.has_value() ? mTextMetrics->graphemeFormat( partId ) : QgsTextCharacterFormat();
}

bool QgsTextLabelFeature::hasCharacterFormat( int partId ) const
{
  return mTextMetrics.has_value() && partId < mTextMetrics->graphemeFormatCount();
}

QgsPrecalculatedTextMetrics QgsTextLabelFeature::calculateTextMetrics( const QgsMapToPixel *xform, const QgsRenderContext &context, const QgsTextFormat &format, const QFont &baseFont, const QFontMetricsF &fontMetrics, double letterSpacing, double wordSpacing, const QString &text, QgsTextDocument *document, QgsTextDocumentMetrics * )
{
  const double tabStopDistancePainterUnits = format.tabStopDistanceUnit() == Qgis::RenderUnit::Percentage
      ? format.tabStopDistance() * baseFont.pixelSize()
      : context.convertToPainterUnits( format.tabStopDistance(), format.tabStopDistanceUnit(), format.tabStopDistanceMapUnitScale() );

  const QList< QgsTextFormat::Tab > tabPositions = format.tabPositions();
  QList< double > tabStopDistancesPainterUnits;
  tabStopDistancesPainterUnits.reserve( tabPositions.size() );
  for ( const QgsTextFormat::Tab &tab : tabPositions )
  {
    tabStopDistancesPainterUnits.append(
      format.tabStopDistanceUnit() == Qgis::RenderUnit::Percentage
      ? tab.position() * baseFont.pixelSize()
      : context.convertToPainterUnits( tab.position(), format.tabStopDistanceUnit(), format.tabStopDistanceMapUnitScale() )
    );
  }


  // create label info!
  const double mapScale = xform->mapUnitsPerPixel();
  QStringList graphemes;
  QVector< QgsTextCharacterFormat > graphemeFormats;

  if ( document )
  {
    for ( const QgsTextBlock &block : std::as_const( *document ) )
    {
      for ( const QgsTextFragment &fragment : block )
      {
        const QStringList fragmentGraphemes = QgsPalLabeling::splitToGraphemes( fragment.text() );
        for ( const QString &grapheme : fragmentGraphemes )
        {
          graphemes.append( grapheme );
          graphemeFormats.append( fragment.characterFormat() );
        }
      }
    }
  }
  else
  {
    //split string by valid grapheme boundaries - required for certain scripts (see #6883)
    graphemes = QgsPalLabeling::splitToGraphemes( text );
  }

  QVector< double > characterWidths( graphemes.count() );
  QVector< double > characterHeights( graphemes.count() );
  QVector< double > characterDescents( graphemes.count() );

  QFont previousNonSuperSubScriptFont;

  double currentWidth = 0;
  for ( int i = 0; i < graphemes.count(); i++ )
  {
    // reconstruct how Qt creates word spacing, then adjust per individual stored character
    // this will allow PAL to create each candidate width = character width + correct spacing

    double graphemeFirstCharHorizontalAdvanceWithLetterSpacing = 0;
    double graphemeFirstCharHorizontalAdvance = 0;
    double graphemeHorizontalAdvance = 0;
    double characterDescent = 0;
    double characterHeight = 0;
    if ( const QgsTextCharacterFormat *graphemeFormat = !graphemeFormats.empty() ? &graphemeFormats[i] : nullptr )
    {
      QFont graphemeFont = baseFont;
      graphemeFormat->updateFontForFormat( graphemeFont, context, 1 );

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
              graphemeFont.setPixelSize( static_cast< int >( std::round( graphemeFont.pixelSize() * QgsTextRenderer::SUPERSCRIPT_SUBSCRIPT_FONT_SIZE_SCALING_FACTOR ) ) );
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
      graphemeFirstCharHorizontalAdvance = graphemeFontMetrics.horizontalAdvance( QString( graphemes[i].at( 0 ) ) );
      graphemeFirstCharHorizontalAdvanceWithLetterSpacing = graphemeFontMetrics.horizontalAdvance( graphemes[i].at( 0 ) ) + letterSpacing;
      graphemeHorizontalAdvance = graphemeFontMetrics.horizontalAdvance( QString( graphemes[i] ) );
      characterDescent = graphemeFontMetrics.descent();
      characterHeight = graphemeFontMetrics.height();
    }
    else if ( graphemes[i] == '\t' )
    {
      double nextTabStop = 0;
      if ( !tabStopDistancesPainterUnits.empty() )
      {
        // if we don't find a tab stop before the current length of line, we just ignore the tab character entirely
        nextTabStop = currentWidth;
        for ( const double tabStop : std::as_const( tabStopDistancesPainterUnits ) )
        {
          if ( tabStop >= currentWidth )
          {
            nextTabStop = tabStop;
            break;
          }
        }
      }
      else
      {
        nextTabStop = ( std::floor( currentWidth / tabStopDistancePainterUnits ) + 1 ) * tabStopDistancePainterUnits;
      }

      const double thisTabWidth = nextTabStop - currentWidth;

      graphemeFirstCharHorizontalAdvance = thisTabWidth;
      graphemeFirstCharHorizontalAdvanceWithLetterSpacing = thisTabWidth;
      graphemeHorizontalAdvance = thisTabWidth;
      characterDescent = fontMetrics.descent();
      characterHeight = fontMetrics.height();
    }
    else
    {
      graphemeFirstCharHorizontalAdvance = fontMetrics.horizontalAdvance( QString( graphemes[i].at( 0 ) ) );
      graphemeFirstCharHorizontalAdvanceWithLetterSpacing = fontMetrics.horizontalAdvance( graphemes[i].at( 0 ) ) + letterSpacing;
      graphemeHorizontalAdvance = fontMetrics.horizontalAdvance( QString( graphemes[i] ) );
      characterDescent = fontMetrics.descent();
      characterHeight = fontMetrics.height();
    }

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
    characterWidths[i] = mapScale * charWidth;
    characterHeights[i] = mapScale * characterHeight;
    characterDescents[i] = mapScale * characterDescent;

    currentWidth += charWidth;
  }

  QgsPrecalculatedTextMetrics res( graphemes, std::move( characterWidths ), std::move( characterHeights ), std::move( characterDescents ) );
  res.setGraphemeFormats( graphemeFormats );
  return res;
}

void QgsTextLabelFeature::setDocument( const QgsTextDocument &document, const QgsTextDocumentMetrics &metrics )
{
  mDocument = document;
  mDocumentMetrics = metrics;
}
