/***************************************************************************
  qgstextdocumentmetrics.cpp
  -----------------
   begin                : September 2022
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
#include "qgstextdocumentmetrics.h"
#include "qgis.h"
#include "qgsstringutils.h"
#include "qgstextblock.h"
#include "qgstextfragment.h"
#include "qgstextformat.h"
#include "qgstextdocument.h"
#include "qgsrendercontext.h"

#include <QFontMetricsF>

// to match QTextEngine handling of superscript/subscript font sizes
constexpr double SUPERSCRIPT_SUBSCRIPT_FONT_SIZE_SCALING_FACTOR = 2.0 / 3.0;

// to match Qt behavior in QTextLine::draw
constexpr double SUPERSCRIPT_VERTICAL_BASELINE_ADJUSTMENT_FACTOR = 0.5;
constexpr double SUBSCRIPT_VERTICAL_BASELINE_ADJUSTMENT_FACTOR = 1.0 / 6.0;

QgsTextDocumentMetrics QgsTextDocumentMetrics::calculateMetrics( const QgsTextDocument &document, const QgsTextFormat &format, const QgsRenderContext &context, double scaleFactor )
{
  QgsTextDocumentMetrics res;

  const QFont font = format.scaledFont( context, scaleFactor, &res.mIsNullSize );
  if ( res.isNullFontSize() )
    return res;

  // for absolute line heights
  const double lineHeightPainterUnits = context.convertToPainterUnits( format.lineHeight(), format.lineHeightUnit() );

  double width = 0;
  double heightLabelMode = 0;
  double heightPointRectMode = 0;
  double heightCapHeightMode = 0;
  double heightAscentMode = 0;
  const int blockSize = document.size();
  res.mFragmentFonts.reserve( blockSize );
  double currentLabelBaseline = 0;
  double currentPointBaseline = 0;
  double currentRectBaseline = 0;
  double currentCapHeightBasedBaseline = 0;
  double currentAscentBasedBaseline = 0;
  double lastLineLeading = 0;

  double heightVerticalOrientation = 0;

  QVector < double > blockVerticalLineSpacing;

  double outerXMin = 0;
  double outerXMax = 0;
  double outerYMinLabel = 0;
  double outerYMaxLabel = 0;

  for ( int blockIndex = 0; blockIndex < blockSize; blockIndex++ )
  {
    const QgsTextBlock &block = document.at( blockIndex );

    double blockWidth = 0;
    double blockXMax = 0;
    double blockYMaxAdjustLabel = 0;

    double blockHeightUsingAscentDescent = 0;
    double blockHeightUsingLineSpacing = 0;
    double blockHeightVerticalOrientation = 0;

    double blockHeightUsingAscentAccountingForVerticalOffset = 0;

    const int fragmentSize = block.size();

    double maxBlockAscent = 0;
    double maxBlockDescent = 0;
    double maxLineSpacing = 0;
    double maxBlockLeading = 0;
    double maxBlockMaxWidth = 0;
    double maxBlockCapHeight = 0;

    QList< double > fragmentVerticalOffsets;
    fragmentVerticalOffsets.reserve( fragmentSize );

    QList< QFont > fragmentFonts;
    fragmentFonts.reserve( fragmentSize );
    QList< double >fragmentHorizontalAdvance;
    fragmentHorizontalAdvance.reserve( fragmentSize );

    QFont previousNonSuperSubScriptFont;

    for ( int fragmentIndex = 0; fragmentIndex < fragmentSize; ++fragmentIndex )
    {
      const QgsTextFragment &fragment = block.at( fragmentIndex );
      const QgsTextCharacterFormat &fragmentFormat = fragment.characterFormat();

      double fragmentHeightForVerticallyOffsetText = 0;
      double fragmentYMaxAdjust = 0;

      QFont updatedFont = font;
      fragmentFormat.updateFontForFormat( updatedFont, context, scaleFactor );

      QFontMetricsF fm( updatedFont );

      if ( fragmentIndex == 0 )
        previousNonSuperSubScriptFont = updatedFont;

      double fragmentVerticalOffset = 0;
      if ( fragmentFormat.hasVerticalAlignmentSet() )
      {
        switch ( fragmentFormat.verticalAlignment() )
        {
          case Qgis::TextCharacterVerticalAlignment::Normal:
            previousNonSuperSubScriptFont = updatedFont;
            break;

          case Qgis::TextCharacterVerticalAlignment::SuperScript:
          {
            const QFontMetricsF previousFM( previousNonSuperSubScriptFont );

            if ( fragmentFormat.fontPointSize() < 0 )
            {
              // if fragment has no explicit font size set, then we scale the inherited font size to 60% of base font size
              // this allows for easier use of super/subscript in labels as "my text<sup>2</sup>" will automatically render
              // the superscript in a smaller font size. BUT if the fragment format HAS a non -1 font size then it indicates
              // that the document has an explicit font size for the super/subscript element, eg "my text<sup style="font-size: 6pt">2</sup>"
              // which we should respect
              updatedFont.setPixelSize( static_cast< int >( std::round( updatedFont.pixelSize() * SUPERSCRIPT_SUBSCRIPT_FONT_SIZE_SCALING_FACTOR ) ) );
              fm = QFontMetricsF( updatedFont );
            }

            // to match Qt behavior in QTextLine::draw
            fragmentVerticalOffset = -( previousFM.ascent() + previousFM.descent() ) * SUPERSCRIPT_VERTICAL_BASELINE_ADJUSTMENT_FACTOR / scaleFactor;

            // note -- this should really be fm.ascent(), not fm.capHeight() -- but in practice the ascent of most fonts is too large
            // and causes unnecessarily large bounding boxes of vertically offset text!
            fragmentHeightForVerticallyOffsetText = -fragmentVerticalOffset + fm.capHeight() / scaleFactor;
            break;
          }

          case Qgis::TextCharacterVerticalAlignment::SubScript:
          {
            const QFontMetricsF previousFM( previousNonSuperSubScriptFont );

            if ( fragmentFormat.fontPointSize() < 0 )
            {
              // see above!!
              updatedFont.setPixelSize( static_cast< int>( std::round( updatedFont.pixelSize() * SUPERSCRIPT_SUBSCRIPT_FONT_SIZE_SCALING_FACTOR ) ) );
              fm = QFontMetricsF( updatedFont );
            }

            // to match Qt behavior in QTextLine::draw
            fragmentVerticalOffset = ( previousFM.ascent() + previousFM.descent() ) * SUBSCRIPT_VERTICAL_BASELINE_ADJUSTMENT_FACTOR / scaleFactor;

            fragmentYMaxAdjust = fragmentVerticalOffset + fm.descent() / scaleFactor;
            break;
          }
        }
      }
      else
      {
        previousNonSuperSubScriptFont = updatedFont;
      }
      fragmentVerticalOffsets << fragmentVerticalOffset;

      const double fragmentWidth = fm.horizontalAdvance( fragment.text() ) / scaleFactor;

      fragmentHorizontalAdvance << fragmentWidth;

      const double fragmentHeightUsingAscentDescent = ( fm.ascent() + fm.descent() ) / scaleFactor;
      const double fragmentHeightUsingLineSpacing = fm.lineSpacing() / scaleFactor;

      blockWidth += fragmentWidth;
      blockXMax += fragmentWidth;
      blockHeightUsingAscentDescent = std::max( blockHeightUsingAscentDescent, fragmentHeightUsingAscentDescent );

      blockHeightUsingLineSpacing = std::max( blockHeightUsingLineSpacing, fragmentHeightUsingLineSpacing );
      maxBlockAscent = std::max( maxBlockAscent, fm.ascent() / scaleFactor );

      maxBlockCapHeight = std::max( maxBlockCapHeight, fm.capHeight() / scaleFactor );

      blockHeightUsingAscentAccountingForVerticalOffset = std::max( std::max( maxBlockAscent, fragmentHeightForVerticallyOffsetText ), blockHeightUsingAscentAccountingForVerticalOffset );

      maxBlockDescent = std::max( maxBlockDescent, fm.descent() / scaleFactor );
      maxBlockMaxWidth = std::max( maxBlockMaxWidth, fm.maxWidth() / scaleFactor );

      blockYMaxAdjustLabel = std::max( blockYMaxAdjustLabel, fragmentYMaxAdjust );

      if ( ( fm.lineSpacing() / scaleFactor ) > maxLineSpacing )
      {
        maxLineSpacing = fm.lineSpacing() / scaleFactor;
        maxBlockLeading = fm.leading() / scaleFactor;
      }

      fragmentFonts << updatedFont;

      const double verticalOrientationFragmentHeight = fragmentIndex == 0 ? ( fm.ascent() / scaleFactor * fragment.text().size() + ( fragment.text().size() - 1 ) * updatedFont.letterSpacing() / scaleFactor )
          : ( fragment.text().size() * ( fm.ascent() / scaleFactor + updatedFont.letterSpacing() / scaleFactor ) );
      blockHeightVerticalOrientation += verticalOrientationFragmentHeight;
    }

    if ( blockIndex == 0 )
    {
      // same logic as used in QgsTextRenderer. (?!!)
      // needed to move bottom of text's descender to within bottom edge of label
      res.mFirstLineAscentOffset = 0.25 * maxBlockAscent; // descent() is not enough
      res.mLastLineAscentOffset = res.mFirstLineAscentOffset;
      res.mFirstLineCapHeight = maxBlockCapHeight;
      const double lineHeight = ( maxBlockAscent + maxBlockDescent ); // ignore +1 for baseline

      // rendering labels needs special handling - in this case text should be
      // drawn with the bottom left corner coinciding with origin, vs top left
      // for standard text rendering. Line height is also slightly different.
      currentLabelBaseline = -res.mFirstLineAscentOffset;

      if ( blockHeightUsingAscentAccountingForVerticalOffset > maxBlockAscent )
        outerYMinLabel = maxBlockAscent - blockHeightUsingAscentAccountingForVerticalOffset;

      // standard rendering - designed to exactly replicate QPainter's drawText method
      currentRectBaseline = -res.mFirstLineAscentOffset + lineHeight - 1 /*baseline*/;

      currentCapHeightBasedBaseline = res.mFirstLineCapHeight;
      currentAscentBasedBaseline = maxBlockAscent;

      // standard rendering - designed to exactly replicate QPainter's drawText rect method
      currentPointBaseline = 0;

      heightLabelMode += blockHeightUsingAscentDescent;
      heightPointRectMode += blockHeightUsingAscentDescent;
      heightCapHeightMode += maxBlockCapHeight;
      heightAscentMode += maxBlockAscent;
    }
    else
    {
      const double thisLineHeightUsingAscentDescent = format.lineHeightUnit() == QgsUnitTypes::RenderPercentage ? ( format.lineHeight() * ( maxBlockAscent + maxBlockDescent ) ) : lineHeightPainterUnits;
      const double thisLineHeightUsingLineSpacing = format.lineHeightUnit() == QgsUnitTypes::RenderPercentage ? ( format.lineHeight() * maxLineSpacing ) : lineHeightPainterUnits;

      currentLabelBaseline += thisLineHeightUsingAscentDescent;
      currentRectBaseline += thisLineHeightUsingLineSpacing;
      currentPointBaseline += thisLineHeightUsingLineSpacing;
      // using cap height??
      currentCapHeightBasedBaseline += thisLineHeightUsingLineSpacing;
      // using ascent?
      currentAscentBasedBaseline += thisLineHeightUsingLineSpacing;

      heightLabelMode += thisLineHeightUsingAscentDescent;
      heightPointRectMode += thisLineHeightUsingLineSpacing;
      heightCapHeightMode += thisLineHeightUsingLineSpacing;
      heightAscentMode += thisLineHeightUsingLineSpacing;
      if ( blockIndex == blockSize - 1 )
        res.mLastLineAscentOffset = 0.25 * maxBlockAscent;
    }

    if ( blockIndex == blockSize - 1 )
    {
      if ( blockYMaxAdjustLabel > maxBlockDescent )
        outerYMaxLabel = blockYMaxAdjustLabel - maxBlockDescent;
    }

    blockVerticalLineSpacing << ( format.lineHeightUnit() == QgsUnitTypes::RenderPercentage ? ( maxBlockMaxWidth * format.lineHeight() ) : lineHeightPainterUnits );

    res.mBlockHeights << blockHeightUsingLineSpacing;

    width = std::max( width, blockWidth );
    outerXMax = std::max( outerXMax, blockXMax );

    heightVerticalOrientation = std::max( heightVerticalOrientation, blockHeightVerticalOrientation );
    res.mBlockWidths << blockWidth;
    res.mFragmentFonts << fragmentFonts;
    res.mBaselineOffsetsLabelMode << currentLabelBaseline;
    res.mBaselineOffsetsPointMode << currentPointBaseline;
    res.mBaselineOffsetsRectMode << currentRectBaseline;
    res.mBaselineOffsetsCapHeightMode << currentCapHeightBasedBaseline;
    res.mBaselineOffsetsAscentBased << currentAscentBasedBaseline;
    res.mBlockMaxDescent << maxBlockDescent;
    res.mBlockMaxCharacterWidth << maxBlockMaxWidth;
    res.mFragmentVerticalOffsetsLabelMode << fragmentVerticalOffsets;
    res.mFragmentVerticalOffsetsRectMode << fragmentVerticalOffsets;
    res.mFragmentVerticalOffsetsPointMode << fragmentVerticalOffsets;
    res.mFragmentHorizontalAdvance << fragmentHorizontalAdvance;

    if ( blockIndex > 0 )
      lastLineLeading = maxBlockLeading;
  }

  heightLabelMode -= lastLineLeading;
  heightPointRectMode -= lastLineLeading;

  res.mDocumentSizeLabelMode = QSizeF( width, heightLabelMode );
  res.mDocumentSizePointRectMode = QSizeF( width, heightPointRectMode );
  res.mDocumentSizeCapHeightMode = QSizeF( width, heightCapHeightMode );
  res.mDocumentSizeAscentMode = QSizeF( width, heightAscentMode );

  // adjust baselines
  if ( !res.mBaselineOffsetsLabelMode.isEmpty() )
  {
    const double labelModeBaselineAdjust = res.mBaselineOffsetsLabelMode.constLast() + res.mLastLineAscentOffset;
    const double pointModeBaselineAdjust = res.mBaselineOffsetsPointMode.constLast();
    for ( int i = 0; i < blockSize; ++i )
    {
      res.mBaselineOffsetsLabelMode[i] -= labelModeBaselineAdjust;
      res.mBaselineOffsetsPointMode[i] -= pointModeBaselineAdjust;
    }
  }

  if ( !res.mBlockMaxCharacterWidth.isEmpty() )
  {
    QList< double > adjustedRightToLeftXOffsets;
    double currentOffset = 0;
    const int size = res.mBlockMaxCharacterWidth.size();

    double widthVerticalOrientation = 0;
    for ( int i = 0; i < size; ++i )
    {
      const double rightToLeftBlockMaxCharacterWidth = res.mBlockMaxCharacterWidth[size - 1 - i ];
      const double rightToLeftLineSpacing = blockVerticalLineSpacing[ size - 1 - i ];

      adjustedRightToLeftXOffsets << currentOffset;
      currentOffset += rightToLeftLineSpacing;

      if ( i == size - 1 )
        widthVerticalOrientation += rightToLeftBlockMaxCharacterWidth;
      else
        widthVerticalOrientation += rightToLeftLineSpacing;
    }
    std::reverse( adjustedRightToLeftXOffsets.begin(), adjustedRightToLeftXOffsets.end() );
    res.mVerticalOrientationXOffsets = adjustedRightToLeftXOffsets;

    res.mDocumentSizeVerticalOrientation = QSizeF( widthVerticalOrientation, heightVerticalOrientation );
  }

  res.mOuterBoundsLabelMode = QRectF( outerXMin, -outerYMaxLabel,
                                      outerXMax - outerXMin,
                                      heightLabelMode - outerYMinLabel + outerYMaxLabel );

  return res;
}

