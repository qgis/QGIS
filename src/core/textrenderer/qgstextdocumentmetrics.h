/***************************************************************************
  qgstextdocumentmetrics.h
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

#ifndef QGSTEXTDOCUMENTMETRICS_H
#define QGSTEXTDOCUMENTMETRICS_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include "qgis.h"
#include "qgstextdocument.h"

#include <QVector>
#include <QSizeF>
#include <QRectF>

class QgsRenderContext;
class QgsTextFormat;
struct DocumentMetrics;
struct BlockMetrics;

/**
 * \class QgsTextDocumentRenderContext
 * \ingroup core
 *
 * \brief Encapsulates the context in which a text document is to be rendered.
 *
 * \warning This API is not considered stable and may change in future QGIS versions.
 *
 * \since QGIS 3.40
 */
class CORE_EXPORT QgsTextDocumentRenderContext
{
  public:

    /**
     * Returns associated text renderer flags.
     *
     * \see setFlags()
     */
    Qgis::TextRendererFlags flags() const { return mFlags; }

    /**
     * Sets associated text renderer flags.
     *
     * \see flags()
     */
    void setFlags( Qgis::TextRendererFlags flags ) { mFlags = flags; }

    /**
     * Returns the maximum width (in painter units) for rendered text.
     *
     * This is used to control text wrapping, when the Qgis::TextRendererFlag::WrapLines flag is set.
     *
     * \see setMaximumWidth()
     */
    double maximumWidth() const { return mMaximumWidth; }

    /**
     * Sets the maximum width (in painter units) for rendered text.
     *
     * This is used to control text wrapping, when the Qgis::TextRendererFlag::WrapLines flag is set.
     *
     * \see maximumWidth()
     */
    void setMaximumWidth( double width ) { mMaximumWidth = width; }

  private:

    Qgis::TextRendererFlags mFlags;
    double mMaximumWidth = 0;

};

/**
 * \class QgsTextDocumentMetrics
 * \ingroup core
 *
 * \brief Contains pre-calculated metrics of a QgsTextDocument.
 *
 * \warning This API is not considered stable and may change in future QGIS versions.
 *
 * \since QGIS 3.28
 */
class CORE_EXPORT QgsTextDocumentMetrics
{
  public:

    /**
     * Returns precalculated text metrics for a text \a document, when rendered using the
     * given base \a format and render \a context.
     *
     * The optional \a scaleFactor parameter can specify a font size scaling factor. It is recommended to set this to
     * QgsTextRenderer::calculateScaleFactorForFormat() and then manually calculations
     * based on the resultant font metrics. Failure to do so will result in poor quality text rendering
     * at small font sizes.
     *
     * Since QGIS 3.40 the optional \a documentContext argument can be used to pass text renderer context to change the
     * logistics of the calculated metrics.
     */
    static QgsTextDocumentMetrics calculateMetrics( const QgsTextDocument &document, const QgsTextFormat &format, const QgsRenderContext &context, double scaleFactor = 1.0,
        const QgsTextDocumentRenderContext &documentContext = QgsTextDocumentRenderContext() );

    /**
     * Returns TRUE if the metrics could not be calculated because the text format has a null font size.
     *
     * \since QGIS 3.30
     */
    bool isNullFontSize() const { return mIsNullSize; }

    /**
     * Returns the document associated with the calculated metrics.
     *
     * Note that this may not exactly match the original document which was used in the call to calculateMetrics(),
     * as certain settings (such as text wrapping) require restructuring the document.
     *
     * \since QGIS 3.40
     */
    const QgsTextDocument &document() const { return mDocument; }

    /**
     * Returns the overall size of the document.
     */
    QSizeF documentSize( Qgis::TextLayoutMode mode, Qgis::TextOrientation orientation ) const;

    /**
     * Returns the outer bounds of the document, which is the documentSize() adjusted to account
     * for any text elements which fall outside of the usual document margins (such as super or
     * sub script elements)
     *
     * \warning Currently this is only supported for the Qgis::TextLayoutMode::Labeling mode.
     *
     * \since QGIS 3.30
     */
    QRectF outerBounds( Qgis::TextLayoutMode mode, Qgis::TextOrientation orientation ) const;

    /**
     * Returns the width of the block at the specified index.
     */
    double blockWidth( int blockIndex ) const;

    /**
     * Returns the height of the block at the specified index.
     */
    double blockHeight( int blockIndex ) const;

    /**
     * Returns the cap height for the first line of text.
     *
     * \since QGIS 3.30
     */
    double firstLineCapHeight() const;

    /**
     * Returns the offset from the top of the document to the text baseline for the given block index.
     */
    double baselineOffset( int blockIndex, Qgis::TextLayoutMode mode ) const;

    /**
     * Returns the horizontal advance of the fragment at the specified block and fragment index.
     *
     * \since QGIS 3.30
     */
    double fragmentHorizontalAdvance( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode mode ) const;

    /**
     * Returns the vertical offset from a text block's baseline which should be applied
     * to the fragment at the specified index within that block.
     *
     * \since QGIS 3.30
     */
    double fragmentVerticalOffset( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode mode ) const;

