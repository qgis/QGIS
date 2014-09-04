/***************************************************************************
                         qgscomposermapgrid.h
                         --------------------
    begin                : December 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERMAPGRID_H
#define QGSCOMPOSERMAPGRID_H

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgsrendercontext.h"
#include <QString>
#include <QPainter>

class QgsCoordinateTransform;
class QgsLineSymbolV2;
class QgsMarkerSymbolV2;
class QgsComposerMapGrid;
class QgsComposerMap;
class QDomDocument;
class QDomElement;
class QPainter;

/**\ingroup MapComposer
 * \class QgsComposerMapGridStack
 * \brief A collection of grids which is drawn above the map content in a
 * QgsComposerMap. The grid stack controls which grids are drawn and the
 * order they are drawn in.
 * \note added in QGIS 2.5
 * \see QgsComposerMapGrid
 */
class CORE_EXPORT QgsComposerMapGridStack
{
  public:

    /**Constructor for QgsComposerMapGridStack.
     * @param map QgsComposerMap the grid stack is attached to
    */
    QgsComposerMapGridStack( QgsComposerMap* map );

    virtual ~QgsComposerMapGridStack();

    /**Adds a new map grid to the stack and takes ownership of the grid.
     * The grid will be added to the end of the stack, and rendered
     * above any existing map grids already present in the stack.
     * @param grid QgsComposerMapGrid to add to the stack
     * @note after adding a grid to the stack, updateBoundingRect() and update()
     * should be called for the QgsComposerMap to prevent rendering artifacts
     * @see removeGrid
    */
    void addGrid( QgsComposerMapGrid* grid );

    /**Removes a grid from the stack and deletes the corresponding QgsComposerMapGrid
     * @param gridId id for the QgsComposerMapGrid to remove
     * @note after removing a grid from the stack, updateBoundingRect() and update()
     * should be called for the QgsComposerMap to prevent rendering artifacts
     * @see addGrid
    */
    void removeGrid( const QString& gridId );

    /**Moves a grid up the stack, causing it to be rendered above other grids
     * @param gridId id for the QgsComposerMapGrid to move up
     * @note after moving a grid within the stack, update() should be
     * called for the QgsComposerMap to redraw the map with the new grid stack order
     * @see moveGridDown
    */
    void moveGridUp( const QString& gridId );

    /**Moves a grid up the stack, causing it to be rendered above other grids
     * @param gridId id for the QgsComposerMapGrid to move up
     * @note after moving a grid within the stack, update() should be
     * called for the QgsComposerMap to redraw the map with the new grid stack order
     * @see moveGridDown
    */
    void moveGridDown( const QString& gridId );

    /**Returns a const reference to a grid within the stack
     * @param gridId id for the QgsComposerMapGrid to find
     * @returns const reference to grid, if found
     * @see grid
    */
    const QgsComposerMapGrid* constGrid( const QString& gridId ) const;

    /**Returns a reference to a grid within the stack
     * @param gridId id for the QgsComposerMapGrid to find
     * @returns reference to grid if found
     * @see constGrid
    */
    QgsComposerMapGrid* grid( const QString& gridId ) const;

    /**Returns a reference to a grid within the stack
     * @param index grid position in the stack
     * @returns reference to grid if found
     * @see constGrid
    */
    QgsComposerMapGrid* grid( const int index ) const;

    /**Returns a reference to a grid within the stack
     * @param idx grid position in the stack
     * @returns reference to grid if found
     * @see constGrid
     * @see grid
    */
    QgsComposerMapGrid &operator[]( int idx );

    /**Returns a list of QgsComposerMapGrids contained by the stack
     * @returns list of grids
    */
    QList< QgsComposerMapGrid* > asList() const;


    /**Returns the number of grids in the stack
     * @returns number of grids in the stack
    */
    int size() const { return mGrids.size(); }

