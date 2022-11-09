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

#include <QVector>
#include <QSizeF>

class QgsTextDocument;
class QgsRenderContext;
class QgsTextFormat;

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
     * QgsTextRenderer::FONT_WORKAROUND_SCALE and then manually calculations
     * based on the resultant font metrics. Failure to do so will result in poor quality text rendering
     * at small font sizes.
     */
    static QgsTextDocumentMetrics calculateMetrics( const QgsTextDocument &document, const QgsTextFormat &format, const QgsRenderContext &context, double scaleFactor = 1.0 );

    /**
     * Returns the overall size of the document.
     */
    QSizeF documentSize( Qgis::TextLayoutMode mode, Qgis::TextOrientation orientation ) const;

    /**
     * Returns the width of the block at the specified index.
     */
    double blockWidth( int blockIndex ) const;

    /**
     * Returns the height of the block at the specified index.
     */
    double blockHeight( int blockIndex ) const;

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
     * Returns the vertical orientation x offset for the specified block.
     */
    double verticalOrientationXOffset( int blockIndex ) const;

    /**
     * Returns the maximum character width for the specified block.
     */
    double blockMaximumCharacterWidth( int blockIndex ) const;

    /**
     * Returns the maximum descent encountered in the specified block.
     */
    double blockMaximumDescent( int blockIndex ) const;

    /**
     * Returns the calculated font for the fragment at the specified block and fragment indices.
     */
    QFont fragmentFont( int blockIndex, int fragmentIndex ) const;

    /**
     * Returns the ascent offset of the first block in the document.
     */
    double ascentOffset() const { return mFirstLineAscentOffset; }

  private:

    QSizeF mDocumentSizeLabelMode;
    QSizeF mDocumentSizePointRectMode;
    QSizeF mDocumentSizeVerticalOrientation;

    QList < QList< QFont > > mFragmentFonts;
    QList< double > mBlockWidths;
    QList< double > mBlockHeights;
    QList< double > mBaselineOffsetsLabelMode;
    QList< double > mBaselineOffsetsPointMode;
    QList< double > mBaselineOffsetsRectMode;

    QList< QList< double > > mFragmentHorizontalAdvance;

    QList< double > mVerticalOrientationXOffsets;
    QList< double > mBlockMaxDescent;
    QList< double > mBlockMaxCharacterWidth;
    double mFirstLineAscentOffset = 0;
    double mLastLineAscentOffset = 0;

};

#endif // QGSTEXTDOCUMENTMETRICS_H
