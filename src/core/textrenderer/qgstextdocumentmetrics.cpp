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
#include "qgstextrenderer.h"
#include "qgsapplication.h"
#include "qgsimagecache.h"

#include <QFontMetricsF>

// to match Qt behavior in QTextLine::draw
constexpr double SUPERSCRIPT_VERTICAL_BASELINE_ADJUSTMENT_FACTOR = 0.5;
constexpr double SUBSCRIPT_VERTICAL_BASELINE_ADJUSTMENT_FACTOR = 1.0 / 6.0;

struct DocumentMetrics
{
  double tabStopDistancePainterUnits = 0;
  QList< double > tabStopDistancesPainterUnits;
  double width = 0;
  double heightLabelMode = 0;
  double heightPointRectMode = 0;
  double heightCapHeightMode = 0;
  double heightAscentMode = 0;
  int blockSize = 0;
  double currentLabelBaseline = 0;
  double currentPointBaseline = 0;
  double currentRectBaseline = 0;
  double currentCapHeightBasedBaseline = 0;
  double currentAscentBasedBaseline = 0;
  double lastLineLeading = 0;

  double heightVerticalOrientation = 0;

  QVector < double > blockVerticalLineSpacing;

  /**
   * Calculated vertical margins between blocks. The first entry is the
   * margin before the first block, and the last entry is the margin after
   * the final block
   */
  QVector< double > verticalMarginsBetweenBlocks;

  QVector< double > blockLeftMargin;
  QVector< double > blockRightMargin;

  double outerXMin = 0;
  double outerXMax = 0;
  double outerYMinLabel = 0;
  double outerYMaxLabel = 0;
};

struct BlockMetrics
{
  bool isFirstBlock = false;
  bool isLastBlock = false;
  double maxLineSpacing = 0;
  double blockWidth = 0;
  double blockXMax = 0;
  double blockYMaxAdjustLabel = 0;
  double blockHeightUsingAscentAccountingForVerticalOffset = 0;
  double blockHeightVerticalOrientation = 0;
  double blockHeightUsingAscentDescent = 0;
  double blockHeightUsingLineSpacing = 0;
  double maxBlockFixedItemHeight = 0;
  double maxBlockAscentForTextFragments = 0;
  double maxBlockCapHeight = 0;
  double maxBlockAscent = 0;
  double maxBlockDescent = 0;
  double maxBlockMaxWidth = 0;
  double maxBlockLeading = 0;

  QList< QFont > fragmentFonts;
  QList< double > fragmentVerticalOffsets;
  QList< double > fragmentFixedHeights;
  QList< double > fragmentHorizontalAdvance;
  QList< double > fragmentAscent;
  QList< double > fragmentDescent;

  QFont previousNonSuperSubScriptFont;
  bool isFirstNonTabFragment = true;

  // non calculated properties
  double lineHeightPainterUnits = 0;
  double lineHeightPercentage = 0;

  double marginTop = 0;
  double marginBottom = 0;
  double marginLeft = 0;
  double marginRight = 0;

  void resetCalculatedStats()
  {
    isFirstBlock = false;
    isLastBlock = false;
    maxLineSpacing = 0;
    blockWidth = 0;
    blockXMax = 0;
    blockYMaxAdjustLabel = 0;
    blockHeightUsingAscentAccountingForVerticalOffset = 0;
    blockHeightVerticalOrientation = 0;
    blockHeightUsingAscentDescent = 0;
    blockHeightUsingLineSpacing = 0;
    maxBlockFixedItemHeight = 0;
    maxBlockAscentForTextFragments = 0;
    maxBlockCapHeight = 0;
    maxBlockAscent = 0;
    maxBlockDescent = 0;
    maxBlockMaxWidth = 0;
    maxBlockLeading = 0;
    marginTop = 0;
    marginBottom = 0;

    fragmentFonts.clear();
    fragmentVerticalOffsets.clear();
    fragmentFixedHeights.clear();
    fragmentHorizontalAdvance.clear();
    fragmentAscent.clear();
    fragmentDescent.clear();
    previousNonSuperSubScriptFont = QFont();
    isFirstNonTabFragment = true;
  }
};