    /**Stores the state of the grid stack in a DOM node
     * @param elem is DOM element corresponding to a 'ComposerMap' tag
     * @param doc DOM document
     * @returns true if write was successful
     * @see readXML
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /**Sets the grid stack's state from a DOM document
     * @param elem is DOM node corresponding to 'a ComposerMap' tag
     * @param doc DOM document
     * @returns true if read was successful
     * @see writeXML
     */
    bool readXML( const QDomElement& elem, const QDomDocument& doc );

    /**Draws the grids from the stack on a specified painter
     * @param painter destination QPainter
     */
    void drawGrids( QPainter* painter );

    /**Calculates the maximum distance grids within the stack extend
     * beyond the QgsComposerMap's item rect
     * @returns maximum grid extension
     */
    double maxGridExtension() const;

    /**Returns whether any grids within the stack contain advanced effects,
     * such as blending modes
     * @returns true if grid stack contains advanced effects
     */
    bool containsAdvancedEffects() const;

  private:

    QList< QgsComposerMapGrid* > mGrids;

    QgsComposerMap* mComposerMap;

    /**Clears the grid stack and deletes all QgsComposerMapGrids contained
     * by the stack
     */
    void removeGrids();
};

//
// QgsComposerMapGrid
//

/**\ingroup MapComposer
 * \class QgsComposerMapGrid
 * \brief An individual grid which is drawn above the map content in a
 * QgsComposerMap.
 * \note added in QGIS 2.5
 * \see QgsComposerMapGridStack
 */
class CORE_EXPORT QgsComposerMapGrid
{
  public:

    /** Unit for grid values
     */
    enum GridUnit
    {
      MapUnit, /*< grid units follow map units */
      MM, /*< grid units in millimetres */
      CM /*< grid units in centimetres */
    };

    /** Grid drawing style
     */
    enum GridStyle
    {
      Solid = 0,
      Cross, /*< draw line crosses at intersections of grid lines */
      Markers, /*< draw markers at intersections of grid lines */
      FrameAnnotationsOnly /*< no grid lines over the map, only draw frame and annotations */
    };

    /** Position for grid annotations
     */
    enum AnnotationPosition
    {
      InsideMapFrame = 0,
      OutsideMapFrame, /*< draw annotations outside the map frame */
      Disabled /*< disable annotation */
    };

    /** Direction of grid annotations
     */
    enum AnnotationDirection
    {
      Horizontal = 0, /*< draw annotations horizontally */
      Vertical, /*< draw annotations vertically */
      BoundaryDirection /*< annotations follow the boundary direction */
    };

    /** Format for displaying grid annotations
     */
    enum AnnotationFormat
    {
      Decimal = 0, /*< decimal degrees, use - for S/W coordinates */
      DegreeMinute, /*< degree/minutes, use NSEW suffix */
      DegreeMinuteSecond, /*< degree/minutes/seconds, use NSEW suffix */
      DecimalWithSuffix, /*< decimal degrees, use NSEW suffix */
      DegreeMinuteNoSuffix, /*< degree/minutes, use - for S/W coordinates */
      DegreeMinutePadded, /*< degree/minutes, with minutes using leading zeros were required */
      DegreeMinuteSecondNoSuffix, /*< degree/minutes/seconds, use - for S/W coordinates */
      DegreeMinuteSecondPadded /*< degree/minutes/seconds, with minutes using leading zeros were required */
    };

    /** Border sides for annotations
     */
    enum BorderSide
    {
      Left,
      Right, /*< right border */
      Bottom, /*< bottom border */
      Top /*< top border */
    };

    /** Style for grid frame
     */
    enum FrameStyle
    {
      NoFrame = 0, /*< disable grid frame */
      Zebra, /*< black/white pattern */
      InteriorTicks,  /*< tick markers drawn inside map frame */
      ExteriorTicks,  /*< tick markers drawn outside map frame */
      InteriorExteriorTicks, /*< tick markers drawn both inside and outside the map frame */
      LineBorder /*< simple solid line frame */
    };

