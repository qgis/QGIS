/***************************************************************************
  qgslegendsettings.h
  --------------------------------------
  Date                 : July 2014
  Copyright            : (C) 2014 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLEGENDSETTINGS_H
#define QGSLEGENDSETTINGS_H

#include <QColor>
#include <QSizeF>

class QRectF;

#include "qgscomposerlegendstyle.h"

class QgsComposerLegendItem;


/**
 * @brief The QgsLegendSettings class stores the appearance and layout settings
 * for legend drawing with QgsLegendRenderer. The content of the legend is given
 * in QgsLegendModel class.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsLegendSettings
{
  public:
    QgsLegendSettings();

    void setTitle( const QString& t ) { mTitle = t; }
    QString title() const { return mTitle; }

    /** Returns the alignment of the legend title
     * @returns Qt::AlignmentFlag for the legend title
     * @see setTitleAlignment
    */
    Qt::AlignmentFlag titleAlignment() const { return mTitleAlignment; }
    /** Sets the alignment of the legend title
     * @param alignment Text alignment for drawing the legend title
     * @see titleAlignment
    */
    void setTitleAlignment( Qt::AlignmentFlag alignment ) { mTitleAlignment = alignment; }

    /** Returns reference to modifiable style */
    QgsComposerLegendStyle & rstyle( QgsComposerLegendStyle::Style s ) { return mStyleMap[s]; }
    /** Returns style */
    QgsComposerLegendStyle style( QgsComposerLegendStyle::Style s ) const { return mStyleMap.value( s ); }
    void setStyle( QgsComposerLegendStyle::Style s, const QgsComposerLegendStyle style ) { mStyleMap[s] = style; }

    double boxSpace() const {return mBoxSpace;}
    void setBoxSpace( double s ) {mBoxSpace = s;}

    void setWrapChar( const QString& t ) {mWrapChar = t;}
    QString wrapChar() const {return mWrapChar;}

    double columnSpace() const {return mColumnSpace;}
    void setColumnSpace( double s ) { mColumnSpace = s;}

    int columnCount() const { return mColumnCount; }
    void setColumnCount( int c ) { mColumnCount = c;}

    int splitLayer() const { return mSplitLayer; }
    void setSplitLayer( bool s ) { mSplitLayer = s;}

    int equalColumnWidth() const { return mEqualColumnWidth; }
    void setEqualColumnWidth( bool s ) { mEqualColumnWidth = s;}

    QColor fontColor() const {return mFontColor;}
    void setFontColor( const QColor& c ) {mFontColor = c;}

    QSizeF symbolSize() const {return mSymbolSize;}
    void setSymbolSize( QSizeF s ) {mSymbolSize = s;}

    QSizeF wmsLegendSize() const {return mWmsLegendSize;}
    void setWmsLegendSize( QSizeF s ) {mWmsLegendSize = s;}

    double lineSpacing() const { return mLineSpacing; }
    void setLineSpacing( double s ) { mLineSpacing = s; }

    double mmPerMapUnit() const { return mMmPerMapUnit; }
    void setMmPerMapUnit( double mmPerMapUnit ) { mMmPerMapUnit = mmPerMapUnit; }

    bool useAdvancedEffects() const { return mUseAdvancedEffects; }
    void setUseAdvancedEffects( bool use ) { mUseAdvancedEffects = use; }

    double mapScale() const { return mMapScale; }
    void setMapScale( double scale ) { mMapScale = scale; }

    int dpi() const { return mDpi; }
    void setDpi( int dpi ) { mDpi = dpi; }

    // utility functions

    /** Splits a string using the wrap char taking into account handling empty
      wrap char which means no wrapping */
    QStringList splitStringForWrapping( QString stringToSplt ) const;

    /** Draws Text. Takes care about all the composer specific issues (calculation to pixel, scaling of font and painter
     to work around the Qt font bug)*/
    void drawText( QPainter* p, double x, double y, const QString& text, const QFont& font ) const;

    /** Like the above, but with a rectangle for multiline text
     * @param p painter to use
     * @param rect rectangle to draw into
     * @param text text to draw
     * @param font font to use
     * @param halignment optional horizontal alignment
     * @param valignment optional vertical alignment
     * @param flags allows for passing Qt::TextFlags to control appearance of rendered text
    */
    void drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignment = Qt::AlignLeft, Qt::AlignmentFlag valignment = Qt::AlignTop, int flags = Qt::TextWordWrap ) const;

    /** Returns a font where size is in pixel and font size is upscaled with FONT_WORKAROUND_SCALE */
    QFont scaledFontPixelSize( const QFont& font ) const;

    /** Calculates font to from point size to pixel size */
    double pixelFontSize( double pointSize ) const;

    /** Returns the font width in millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE */
    double textWidthMillimeters( const QFont& font, const QString& text ) const;

    /** Returns the font height of a character in millimeters */
    double fontHeightCharacterMM( const QFont& font, const QChar& c ) const;

    /** Returns the font ascent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE */
    double fontAscentMillimeters( const QFont& font ) const;

    /** Returns the font descent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE */
    double fontDescentMillimeters( const QFont& font ) const;

  private:

    QString mTitle;

    /** Title alignment, one of Qt::AlignLeft, Qt::AlignHCenter, Qt::AlignRight) */
    Qt::AlignmentFlag mTitleAlignment;

    QString mWrapChar;

    QColor mFontColor;

    /** Space between item box and contents */
    qreal mBoxSpace;

    /** Width and height of symbol icon */
    QSizeF mSymbolSize;

    /** Width and height of WMS legendGraphic pixmap */
    QSizeF mWmsLegendSize;

    /** Spacing between lines when wrapped */
    double mLineSpacing;

    /** Space between columns */
    double mColumnSpace;

    /** Number of legend columns */
    int mColumnCount;

    /** Allow splitting layers into multiple columns */
    bool mSplitLayer;

    /** Use the same width (maximum) for all columns */
    bool mEqualColumnWidth;

    QMap<QgsComposerLegendStyle::Style, QgsComposerLegendStyle> mStyleMap;

    /** Conversion ratio between millimeters and map units - for symbols with size given in map units */
    double mMmPerMapUnit;

    /** Whether to use advanced effects like transparency for symbols - may require their rasterization */
    bool mUseAdvancedEffects;

    /** Denominator of map's scale */
    double mMapScale;

    /** DPI to be used when rendering legend */
    int mDpi;
};



#endif // QGSLEGENDSETTINGS_H
