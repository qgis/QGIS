/** \ingroup core
 * \class QgsComposerMapGridStack
 * \brief A collection of grids which is drawn above the map content in a
 * QgsComposerMap. The grid stack controls which grids are drawn and the
 * order they are drawn in.
 * \note added in QGIS 2.5
 * \see QgsComposerMapGrid
 */
class QgsComposerMapGridStack : QgsComposerMapItemStack
{
%TypeHeaderCode
#include <qgscomposermapgrid.h>
%End

  public:

    /** Constructor for QgsComposerMapGridStack.
     * @param map QgsComposerMap the grid stack is attached to
     */
    QgsComposerMapGridStack( QgsComposerMap* map );

    virtual ~QgsComposerMapGridStack();

    /** Adds a new map grid to the stack and takes ownership of the grid.
     * The grid will be added to the end of the stack, and rendered
     * above any existing map grids already present in the stack.
     * @param grid QgsComposerMapGrid to add to the stack
     * @note after adding a grid to the stack, updateBoundingRect() and update()
     * should be called for the QgsComposerMap to prevent rendering artifacts
     * @see removeGrid
     */
    void addGrid( QgsComposerMapGrid* grid /Transfer/ );

    /** Removes a grid from the stack and deletes the corresponding QgsComposerMapGrid
     * @param gridId id for the QgsComposerMapGrid to remove
     * @note after removing a grid from the stack, updateBoundingRect() and update()
     * should be called for the QgsComposerMap to prevent rendering artifacts
     * @see addGrid
     */
    void removeGrid( const QString& gridId );

    /** Moves a grid up the stack, causing it to be rendered above other grids
     * @param gridId id for the QgsComposerMapGrid to move up
     * @note after moving a grid within the stack, update() should be
     * called for the QgsComposerMap to redraw the map with the new grid stack order
     * @see moveGridDown
     */
    void moveGridUp( const QString& gridId );

    /** Moves a grid down the stack, causing it to be rendered below other grids
     * @param gridId id for the QgsComposerMapGrid to move down
     * @note after moving a grid within the stack, update() should be
     * called for the QgsComposerMap to redraw the map with the new grid stack order
     * @see moveGridUp
     */
    void moveGridDown( const QString& gridId );

    /** Returns a const reference to a grid within the stack
     * @param gridId id for the QgsComposerMapGrid to find
     * @returns const reference to grid, if found
     * @see grid
     */
    const QgsComposerMapGrid* constGrid( const QString& gridId ) const;

    /** Returns a reference to a grid within the stack
     * @param gridId id for the QgsComposerMapGrid to find
     * @returns reference to grid if found
     * @see constGrid
     */
    QgsComposerMapGrid* grid( const QString& gridId ) const;

    /** Returns a reference to a grid within the stack
     * @param index grid position in the stack
     * @returns reference to grid if found
     * @see constGrid
     */
    QgsComposerMapGrid* grid( const int index ) const;

    /** Returns a reference to a grid within the stack
     * @param idx grid position in the stack
     * @returns reference to grid if found
     * @see constGrid
     * @see grid
     */
    QgsComposerMapGrid &operator[]( int idx );

    /** Returns a list of QgsComposerMapGrids contained by the stack
     * @returns list of grids
     */
    QList< QgsComposerMapGrid* > asList() const;

    /** Sets the grid stack's state from a DOM document
     * @param elem is DOM node corresponding to 'a ComposerMap' tag
     * @param doc DOM document
     * @returns true if read was successful
     * @see writeXML
     */
    bool readXML( const QDomElement& elem, const QDomDocument& doc );

    /** Calculates the maximum distance grids within the stack extend
     * beyond the QgsComposerMap's item rect
     * @returns maximum grid extension
     * @see calculateMaxGridExtension()
     */
    double maxGridExtension() const;

    /** Calculates the maximum distance grids within the stack extend beyond the
     * QgsComposerMap's item rect. This method calculates the distance for each side of the
     * map item separately
     * @param top storage for top extension
     * @param right storage for right extension
     * @param bottom storage for bottom extension
     * @param left storage for left extension
     * @note added in QGIS 2.12
     * @see maxGridExtension()
     */
    void calculateMaxGridExtension( double& top, double& right, double& bottom, double& left ) const;
};

//
// QgsComposerMapGrid
//