    /** Flags for controlling which side of the map a frame is drawn on
     */
    enum FrameSideFlag
    {
      FrameLeft = 0x01, /*< left side of map */
      FrameRight = 0x02, /*< right side of map */
      FrameTop = 0x04, /*< top side of map */
      FrameBottom = 0x08 /*< bottom side of map */
    };
    Q_DECLARE_FLAGS( FrameSideFlags, FrameSideFlag )

    /** Annotation coordinate type
     */
    enum AnnotationCoordinate
    {
      Longitude = 0, /*< coordinate is a longitude value */
      Latitude /*< coordinate is a latitude value */
    };

    /**Constructor for QgsComposerMapGrid.
     * @param name friendly display name for grid
     * @param map QgsComposerMap the grid stack is attached to
    */
    QgsComposerMapGrid( const QString& name, QgsComposerMap* map );

    virtual ~QgsComposerMapGrid();

    /**Draws a grid
     * @param painter destination QPainter
     */
    void drawGrid( QPainter* painter ) const;

    /**Stores grid state in DOM element
     * @param elem is DOM element corresponding to a 'ComposerMap' tag
     * @param doc DOM document
     * @see readXML
    */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /**Sets grid state from a DOM document
     * @param itemElem is DOM node corresponding to a 'ComposerMapGrid' tag
     * @param doc is DOM document
     * @see writeXML
    */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /**Sets composer map for the grid
     * @param map composer map
     * @see composerMap
    */
    void setComposerMap( QgsComposerMap* map );

    /**Get composer map for the grid
     * @returns composer map
     * @see setComposerMap
    */
    const QgsComposerMap* composerMap() const { return mComposerMap; }

    /**Sets the friendly display name for the grid
     * @param name display name
     * @see name
    */
    void setName( const QString& name ) { mName = name; }

    /**Get friendly display name for the grid
     * @returns display name
     * @see setName
    */
    QString name() const { return mName; }

    /**Get the unique id for the grid
     * @returns unique id
     * @see name
    */
    QString id() const { return mUuid; }

    /**Controls whether the grid will be drawn
     * @param enabled set to true to enable drawing of the grid
     * @see enabled
    */
    void setEnabled( const bool enabled ) { mGridEnabled = enabled; }

    /**Returns whether the grid will be drawn
     * @returns true if grid will be drawn on the map
     * @see setEnabled
    */
    bool enabled() const { return mGridEnabled; }

    /**Sets the CRS for the grid.
     * @param crs coordinate reference system for grid
     * @see crs
    */
    void setCrs( const QgsCoordinateReferenceSystem& crs ) { mCRS = crs; }

    /**Retrieves the CRS for the grid.
     * @returns coordinate reference system for grid
     * @see setCrs
    */
    QgsCoordinateReferenceSystem crs() const { return mCRS; }

    /**Sets the blending mode used for drawing the grid.
     * @param mode blending mode for grid
     * @see blendMode
    */
    void setBlendMode( const QPainter::CompositionMode mode ) { mBlendMode = mode; }

    /**Retrieves the blending mode used for drawing the grid.
     * @returns blending mode for grid
     * @see setBlendMode
    */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    /**Calculates the maximum distance the grid extends beyond the QgsComposerMap's
     * item rect
     * @returns maximum extension in millimetres
     */
    double maxExtension() const;

    //
    // GRID UNITS
    //

    /**Sets the units to use for grid measurements such as the interval
     * and offset for grid lines.
     * @param unit unit for grid measurements
     * @see units
    */
    void setUnits( const GridUnit unit ) { mGridUnit = unit; }

    /**Gets the units used for grid measurements such as the interval
     * and offset for grid lines.
     * @returns for grid measurements
     * @see setUnits
    */
    GridUnit units() const { return mGridUnit; }

    /**Sets the interval between grid lines in the x-direction. The units
     * are controlled through the setUnits method
     * @param interval interval between horizontal grid lines
     * @see setIntervalY
     * @see intervalX
    */
    void setIntervalX( const double interval ) { mGridIntervalX = interval; }