QSizeF QgsTextDocumentMetrics::documentSize( Qgis::TextLayoutMode mode, Qgis::TextOrientation orientation ) const
{
  switch ( orientation )
  {
    case Qgis::TextOrientation::Horizontal:
      switch ( mode )
      {
        case Qgis::TextLayoutMode::Rectangle:
        case Qgis::TextLayoutMode::Point:
          return mDocumentSizePointRectMode;

        case Qgis::TextLayoutMode::RectangleCapHeightBased:
          return mDocumentSizeCapHeightMode;

        case Qgis::TextLayoutMode::RectangleAscentBased:
          return mDocumentSizeAscentMode;

        case Qgis::TextLayoutMode::Labeling:
          return mDocumentSizeLabelMode;
      };
      BUILTIN_UNREACHABLE

    case Qgis::TextOrientation::Vertical:
      return mDocumentSizeVerticalOrientation;
    case Qgis::TextOrientation::RotationBased:
      return QSizeF(); // label mode only
  }

  BUILTIN_UNREACHABLE
}

QRectF QgsTextDocumentMetrics::outerBounds( Qgis::TextLayoutMode mode, Qgis::TextOrientation orientation ) const
{
  switch ( orientation )
  {
    case Qgis::TextOrientation::Horizontal:
      switch ( mode )
      {
        case Qgis::TextLayoutMode::Rectangle:
        case Qgis::TextLayoutMode::RectangleCapHeightBased:
        case Qgis::TextLayoutMode::RectangleAscentBased:
        case Qgis::TextLayoutMode::Point:
          return QRectF();

        case Qgis::TextLayoutMode::Labeling:
          return mOuterBoundsLabelMode;
      };
      BUILTIN_UNREACHABLE

    case Qgis::TextOrientation::Vertical:
    case Qgis::TextOrientation::RotationBased:
      return QRectF(); // label mode only
  }

  BUILTIN_UNREACHABLE
}

