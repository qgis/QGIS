/***************************************************************************
                            qgscomposerscalebar.h
                             -------------------
    begin                : March 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERSCALEBAR_H
#define QGSCOMPOSERSCALEBAR_H

#include "qgscomposeritem.h"
#include <QFont>
#include <QPen>
#include <QColor>

class QgsComposerMap;
class QgsScaleBarStyle;
/** \ingroup MapComposer
 * A scale bar item that can be added to a map composition.
 */

class CORE_EXPORT QgsComposerScaleBar: public QgsComposerItem
{
    Q_OBJECT

  public:

    enum Alignment
    {
      Left = 0,
      Middle,
      Right
    };

    enum ScaleBarUnits
    {
      MapUnits = 0,
      Meters,
      Feet,
      NauticalMiles
    };

    QgsComposerScaleBar( QgsComposition* composition );
    ~QgsComposerScaleBar();

    /** return correct graphics item type. */
    virtual int type() const { return ComposerScaleBar; }

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    //getters and setters
    int numSegments() const {return mNumSegments;}
    void setNumSegments( int nSegments );

    int numSegmentsLeft() const {return mNumSegmentsLeft;}
    void setNumSegmentsLeft( int nSegmentsLeft );

    double numUnitsPerSegment() const {return mNumUnitsPerSegment;}
    void setNumUnitsPerSegment( double units );

    double numMapUnitsPerScaleBarUnit() const {return mNumMapUnitsPerScaleBarUnit;}
    void setNumMapUnitsPerScaleBarUnit( double d ) {mNumMapUnitsPerScaleBarUnit = d;}

    QString unitLabeling() const {return mUnitLabeling;}
    void setUnitLabeling( const QString& label ) {mUnitLabeling = label;}

    QFont font() const;
    void setFont( const QFont& font );

    /**Returns the color used for drawing text in the scalebar.
     * @returns font color for scalebar.
     * @see setFontColor
     * @see font
    */
    QColor fontColor() const {return mFontColor;}

    /**Sets the color used for drawing text in the scalebar.
     * @param c font color for scalebar.
     * @see fontColor
     * @see setFont
    */
    void setFontColor( const QColor& c ) {mFontColor = c;}

    /**Returns the pen used for drawing the scalebar.
     * @returns QPen used for drawing the scalebar outlines.
     * @see setPen
     * @see brush
    */
    QPen pen() const {return mPen;}

    /**Sets the pen used for drawing the scalebar.
     * @param pen QPen to use for drawing the scalebar outlines.
     * @see pen
     * @see setBrush
    */
    void setPen( const QPen& pen ) {mPen = pen;}

    /**Returns the primary brush for the scalebar.
     * @returns QBrush used for filling the scalebar
     * @see setBrush
     * @see brush2
     * @see pen
    */
    QBrush brush() const {return mBrush;}

    /**Sets primary brush for the scalebar.
     * @param brush QBrush to use for filling the scalebar
     * @see brush
     * @see setBrush2
     * @see setPen
    */
    void setBrush( const QBrush& brush ) {mBrush = brush;}

    /**Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * @returns QBrush used for secondary color areas
     * @see setBrush2
     * @see brush
    */
    QBrush brush2() const {return mBrush2;}

    /**Sets secondary brush for the scalebar. This is used for alternating color style scalebars, such
     * as single and double box styles.
     * @param brush QBrush to use for secondary color areas
     * @see brush2
     * @see setBrush
    */
    void setBrush2( const QBrush& brush ) {mBrush2 = brush;}

    double height() const {return mHeight;}
    void setHeight( double h ) {mHeight = h;}

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap() const {return mComposerMap;}

    double labelBarSpace() const {return mLabelBarSpace;}
    void setLabelBarSpace( double space ) {mLabelBarSpace = space;}

    double boxContentSpace() const {return mBoxContentSpace;}
    void setBoxContentSpace( double space );

    double segmentMillimeters() const {return mSegmentMillimeters;}