    /**Gets the interval between grid lines in the x-direction. The units
     * are retrieved through the units() method.
     * @returns interval between horizontal grid lines
     * @see setIntervalX
     * @see intervalY
    */
    double intervalX() const { return mGridIntervalX; }

    /**Sets the interval between grid lines in the y-direction. The units
     * are controlled through the setUnits method
     * @param interval interval between vertical grid lines
     * @see setIntervalX
     * @see intervalY
    */
    void setIntervalY( const double interval ) { mGridIntervalY = interval; }

    /**Gets the interval between grid lines in the y-direction. The units
     * are retrieved through the units() method.
     * @returns interval between vertical grid lines
     * @see setIntervalY
     * @see intervalX
    */
    double intervalY() const { return mGridIntervalY; }

    /**Sets the offset for grid lines in the x-direction. The units
     * are controlled through the setUnits method
     * @param offset offset for horizontal grid lines
     * @see setOffsetY
     * @see offsetX
    */
    void setOffsetX( const double offset ) { mGridOffsetX = offset; }

    /**Gets the offset for grid lines in the x-direction. The units
     * are retrieved through the units() method.
     * @returns offset for horizontal grid lines
     * @see setOffsetX
     * @see offsetY
    */
    double offsetX() const { return mGridOffsetX; }

    /**Sets the offset for grid lines in the y-direction. The units
     * are controlled through the setUnits method
     * @param offset offset for vertical grid lines
     * @see setOffsetX
     * @see offsetY
    */
    void setOffsetY( const double offset ) { mGridOffsetY = offset; }

    /**Gets the offset for grid lines in the y-direction. The units
     * are retrieved through the units() method.
     * @returns offset for vertical grid lines
     * @see setOffsetY
     * @see offsetX
    */
    double offsetY() const { return mGridOffsetY; }

    //
    // GRID APPEARANCE
    //

    /**Sets the grid style, which controls how the grid is drawn
     * over the map's contents
     * @param style desired grid style
     * @see style
    */
    void setStyle( const GridStyle style ) { mGridStyle = style; }

    /**Gets the grid's style, which controls how the grid is drawn
     * over the map's contents
     * @returns current grid style
     * @see setStyle
    */
    GridStyle style() const { return mGridStyle; }

    /**Sets the length of the cross segments drawn for the grid. This is only used for grids
     * with QgsComposerMapGrid::Cross styles
     * @param length cross length in millimetres
     * @see crossLength
    */
    void setCrossLength( const double length ) { mCrossLength = length; }

    /**Retrieves the length of the cross segments drawn for the grid. This is only used for grids
     * with QgsComposerMapGrid::Cross styles
     * @returns cross length in millimetres
     * @see setCrossLength
    */
    double crossLength() const { return mCrossLength; }

    /**Sets width of grid lines. This is only used for grids with QgsComposerMapGrid::Solid
     * or QgsComposerMapGrid::Cross styles. For more control over grid line appearance, use
     * setLineSymbol instead.
     * @param width grid line width
     * @see setLineSymbol
     * @see setGridLineColor
    */
    void setGridLineWidth( const double width );

    /**Sets color of grid lines. This is only used for grids with QgsComposerMapGrid::Solid
     * or QgsComposerMapGrid::Cross styles. For more control over grid line appearance, use
     * setLineSymbol instead.
     * @param color color of grid lines
     * @see setLineSymbol
     * @see setGridLineWidth
    */
    void setGridLineColor( const QColor& color );

    /**Sets the line symbol used for drawing grid lines. This is only used for grids with
     * QgsComposerMapGrid::Solid or QgsComposerMapGrid::Cross styles.
     * @param symbol line symbol for grid lines
     * @see lineSymbol
     * @see setMarkerSymbol
     * @see setStyle
    */
    void setLineSymbol( QgsLineSymbolV2* symbol );

    /**Gets the line symbol used for drawing grid lines. This is only used for grids with
     * QgsComposerMapGrid::Solid or QgsComposerMapGrid::Cross styles.
     * @returns line symbol for grid lines
     * @see setLineSymbol
     * @see markerSymbol
     * @see style
    */
    const QgsLineSymbolV2* lineSymbol() const { return mGridLineSymbol; }