void QgsTextDocumentMetrics::finalizeBlock( QgsTextDocumentMetrics &res, const QgsTextFormat &, DocumentMetrics &documentMetrics, QgsTextBlock &outputBlock, BlockMetrics &metrics )
{
  if ( metrics.isFirstBlock )
  {
    documentMetrics.verticalMarginsBetweenBlocks.append( metrics.marginTop );
    documentMetrics.verticalMarginsBetweenBlocks.append( metrics.marginBottom );

    documentMetrics.currentLabelBaseline += metrics.marginTop;
    documentMetrics.currentRectBaseline += metrics.marginTop;
    documentMetrics.currentPointBaseline += metrics.marginTop;
    documentMetrics.currentCapHeightBasedBaseline += metrics.marginTop;
    documentMetrics.currentAscentBasedBaseline += metrics.marginTop;

    // same logic as used in QgsTextRenderer. (?!!)
    // needed to move bottom of text's descender to within bottom edge of label
    res.mFirstLineAscentOffset = 0.25 * metrics.maxBlockAscentForTextFragments; // descent() is not enough
    res.mLastLineAscentOffset = res.mFirstLineAscentOffset;
    res.mFirstLineCapHeight = metrics.maxBlockCapHeight;
    const double lineHeight = ( metrics.maxBlockAscent + metrics.maxBlockDescent ); // ignore +1 for baseline

    // rendering labels needs special handling - in this case text should be
    // drawn with the bottom left corner coinciding with origin, vs top left
    // for standard text rendering. Line height is also slightly different.
    documentMetrics.currentLabelBaseline = -res.mFirstLineAscentOffset;

    if ( metrics.blockHeightUsingAscentAccountingForVerticalOffset > metrics.maxBlockAscent )
      documentMetrics.outerYMinLabel = metrics.maxBlockAscent - metrics.blockHeightUsingAscentAccountingForVerticalOffset;

    // standard rendering - designed to exactly replicate QPainter's drawText method
    documentMetrics.currentRectBaseline = -res.mFirstLineAscentOffset + lineHeight - 1 /*baseline*/;

    documentMetrics.currentCapHeightBasedBaseline += res.mFirstLineCapHeight;
    documentMetrics.currentAscentBasedBaseline += metrics.maxBlockAscent;

    // standard rendering - designed to exactly replicate QPainter's drawText rect method
    documentMetrics.currentPointBaseline = 0;

    documentMetrics.heightLabelMode += metrics.blockHeightUsingAscentDescent + metrics.marginTop;
    documentMetrics.heightPointRectMode += metrics.blockHeightUsingAscentDescent + metrics.marginTop;
    documentMetrics.heightCapHeightMode += metrics.maxBlockCapHeight + metrics.marginTop;
    documentMetrics.heightAscentMode += metrics.maxBlockAscent + metrics.marginTop;
  }
  else
  {
    // html vertical margins between blocks collapse and take the size of the highest margin:
    const double verticalMarginBeforeBlock = std::max( documentMetrics.verticalMarginsBetweenBlocks.last(), metrics.marginTop );
    documentMetrics.verticalMarginsBetweenBlocks.last() = verticalMarginBeforeBlock;
    documentMetrics.verticalMarginsBetweenBlocks.append( metrics.marginBottom );

    double thisLineHeightUsingAscentDescent = metrics.lineHeightPercentage != 0 ? ( metrics.lineHeightPercentage * ( metrics.maxBlockAscent + metrics.maxBlockDescent ) ) : metrics.lineHeightPainterUnits;
    double thisLineHeightUsingLineSpacing = metrics.lineHeightPercentage != 0 ? ( metrics.lineHeightPercentage * metrics.maxLineSpacing ) : metrics.lineHeightPainterUnits;

    thisLineHeightUsingAscentDescent = std::max( thisLineHeightUsingAscentDescent, metrics.maxBlockFixedItemHeight );
    thisLineHeightUsingLineSpacing = std::max( thisLineHeightUsingLineSpacing, metrics.maxBlockFixedItemHeight );

    documentMetrics.currentLabelBaseline += verticalMarginBeforeBlock + thisLineHeightUsingAscentDescent;
    documentMetrics.currentRectBaseline += verticalMarginBeforeBlock + thisLineHeightUsingLineSpacing;
    documentMetrics.currentPointBaseline += verticalMarginBeforeBlock + thisLineHeightUsingLineSpacing;
    // using cap height??
    documentMetrics.currentCapHeightBasedBaseline += verticalMarginBeforeBlock + thisLineHeightUsingLineSpacing;
    // using ascent?
    documentMetrics.currentAscentBasedBaseline += verticalMarginBeforeBlock + thisLineHeightUsingLineSpacing;

    documentMetrics.heightLabelMode += verticalMarginBeforeBlock + thisLineHeightUsingAscentDescent;
    documentMetrics.heightPointRectMode += verticalMarginBeforeBlock + thisLineHeightUsingLineSpacing;
    documentMetrics.heightCapHeightMode += verticalMarginBeforeBlock + thisLineHeightUsingLineSpacing;
    documentMetrics.heightAscentMode += verticalMarginBeforeBlock + thisLineHeightUsingLineSpacing;
    if ( metrics.isLastBlock )
    {
      res.mLastLineAscentOffset = 0.25 * metrics.maxBlockAscentForTextFragments;
      documentMetrics.heightLabelMode += metrics.marginBottom;
      documentMetrics.heightPointRectMode += metrics.marginBottom;
      documentMetrics.heightCapHeightMode += metrics.marginBottom;
      documentMetrics.heightAscentMode += metrics.marginBottom;
    }
  }

  documentMetrics.blockLeftMargin << metrics.marginLeft;
  documentMetrics.blockRightMargin << metrics.marginRight;

  if ( metrics.isLastBlock )
  {
    if ( metrics.blockYMaxAdjustLabel > metrics.maxBlockDescent )
      documentMetrics.outerYMaxLabel = metrics.blockYMaxAdjustLabel - metrics.maxBlockDescent;
  }

  documentMetrics.blockVerticalLineSpacing << ( metrics.lineHeightPercentage != 0 ? ( metrics.maxBlockMaxWidth * metrics.lineHeightPercentage ) : metrics.lineHeightPainterUnits );

  res.mBlockHeights << metrics.blockHeightUsingLineSpacing;

  documentMetrics.width = std::max( documentMetrics.width, metrics.blockWidth + metrics.marginLeft + metrics.marginRight );
  documentMetrics.outerXMax = std::max( documentMetrics.outerXMax, metrics.blockXMax );

  documentMetrics.heightVerticalOrientation = std::max( documentMetrics.heightVerticalOrientation, metrics.blockHeightVerticalOrientation );
  res.mBlockWidths << metrics.blockWidth;
  res.mFragmentFonts << metrics.fragmentFonts;
  res.mBaselineOffsetsLabelMode << documentMetrics.currentLabelBaseline;
  res.mBaselineOffsetsPointMode << documentMetrics.currentPointBaseline;
  res.mBaselineOffsetsRectMode << documentMetrics.currentRectBaseline;
  res.mBaselineOffsetsCapHeightMode << documentMetrics.currentCapHeightBasedBaseline;
  res.mBaselineOffsetsAscentBased << documentMetrics.currentAscentBasedBaseline;
  res.mBlockMaxDescent << metrics.maxBlockDescent;
  res.mBlockMaxAscent << metrics.maxBlockAscent;
  res.mBlockMaxCharacterWidth << metrics.maxBlockMaxWidth;
  res.mFragmentVerticalOffsetsLabelMode << metrics.fragmentVerticalOffsets;
  res.mFragmentFixedHeights << metrics.fragmentFixedHeights;
  res.mFragmentVerticalOffsetsRectMode << metrics.fragmentVerticalOffsets;
  res.mFragmentVerticalOffsetsPointMode << metrics.fragmentVerticalOffsets;
  res.mFragmentHorizontalAdvance << metrics.fragmentHorizontalAdvance;
  res.mFragmentAscent << metrics.fragmentAscent;
  res.mFragmentDescent << metrics.fragmentDescent;

  res.mDocument.append( outputBlock );
  outputBlock.clear();

  if ( !metrics.isFirstBlock )
    documentMetrics.lastLineLeading = metrics.maxBlockLeading;

  // reset metrics for next block
  metrics.resetCalculatedStats();
};