double QgsTextDocumentMetrics::blockWidth( int blockIndex ) const
{
  return mBlockWidths.value( blockIndex );
}

double QgsTextDocumentMetrics::blockHeight( int blockIndex ) const
{
  return mBlockHeights.value( blockIndex );
}

double QgsTextDocumentMetrics::firstLineCapHeight() const
{
  return mFirstLineCapHeight;
}

double QgsTextDocumentMetrics::baselineOffset( int blockIndex, Qgis::TextLayoutMode mode ) const
{
  switch ( mode )
  {
    case Qgis::TextLayoutMode::Rectangle:
      return mBaselineOffsetsRectMode.value( blockIndex );
    case Qgis::TextLayoutMode::RectangleCapHeightBased:
      return mBaselineOffsetsCapHeightMode.value( blockIndex );
    case Qgis::TextLayoutMode::RectangleAscentBased:
      return mBaselineOffsetsAscentBased.value( blockIndex );
    case Qgis::TextLayoutMode::Point:
      return mBaselineOffsetsPointMode.value( blockIndex );
    case Qgis::TextLayoutMode::Labeling:
      return mBaselineOffsetsLabelMode.value( blockIndex );
  }
  BUILTIN_UNREACHABLE
}

double QgsTextDocumentMetrics::fragmentHorizontalAdvance( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode ) const
{
  return mFragmentHorizontalAdvance.value( blockIndex ).value( fragmentIndex );
}