    /**Gets the line symbol used for drawing grid lines. This is only used for grids with
     * QgsComposerMapGrid::Solid or QgsComposerMapGrid::Cross styles.
     * @returns line symbol for grid lines
     * @see setLineSymbol
     * @see markerSymbol
     * @see style
    */
    QgsLineSymbolV2* lineSymbol() { return mGridLineSymbol; }

    /**Sets the marker symbol used for drawing grid points. This is only used for grids with a
     * QgsComposerMapGrid::Markers style.
     * @param symbol marker symbol for grid intersection points
     * @see markerSymbol
     * @see setLineSymbol
     * @see setStyle
    */
    void setMarkerSymbol( QgsMarkerSymbolV2* symbol );

    /**Gets the marker symbol used for drawing grid points. This is only used for grids with a
     * QgsComposerMapGrid::Markers style.
     * @returns marker symbol for grid intersection points
     * @see setMarkerSymbol
     * @see lineSymbol
     * @see style
    */
    const QgsMarkerSymbolV2* markerSymbol() const { return mGridMarkerSymbol; }

    /**Gets the marker symbol used for drawing grid points. This is only used for grids with a
     * QgsComposerMapGrid::Markers style.
     * @returns marker symbol for grid intersection points
     * @see setMarkerSymbol
     * @see lineSymbol
     * @see style
    */
    QgsMarkerSymbolV2* markerSymbol() { return mGridMarkerSymbol; }

    //
    // ANNOTATIONS
    //

    /**Sets whether annotations should be shown for the grid.
     * @param enabled set to true to draw annotations for the grid
     * @see annotationEnabled
    */
    void setAnnotationEnabled( const bool enabled ) { mShowGridAnnotation = enabled; }

    /**Gets whether annotations are shown for the grid.
     * @returns true if annotations are drawn for the grid
     * @see setAnnotationEnabled
    */
    bool annotationEnabled() const { return mShowGridAnnotation; }

    /**Sets the font used for drawing grid annotations
     * @param font font for annotations
     * @see annotationFont
    */
    void setAnnotationFont( const QFont& font ) { mGridAnnotationFont = font; }

    /**Gets the font used for drawing grid annotations
     * @returns font for annotations
     * @see setAnnotationFont
    */
    QFont annotationFont() const { return mGridAnnotationFont; }

    /**Sets the font color used for drawing grid annotations
     * @param color font color for annotations
     * @see annotationFontColor
    */
    void setAnnotationFontColor( const QColor& color ) { mGridAnnotationFontColor = color; }

    /**Gets the font color used for drawing grid annotations
     * @returns font color for annotations
     * @see setAnnotationFontColor
    */
    QColor annotationFontColor() const { return mGridAnnotationFontColor; }

    /**Sets the coordinate precision for grid annotations
     * @param precision number of decimal places to show when drawing grid annotations
     * @see annotationPrecision
    */
    void setAnnotationPrecision( const int precision ) { mGridAnnotationPrecision = precision; }

    /**Returns the coordinate precision for grid annotations
     * @returns number of decimal places shown when drawing grid annotations
     * @see setAnnotationPrecision
    */
    int annotationPrecision() const { return mGridAnnotationPrecision; }

    /**Sets the position for the grid annotations on a specified side of the map
     * frame.
     * @param position position to draw grid annotations
     * @param border side of map for annotations
     * @see annotationPosition
    */
    void setAnnotationPosition( const AnnotationPosition position, const BorderSide border );

    /**Gets the position for the grid annotations on a specified side of the map
     * frame.
     * @param border side of map for annotations
     * @returns position that grid annotations are drawn in
     * @see setAnnotationPosition
    */
    AnnotationPosition annotationPosition( const BorderSide border ) const;

    /**Sets the distance between the map frame and annotations. Units are in millimetres.
     * @param distance margin between map frame and annotations
     * @see annotationFrameDistance
    */
    void setAnnotationFrameDistance( const double distance ) { mAnnotationFrameDistance = distance; }