void QgsTextDocumentMetrics::processFragment( QgsTextDocumentMetrics &res, const QgsTextFormat &format, const QgsRenderContext &context, const QgsTextDocumentRenderContext &documentContext, double scaleFactor, DocumentMetrics &documentMetrics, BlockMetrics &thisBlockMetrics, const QFont &font, const QgsTextFragment &fragment, QgsTextBlock &currentOutputBlock )
{
  if ( fragment.isTab() )
  {
    // special handling for tab characters
    double nextTabStop = 0;
    if ( !documentMetrics.tabStopDistancesPainterUnits.isEmpty() )
    {
      // if we don't find a tab stop before the current length of line, we just ignore the tab character entirely
      nextTabStop = thisBlockMetrics.blockXMax;
      for ( const double tabStop : std::as_const( documentMetrics.tabStopDistancesPainterUnits ) )
      {
        if ( tabStop >= thisBlockMetrics.blockXMax )
        {
          nextTabStop = tabStop;
          break;
        }
      }
    }
    else
    {
      nextTabStop = ( std::floor( thisBlockMetrics.blockXMax / documentMetrics.tabStopDistancePainterUnits ) + 1 ) * documentMetrics.tabStopDistancePainterUnits;
    }
    const double fragmentWidth = nextTabStop - thisBlockMetrics.blockXMax;

    thisBlockMetrics.blockWidth += fragmentWidth;
    thisBlockMetrics.blockXMax += fragmentWidth;

    thisBlockMetrics.fragmentVerticalOffsets << 0;
    thisBlockMetrics.fragmentHorizontalAdvance << fragmentWidth;
    thisBlockMetrics.fragmentFixedHeights << -1;
    thisBlockMetrics.fragmentFonts << QFont();
    thisBlockMetrics.fragmentAscent << 0;
    thisBlockMetrics.fragmentDescent << 0;
    currentOutputBlock.append( fragment );
  }
  else
  {
    const QgsTextCharacterFormat &fragmentFormat = fragment.characterFormat();

    double fragmentHeightForVerticallyOffsetText = 0;
    double fragmentYMaxAdjust = 0;

    QFont updatedFont = font;
    fragmentFormat.updateFontForFormat( updatedFont, context, scaleFactor );

    QFontMetricsF fm( updatedFont );

    // first, just do what we need to calculate the fragment width. We need this upfront to determine if we need to split this fragment up into a new block
    // in order to respect text wrapping
    if ( thisBlockMetrics.isFirstNonTabFragment )
      thisBlockMetrics.previousNonSuperSubScriptFont = updatedFont;

    double fragmentVerticalOffset = 0;
    if ( fragmentFormat.hasVerticalAlignmentSet() )
    {
      switch ( fragmentFormat.verticalAlignment() )
      {
        case Qgis::TextCharacterVerticalAlignment::Normal:
          thisBlockMetrics.previousNonSuperSubScriptFont = updatedFont;
          break;

        case Qgis::TextCharacterVerticalAlignment::SuperScript:
        {
          const QFontMetricsF previousFM( thisBlockMetrics.previousNonSuperSubScriptFont );

          if ( fragmentFormat.fontPointSize() < 0 )
          {
            // if fragment has no explicit font size set, then we scale the inherited font size to 60% of base font size
            // this allows for easier use of super/subscript in labels as "my text<sup>2</sup>" will automatically render
            // the superscript in a smaller font size. BUT if the fragment format HAS a non -1 font size then it indicates
            // that the document has an explicit font size for the super/subscript element, eg "my text<sup style="font-size: 6pt">2</sup>"
            // which we should respect
            updatedFont.setPixelSize( static_cast< int >( std::round( updatedFont.pixelSize() * QgsTextRenderer::SUPERSCRIPT_SUBSCRIPT_FONT_SIZE_SCALING_FACTOR ) ) );
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
          const QFontMetricsF previousFM( thisBlockMetrics.previousNonSuperSubScriptFont );

          if ( fragmentFormat.fontPointSize() < 0 )
          {
            // see above!!
            updatedFont.setPixelSize( static_cast< int>( std::round( updatedFont.pixelSize() * QgsTextRenderer::SUPERSCRIPT_SUBSCRIPT_FONT_SIZE_SCALING_FACTOR ) ) );
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
      thisBlockMetrics.previousNonSuperSubScriptFont = updatedFont;
    }

    auto updateCommonBlockMetrics = [ &fragmentVerticalOffset,
                                      &fragmentYMaxAdjust,
                                      &fragmentHeightForVerticallyOffsetText,
                                      &updatedFont,
                                      &fm,
                                      scaleFactor]( BlockMetrics & thisBlockMetrics, double fragmentWidth, const QgsTextFragment & fragment )
    {
      thisBlockMetrics.fragmentVerticalOffsets << fragmentVerticalOffset;
      thisBlockMetrics.blockYMaxAdjustLabel = std::max( thisBlockMetrics.blockYMaxAdjustLabel, fragmentYMaxAdjust );
      thisBlockMetrics.blockHeightUsingAscentAccountingForVerticalOffset = std::max( std::max( thisBlockMetrics.maxBlockAscent, fragmentHeightForVerticallyOffsetText ), thisBlockMetrics.blockHeightUsingAscentAccountingForVerticalOffset );

      thisBlockMetrics.fragmentHorizontalAdvance << fragmentWidth;

      thisBlockMetrics.blockWidth += fragmentWidth;
      thisBlockMetrics.blockXMax += fragmentWidth;

      thisBlockMetrics.fragmentFonts << updatedFont;

      const double verticalOrientationFragmentHeight = thisBlockMetrics.isFirstNonTabFragment ? ( fm.ascent() / scaleFactor * fragment.text().size() + ( fragment.text().size() - 1 ) * updatedFont.letterSpacing() / scaleFactor )
          : ( fragment.text().size() * ( fm.ascent() / scaleFactor + updatedFont.letterSpacing() / scaleFactor ) );
      thisBlockMetrics.blockHeightVerticalOrientation += verticalOrientationFragmentHeight;

      thisBlockMetrics.isFirstNonTabFragment = false;
    };

    // calculate width of fragment
    if ( fragment.isImage() )
    {
      double imageHeight = 0;
      double imageWidth = 0;
      if ( ( qgsDoubleNear( fragmentFormat.imageSize().width(), 0 ) || fragmentFormat.imageSize().width() < 0 )
           && ( qgsDoubleNear( fragmentFormat.imageSize().height(), 0 ) || fragmentFormat.imageSize().height() < 0 ) )
      {
        // use original image size
        const QSize imageSize = QgsApplication::imageCache()->originalSize( fragmentFormat.imagePath(), context.flags() & Qgis::RenderContextFlag::RenderBlocking );
        // TODO: maybe there's more optimal logic we could use here, but for now we assume 96dpi image resolution...
        const QSizeF originalSizeMmAt96Dpi = imageSize / 3.7795275590551185;
        const double pixelsPerMm = context.scaleFactor();
        imageWidth = originalSizeMmAt96Dpi.width() * pixelsPerMm;
        imageHeight = originalSizeMmAt96Dpi.height() * pixelsPerMm;
      }
      else if ( ( qgsDoubleNear( fragmentFormat.imageSize().width(), 0 ) || fragmentFormat.imageSize().width() < 0 ) )
      {
        // height specified, calculate width
        const QSize originalImageSize = QgsApplication::imageCache()->originalSize( fragmentFormat.imagePath(), context.flags() & Qgis::RenderContextFlag::RenderBlocking );
        imageHeight = context.convertToPainterUnits( fragmentFormat.imageSize().height(), Qgis::RenderUnit::Points );
        imageWidth = originalImageSize.width() * imageHeight / originalImageSize.height();
      }
      else if ( ( qgsDoubleNear( fragmentFormat.imageSize().height(), 0 ) || fragmentFormat.imageSize().height() < 0 ) )
      {
        // width specified, calculate height
        const QSize originalImageSize = QgsApplication::imageCache()->originalSize( fragmentFormat.imagePath(), context.flags() & Qgis::RenderContextFlag::RenderBlocking );
        imageWidth = context.convertToPainterUnits( fragmentFormat.imageSize().width(), Qgis::RenderUnit::Points );
        imageHeight = originalImageSize.height() * imageWidth / originalImageSize.width();
      }
      else
      {
        imageWidth = context.convertToPainterUnits( fragmentFormat.imageSize().width(), Qgis::RenderUnit::Points );
        imageHeight = context.convertToPainterUnits( fragmentFormat.imageSize().height(), Qgis::RenderUnit::Points );
      }

      // do we need to move this image fragment to a new block to respect wrapping?
      if ( documentContext.flags() & Qgis::TextRendererFlag::WrapLines && documentContext.maximumWidth() > 0
           && ( thisBlockMetrics.blockXMax + imageWidth > documentContext.maximumWidth() )
           && !currentOutputBlock.empty() )
      {
        // yep, need to wrap before the image
        finalizeBlock( res, format, documentMetrics, currentOutputBlock, thisBlockMetrics );
        thisBlockMetrics.isFirstBlock = false;
      }

      // we consider the whole image as ascent, and descent as 0
      thisBlockMetrics.blockHeightUsingAscentDescent = std::max( thisBlockMetrics.blockHeightUsingAscentDescent, imageHeight + fm.descent() / scaleFactor );
      thisBlockMetrics.blockHeightUsingLineSpacing = std::max( thisBlockMetrics.blockHeightUsingLineSpacing, imageHeight + fm.leading() );

      thisBlockMetrics.maxBlockAscent = std::max( thisBlockMetrics.maxBlockAscent, imageHeight );
      thisBlockMetrics.maxBlockCapHeight = std::max( thisBlockMetrics.maxBlockCapHeight, imageHeight );
      thisBlockMetrics.fragmentAscent << imageHeight;
      thisBlockMetrics.fragmentDescent << 0;
      thisBlockMetrics.maxLineSpacing = std::max( thisBlockMetrics.maxLineSpacing, imageHeight + fm.leading() / scaleFactor );
      thisBlockMetrics.maxBlockLeading = std::max( thisBlockMetrics.maxBlockLeading, fm.leading() / scaleFactor );
      thisBlockMetrics.maxBlockMaxWidth = std::max( thisBlockMetrics.maxBlockMaxWidth, imageWidth );
      thisBlockMetrics.maxBlockFixedItemHeight = std::max( thisBlockMetrics.maxBlockFixedItemHeight, imageHeight );
      thisBlockMetrics.fragmentFixedHeights << imageHeight;
      updateCommonBlockMetrics( thisBlockMetrics, imageWidth, fragment );
      currentOutputBlock.append( fragment );
    }
    else
    {
      const double fragmentHeightUsingAscentDescent = ( fm.ascent() + fm.descent() ) / scaleFactor;
      const double fragmentHeightUsingLineSpacing = fm.lineSpacing() / scaleFactor;

      auto finalizeTextFragment = [fragmentHeightUsingAscentDescent,
                                   fragmentHeightUsingLineSpacing,
                                   &fm,
                                   scaleFactor,
                                   &currentOutputBlock,
                                   &updateCommonBlockMetrics
                                  ]( BlockMetrics & thisBlockMetrics, const QgsTextFragment & fragment, double fragmentWidth )
      {
        thisBlockMetrics.blockHeightUsingAscentDescent = std::max( thisBlockMetrics.blockHeightUsingAscentDescent, fragmentHeightUsingAscentDescent );

        thisBlockMetrics.blockHeightUsingLineSpacing = std::max( thisBlockMetrics.blockHeightUsingLineSpacing, fragmentHeightUsingLineSpacing );
        const double ascent = fm.ascent() / scaleFactor;
        thisBlockMetrics.fragmentAscent << ascent;
        thisBlockMetrics.maxBlockAscent = std::max( thisBlockMetrics.maxBlockAscent, ascent );
        thisBlockMetrics.maxBlockAscentForTextFragments = std::max( thisBlockMetrics.maxBlockAscentForTextFragments, ascent );

        thisBlockMetrics.maxBlockCapHeight = std::max( thisBlockMetrics.maxBlockCapHeight, fm.capHeight() / scaleFactor );

        const double descent = fm.descent() / scaleFactor;
        thisBlockMetrics.fragmentDescent << descent;

        thisBlockMetrics.maxBlockDescent = std::max( thisBlockMetrics.maxBlockDescent, descent );
        thisBlockMetrics.maxBlockMaxWidth = std::max( thisBlockMetrics.maxBlockMaxWidth, fm.maxWidth() / scaleFactor );

        if ( ( fm.lineSpacing() / scaleFactor ) > thisBlockMetrics.maxLineSpacing )
        {
          thisBlockMetrics.maxLineSpacing = fm.lineSpacing() / scaleFactor;
          thisBlockMetrics.maxBlockLeading = fm.leading() / scaleFactor;
        }
        thisBlockMetrics.fragmentFixedHeights << -1;
        updateCommonBlockMetrics( thisBlockMetrics, fragmentWidth, fragment );
        currentOutputBlock.append( fragment );
      };

      double fragmentWidth = fm.horizontalAdvance( fragment.text() ) / scaleFactor;

      // do we need to split this fragment to respect wrapping?
      if ( documentContext.flags() & Qgis::TextRendererFlag::WrapLines && documentContext.maximumWidth() > 0
           && ( thisBlockMetrics.blockXMax + fragmentWidth > documentContext.maximumWidth() ) )
      {
        // yep, need to split the fragment!

        //first step is to identify words which must be on their own line (too long to fit)
        const QStringList words = fragment.text().split( ' ' );
        QStringList linesToProcess;
        QStringList wordsInCurrentLine;
        double remainingWidthInCurrentLine = documentContext.maximumWidth() - thisBlockMetrics.blockXMax;
        for ( const QString &word : words )
        {
          const double wordWidth = fm.horizontalAdvance( word ) / scaleFactor;
          if ( wordWidth > remainingWidthInCurrentLine )
          {
            //too long to fit
            if ( !wordsInCurrentLine.isEmpty() )
              linesToProcess << wordsInCurrentLine.join( ' ' );
            wordsInCurrentLine.clear();
            linesToProcess << word;
            remainingWidthInCurrentLine = documentContext.maximumWidth();
          }
          else
          {
            wordsInCurrentLine.append( word );
          }
        }
        if ( !wordsInCurrentLine.isEmpty() )
          linesToProcess << wordsInCurrentLine.join( ' ' );

        remainingWidthInCurrentLine = documentContext.maximumWidth() - thisBlockMetrics.blockXMax;
        for ( int lineIndex = 0; lineIndex < linesToProcess.size(); ++lineIndex )
        {
          QString remainingText = linesToProcess.at( lineIndex );
          int lastPos = remainingText.lastIndexOf( ' ' );
          while ( lastPos > -1 )
          {
            //check if remaining text is short enough to go in one line
            if ( ( fm.horizontalAdvance( remainingText ) / scaleFactor ) <= remainingWidthInCurrentLine )
            {
              break;
            }

            const double widthTextToLastPos = fm.horizontalAdvance( remainingText.left( lastPos ) ) / scaleFactor;
            if ( widthTextToLastPos <= remainingWidthInCurrentLine )
            {
              QgsTextFragment thisLineFragment;
              thisLineFragment.setCharacterFormat( fragment.characterFormat() );
              thisLineFragment.setText( remainingText.left( lastPos ) );
              finalizeTextFragment( thisBlockMetrics, thisLineFragment, widthTextToLastPos );
              // move to new block
              finalizeBlock( res, format, documentMetrics, currentOutputBlock, thisBlockMetrics );
              thisBlockMetrics.isFirstBlock = false;
              remainingWidthInCurrentLine = documentContext.maximumWidth();
              remainingText = remainingText.mid( lastPos + 1 );
              lastPos = 0;
            }
            lastPos = remainingText.lastIndexOf( ' ', lastPos - 1 );
          }

          // if too big, and block is not empty, then flush current block first
          if ( ( fm.horizontalAdvance( remainingText ) / scaleFactor ) > remainingWidthInCurrentLine && !currentOutputBlock.empty() )
          {
            finalizeBlock( res, format, documentMetrics, currentOutputBlock, thisBlockMetrics );
            thisBlockMetrics.isFirstBlock = false;
            remainingWidthInCurrentLine = documentContext.maximumWidth();
          }

          QgsTextFragment thisLineFragment;
          thisLineFragment.setCharacterFormat( fragment.characterFormat() );
          thisLineFragment.setText( remainingText );
          finalizeTextFragment( thisBlockMetrics, thisLineFragment, fm.horizontalAdvance( remainingText ) / scaleFactor );

          if ( lineIndex < linesToProcess.size() - 1 )
          {
            // start new block if we aren't at the last line
            finalizeBlock( res, format, documentMetrics, currentOutputBlock, thisBlockMetrics );
            thisBlockMetrics.isFirstBlock = false;
            remainingWidthInCurrentLine = documentContext.maximumWidth();
          }

          thisBlockMetrics.isFirstBlock = false;
        }
      }
      else
      {
        // simple case, no wrapping
        finalizeTextFragment( thisBlockMetrics, fragment, fragmentWidth );
      }
    }
  }
}

QgsTextDocumentMetrics QgsTextDocumentMetrics::calculateMetrics( const QgsTextDocument &document, const QgsTextFormat &format, const QgsRenderContext &context, double scaleFactor, const QgsTextDocumentRenderContext &documentContext )
{
  QgsTextDocumentMetrics res;

  const QFont font = format.scaledFont( context, scaleFactor, &res.mIsNullSize );
  if ( res.isNullFontSize() )
    return res;

  DocumentMetrics documentMetrics;

  // for absolute line heights
  const double documentLineHeightPainterUnits = context.convertToPainterUnits( format.lineHeight(), format.lineHeightUnit() );

  documentMetrics.tabStopDistancePainterUnits = format.tabStopDistanceUnit() == Qgis::RenderUnit::Percentage
      ? format.tabStopDistance() * font.pixelSize() / scaleFactor
      : context.convertToPainterUnits( format.tabStopDistance(), format.tabStopDistanceUnit(), format.tabStopDistanceMapUnitScale() );

  const QList< QgsTextFormat::Tab > tabPositions = format.tabPositions();
  documentMetrics.tabStopDistancesPainterUnits.reserve( tabPositions.size() );
  for ( const QgsTextFormat::Tab &tab : tabPositions )
  {
    documentMetrics.tabStopDistancesPainterUnits.append(
      format.tabStopDistanceUnit() == Qgis::RenderUnit::Percentage
      ? tab.position() * font.pixelSize() / scaleFactor
      : context.convertToPainterUnits( tab.position(), format.tabStopDistanceUnit(), format.tabStopDistanceMapUnitScale() )
    );
  }

  documentMetrics.blockSize = document.size();
  res.mDocument.reserve( documentMetrics.blockSize );
  res.mFragmentFonts.reserve( documentMetrics.blockSize );

  for ( int blockIndex = 0; blockIndex < documentMetrics.blockSize; blockIndex++ )
  {
    const QgsTextBlock &block = document.at( blockIndex );
    QgsTextBlock outputBlock;
    outputBlock.setBlockFormat( block.blockFormat() );
    outputBlock.reserve( block.size() );

    const int fragmentSize = block.size();

    BlockMetrics thisBlockMetrics;
    thisBlockMetrics.lineHeightPainterUnits = documentLineHeightPainterUnits;
    // apply block line height if set
    if ( !std::isnan( block.blockFormat().lineHeightPercentage() ) )
    {
      thisBlockMetrics.lineHeightPercentage = block.blockFormat().lineHeightPercentage();
    }
    else if ( !std::isnan( block.blockFormat().lineHeight() ) )
    {
      thisBlockMetrics.lineHeightPainterUnits = context.convertToPainterUnits( block.blockFormat().lineHeight(), Qgis::RenderUnit::Points );
    }
    else if ( format.lineHeightUnit() == Qgis::RenderUnit::Percentage )
    {
      thisBlockMetrics.lineHeightPercentage = format.lineHeight();
    }

    thisBlockMetrics.fragmentVerticalOffsets.reserve( fragmentSize );
    thisBlockMetrics.fragmentFonts.reserve( fragmentSize );
    thisBlockMetrics.fragmentHorizontalAdvance.reserve( fragmentSize );
    thisBlockMetrics.fragmentFixedHeights.reserve( fragmentSize );

    thisBlockMetrics.isFirstBlock = blockIndex == 0;
    thisBlockMetrics.isLastBlock = blockIndex == documentMetrics.blockSize - 1;

    thisBlockMetrics.marginTop = context.convertToPainterUnits(
                                   !std::isnan( block.blockFormat().margins().top() ) ? block.blockFormat().margins().top() : 0, Qgis::RenderUnit::Points );
    thisBlockMetrics.marginBottom = context.convertToPainterUnits(
                                      !std::isnan( block.blockFormat().margins().bottom() ) ? block.blockFormat().margins().bottom() : 0, Qgis::RenderUnit::Points );
    thisBlockMetrics.marginLeft = context.convertToPainterUnits(
                                    !std::isnan( block.blockFormat().margins().left() ) ? block.blockFormat().margins().left() : 0, Qgis::RenderUnit::Points );
    thisBlockMetrics.marginRight = context.convertToPainterUnits(
                                     !std::isnan( block.blockFormat().margins().right() ) ? block.blockFormat().margins().right() : 0, Qgis::RenderUnit::Points );

    for ( int fragmentIndex = 0; fragmentIndex < fragmentSize; ++fragmentIndex )
    {
      const QgsTextFragment &fragment = block.at( fragmentIndex );
      processFragment( res, format, context, documentContext, scaleFactor, documentMetrics, thisBlockMetrics, font, fragment, outputBlock );
    }

    finalizeBlock( res, format, documentMetrics, outputBlock, thisBlockMetrics );
  }

  documentMetrics.heightLabelMode -= documentMetrics.lastLineLeading;
  documentMetrics.heightPointRectMode -= documentMetrics.lastLineLeading;

  res.mDocumentSizeLabelMode = QSizeF( documentMetrics.width, documentMetrics.heightLabelMode );
  res.mDocumentSizePointRectMode = QSizeF( documentMetrics.width, documentMetrics.heightPointRectMode );
  res.mDocumentSizeCapHeightMode = QSizeF( documentMetrics.width, documentMetrics.heightCapHeightMode );
  res.mDocumentSizeAscentMode = QSizeF( documentMetrics.width, documentMetrics.heightAscentMode );

  // adjust baselines
  if ( !res.mBaselineOffsetsLabelMode.isEmpty() )
  {
    const double labelModeBaselineAdjust = res.mBaselineOffsetsLabelMode.constLast() + res.mLastLineAscentOffset;
    const double pointModeBaselineAdjust = res.mBaselineOffsetsPointMode.constLast();
    for ( int i = 0; i < documentMetrics.blockSize; ++i )
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
      const double rightToLeftLineSpacing = documentMetrics.blockVerticalLineSpacing[ size - 1 - i ];

      adjustedRightToLeftXOffsets << currentOffset;
      currentOffset += rightToLeftLineSpacing;

      if ( i == size - 1 )
        widthVerticalOrientation += rightToLeftBlockMaxCharacterWidth;
      else
        widthVerticalOrientation += rightToLeftLineSpacing;
    }
    std::reverse( adjustedRightToLeftXOffsets.begin(), adjustedRightToLeftXOffsets.end() );
    res.mVerticalOrientationXOffsets = adjustedRightToLeftXOffsets;

    res.mDocumentSizeVerticalOrientation = QSizeF( widthVerticalOrientation, documentMetrics.heightVerticalOrientation );
  }

  res.mVerticalMarginsBetweenBlocks = documentMetrics.verticalMarginsBetweenBlocks;
  res.mLeftBlockMargins = documentMetrics.blockLeftMargin;
  res.mRightBlockMargins = documentMetrics.blockRightMargin;

  res.mOuterBoundsLabelMode = QRectF( documentMetrics.outerXMin, -documentMetrics.outerYMaxLabel,
                                      documentMetrics.outerXMax - documentMetrics.outerXMin,
                                      documentMetrics.heightLabelMode - documentMetrics.outerYMinLabel + documentMetrics.outerYMaxLabel );

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
  double verticalAdjustmentForBlockMargins = 0;
  for ( int i = 0; i < blockIndex; ++i )
  {
    double marginBeforeBlock = 0;
    verticalAdjustmentForBlockMargins += marginBeforeBlock;
  }

  switch ( mode )
  {
    case Qgis::TextLayoutMode::Rectangle:
      return mBaselineOffsetsRectMode.value( blockIndex ) + verticalAdjustmentForBlockMargins;
    case Qgis::TextLayoutMode::RectangleCapHeightBased:
      return mBaselineOffsetsCapHeightMode.value( blockIndex ) + verticalAdjustmentForBlockMargins;
    case Qgis::TextLayoutMode::RectangleAscentBased:
      return mBaselineOffsetsAscentBased.value( blockIndex ) + verticalAdjustmentForBlockMargins;
    case Qgis::TextLayoutMode::Point:
      return mBaselineOffsetsPointMode.value( blockIndex ) + verticalAdjustmentForBlockMargins;
    case Qgis::TextLayoutMode::Labeling:
      return mBaselineOffsetsLabelMode.value( blockIndex ) + verticalAdjustmentForBlockMargins;
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

double QgsTextDocumentMetrics::fragmentFixedHeight( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode ) const
{
  return mFragmentFixedHeights.value( blockIndex ).value( fragmentIndex );
}

double QgsTextDocumentMetrics::fragmentAscent( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode ) const
{
  return mFragmentAscent.value( blockIndex ).value( fragmentIndex );
}

double QgsTextDocumentMetrics::fragmentDescent( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode ) const
{
  return mFragmentDescent.value( blockIndex ).value( fragmentIndex );
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

double QgsTextDocumentMetrics::blockMaximumAscent( int blockIndex ) const
{
  return mBlockMaxAscent.value( blockIndex );
}

QFont QgsTextDocumentMetrics::fragmentFont( int blockIndex, int fragmentIndex ) const
{
  return mFragmentFonts.value( blockIndex ).value( fragmentIndex );
}

double QgsTextDocumentMetrics::blockVerticalMargin( int blockIndex ) const
{
  if ( blockIndex < 0 )
    return mVerticalMarginsBetweenBlocks.value( 0 );

  return mVerticalMarginsBetweenBlocks.value( blockIndex + 1 );
}

double QgsTextDocumentMetrics::blockLeftMargin( int blockIndex ) const
{
  return mLeftBlockMargins.value( blockIndex );
}

double QgsTextDocumentMetrics::blockRightMargin( int blockIndex ) const
{
  return mRightBlockMargins.value( blockIndex );
}