    /**Left / Middle/ Right */
    Alignment alignment() const { return mAlignment; }

    void setAlignment( Alignment a );

    ScaleBarUnits units() const { return mUnits; }

    void setUnits( ScaleBarUnits u );

    /** Returns the join style used for drawing lines in the scalebar
     * @returns Join style for lines
     * @note introduced in 2.3
     * @see setLineJoinStyle
     */
    Qt::PenJoinStyle lineJoinStyle() const { return mLineJoinStyle; }
    /** Sets join style used when drawing the lines in the scalebar
     * @param style Join style for lines
     * @returns nothing
     * @note introduced in 2.3
     * @see lineJoinStyle
     */
    void setLineJoinStyle( Qt::PenJoinStyle style );

    /** Returns the cap style used for drawing lines in the scalebar
     * @returns Cap style for lines
     * @note introduced in 2.3
     * @see setLineCapStyle
     */
    Qt::PenCapStyle lineCapStyle() const { return mLineCapStyle; }
    /** Sets cap style used when drawing the lines in the scalebar
     * @param style Cap style for lines
     * @returns nothing
     * @note introduced in 2.3
     * @see lineCapStyle
     */
    void setLineCapStyle( Qt::PenCapStyle style );

    /**Apply default settings*/
    void applyDefaultSettings();
    /**Apply default size (scale bar 1/5 of map item width)
      */
    void applyDefaultSize( ScaleBarUnits u = Meters );

    /**Sets style by name
     @param styleName (untranslated) style name. Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle', 'Line Ticks Down', 'Line Ticks Up', 'Numeric'*/
    void setStyle( const QString& styleName );

    /**Returns style name*/
    QString style() const;

    /**Returns the x - positions of the segment borders (in item coordinates) and the width
     of the segment
     @note python bindings not available on android
     */
    void segmentPositions( QList<QPair<double, double> >& posWidthList ) const;

    /**Sets box size suitable to content*/
    void adjustBoxSize();

    /**Adjusts box size and calls QgsComposerItem::update()*/
    void update();

    /**Returns string of first label (important for drawing, labeling, size calculation*/
    QString firstLabelString() const;

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Moves scalebar position to the left / right depending on alignment and change in item width*/
    void correctXPositionAlignment( double width, double widthAfter );

    //overriden to apply minimum size
    void setSceneRect( const QRectF &rectangle );

  public slots:
    void updateSegmentSize();
    /**Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();

  protected:

    /**Reference to composer map object*/
    const QgsComposerMap* mComposerMap;
    /**Number of segments on right side*/
    int mNumSegments;
    /**Number of segments on left side*/
    int mNumSegmentsLeft;
    /**Size of a segment (in map units)*/
    double mNumUnitsPerSegment;
    /**Number of map units per scale bar units (e.g. 1000 to have km for a map with m units)*/
    double mNumMapUnitsPerScaleBarUnit;

    /**Labeling of map units*/
    QString mUnitLabeling;
    /**Font*/
    QFont mFont;
    QColor mFontColor;
    /**Outline*/
    QPen mPen;
    /**Fill*/
    QBrush mBrush;
    /**Secondary fill*/
    QBrush mBrush2;
    /**Height of bars/lines*/
    double mHeight;
    /**Scalebar style*/
    QgsScaleBarStyle* mStyle;

    /**Space between bar and Text labels*/
    double mLabelBarSpace;

    /**Space between content and item box*/
    double mBoxContentSpace;

    /**Width of a segment (in mm)*/
    double mSegmentMillimeters;

    Alignment mAlignment;

    ScaleBarUnits mUnits;

    Qt::PenJoinStyle mLineJoinStyle;
    Qt::PenCapStyle mLineCapStyle;

    /**Calculates with of a segment in mm and stores it in mSegmentMillimeters*/
    void refreshSegmentMillimeters();

    /**Returns diagonal of composer map in selected units (map units / meters / feet / nautical miles)*/
    double mapWidth() const;
};

#endif //QGSCOMPOSERSCALEBAR_H