/** \ingroup core
 * \class QgsComposerMapGrid
 * \brief An individual grid which is drawn above the map content in a
 * QgsComposerMap.
 * \note added in QGIS 2.5
 * \see QgsComposerMapGridStack
 */
class QgsComposerMapGrid : QgsComposerMapItem
{
%TypeHeaderCode
#include <qgscomposermapgrid.h>
%End

  public:

    /** Unit for grid values
     */
    enum GridUnit
    {
      MapUnit, /*!< grid units follow map units */
      MM, /*!< grid units in millimeters */
      CM /*!< grid units in centimeters */
    };

    /** Grid drawing style
     */
    enum GridStyle
    {
      Solid,
      Cross, /*!< draw line crosses at intersections of grid lines */
      Markers, /*!< draw markers at intersections of grid lines */
      FrameAnnotationsOnly /*!< no grid lines over the map, only draw frame and annotations */
    };

    /** Display settings for grid annotations and frames
     */
    enum DisplayMode
    {
      ShowAll = 0, /*!< show both latitude and longitude annotations/divisions */
      LatitudeOnly, /*!< show latitude/y annotations/divisions only */
      LongitudeOnly, /*!< show longitude/x annotations/divisions only */
      HideAll /*!< no annotations */
    };

    /** Position for grid annotations
     */
    enum AnnotationPosition
    {
      InsideMapFrame = 0,
      OutsideMapFrame, /*!< draw annotations outside the map frame */
      Disabled /*!< disable annotation
                  * @deprecated in QGIS 2.7, use QgsComposerMapGrid::HideAll instead */
    };

    /** Direction of grid annotations
     */
    enum AnnotationDirection
    {
      Horizontal = 0, /*!< draw annotations horizontally */
      Vertical, /*!< draw annotations vertically, ascending */
      VerticalDescending, /*!< draw annotations vertically, descending */
      BoundaryDirection /*!< annotations follow the boundary direction */
    };

    /** Format for displaying grid annotations
     */
    enum AnnotationFormat
    {
      Decimal, /*!< decimal degrees, use - for S/W coordinates */
      DegreeMinute, /*!< degree/minutes, use NSEW suffix */
      DegreeMinuteSecond, /*!< degree/minutes/seconds, use NSEW suffix */
      DecimalWithSuffix, /*!< decimal degrees, use NSEW suffix */
      DegreeMinuteNoSuffix, /*!< degree/minutes, use - for S/W coordinates */
      DegreeMinutePadded, /*!< degree/minutes, with minutes using leading zeros where required */
      DegreeMinuteSecondNoSuffix, /*!< degree/minutes/seconds, use - for S/W coordinates */
      DegreeMinuteSecondPadded, /*!< degree/minutes/seconds, with minutes using leading zeros where required */
      CustomFormat /*!< custom expression-based format */
    };

    /** Border sides for annotations
     */
    enum BorderSide
    {
      Left,
      Right, /*!< right border */
      Bottom, /*!< bottom border */
      Top /*!< top border */
    };

    /** Style for grid frame
     */
    enum FrameStyle
    {
      NoFrame, /*!< disable grid frame */
      Zebra, /*!< black/white pattern */
      InteriorTicks,  /*!< tick markers drawn inside map frame */
      ExteriorTicks,  /*!< tick markers drawn outside map frame */
      InteriorExteriorTicks, /*!< tick markers drawn both inside and outside the map frame */
      LineBorder /*!< simple solid line frame */
    };

    /** Flags for controlling which side of the map a frame is drawn on
     */
    enum FrameSideFlag
    {
      FrameLeft, /*!< left side of map */
      FrameRight, /*!< right side of map */
      FrameTop, /*!< top side of map */
      FrameBottom /*!< bottom side of map */
    };
    typedef QFlags<QgsComposerMapGrid::FrameSideFlag> FrameSideFlags;

    /** Annotation coordinate type
     */
    enum AnnotationCoordinate
    {
      Longitude, /*!< coordinate is a longitude value */
      Latitude /*!< coordinate is a latitude value */
    };

    /** Constructor for QgsComposerMapGrid.
     * @param name friendly display name for grid
     * @param map QgsComposerMap the grid is attached to
     */
    QgsComposerMapGrid( const QString& name, QgsComposerMap* map );

    virtual ~QgsComposerMapGrid();