double QgsTextDocumentMetrics::fragmentVerticalOffset( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode mode ) const
{
  switch ( mode )
  {
    case Qgis::TextLayoutMode::Rectangle:
    case Qgis::TextLayoutMode::RectangleCapHeightBased:
    case Qgis::TextLayoutMode::RectangleAscentBased:
      return mFragmentVerticalOffsetsRectMode.value( blockIndex ).value( fragmentIndex );
    case Qgis::TextLayoutMode::Point:
      return mFragmentVerticalOffsetsPointMode.value( blockIndex ).value( fragmentIndex );
    case Qgis::TextLayoutMode::Labeling:
      return mFragmentVerticalOffsetsLabelMode.value( blockIndex ).value( fragmentIndex );
  }
  BUILTIN_UNREACHABLE
}

double QgsTextDocumentMetrics::verticalOrientationXOffset( int blockIndex ) const
{
  return mVerticalOrientationXOffsets.value( blockIndex );
}

double QgsTextDocumentMetrics::blockMaximumCharacterWidth( int blockIndex ) const
{
  return mBlockMaxCharacterWidth.value( blockIndex );
}

double QgsTextDocumentMetrics::blockMaximumDescent( int blockIndex ) const
{
  return mBlockMaxDescent.value( blockIndex );
}

QFont QgsTextDocumentMetrics::fragmentFont( int blockIndex, int fragmentIndex ) const
{
  return mFragmentFonts.value( blockIndex ).value( fragmentIndex );
}