    /**Gets the distance between the map frame and annotations. Units are in millimetres.
     * @returns margin between map frame and annotations
     * @see setAnnotationFrameDistance
    */
    double annotationFrameDistance() const { return mAnnotationFrameDistance; }

    /**Sets the direction for drawing frame annotations.
     * @param direction direction for frame annotations
     * @param border side of map for annotations
     * @see annotationDirection
    */
    void setAnnotationDirection( const AnnotationDirection direction, const BorderSide border );

    /**Sets the direction for drawing all frame annotations.
     * @param direction direction for frame annotations
     * @see annotationDirection
    */
    void setAnnotationDirection( const AnnotationDirection direction );

    /**Gets the direction for drawing frame annotations.
     * @param border side of map for annotations
     * @returns direction for frame annotations
     * @see setAnnotationDirection
    */
    AnnotationDirection annotationDirection( const BorderSide border ) const;

    /**Sets the format for drawing grid annotations.
     * @param format format for grid annotations
     * @see annotationFormat
    */
    void setAnnotationFormat( const AnnotationFormat format ) { mGridAnnotationFormat = format; }

    /**Gets the format for drawing grid annotations.
     * @returns format for grid annotations
     * @see setAnnotationFormat
    */
    AnnotationFormat annotationFormat() const { return mGridAnnotationFormat; }

    //
    // GRID FRAME
    //

    /**Sets the grid frame style.
     * @param style style for grid frame
     * @see frameStyle
    */
    void setFrameStyle( const FrameStyle style ) { mGridFrameStyle = style; }

    /**Gets the grid frame style.
     * @returns style for grid frame
     * @see setFrameStyle
    */
    FrameStyle frameStyle() const { return mGridFrameStyle; }

    /**Sets flags for grid frame sides. Setting these flags controls which sides
     * of the map item the grid frame is drawn on.
     * @param flags flags for grid frame sides
     * @see setFrameSideFlag
     * @see frameSideFlags
     * @see testFrameSideFlag
    */
    void setFrameSideFlags( const FrameSideFlags flags );

    /**Sets whether the grid frame is drawn for a certain side of the map item.
     * @param flag flag for grid frame side
     * @param on set to true to draw grid frame on that side of the map
     * @see setFrameSideFlags
     * @see frameSideFlags
     * @see testFrameSideFlag
    */
    void setFrameSideFlag( const FrameSideFlag flag, bool on = true );

    /**Returns the flags which control which sides of the map item the grid frame
     * is drawn on.
     * @returns flags for side of map grid is drawn on
     * @see setFrameSideFlags
     * @see setFrameSideFlag
     * @see testFrameSideFlag
    */
    FrameSideFlags frameSideFlags() const;

    /**Tests whether the grid frame should be drawn on a specified side of the map
     * item.
     * @param flag flag for grid frame side
     * @returns true if grid frame should be drawn for that side of the map
     * @see setFrameSideFlags
     * @see setFrameSideFlag
     * @see frameSideFlags
    */
    bool testFrameSideFlag( const FrameSideFlag flag ) const;

    /**Sets the grid frame width. This property controls how wide the grid frame is.
     * The size of the line outlines drawn in the frame is controlled through the
     * setFramePenSize method.
     * @param width width of grid frame in millimetres
     * @see frameWidth
    */
    void setFrameWidth( const double width ) { mGridFrameWidth = width; }

    /**Gets the grid frame width. This property controls how wide the grid frame is.
     * The size of the line outlines drawn in the frame can be retrieved via the
     * framePenSize method.
     * @returns width of grid frame in millimetres
     * @see setFrameWidth
    */
    double frameWidth() const { return mGridFrameWidth; }

    /**Sets the width of the outline drawn in the grid frame.
     * @param width width of grid frame outline
     * @see framePenSize
     * @see setFramePenColor
    */
    void setFramePenSize( const double width ) { mGridFramePenThickness = width; }

