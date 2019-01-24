/***************************************************************************
                         qgslayoutitemmapgrid.h
                         ----------------------
    begin                : October 2017
    copyright            : (C) 2017 by Marco Hugentobler, Nyall Dawson
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

#ifndef QGSLAYOUTITEMMAPGRID_H
#define QGSLAYOUTITEMMAPGRID_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayoutitemmapitem.h"
#include "qgssymbol.h"
#include <QPainter>

class QgsCoordinateTransform;
class QgsLayoutItemMapGrid;
class QgsLayoutItemMap;
class QDomDocument;
class QDomElement;
class QPainter;
class QgsRenderContext;

/**
 * \ingroup core
 * \class QgsLayoutItemMapGridStack
 * \brief A collection of grids which is drawn above the map content in a
 * QgsLayoutItemMap. The grid stack controls which grids are drawn and the
 * order they are drawn in.
 * \see QgsLayoutItemMapGrid
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMapGridStack : public QgsLayoutItemMapItemStack
{
  public:

    /**
     * Constructor for QgsLayoutItemMapGridStack, attached to the specified \a map.
     */
    QgsLayoutItemMapGridStack( QgsLayoutItemMap *map );

    /**
     * Adds a new map \a grid to the stack and takes ownership of the grid.
     * The grid will be added to the end of the stack, and rendered
     * above any existing map grids already present in the stack.
     * \note After adding a grid to the stack, updateBoundingRect() and update()
     * should be called for the QgsLayoutItemMap to prevent rendering artifacts.
     * \see removeGrid()
     */
    void addGrid( QgsLayoutItemMapGrid *grid SIP_TRANSFER );

    /**
     * Removes a grid with matching \a gridId from the stack and deletes the corresponding QgsLayoutItemMapGrid.
     * \note After removing a grid from the stack, updateBoundingRect() and update()
     * should be called for the QgsLayoutItemMap to prevent rendering artifacts.
     * \see addGrid()
     */
    void removeGrid( const QString &gridId );

    /**
     * Moves a grid with matching \a gridId up the stack, causing it to be rendered above other grids.
     * \note After moving a grid within the stack, update() should be
     * called for the QgsLayoutItemMap to redraw the map with the new grid stack order.
     * \see moveGridDown()
     */
    void moveGridUp( const QString &gridId );

    /**
     * Moves a grid with matching \a gridId down the stack, causing it to be rendered below other grids.
     * \note After moving a grid within the stack, update() should be
     * called for the QgsLayoutItemMap to redraw the map with the new grid stack order.
     * \see moveGridUp()
     */
    void moveGridDown( const QString &gridId );

    /**
     * Returns a reference to a grid with matching \a gridId within the stack.
     */
    QgsLayoutItemMapGrid *grid( const QString &gridId ) const;

    /**
     * Returns a reference to a grid at the specified \a index within the stack.
     */
    QgsLayoutItemMapGrid *grid( int index ) const;

    /**
     * Returns a reference to a grid at the specified \a index within the stack.
     * \see grid()
     */
    QgsLayoutItemMapGrid &operator[]( int index );

    /**
     * Returns a list of QgsLayoutItemMapGrids contained by the stack.
     */
    QList< QgsLayoutItemMapGrid * > asList() const;

    bool readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context ) override;

    /**
     * Calculates the maximum distance grids within the stack extend
     * beyond the QgsLayoutItemMap's item rect.
     * \see calculateMaxGridExtension()
     */
    double maxGridExtension() const;

    /**
     * Calculates the maximum distance grids within the stack extend beyond the
     * QgsLayoutItemMap's item rect. This method calculates the distance for each side of the
     * map item separately.
     * \see maxGridExtension()
     */
    void calculateMaxGridExtension( double &top, double &right, double &bottom, double &left ) const;
};

//
// QgsLayoutItemMapGrid
//