    /** Draws a grid
     * @param painter destination QPainter
     */
    void draw( QPainter* painter );

    /** Stores grid state in DOM element
     * @param elem is DOM element corresponding to a 'ComposerMap' tag
     * @param doc DOM document
     * @see readXML
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** Sets grid state from a DOM document
     * @param itemElem is DOM node corresponding to a 'ComposerMapGrid' tag
     * @param doc is DOM document
     * @see writeXML
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    /** Sets the CRS for the grid.
     * @param crs coordinate reference system for grid
     * @see crs
     */
    void setCrs( const QgsCoordinateReferenceSystem& crs );

    /** Retrieves the CRS for the grid.
     * @returns coordinate reference system for grid
     * @see setCrs
     */
    QgsCoordinateReferenceSystem crs() const;

    /** Sets the blending mode used for drawing the grid.
     * @param mode blending mode for grid
     * @see blendMode
     */
    void setBlendMode( const QPainter::CompositionMode mode );

    /** Retrieves the blending mode used for drawing the grid.
     * @returns blending mode for grid
     * @see setBlendMode
     */
    QPainter::CompositionMode blendMode() const;

    bool usesAdvancedEffects() const;

    /** Calculates the maximum distance the grid extends beyond the QgsComposerMap's
     * item rect
     * @returns maximum extension in millimeters
     */
    double maxExtension();

    /** Calculates the maximum distance the grid extends beyond the
     * QgsComposerMap's item rect. This method calculates the distance for each side of the
     * map item separately
     * @param top storage for top extension
     * @param right storage for right extension
     * @param bottom storage for bottom extension
     * @param left storage for left extension
     * @note added in QGIS 2.12
     * @see maxExtension()
     */
    void calculateMaxExtension( double& top, double& right, double& bottom, double& left );

    //
    // GRID UNITS
    //

    /** Sets the units to use for grid measurements such as the interval
     * and offset for grid lines.
     * @param unit unit for grid measurements
     * @see units
     */
    void setUnits( const GridUnit unit );

    /** Gets the units used for grid measurements such as the interval
     * and offset for grid lines.
     * @returns for grid measurements
     * @see setUnits
     */
    GridUnit units() const;

    /** Sets the interval between grid lines in the x-direction. The units
     * are controlled through the setUnits method
     * @param interval interval between horizontal grid lines
     * @see setIntervalY
     * @see intervalX
     */
    void setIntervalX( const double interval );

    /** Gets the interval between grid lines in the x-direction. The units
     * are retrieved through the units() method.
     * @returns interval between horizontal grid lines
     * @see setIntervalX
     * @see intervalY
     */
    double intervalX() const;

    /** Sets the interval between grid lines in the y-direction. The units
     * are controlled through the setUnits method
     * @param interval interval between vertical grid lines
     * @see setIntervalX
     * @see intervalY
     */
    void setIntervalY( const double interval );

    /** Gets the interval between grid lines in the y-direction. The units
     * are retrieved through the units() method.
     * @returns interval between vertical grid lines
     * @see setIntervalY
     * @see intervalX
     */
    double intervalY() const;

    /** Sets the offset for grid lines in the x-direction. The units
     * are controlled through the setUnits method
     * @param offset offset for horizontal grid lines
     * @see setOffsetY
     * @see offsetX
     */
    void setOffsetX( const double offset );

    /** Gets the offset for grid lines in the x-direction. The units
     * are retrieved through the units() method.
     * @returns offset for horizontal grid lines
     * @see setOffsetX
     * @see offsetY
     */
    double offsetX() const;

    /** Sets the offset for grid lines in the y-direction. The units
     * are controlled through the setUnits method
     * @param offset offset for vertical grid lines
     * @see setOffsetX
     * @see offsetY
     */
    void setOffsetY( const double offset );

    /** Gets the offset for grid lines in the y-direction. The units
     * are retrieved through the units() method.
     * @returns offset for vertical grid lines
     * @see setOffsetY
     * @see offsetX
     */
    double offsetY() const;

    //
    // GRID APPEARANCE
    //

    /** Sets the grid style, which controls how the grid is drawn
     * over the map's contents
     * @param style desired grid style
     * @see style
     */
    void setStyle( const GridStyle style );

