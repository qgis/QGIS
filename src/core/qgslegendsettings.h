#ifndef QGSLEGENDSETTINGS_H
#define QGSLEGENDSETTINGS_H

#include <QColor>
#include <QSizeF>

#include "qgscomposerlegendstyle.h"

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
};

#endif // QGSLEGENDSETTINGS_H
