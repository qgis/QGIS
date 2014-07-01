#ifndef QGSLEGENDRENDERER_H
#define QGSLEGENDRENDERER_H

#include <QColor>
#include <QPointF>
#include <QSizeF>

class QRectF;
class QStandardItem;

class QgsLegendModel;
class QgsComposerLegendItem;
class QgsComposerGroupItem;
class QgsComposerLayerItem;
class QgsSymbolV2;

#include "qgscomposerlegendstyle.h"

/**
 * @brief The QgsLegendRenderer class handles automatic layout and rendering of legend.
 * The content is given by QgsLegendModel instance. Various layout properties can be configured
 * within QgsLegendRenderer.
 *
 * All spacing and sizes are in millimeters.
 *
 * @note added in 2.6
 */
class QgsLegendRenderer
{
  public:
    /** Construct legend renderer. The ownership of legend model does not change */
    explicit QgsLegendRenderer( QgsLegendModel* legendModel );

    /** Run the layout algorithm and determine the size required for legend */
    QSizeF legendSize();

    /** Draw the legend with given painter. It will occupy the area reported in legendSize().
     *  Painter should be scaled beforehand so that units correspond to millimeters.
     */
    void drawLegend( QPainter* painter );

    // setters and getters

    void setTitle( const QString& t ) {mTitle = t;}
    QString title() const {return mTitle;}

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

    double mmPerMapUnit() const { return mMmPerMapUnit; }
    void setMmPerMapUnit( double mmPerMapUnit ) { mMmPerMapUnit = mmPerMapUnit; }

    bool useAdvancedEffects() const { return mUseAdvancedEffects; }
    void setUseAdvancedEffects( bool use ) { mUseAdvancedEffects = use; }

  private:
    /** Nucleon is either group title, layer title or layer child item.
     *  Nucleon is similar to QgsComposerLegendItem but it does not have
     *  the same hierarchy. E.g. layer title nucleon is just title, it does not
     *  include all layer subitems, the same with groups.
     */
    class Nucleon
    {
      public:
        QgsComposerLegendItem* item;
        // Symbol size size without any space around for symbol item
        QSizeF symbolSize;
        // Label size without any space around for symbol item
        QSizeF labelSize;
        QSizeF size;
        // Offset of symbol label, this offset is the same for all symbol labels
        // of the same layer in the same column
        double labelXOffset;
    };

    /** Atom is indivisible set (indivisible into more columns). It may consists
     *  of one or more Nucleon, depending on layer splitting mode:
     *  1) no layer split: [group_title ...] layer_title layer_item [layer_item ...]
     *  2) layer split:    [group_title ...] layer_title layer_item
     *              or:    layer_item
     *  It means that group titles must not be split from layer title and layer title
     *  must not be split from first item, because it would look bad and it would not
     *  be readable to leave group or layer title at the bottom of column.
     */
    class Atom
    {
      public:
        Atom(): size( QSizeF( 0, 0 ) ), column( 0 ) {}
        QList<Nucleon> nucleons;
        // Atom size including nucleons interspaces but without any space around atom.
        QSizeF size;
        int column;
    };

    QSizeF paintAndDetermineSize( QPainter* painter );

    /** Create list of atoms according to current layer splitting mode */
    QList<Atom> createAtomList( QStandardItem* rootItem, bool splitLayer );

    /** Divide atoms to columns and set columns on atoms */
    void setColumns( QList<Atom>& atomList );

    /** Draws title in the legend using the title font and the specified alignment
     * If no painter is specified, function returns the required width/height to draw the title.
     */
    QSizeF drawTitle( QPainter* painter = 0, QPointF point = QPointF(), Qt::AlignmentFlag halignment = Qt::AlignLeft, double legendWidth = 0 );

    double spaceAboveAtom( Atom atom );

    /** Draw atom and return its actual size, the atom is drawn with the space above it
     *  so that first atoms in column are all aligned to the same line regardles their
     * style top space */
    QSizeF drawAtom( Atom atom, QPainter* painter = 0, QPointF point = QPointF() );

    /** Splits a string using the wrap char taking into account handling empty
      wrap char which means no wrapping */
    QStringList splitStringForWrapping( QString stringToSplt );

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


    /** Draws a group item and all subitems
     * Returns list of sizes of layers and groups including this group.
     */
    QSizeF drawGroupItemTitle( QgsComposerGroupItem* groupItem, QPainter* painter = 0, QPointF point = QPointF() );

    /** Draws a layer item and all subitems */
    QSizeF drawLayerItemTitle( QgsComposerLayerItem* layerItem, QPainter* painter = 0, QPointF point = QPointF() );

    Nucleon drawSymbolItem( QgsComposerLegendItem* symbolItem, QPainter* painter = 0, QPointF point = QPointF(), double labelXOffset = 0. );

    /** Draws a symbol at the current y position and returns the new x position. Returns real symbol height, because for points,
     it is possible that it differs from mSymbolHeight */
    void drawSymbolV2( QPainter* p, QgsSymbolV2* s, double currentYCoord, double& currentXPosition, double& symbolHeight, int opacity = 255 ) const;


  private:
    QgsLegendModel* mLegendModel;

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

#endif // QGSLEGENDRENDERER_H