    /** Gets the grid's style, which controls how the grid is drawn
     * over the map's contents
     * @returns current grid style
     * @see setStyle
     */
    GridStyle style() const;

    /** Sets the length of the cross segments drawn for the grid. This is only used for grids
     * with QgsComposerMapGrid::Cross styles
     * @param length cross length in millimeters
     * @see crossLength
     */
    void setCrossLength( const double length );

    /** Retrieves the length of the cross segments drawn for the grid. This is only used for grids
     * with QgsComposerMapGrid::Cross styles
     * @returns cross length in millimeters
     * @see setCrossLength
     */
    double crossLength() const;

    /** Sets width of grid lines. This is only used for grids with QgsComposerMapGrid::Solid
     * or QgsComposerMapGrid::Cross styles. For more control over grid line appearance, use
     * setLineSymbol instead.
     * @param width grid line width
     * @see setLineSymbol
     * @see setGridLineColor
     */
    void setGridLineWidth( const double width );

    /** Sets color of grid lines. This is only used for grids with QgsComposerMapGrid::Solid
     * or QgsComposerMapGrid::Cross styles. For more control over grid line appearance, use
     * setLineSymbol instead.
     * @param color color of grid lines
     * @see setLineSymbol
     * @see setGridLineWidth
     */
    void setGridLineColor( const QColor& color );

    /** Sets the line symbol used for drawing grid lines. This is only used for grids with
     * QgsComposerMapGrid::Solid or QgsComposerMapGrid::Cross styles.
     * @param symbol line symbol for grid lines
     * @see lineSymbol
     * @see setMarkerSymbol
     * @see setStyle
     */
    void setLineSymbol( QgsLineSymbolV2* symbol /Transfer/ );

    /** Gets the line symbol used for drawing grid lines. This is only used for grids with
     * QgsComposerMapGrid::Solid or QgsComposerMapGrid::Cross styles.
     * @returns line symbol for grid lines
     * @see setLineSymbol
     * @see markerSymbol
     * @see style
     * @note not available in python bindings
     */
    // const QgsLineSymbolV2* lineSymbol() const { return mGridLineSymbol; }

    /** Gets the line symbol used for drawing grid lines. This is only used for grids with
     * QgsComposerMapGrid::Solid or QgsComposerMapGrid::Cross styles.
     * @returns line symbol for grid lines
     * @see setLineSymbol
     * @see markerSymbol
     * @see style
     */
    QgsLineSymbolV2* lineSymbol();

    /** Sets the marker symbol used for drawing grid points. This is only used for grids with a
     * QgsComposerMapGrid::Markers style.
     * @param symbol marker symbol for grid intersection points
     * @see markerSymbol
     * @see setLineSymbol
     * @see setStyle
     */
    void setMarkerSymbol( QgsMarkerSymbolV2* symbol /Transfer/);

    /** Gets the marker symbol used for drawing grid points. This is only used for grids with a
     * QgsComposerMapGrid::Markers style.
     * @returns marker symbol for grid intersection points
     * @see setMarkerSymbol
     * @see lineSymbol
     * @see style
     * @note not available in python bindings
     */
    // const QgsMarkerSymbolV2* markerSymbol() const { return mGridMarkerSymbol; }

    /** Gets the marker symbol used for drawing grid points. This is only used for grids with a
     * QgsComposerMapGrid::Markers style.
     * @returns marker symbol for grid intersection points
     * @see setMarkerSymbol
     * @see lineSymbol
     * @see style
     */
    QgsMarkerSymbolV2* markerSymbol();

    //
    // ANNOTATIONS
    //

    /** Sets whether annotations should be shown for the grid.
     * @param enabled set to true to draw annotations for the grid
     * @see annotationEnabled
     */
    void setAnnotationEnabled( const bool enabled );

    /** Gets whether annotations are shown for the grid.
     * @returns true if annotations are drawn for the grid
     * @see setAnnotationEnabled
     */
    bool annotationEnabled() const;

    /** Sets the font used for drawing grid annotations
     * @param font font for annotations
     * @see annotationFont
     */
    void setAnnotationFont( const QFont& font );

    /** Gets the font used for drawing grid annotations
     * @returns font for annotations
     * @see setAnnotationFont
     */
    QFont annotationFont() const;

    /** Sets the font color used for drawing grid annotations
     * @param color font color for annotations
     * @see annotationFontColor
     */
    void setAnnotationFontColor( const QColor& color );