/**
 * \ingroup core
 * \class QgsLayoutItemMapGrid
 * \brief An individual grid which is drawn above the map content in a
 * QgsLayoutItemMap.
 * \see QgsLayoutItemMapGridStack
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMapGrid : public QgsLayoutItemMapItem
{

    Q_OBJECT

  public:

    /**
     * Unit for grid values
     */
    enum GridUnit
    {
      MapUnit, //!< Grid units follow map units
      MM, //!< Grid units in millimeters
      CM //!< Grid units in centimeters
    };

    /**
     * Grid drawing style
     */
    enum GridStyle
    {
      Solid = 0,
      Cross, //!< Draw line crosses at intersections of grid lines
      Markers, //!< Draw markers at intersections of grid lines
      FrameAnnotationsOnly //!< No grid lines over the map, only draw frame and annotations
    };

    /**
     * Display settings for grid annotations and frames
     */
    enum DisplayMode
    {
      ShowAll = 0, //!< Show both latitude and longitude annotations/divisions
      LatitudeOnly, //!< Show latitude/y annotations/divisions only
      LongitudeOnly, //!< Show longitude/x annotations/divisions only
      HideAll //!< No annotations
    };

    /**
     * Position for grid annotations
     */
    enum AnnotationPosition
    {
      InsideMapFrame = 0, //!< Draw annotations inside the map frame
      OutsideMapFrame, //!< Draw annotations outside the map frame
    };

    /**
     * Direction of grid annotations
     */
    enum AnnotationDirection
    {
      Horizontal = 0, //!< Draw annotations horizontally
      Vertical, //!< Draw annotations vertically, ascending
      VerticalDescending, //!< Draw annotations vertically, descending
      BoundaryDirection //!< Annotations follow the boundary direction
    };

    /**
     * Format for displaying grid annotations
     */
    enum AnnotationFormat
    {
      Decimal = 0, //!< Decimal degrees, use - for S/W coordinates
      DegreeMinute, //!< Degree/minutes, use NSEW suffix
      DegreeMinuteSecond, //!< Degree/minutes/seconds, use NSEW suffix
      DecimalWithSuffix, //!< Decimal degrees, use NSEW suffix
      DegreeMinuteNoSuffix, //!< Degree/minutes, use - for S/W coordinates
      DegreeMinutePadded, //!< Degree/minutes, with minutes using leading zeros where required
      DegreeMinuteSecondNoSuffix, //!< Degree/minutes/seconds, use - for S/W coordinates
      DegreeMinuteSecondPadded, //!< Degree/minutes/seconds, with minutes using leading zeros where required
      CustomFormat //!< Custom expression-based format
    };

    /**
     * Border sides for annotations
     */
    enum BorderSide
    {
      Left, //!< Left border
      Right, //!< Right border
      Bottom, //!< Bottom border
      Top //!< Top border
    };

    /**
     * Style for grid frame
     */
    enum FrameStyle
    {
      NoFrame = 0, //!< Disable grid frame
      Zebra, //!< Black/white pattern
      InteriorTicks,  //!< Tick markers drawn inside map frame
      ExteriorTicks,  //!< Tick markers drawn outside map frame
      InteriorExteriorTicks, //!< Tick markers drawn both inside and outside the map frame
      LineBorder, //!< Simple solid line frame
      LineBorderNautical, //!< Simple solid line frame, with nautical style diagonals on corners
      ZebraNautical, //!< Black/white pattern, with nautical style diagonals on corners
    };

    /**
     * Flags for controlling which side of the map a frame is drawn on
     */
    enum FrameSideFlag
    {
      FrameLeft = 0x01, //!< Left side of map
      FrameRight = 0x02, //!< Right side of map
      FrameTop = 0x04, //!< Top side of map
      FrameBottom = 0x08 //!< Bottom side of map
    };
    Q_DECLARE_FLAGS( FrameSideFlags, FrameSideFlag )

    /**
     * Annotation coordinate type
     */
    enum AnnotationCoordinate
    {
      Longitude = 0, //!< Coordinate is a longitude value
      Latitude //!< Coordinate is a latitude value
    };

    /**
     * Constructor for QgsLayoutItemMapGrid.
     * \param name friendly display name for grid
     * \param map QgsLayoutItemMap the grid is attached to
     */
    QgsLayoutItemMapGrid( const QString &name, QgsLayoutItemMap *map );

    void draw( QPainter *painter ) override;
    bool writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context ) override;

    /**
     * Sets the \a crs for the grid.
     * \see crs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Retrieves the CRS for the grid.
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const { return mCRS; }

    /**
     * Sets the blending \a mode used for drawing the grid.
     * \see blendMode()
     */
    void setBlendMode( const QPainter::CompositionMode mode ) { mBlendMode = mode; }

    /**
     * Retrieves the blending mode used for drawing the grid.
     * \see setBlendMode()
     */
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    bool usesAdvancedEffects() const override;

    /**
     * Calculates the maximum distance the grid extends beyond the QgsLayoutItemMap's
     * item rect (in layout units).
     */
    double maxExtension() const;

    /**
     * Calculates the maximum distance the grid extends beyond the
     * QgsLayoutItemMap's item rect. This method calculates the distance for each side of the
     * map item separately.
     * \see maxExtension()
     */
    void calculateMaxExtension( double &top, double &right, double &bottom, double &left ) const;

    //
    // GRID UNITS
    //

    /**
     * Sets the \a unit to use for grid measurements such as the interval
     * and offset for grid lines.
     * \see units
     */
    void setUnits( GridUnit unit );

    /**
     * Returns the units used for grid measurements such as the interval
     * and offset for grid lines.
     * \see setUnits()
     */
    GridUnit units() const { return mGridUnit; }

    /**
     * Sets the \a interval between grid lines in the x-direction. The units
     * are controlled through the setUnits method
     * \see setIntervalY()
     * \see intervalX()
     */
    void setIntervalX( double interval );

    /**
     * Returns the interval between grid lines in the x-direction. The units
     * are retrieved through the units() method.
     * \see setIntervalX()
     * \see intervalY()
     */
    double intervalX() const { return mGridIntervalX; }

    /**
     * Sets the \a interval between grid lines in the y-direction. The units
     * are controlled through the setUnits method
     * \see setIntervalX()
     * \see intervalY()
     */
    void setIntervalY( double interval );

    /**
     * Returns the interval between grid lines in the y-direction. The units
     * are retrieved through the units() method.
     * \see setIntervalY()
     * \see intervalX()
     */
    double intervalY() const { return mGridIntervalY; }

    /**
     * Sets the \a offset for grid lines in the x-direction. The units
     * are controlled through the setUnits method.
     * \see setOffsetY()
     * \see offsetX()
     */
    void setOffsetX( double offset );

    /**
     * Returns the offset for grid lines in the x-direction. The units
     * are retrieved through the units() method.
     * \see setOffsetX()
     * \see offsetY()
     */
    double offsetX() const { return mGridOffsetX; }

    /**
     * Sets the \a offset for grid lines in the y-direction. The units
     * are controlled through the setUnits method.
     * \see setOffsetX()
     * \see offsetY()
     */
    void setOffsetY( double offset );

    /**
     * Returns the offset for grid lines in the y-direction. The units
     * are retrieved through the units() method.
     * \see setOffsetY()
     * \see offsetX()
     */
    double offsetY() const { return mGridOffsetY; }

    //
    // GRID APPEARANCE
    //

    /**
     * Sets the grid \a style, which controls how the grid is drawn
     * over the map's contents.
     * \see style()
     */
    void setStyle( GridStyle style );

    /**
     * Returns the grid's style, which controls how the grid is drawn
     * over the map's contents.
     * \see setStyle()
     */
    GridStyle style() const { return mGridStyle; }

    /**
     * Sets the \a length (in layout units) of the cross segments drawn for the grid. This is only used for grids
     * with QgsLayoutItemMapGrid::Cross styles.
     * \see crossLength()
     */
    void setCrossLength( const double length ) { mCrossLength = length; }

    /**
     * Retrieves the length (in layout units) of the cross segments drawn for the grid. This is only used for grids
     * with QgsLayoutItemMapGrid::Cross styles.
     * \see setCrossLength()
     */
    double crossLength() const { return mCrossLength; }

    /**
     * Sets the \a width of grid lines (in layout units). This is only used for grids with QgsLayoutItemMapGrid::Solid
     * or QgsLayoutItemMapGrid::Cross styles. For more control over grid line appearance, use
     * setLineSymbol instead.
     * \see setLineSymbol()
     * \see setGridLineColor()
     */
    void setGridLineWidth( double width );

    /**
     * Sets the \a color of grid lines. This is only used for grids with QgsLayoutItemMapGrid::Solid
     * or QgsLayoutItemMapGrid::Cross styles. For more control over grid line appearance, use
     * setLineSymbol instead.
     * \see setLineSymbol()
     * \see setGridLineWidth()
     */
    void setGridLineColor( const QColor &color );

    /**
     * Sets the line \a symbol used for drawing grid lines. This is only used for grids with
     * QgsLayoutItemMapGrid::Solid or QgsLayoutItemMapGrid::Cross styles.
     * Ownership of \a symbol is transferred to the grid.
     * \see lineSymbol()
     * \see setMarkerSymbol()
     * \see setStyle()
     */
    void setLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the line symbol used for drawing grid lines. This is only used for grids with
     * QgsLayoutItemMapGrid::Solid or QgsLayoutItemMapGrid::Cross styles.
     * \see setLineSymbol()
     * \see markerSymbol()
     * \see style()
     * \note not available in Python bindings
     */
    const QgsLineSymbol *lineSymbol() const; SIP_SKIP

    /**
     * Returns the line symbol used for drawing grid lines. This is only used for grids with
     * QgsLayoutItemMapGrid::Solid or QgsLayoutItemMapGrid::Cross styles.
     * \see setLineSymbol()
     * \see markerSymbol()
     * \see style()
     */
    QgsLineSymbol *lineSymbol();

    /**
     * Sets the marker \a symbol used for drawing grid points. This is only used for grids with a
     * QgsLayoutItemMapGrid::Markers style.
     * Ownership of \a symbol is transferred to the grid.
     * \see markerSymbol()
     * \see setLineSymbol()
     * \see setStyle()
     */
    void setMarkerSymbol( QgsMarkerSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the marker symbol used for drawing grid points. This is only used for grids with a
     * QgsLayoutItemMapGrid::Markers style.
     * \see setMarkerSymbol()
     * \see lineSymbol()
     * \see style()
     * \note not available in Python bindings
     */
    const QgsMarkerSymbol *markerSymbol() const; SIP_SKIP

    /**
     * Returns the marker symbol used for drawing grid points. This is only used for grids with a
     * QgsLayoutItemMapGrid::Markers style.
     * \see setMarkerSymbol()
     * \see lineSymbol()
     * \see style()
     */
    QgsMarkerSymbol *markerSymbol();

    //
    // ANNOTATIONS
    //

    /**
     * Sets whether annotations should be shown for the grid.
     * \see annotationEnabled()
     */
    void setAnnotationEnabled( const bool enabled ) { mShowGridAnnotation = enabled; }

    /**
     * Returns whether annotations are shown for the grid.
     * \see setAnnotationEnabled()
     */
    bool annotationEnabled() const { return mShowGridAnnotation; }

    /**
     * Sets the \a font used for drawing grid annotations.
     * \see annotationFont()
     */
    void setAnnotationFont( const QFont &font ) { mGridAnnotationFont = font; }

    /**
     * Returns the font used for drawing grid annotations.
     * \see setAnnotationFont()
     */
    QFont annotationFont() const { return mGridAnnotationFont; }

    /**
     * Sets the font \a color used for drawing grid annotations.
     * \see annotationFontColor()
     */
    void setAnnotationFontColor( const QColor &color ) { mGridAnnotationFontColor = color; }

    /**
     * Returns the font color used for drawing grid annotations.
     * \see setAnnotationFontColor()
     */
    QColor annotationFontColor() const { return mGridAnnotationFontColor; }

    /**
     * Sets the coordinate \a precision for grid annotations.
     * The \a precision indicates the number of decimal places to show when drawing grid annotations.
     * \see annotationPrecision()
     */
    void setAnnotationPrecision( const int precision ) { mGridAnnotationPrecision = precision; }

    /**
     * Returns the coordinate precision for grid annotations, which is the
     * number of decimal places shown when drawing grid annotations.
     * \see setAnnotationPrecision()
     */
    int annotationPrecision() const { return mGridAnnotationPrecision; }

    /**
     * Sets what types of grid annotations should be drawn for a specified side of the map frame,
     * or whether grid annotations should be disabled for the side.
     * \param display display mode for annotations
     * \param border side of map for annotations
     * \see annotationDisplay()
     */
    void setAnnotationDisplay( DisplayMode display, BorderSide border );

    /**
     * Returns the display mode for the grid annotations on a specified side of the map
     * frame. This property also specifies whether annotations have been disabled
     * from a side of the map frame.
     * \param border side of map for annotations
     * \returns display mode for grid annotations
     * \see setAnnotationDisplay()
     */
    DisplayMode annotationDisplay( BorderSide border ) const;

    /**
     * Sets the \a position for the grid annotations on a specified \a side of the map
     * frame.
     * \see annotationPosition()
     */
    void setAnnotationPosition( AnnotationPosition position, BorderSide side );

    /**
     * Returns the position for the grid annotations on a specified \a side of the map
     * frame.
     * \see setAnnotationPosition()
     */
    AnnotationPosition annotationPosition( BorderSide side ) const;

    /**
     * Sets the \a distance between the map frame and annotations. Units are layout units.
     * \see annotationFrameDistance()
     */
    void setAnnotationFrameDistance( const double distance ) { mAnnotationFrameDistance = distance; }

    /**
     * Returns the distance between the map frame and annotations. Units are in layout units.
     * \see setAnnotationFrameDistance()
     */
    double annotationFrameDistance() const { return mAnnotationFrameDistance; }

    /**
     * Sets the \a direction for drawing frame annotations for the specified map \a side.
     * \see annotationDirection()
     */
    void setAnnotationDirection( AnnotationDirection direction, BorderSide side );

    /**
     * Sets the \a direction for drawing all frame annotations.
     * \see annotationDirection()
     */
    void setAnnotationDirection( AnnotationDirection direction );

    /**
     * Returns the direction for drawing frame annotations, on the specified \a side
     * of the map.
     * \see setAnnotationDirection()
     */
    AnnotationDirection annotationDirection( BorderSide border ) const;

    /**
     * Sets the \a format for drawing grid annotations.
     * \see annotationFormat()
     */
    void setAnnotationFormat( const AnnotationFormat format ) { mGridAnnotationFormat = format; }

    /**
     * Returns the format for drawing grid annotations.
     * \see setAnnotationFormat()
     */
    AnnotationFormat annotationFormat() const { return mGridAnnotationFormat; }

    /**
     * Sets the \a expression used for drawing grid annotations. This is only used when annotationFormat()
     * is QgsLayoutItemMapGrid::CustomFormat.
     * \see annotationExpression()
     */
    void setAnnotationExpression( const QString &expression ) { mGridAnnotationExpressionString = expression; mGridAnnotationExpression.reset(); }

    /**
     * Returns the expression used for drawing grid annotations. This is only used when annotationFormat()
     * is QgsLayoutItemMapGrid::CustomFormat.
     * \see setAnnotationExpression()
     */
    QString annotationExpression() const { return mGridAnnotationExpressionString; }

    //
    // GRID FRAME
    //

    /**
     * Sets the grid frame \a style.
     * \see frameStyle()
     */
    void setFrameStyle( const FrameStyle style ) { mGridFrameStyle = style; }

    /**
     * Returns the grid frame style.
     * \see setFrameStyle()
     */
    FrameStyle frameStyle() const { return mGridFrameStyle; }

    /**
     * Sets what type of grid \a divisions should be used for frames on a specified \a side of the map.
     * \see frameDivisions()
     */
    void setFrameDivisions( DisplayMode divisions, BorderSide side );

    /**
     * Returns the type of grid divisions which are used for frames on a specified \a side of the map.
     * \see setFrameDivisions()
     */
    DisplayMode frameDivisions( BorderSide side ) const;

    /**
     * Sets \a flags for grid frame sides. Setting these flags controls which sides
     * of the map item the grid frame is drawn on.
     * \see setFrameSideFlag()
     * \see frameSideFlags()
     * \see testFrameSideFlag()
     */
    void setFrameSideFlags( QgsLayoutItemMapGrid::FrameSideFlags flags );

    /**
     * Sets whether the grid frame is drawn for a certain side of the map item.
     * \param flag flag for grid frame side
     * \param on set to true to draw grid frame on that side of the map
     * \see setFrameSideFlags()
     * \see frameSideFlags()
     * \see testFrameSideFlag()
     */
    void setFrameSideFlag( QgsLayoutItemMapGrid::FrameSideFlag flag, bool on = true );

    /**
     * Returns the flags which control which sides of the map item the grid frame
     * is drawn on.
     * \see setFrameSideFlags()
     * \see setFrameSideFlag()
     * \see testFrameSideFlag()
     */
    QgsLayoutItemMapGrid::FrameSideFlags frameSideFlags() const;

    /**
     * Tests whether the grid frame should be drawn on a specified side of the map
     * item.
     * \param flag flag for grid frame side
     * \returns true if grid frame should be drawn for that side of the map
     * \see setFrameSideFlags()
     * \see setFrameSideFlag()
     * \see frameSideFlags()
     */
    bool testFrameSideFlag( FrameSideFlag flag ) const;

    /**
     * Sets the grid frame \a width (in layout units). This property controls how wide the grid frame is.
     * The size of the line outlines drawn in the frame is controlled through the
     * setFramePenSize method.
     * \see frameWidth()
     */
    void setFrameWidth( const double width ) { mGridFrameWidth = width; }

    /**
     * Gets the grid frame width in layout units. This property controls how wide the grid frame is.
     * The size of the line outlines drawn in the frame can be retrieved via the
     * framePenSize method.
     * \see setFrameWidth()
     */
    double frameWidth() const { return mGridFrameWidth; }

    /**
     * Sets the grid frame margin (in layout units).
     * This property controls distance between the map frame and the grid frame.
     * \see frameMargin()
     * \since QGIS 3.6
     */
    void setFrameMargin( const double margin ) { mGridFrameMargin = margin; }

    /**
     * Sets the grid frame Margin (in layout units).
     * This property controls distance between the map frame and the grid frame.
     * \see setFrameMargin()
     * \since QGIS 3.6
     */
    double frameMargin() const { return mGridFrameMargin; }

    /**
     * Sets the \a width of the stroke drawn in the grid frame.
     * \see framePenSize()
     * \see setFramePenColor()
     */
    void setFramePenSize( const double width ) { mGridFramePenThickness = width; }

    /**
     * Retrieves the width of the stroke drawn in the grid frame.
     * \see setFramePenSize()
     * \see framePenColor()
     */
    double framePenSize() const { return mGridFramePenThickness; }

    /**
     * Sets the \a color of the stroke drawn in the grid frame.
     * \see framePenColor()
     * \see setFramePenSize()
     * \see setFrameFillColor1()
     * \see setFrameFillColor2()
     */
    void setFramePenColor( const QColor &color ) { mGridFramePenColor = color; }

    /**
     * Retrieves the color of the stroke drawn in the grid frame.
     * \see setFramePenColor()
     * \see framePenSize()
     * \see frameFillColor1()
     * \see frameFillColor2()
     */
    QColor framePenColor() const {return mGridFramePenColor;}

    /**
     * Sets the first fill \a color used for the grid frame.
     * \see frameFillColor1()
     * \see setFramePenColor()
     * \see setFrameFillColor2()
     */
    void setFrameFillColor1( const QColor &color ) { mGridFrameFillColor1 = color; }

    /**
     * Retrieves the first fill color for the grid frame.
     * \see setFrameFillColor1()
     * \see framePenColor()
     * \see frameFillColor2()
     */
    QColor frameFillColor1() const { return mGridFrameFillColor1; }

    /**
     * Sets the second fill \a color used for the grid frame.
     * \see frameFillColor2()
     * \see setFramePenColor()
     * \see setFrameFillColor1()
     */
    void setFrameFillColor2( const QColor &color ) { mGridFrameFillColor2 = color; }

    /**
     * Retrieves the second fill color for the grid frame.
     * \see setFrameFillColor2()
     * \see framePenColor(
     * \see frameFillColor1()
     */
    QColor frameFillColor2() const { return mGridFrameFillColor2; }

    QgsExpressionContext createExpressionContext() const override;

  private:

    QgsLayoutItemMapGrid() = delete;

    struct GridExtension
    {
      GridExtension() = default;
      double top = 0.0;
      double right = 0.0;
      double bottom = 0.0;
      double left = 0.0;
    };

    //! True if a re-transformation of grid lines is required
    mutable bool mTransformDirty = true;

    //! Solid or crosses
    GridStyle mGridStyle = QgsLayoutItemMapGrid::Solid;
    //! Grid line interval in x-direction (map units)
    double mGridIntervalX = 0.0;
    //! Grid line interval in y-direction (map units)
    double mGridIntervalY = 0.0;
    //! Grid line offset in x-direction
    double mGridOffsetX = 0.0;
    //! Grid line offset in y-direction
    double mGridOffsetY = 0.0;
    //! Font for grid line annotation
    QFont mGridAnnotationFont;
    //! Font color for grid coordinates
    QColor mGridAnnotationFontColor  = Qt::black;
    //! Digits after the dot
    int mGridAnnotationPrecision = 3;
    //! True if coordinate values should be drawn
    bool mShowGridAnnotation = false;

    //! Annotation display mode for left map side
    DisplayMode mLeftGridAnnotationDisplay = QgsLayoutItemMapGrid::ShowAll;
    //! Annotation display mode for right map side
    DisplayMode mRightGridAnnotationDisplay = QgsLayoutItemMapGrid::ShowAll;
    //! Annotation display mode for top map side
    DisplayMode mTopGridAnnotationDisplay = QgsLayoutItemMapGrid::ShowAll;
    //! Annotation display mode for bottom map side
    DisplayMode mBottomGridAnnotationDisplay = QgsLayoutItemMapGrid::ShowAll;

    //! Annotation position for left map side (inside / outside)
    AnnotationPosition mLeftGridAnnotationPosition = QgsLayoutItemMapGrid::OutsideMapFrame;
    //! Annotation position for right map side (inside / outside)
    AnnotationPosition mRightGridAnnotationPosition = QgsLayoutItemMapGrid::OutsideMapFrame;
    //! Annotation position for top map side (inside / outside)
    AnnotationPosition mTopGridAnnotationPosition = QgsLayoutItemMapGrid::OutsideMapFrame;
    //! Annotation position for bottom map side (inside / outside)
    AnnotationPosition mBottomGridAnnotationPosition = QgsLayoutItemMapGrid::OutsideMapFrame;

    //! Distance between map frame and annotation
    double mAnnotationFrameDistance = 1.0;

    //! Annotation direction on left side ( horizontal or vertical )
    AnnotationDirection mLeftGridAnnotationDirection = QgsLayoutItemMapGrid::Horizontal;
    //! Annotation direction on right side ( horizontal or vertical )
    AnnotationDirection mRightGridAnnotationDirection = QgsLayoutItemMapGrid::Horizontal;
    //! Annotation direction on top side ( horizontal or vertical )
    AnnotationDirection mTopGridAnnotationDirection = QgsLayoutItemMapGrid::Horizontal;
    //! Annotation direction on bottom side ( horizontal or vertical )
    AnnotationDirection mBottomGridAnnotationDirection = QgsLayoutItemMapGrid::Horizontal;
    AnnotationFormat mGridAnnotationFormat = QgsLayoutItemMapGrid::Decimal;

    QString mGridAnnotationExpressionString;
    mutable std::unique_ptr< QgsExpression > mGridAnnotationExpression;

    FrameStyle mGridFrameStyle = QgsLayoutItemMapGrid::NoFrame;
    FrameSideFlags mGridFrameSides;
    double mGridFrameWidth = 2.0;
    double mGridFramePenThickness = 0.3;
    QColor mGridFramePenColor = QColor( 0, 0, 0 );
    QColor mGridFrameFillColor1 = Qt::white;
    QColor mGridFrameFillColor2 = Qt::black;
    double mCrossLength = 3.0;
    double mGridFrameMargin = 0.0;

    //! Divisions for frame on left map side
    DisplayMode mLeftFrameDivisions = QgsLayoutItemMapGrid::ShowAll;
    //! Divisions for frame on right map side
    DisplayMode mRightFrameDivisions = QgsLayoutItemMapGrid::ShowAll;
    //! Divisions for frame on top map side
    DisplayMode mTopFrameDivisions = QgsLayoutItemMapGrid::ShowAll;
    //! Divisions for frame on bottom map side
    DisplayMode mBottomFrameDivisions = QgsLayoutItemMapGrid::ShowAll;

    std::unique_ptr< QgsLineSymbol > mGridLineSymbol;
    std::unique_ptr< QgsMarkerSymbol > mGridMarkerSymbol;

    QgsCoordinateReferenceSystem mCRS;

    GridUnit mGridUnit = MapUnit;

    QPainter::CompositionMode mBlendMode = QPainter::CompositionMode_SourceOver;

    mutable QList< QPair< double, QPolygonF > > mTransformedXLines;
    mutable QList< QPair< double, QPolygonF > > mTransformedYLines;
    mutable QList< QgsPointXY > mTransformedIntersections;
    QRectF mPrevPaintRect;
    mutable QPolygonF mPrevMapPolygon;

    class QgsMapAnnotation
    {
      public:
        double coordinate;
        QPointF itemPosition;
        QgsLayoutItemMapGrid::AnnotationCoordinate coordinateType;
    };

    /**
     * Draws the map grid. If extension is specified, then no grid will be drawn and instead the maximum extension
     * for the grid outside of the map frame will be calculated.
     */
    void drawGridFrame( QPainter *p, const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, GridExtension *extension = nullptr ) const;

    /**
     * Draw coordinates for mGridAnnotationType Coordinate
        \param p drawing painter
        \param hLines horizontal coordinate lines in item coordinates
        \param vLines vertical coordinate lines in item coordinates
        \param expressionContext expression context for evaluating custom annotation formats
        \param extension optional. If specified, nothing will be drawn and instead the maximum extension for the grid
        annotations will be stored in this variable.
     */
    void drawCoordinateAnnotations( QPainter *p, const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, QgsExpressionContext &expressionContext, GridExtension *extension = nullptr ) const;

    /**
     * Draw an annotation. If optional extension argument is specified, nothing will be drawn and instead
     * the extension of the annotation outside of the map frame will be stored in this variable.
     */
    void drawCoordinateAnnotation( QPainter *p, QPointF pos, const QString &annotationString, AnnotationCoordinate coordinateType, GridExtension *extension = nullptr ) const;

    /**
     * Draws a single annotation
     * \param p drawing painter
     * \param pos item coordinates where to draw
     * \param rotation text rotation
     * \param annotationText the text to draw
     */
    void drawAnnotation( QPainter *p, QPointF pos, int rotation, const QString &annotationText ) const;

    QString gridAnnotationString( double value, AnnotationCoordinate coord, QgsExpressionContext &expressionContext ) const;

    /**
     * Returns the grid lines with associated coordinate value
        \returns 0 in case of success*/
    int xGridLines( QList< QPair< double, QLineF > > &lines ) const;

    /**
     * Returns the grid lines for the y-coordinates. Not vertical in case of rotation
        \returns 0 in case of success*/
    int yGridLines( QList< QPair< double, QLineF > > &lines ) const;

    int xGridLinesCrsTransform( const QgsRectangle &bbox, const QgsCoordinateTransform &t, QList< QPair< double, QPolygonF > > &lines ) const;

    int yGridLinesCrsTransform( const QgsRectangle &bbox, const QgsCoordinateTransform &t, QList< QPair< double, QPolygonF > > &lines ) const;

    void drawGridLine( const QLineF &line, QgsRenderContext &context ) const;

    void drawGridLine( const QPolygonF &line, QgsRenderContext &context ) const;

    void sortGridLinesOnBorders( const QList< QPair< double, QLineF > > &hLines, const QList< QPair< double, QLineF > > &vLines, QMap< double, double > &leftFrameEntries,
                                 QMap< double, double > &rightFrameEntries, QMap< double, double > &topFrameEntries, QMap< double, double > &bottomFrameEntries ) const;

    /**
     * Draw the grid frame's border. If optional extension argument is specified, nothing will be drawn and instead
     * the maximum extension of the frame border outside of the map frame will be stored in this variable.
     */
    void drawGridFrameBorder( QPainter *p, const QMap< double, double > &borderPos, BorderSide border, double *extension = nullptr ) const;

    /**
     * Returns the item border of a point (in item coordinates)
     * \param p point
     * \param coordinateType coordinate type
     */
    BorderSide borderForLineCoord( QPointF p, AnnotationCoordinate coordinateType ) const;

    //! Gets parameters for drawing grid in CRS different to map CRS
    int crsGridParams( QgsRectangle &crsRect, QgsCoordinateTransform &inverseTransform ) const;

    static QList<QPolygonF> trimLinesToMap( const QPolygonF &line, const QgsRectangle &rect );

    QPolygonF scalePolygon( const QPolygonF &polygon, double scale ) const;

    //! Draws grid if CRS is different to map CRS
    void drawGridCrsTransform( QgsRenderContext &context, double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
                               QList< QPair< double, QLineF > > &verticalLines, bool calculateLinesOnly = false ) const;

    void drawGridNoTransform( QgsRenderContext &context, double dotsPerMM, QList<QPair<double, QLineF> > &horizontalLines, QList<QPair<double, QLineF> > &verticalLines, bool calculateLinesOnly = false ) const;

    void createDefaultGridLineSymbol();

    void createDefaultGridMarkerSymbol();

    void drawGridMarker( QPointF point, QgsRenderContext &context ) const;

    void drawGridFrameZebraBorder( QPainter *p, const QMap<double, double> &borderPos, BorderSide border, double *extension = nullptr ) const;

    void drawGridFrameTicks( QPainter *p, const QMap<double, double> &borderPos, BorderSide border, double *extension = nullptr ) const;

    void drawGridFrameLineBorder( QPainter *p, BorderSide border, double *extension = nullptr ) const;

    void calculateCrsTransformLines() const;

    bool shouldShowDivisionForSide( AnnotationCoordinate coordinate, BorderSide side ) const;
    bool shouldShowDivisionForDisplayMode( AnnotationCoordinate coordinate, DisplayMode mode ) const;

    friend class TestQgsLayoutMapGrid;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLayoutItemMapGrid::FrameSideFlags )

#endif // QGSLAYOUTITEMMAPGRID_H