    /**Retrieves the width of the outline drawn in the grid frame.
     * @returns width of grid frame outline
     * @see setFramePenSize
     * @see framePenColor
    */
    double framePenSize() const { return mGridFramePenThickness; }

    /**Sets the color of the outline drawn in the grid frame.
     * @param color color of grid frame outline
     * @see framePenColor
     * @see setFramePenSize
     * @see setFrameFillColor1
     * @see setFrameFillColor2
    */
    void setFramePenColor( const QColor& color ) { mGridFramePenColor = color; }

    /**Retrieves the color of the outline drawn in the grid frame.
     * @returns color of grid frame outline
     * @see setFramePenColor
     * @see framePenSize
     * @see frameFillColor1
     * @see frameFillColor2
    */
    QColor framePenColor() const {return mGridFramePenColor;}

    /**Sets the first fill color used for the grid frame.
     * @param color first fill color for grid frame
     * @see frameFillColor1
     * @see setFramePenColor
     * @see setFrameFillColor2
    */
    void setFrameFillColor1( const QColor& color ) { mGridFrameFillColor1 = color; }

    /**Retrieves the first fill color for the grid frame.
     * @returns first fill color for grid frame
     * @see setFrameFillColor1
     * @see framePenColor
     * @see frameFillColor2
    */
    QColor frameFillColor1() const { return mGridFrameFillColor1; }

    /**Sets the second fill color used for the grid frame.
     * @param color second fill color for grid frame
     * @see frameFillColor2
     * @see setFramePenColor
     * @see setFrameFillColor1
    */
    void setFrameFillColor2( const QColor& color ) { mGridFrameFillColor2 = color; }

    /**Retrieves the second fill color for the grid frame.
     * @returns second fill color for grid frame
     * @see setFrameFillColor2
     * @see framePenColor
     * @see frameFillColor1
    */
    QColor frameFillColor2() const { return mGridFrameFillColor2; }

  private:

    QgsComposerMapGrid(); //forbidden

    QgsComposerMap* mComposerMap;
    QString mName;
    QString mUuid;

    /**True if coordinate grid has to be displayed*/
    bool mGridEnabled;
    /**Solid or crosses*/
    GridStyle mGridStyle;
    /**Grid line interval in x-direction (map units)*/
    double mGridIntervalX;
    /**Grid line interval in y-direction (map units)*/
    double mGridIntervalY;
    /**Grid line offset in x-direction*/
    double mGridOffsetX;
    /**Grid line offset in y-direction*/
    double mGridOffsetY;
    /**Font for grid line annotation*/
    QFont mGridAnnotationFont;
    /**Font color for grid coordinates*/
    QColor mGridAnnotationFontColor;
    /**Digits after the dot*/
    int mGridAnnotationPrecision;
    /**True if coordinate values should be drawn*/
    bool mShowGridAnnotation;

    /**Annotation position for left map side (inside / outside / not shown)*/
    AnnotationPosition mLeftGridAnnotationPosition;
    /**Annotation position for right map side (inside / outside / not shown)*/
    AnnotationPosition mRightGridAnnotationPosition;
    /**Annotation position for top map side (inside / outside / not shown)*/
    AnnotationPosition mTopGridAnnotationPosition;
    /**Annotation position for bottom map side (inside / outside / not shown)*/
    AnnotationPosition mBottomGridAnnotationPosition;

    /**Distance between map frame and annotation*/
    double mAnnotationFrameDistance;

    /**Annotation direction on left side ( horizontal or vertical )*/
    AnnotationDirection mLeftGridAnnotationDirection;
    /**Annotation direction on right side ( horizontal or vertical )*/
    AnnotationDirection mRightGridAnnotationDirection;
    /**Annotation direction on top side ( horizontal or vertical )*/
    AnnotationDirection mTopGridAnnotationDirection;
    /**Annotation direction on bottom side ( horizontal or vertical )*/
    AnnotationDirection mBottomGridAnnotationDirection;
    AnnotationFormat mGridAnnotationFormat;
    FrameStyle mGridFrameStyle;
    FrameSideFlags mGridFrameSides;
    double mGridFrameWidth;
    double mGridFramePenThickness;
    QColor mGridFramePenColor;
    QColor mGridFrameFillColor1;
    QColor mGridFrameFillColor2;
    double mCrossLength;