    /** Gets the font color used for drawing grid annotations
     * @returns font color for annotations
     * @see setAnnotationFontColor
     */
    QColor annotationFontColor() const;

    /** Sets the coordinate precision for grid annotations
     * @param precision number of decimal places to show when drawing grid annotations
     * @see annotationPrecision
     */
    void setAnnotationPrecision( const int precision );

    /** Returns the coordinate precision for grid annotations
     * @returns number of decimal places shown when drawing grid annotations
     * @see setAnnotationPrecision
     */
    int annotationPrecision() const;

    /** Sets what types of grid annotations should be drawn for a specified side of the map frame,
     * or whether grid annotations should be disabled for the side.
     * @param display display mode for annotations
     * @param border side of map for annotations
     * @see annotationDisplay
     * @note added in QGIS 2.7
     */
    void setAnnotationDisplay( const DisplayMode display, const BorderSide border );

    /** Gets the display mode for the grid annotations on a specified side of the map
     * frame. This property also specifies whether annotations have been disabled
     * from a side of the map frame.
     * @param border side of map for annotations
     * @returns display mode for grid annotations
     * @see setAnnotationDisplay
     * @note added in QGIS 2.7
     */
    DisplayMode annotationDisplay( const BorderSide border ) const;

    /** Sets the position for the grid annotations on a specified side of the map
     * frame.
     * @param position position to draw grid annotations
     * @param border side of map for annotations
     * @see annotationPosition
     */
    void setAnnotationPosition( const AnnotationPosition position, const BorderSide border );

    /** Gets the position for the grid annotations on a specified side of the map
     * frame.
     * @param border side of map for annotations
     * @returns position that grid annotations are drawn in
     * @see setAnnotationPosition
     */
    AnnotationPosition annotationPosition( const BorderSide border ) const;

    /** Sets the distance between the map frame and annotations. Units are in millimeters.
     * @param distance margin between map frame and annotations
     * @see annotationFrameDistance
     */
    void setAnnotationFrameDistance( const double distance );

    /** Gets the distance between the map frame and annotations. Units are in millimeters.
     * @returns margin between map frame and annotations
     * @see setAnnotationFrameDistance
     */
    double annotationFrameDistance() const;

    /** Sets the direction for drawing frame annotations.
     * @param direction direction for frame annotations
     * @param border side of map for annotations
     * @see annotationDirection
     */
    void setAnnotationDirection( const AnnotationDirection direction, const BorderSide border );

    /** Sets the direction for drawing all frame annotations.
     * @param direction direction for frame annotations
     * @see annotationDirection
     */
    void setAnnotationDirection( const AnnotationDirection direction );

    /** Gets the direction for drawing frame annotations.
     * @param border side of map for annotations
     * @returns direction for frame annotations
     * @see setAnnotationDirection
     */
    AnnotationDirection annotationDirection( const BorderSide border ) const;

    /** Sets the format for drawing grid annotations.
     * @param format format for grid annotations
     * @see annotationFormat
     */
    void setAnnotationFormat( const AnnotationFormat format );

    /** Gets the format for drawing grid annotations.
     * @returns format for grid annotations
     * @see setAnnotationFormat
     */
    AnnotationFormat annotationFormat() const;

    /** Sets the expression used for drawing grid annotations. This is only used when annotationFormat()
     * is QgsComposerMapGrid::CustomFormat.
     * @param expression expression for evaluating custom grid annotations
     * @see annotationExpression
     * @note added in QGIS 2.12
     */
    void setAnnotationExpression( const QString& expression );

    /** Returns the expression used for drawing grid annotations. This is only used when annotationFormat()
     * is QgsComposerMapGrid::CustomFormat.
     * @returns expression for evaluating custom grid annotations
     * @see setAnnotationExpression
     * @note added in QGIS 2.12
     */
    QString annotationExpression() const;

    //
    // GRID FRAME
    //

    /** Sets the grid frame style.
     * @param style style for grid frame
     * @see frameStyle
     */
    void setFrameStyle( const FrameStyle style );

    /** Gets the grid frame style.
     * @returns style for grid frame
     * @see setFrameStyle
     */
    FrameStyle frameStyle() const;