    /**
     * Returns the fixed height of the fragment at the specified block and fragment index, or -1 if the fragment does not have a fixed height.
     *
     * \since QGIS 3.40
     */
    double fragmentFixedHeight( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode mode ) const;

    /**
     * Returns the ascent of the fragment at the specified block and fragment index.
     *
     * \see fragmentDescent()
     *
     * \since QGIS 3.42
     */
    double fragmentAscent( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode mode ) const;

    /**
     * Returns the descent of the fragment at the specified block and fragment index.
     *
     * \see fragmentAscent()
     *
     * \since QGIS 3.42
     */
    double fragmentDescent( int blockIndex, int fragmentIndex, Qgis::TextLayoutMode mode ) const;

    /**
     * Returns the vertical orientation x offset for the specified block.
     */
    double verticalOrientationXOffset( int blockIndex ) const;

    /**
     * Returns the maximum character width for the specified block.
     */
    double blockMaximumCharacterWidth( int blockIndex ) const;

    /**
     * Returns the maximum descent encountered in the specified block.
     *
     * \see blockMaximumAscent()
     */
    double blockMaximumDescent( int blockIndex ) const;

    /**
     * Returns the maximum ascent encountered in the specified block.
     *
     * \see blockMaximumDescent()
     * \since QGIS 3.42
     */
    double blockMaximumAscent( int blockIndex ) const;

    /**
     * Returns the calculated font for the fragment at the specified block and fragment indices.
     */
    QFont fragmentFont( int blockIndex, int fragmentIndex ) const;

    /**
     * Returns the ascent offset of the first block in the document.
     */
    double ascentOffset() const { return mFirstLineAscentOffset; }

    /**
     * Returns the vertical margin for the specified block index.
     *
     * If \a blockIndex >= 0 then the returned value will be the margin to place after the block.
     * If \a blockIndex < 0 then the returned value will be the margin to place before the first block.
     *
     * \see blockLeftMargin()
     * \see blockRightMargin()
     *
     * \since QGIS 3.42
     */
    double blockVerticalMargin( int blockIndex ) const;

    /**
     * Returns the margin for the left side of the specified block index.
     *
     * \see blockVerticalMargin()
     * \see blockRightMargin()
     *
     * \since QGIS 3.42
     */
    double blockLeftMargin( int blockIndex ) const;

    /**
     * Returns the margin for the right side of the specified block index.
     *
     * \see blockVerticalMargin()
     * \see blockLeftMargin()
     *
     * \since QGIS 3.42
     */
    double blockRightMargin( int blockIndex ) const;

  private:

    QgsTextDocument mDocument;

    bool mIsNullSize = false;

    QSizeF mDocumentSizeLabelMode;
    QSizeF mDocumentSizePointRectMode;
    QSizeF mDocumentSizeVerticalOrientation;
    QSizeF mDocumentSizeCapHeightMode;
    QSizeF mDocumentSizeAscentMode;

    QRectF mOuterBoundsLabelMode;

    QList < QList< QFont > > mFragmentFonts;
    QList< double > mBlockWidths;
    QList< double > mBlockHeights;
    QList< double > mBaselineOffsetsLabelMode;
    QList< double > mBaselineOffsetsPointMode;
    QList< double > mBaselineOffsetsRectMode;
    QList< double > mBaselineOffsetsCapHeightMode;
    QList< double > mBaselineOffsetsAscentBased;

    QList< QList< double > > mFragmentHorizontalAdvance;
    QList< QList< double > > mFragmentFixedHeights;

    QList< QList< double > > mFragmentVerticalOffsetsLabelMode;
    QList< QList< double > > mFragmentVerticalOffsetsPointMode;
    QList< QList< double > > mFragmentVerticalOffsetsRectMode;

    QList< QList< double > > mFragmentAscent;
    QList< QList< double > > mFragmentDescent;

    QList< double > mVerticalOrientationXOffsets;
    QList< double > mBlockMaxDescent;
    QList< double > mBlockMaxAscent;
    QList< double > mBlockMaxCharacterWidth;
    double mFirstLineAscentOffset = 0;
    double mLastLineAscentOffset = 0;
    double mFirstLineCapHeight = 0;

    QVector< double > mVerticalMarginsBetweenBlocks;
    QVector< double > mLeftBlockMargins;
    QVector< double > mRightBlockMargins;

    static void finalizeBlock( QgsTextDocumentMetrics &res, const QgsTextFormat &format, DocumentMetrics &documentMetrics, QgsTextBlock &outputBlock, BlockMetrics &metrics );
    static void processFragment( QgsTextDocumentMetrics &res, const QgsTextFormat &format, const QgsRenderContext &context, const QgsTextDocumentRenderContext &documentContext, double scaleFactor, DocumentMetrics &documentMetrics, BlockMetrics &thisBlockMetrics, const QFont &font, const QgsTextFragment &fragment, QgsTextBlock &currentOutputBlock );
};

#endif // QGSTEXTDOCUMENTMETRICS_H