    QgsLineSymbolV2* mGridLineSymbol;
    QgsMarkerSymbolV2* mGridMarkerSymbol;

    QgsCoordinateReferenceSystem mCRS;

    GridUnit mGridUnit;

    QPainter::CompositionMode mBlendMode;

    /**Draws the map grid*/
    void drawGridFrame( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines ) const;

    /**Draw coordinates for mGridAnnotationType Coordinate
        @param p drawing painter
        @param hLines horizontal coordinate lines in item coordinates
        @param vLines vertical coordinate lines in item coordinates*/
    void drawCoordinateAnnotations( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines ) const;

    void drawCoordinateAnnotation( QPainter* p, const QPointF& pos, QString annotationString ) const;

    /**Draws a single annotation
        @param p drawing painter
        @param pos item coordinates where to draw
        @param rotation text rotation
        @param annotationText the text to draw*/
    void drawAnnotation( QPainter* p, const QPointF& pos, int rotation, const QString& annotationText ) const;

    QString gridAnnotationString( double value, AnnotationCoordinate coord ) const;

    /**Returns the grid lines with associated coordinate value
        @return 0 in case of success*/
    int xGridLines( QList< QPair< double, QLineF > >& lines ) const;

    /**Returns the grid lines for the y-coordinates. Not vertical in case of rotation
        @return 0 in case of success*/
    int yGridLines( QList< QPair< double, QLineF > >& lines ) const;

    int xGridLinesCRSTransform( const QgsRectangle& bbox, const QgsCoordinateTransform& t, QList< QPair< double, QPolygonF > >& lines ) const;

    int yGridLinesCRSTransform( const QgsRectangle& bbox, const QgsCoordinateTransform& t, QList< QPair< double, QPolygonF > >& lines ) const;

    void drawGridLine( const QLineF& line, QgsRenderContext &context ) const;

    void drawGridLine( const QPolygonF& line, QgsRenderContext &context ) const;

    void sortGridLinesOnBorders( const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines,  QMap< double, double >& leftFrameEntries,
                                 QMap< double, double >& rightFrameEntries, QMap< double, double >& topFrameEntries, QMap< double, double >& bottomFrameEntries ) const;

    void drawGridFrameBorder( QPainter* p, const QMap< double, double >& borderPos, BorderSide border ) const;

    /**Returns the item border of a point (in item coordinates)*/
    BorderSide borderForLineCoord( const QPointF& p ) const;

    /**Get parameters for drawing grid in CRS different to map CRS*/
    int crsGridParams( QgsRectangle& crsRect, QgsCoordinateTransform& inverseTransform ) const;

    static QPolygonF trimLineToMap( const QPolygonF& line, const QgsRectangle& rect );

    QPolygonF scalePolygon( const QPolygonF &polygon,  const double scale ) const;

    /**Draws grid if CRS is different to map CRS*/
    void drawGridCRSTransform( QgsRenderContext &context , double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
                               QList< QPair< double, QLineF > > &verticalLines ) const;

    void drawGridNoTransform( QgsRenderContext &context, double dotsPerMM, QList<QPair<double, QLineF> > &horizontalLines, QList<QPair<double, QLineF> > &verticalLines ) const;

    void createDefaultGridLineSymbol();

    void createDefaultGridMarkerSymbol();

    void drawGridMarker( const QPointF &point, QgsRenderContext &context ) const;

    void drawGridFrameZebraBorder( QPainter *p, const QMap<double, double> &borderPos, BorderSide border ) const;

    void drawGridFrameTicks( QPainter *p, const QMap<double, double> &borderPos, BorderSide border ) const;

    void drawGridFrameLineBorder( QPainter *p, BorderSide border ) const;

};

#endif // QGSCOMPOSERMAPGRID_H