    /** Sets what type of grid divisions should be used for frames on a specified side of the map.
     * @param divisions grid divisions for frame
     * @param border side of map for frame
     * @see frameDivisions
     * @note added in QGIS 2.7
     */
    void setFrameDivisions( const DisplayMode divisions, const BorderSide border );

    /** Gets the type of grid divisions which are used for frames on a specified side of the map.
     * @param border side of map for frame
     * @returns grid divisions for frame
     * @see setFrameDivisions
     * @note added in QGIS 2.7
     */
    DisplayMode frameDivisions( const BorderSide border ) const;

    /** Sets flags for grid frame sides. Setting these flags controls which sides
     * of the map item the grid frame is drawn on.
     * @param flags flags for grid frame sides
     * @see setFrameSideFlag
     * @see frameSideFlags
     * @see testFrameSideFlag
     */
    void setFrameSideFlags( const FrameSideFlags& flags );

    /** Sets whether the grid frame is drawn for a certain side of the map item.
     * @param flag flag for grid frame side
     * @param on set to true to draw grid frame on that side of the map
     * @see setFrameSideFlags
     * @see frameSideFlags
     * @see testFrameSideFlag
     */
    void setFrameSideFlag( const FrameSideFlag flag, bool on = true );

    /** Returns the flags which control which sides of the map item the grid frame
     * is drawn on.
     * @returns flags for side of map grid is drawn on
     * @see setFrameSideFlags
     * @see setFrameSideFlag
     * @see testFrameSideFlag
     */
    FrameSideFlags frameSideFlags() const;

    /** Tests whether the grid frame should be drawn on a specified side of the map
     * item.
     * @param flag flag for grid frame side
     * @returns true if grid frame should be drawn for that side of the map
     * @see setFrameSideFlags
     * @see setFrameSideFlag
     * @see frameSideFlags
     */
    bool testFrameSideFlag( const FrameSideFlag flag ) const;

    /** Sets the grid frame width. This property controls how wide the grid frame is.
     * The size of the line outlines drawn in the frame is controlled through the
     * setFramePenSize method.
     * @param width width of grid frame in millimeters
     * @see frameWidth
     */
    void setFrameWidth( const double width );

    /** Gets the grid frame width. This property controls how wide the grid frame is.
     * The size of the line outlines drawn in the frame can be retrieved via the
     * framePenSize method.
     * @returns width of grid frame in millimeters
     * @see setFrameWidth
     */
    double frameWidth() const;

    /** Sets the width of the outline drawn in the grid frame.
     * @param width width of grid frame outline
     * @see framePenSize
     * @see setFramePenColor
     */
    void setFramePenSize( const double width );

    /** Retrieves the width of the outline drawn in the grid frame.
     * @returns width of grid frame outline
     * @see setFramePenSize
     * @see framePenColor
     */
    double framePenSize() const;

    /** Sets the color of the outline drawn in the grid frame.
     * @param color color of grid frame outline
     * @see framePenColor
     * @see setFramePenSize
     * @see setFrameFillColor1
     * @see setFrameFillColor2
     */
    void setFramePenColor( const QColor& color );

    /** Retrieves the color of the outline drawn in the grid frame.
     * @returns color of grid frame outline
     * @see setFramePenColor
     * @see framePenSize
     * @see frameFillColor1
     * @see frameFillColor2
     */
    QColor framePenColor() const;

    /** Sets the first fill color used for the grid frame.
     * @param color first fill color for grid frame
     * @see frameFillColor1
     * @see setFramePenColor
     * @see setFrameFillColor2
     */
    void setFrameFillColor1( const QColor& color );

    /** Retrieves the first fill color for the grid frame.
     * @returns first fill color for grid frame
     * @see setFrameFillColor1
     * @see framePenColor
     * @see frameFillColor2
     */
    QColor frameFillColor1() const;

    /** Sets the second fill color used for the grid frame.
     * @param color second fill color for grid frame
     * @see frameFillColor2
     * @see setFramePenColor
     * @see setFrameFillColor1
     */
    void setFrameFillColor2( const QColor& color );

    /** Retrieves the second fill color for the grid frame.
     * @returns second fill color for grid frame
     * @see setFrameFillColor2
     * @see framePenColor
     * @see frameFillColor1
     */
    QColor frameFillColor2() const;

    virtual QgsExpressionContext* createExpressionContext() const;

};
