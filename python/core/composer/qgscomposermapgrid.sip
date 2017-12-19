/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermapgrid.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerMapGridStack : QgsComposerMapItemStack
{
%Docstring
 A collection of grids which is drawn above the map content in a
QgsComposerMap. The grid stack controls which grids are drawn and the
order they are drawn in.

.. versionadded:: 2.5

.. seealso:: :py:class:`QgsComposerMapGrid`
%End

%TypeHeaderCode
#include "qgscomposermapgrid.h"
%End
  public:

    QgsComposerMapGridStack( QgsComposerMap *map );
%Docstring
Constructor for QgsComposerMapGridStack.

:param map: QgsComposerMap the grid stack is attached to
%End

    void addGrid( QgsComposerMapGrid *grid /Transfer/ );
%Docstring
Adds a new map grid to the stack and takes ownership of the grid.
The grid will be added to the end of the stack, and rendered
above any existing map grids already present in the stack.

:param grid: QgsComposerMapGrid to add to the stack

.. note::

   after adding a grid to the stack, updateBoundingRect() and update()
should be called for the QgsComposerMap to prevent rendering artifacts

.. seealso:: :py:func:`removeGrid`
%End

    void removeGrid( const QString &gridId );
%Docstring
Removes a grid from the stack and deletes the corresponding :py:class:`QgsComposerMapGrid`

:param gridId: id for the QgsComposerMapGrid to remove

.. note::

   after removing a grid from the stack, updateBoundingRect() and update()
should be called for the QgsComposerMap to prevent rendering artifacts

.. seealso:: :py:func:`addGrid`
%End

    void moveGridUp( const QString &gridId );
%Docstring
Moves a grid up the stack, causing it to be rendered above other grids

:param gridId: id for the QgsComposerMapGrid to move up

.. note::

   after moving a grid within the stack, update() should be
called for the QgsComposerMap to redraw the map with the new grid stack order

.. seealso:: :py:func:`moveGridDown`
%End

    void moveGridDown( const QString &gridId );
%Docstring
Moves a grid down the stack, causing it to be rendered below other grids

:param gridId: id for the QgsComposerMapGrid to move down

.. note::

   after moving a grid within the stack, update() should be
called for the QgsComposerMap to redraw the map with the new grid stack order

.. seealso:: :py:func:`moveGridUp`
%End

    const QgsComposerMapGrid *constGrid( const QString &gridId ) const;
%Docstring
Returns a const reference to a grid within the stack

:param gridId: id for the QgsComposerMapGrid to find

:return: const reference to grid, if found

.. seealso:: :py:func:`grid`
%End

    QgsComposerMapGrid *grid( const QString &gridId ) const;
%Docstring
Returns a reference to a grid within the stack

:param gridId: id for the QgsComposerMapGrid to find

:return: reference to grid if found

.. seealso:: :py:func:`constGrid`
%End

    QgsComposerMapGrid *grid( const int index ) const;
%Docstring
Returns a reference to a grid within the stack

:param index: grid position in the stack

:return: reference to grid if found

.. seealso:: :py:func:`constGrid`
%End

    QgsComposerMapGrid &operator[]( int idx );

    QList< QgsComposerMapGrid * > asList() const;
%Docstring
Returns a list of QgsComposerMapGrids contained by the stack

:return: list of grids
%End

    virtual bool readXml( const QDomElement &elem, const QDomDocument &doc );

%Docstring
Sets the grid stack's state from a DOM document

:param elem: is DOM node corresponding to 'a ComposerMap' tag
:param doc: DOM document

:return: true if read was successful

.. seealso:: :py:func:`writeXml`
%End

    double maxGridExtension() const;
%Docstring
Calculates the maximum distance grids within the stack extend
beyond the QgsComposerMap's item rect

:return: maximum grid extension

.. seealso:: :py:func:`calculateMaxGridExtension()`
%End

    void calculateMaxGridExtension( double &top, double &right, double &bottom, double &left ) const;
%Docstring
Calculates the maximum distance grids within the stack extend beyond the
QgsComposerMap's item rect. This method calculates the distance for each side of the
map item separately

:param top: storage for top extension
:param right: storage for right extension
:param bottom: storage for bottom extension
:param left: storage for left extension

.. versionadded:: 2.12

.. seealso:: :py:func:`maxGridExtension()`
%End
};


class QgsComposerMapGrid : QgsComposerMapItem
{
%Docstring
 An individual grid which is drawn above the map content in a
:py:class:`QgsComposerMap`.

.. versionadded:: 2.5

.. seealso:: :py:class:`QgsComposerMapGridStack`
%End

%TypeHeaderCode
#include "qgscomposermapgrid.h"
%End
  public:

    enum GridUnit
    {
      MapUnit,
      MM,
      CM
    };

    enum GridStyle
    {
      Solid,
      Cross,
      Markers,
      FrameAnnotationsOnly
    };

    enum DisplayMode
    {
      ShowAll,
      LatitudeOnly,
      LongitudeOnly,
      HideAll
    };

    enum AnnotationPosition
    {
      InsideMapFrame,
      OutsideMapFrame,
    };

    enum AnnotationDirection
    {
      Horizontal,
      Vertical,
      VerticalDescending,
      BoundaryDirection
    };

    enum AnnotationFormat
    {
      Decimal,
      DegreeMinute,
      DegreeMinuteSecond,
      DecimalWithSuffix,
      DegreeMinuteNoSuffix,
      DegreeMinutePadded,
      DegreeMinuteSecondNoSuffix,
      DegreeMinuteSecondPadded,
      CustomFormat
    };

    enum BorderSide
    {
      Left,
      Right,
      Bottom,
      Top
    };

    enum FrameStyle
    {
      NoFrame,
      Zebra,
      InteriorTicks,
      ExteriorTicks,
      InteriorExteriorTicks,
      LineBorder
    };

    enum FrameSideFlag
    {
      FrameLeft,
      FrameRight,
      FrameTop,
      FrameBottom
    };
    typedef QFlags<QgsComposerMapGrid::FrameSideFlag> FrameSideFlags;


    enum AnnotationCoordinate
    {
      Longitude,
      Latitude
    };

    QgsComposerMapGrid( const QString &name, QgsComposerMap *map );
%Docstring
Constructor for QgsComposerMapGrid.

:param name: friendly display name for grid
:param map: QgsComposerMap the grid is attached to
%End

    ~QgsComposerMapGrid();

    virtual void draw( QPainter *painter );

%Docstring
Draws a grid

:param painter: destination QPainter
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores grid state in DOM element

:param elem: is DOM element corresponding to a 'ComposerMap' tag
:param doc: DOM document

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets grid state from a DOM document

:param itemElem: is DOM node corresponding to a 'ComposerMapGrid' tag
:param doc: is DOM document

.. seealso:: :py:func:`writeXml`
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the CRS for the grid.

:param crs: coordinate reference system for grid

.. seealso:: :py:func:`crs`
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Retrieves the CRS for the grid.

:return: coordinate reference system for grid

.. seealso:: :py:func:`setCrs`
%End

    void setBlendMode( const QPainter::CompositionMode mode );
%Docstring
Sets the blending mode used for drawing the grid.

:param mode: blending mode for grid

.. seealso:: :py:func:`blendMode`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Retrieves the blending mode used for drawing the grid.

:return: blending mode for grid

.. seealso:: :py:func:`setBlendMode`
%End

    virtual bool usesAdvancedEffects() const;


    double maxExtension();
%Docstring
Calculates the maximum distance the grid extends beyond the QgsComposerMap's
item rect

:return: maximum extension in millimeters
%End

    void calculateMaxExtension( double &top, double &right, double &bottom, double &left );
%Docstring
Calculates the maximum distance the grid extends beyond the
QgsComposerMap's item rect. This method calculates the distance for each side of the
map item separately

:param top: storage for top extension
:param right: storage for right extension
:param bottom: storage for bottom extension
:param left: storage for left extension

.. versionadded:: 2.12

.. seealso:: :py:func:`maxExtension()`
%End


    void setUnits( const GridUnit unit );
%Docstring
Sets the units to use for grid measurements such as the interval
and offset for grid lines.

:param unit: unit for grid measurements

.. seealso:: :py:func:`units`
%End

    GridUnit units() const;
%Docstring
Gets the units used for grid measurements such as the interval
and offset for grid lines.

:return: for grid measurements

.. seealso:: :py:func:`setUnits`
%End

    void setIntervalX( const double interval );
%Docstring
Sets the interval between grid lines in the x-direction. The units
are controlled through the setUnits method

:param interval: interval between horizontal grid lines

.. seealso:: :py:func:`setIntervalY`

.. seealso:: :py:func:`intervalX`
%End

    double intervalX() const;
%Docstring
Gets the interval between grid lines in the x-direction. The units
are retrieved through the units() method.

:return: interval between horizontal grid lines

.. seealso:: :py:func:`setIntervalX`

.. seealso:: :py:func:`intervalY`
%End

    void setIntervalY( const double interval );
%Docstring
Sets the interval between grid lines in the y-direction. The units
are controlled through the setUnits method

:param interval: interval between vertical grid lines

.. seealso:: :py:func:`setIntervalX`

.. seealso:: :py:func:`intervalY`
%End

    double intervalY() const;
%Docstring
Gets the interval between grid lines in the y-direction. The units
are retrieved through the units() method.

:return: interval between vertical grid lines

.. seealso:: :py:func:`setIntervalY`

.. seealso:: :py:func:`intervalX`
%End

    void setOffsetX( const double offset );
%Docstring
Sets the offset for grid lines in the x-direction. The units
are controlled through the setUnits method

:param offset: offset for horizontal grid lines

.. seealso:: :py:func:`setOffsetY`

.. seealso:: :py:func:`offsetX`
%End

    double offsetX() const;
%Docstring
Gets the offset for grid lines in the x-direction. The units
are retrieved through the units() method.

:return: offset for horizontal grid lines

.. seealso:: :py:func:`setOffsetX`

.. seealso:: :py:func:`offsetY`
%End

    void setOffsetY( const double offset );
%Docstring
Sets the offset for grid lines in the y-direction. The units
are controlled through the setUnits method

:param offset: offset for vertical grid lines

.. seealso:: :py:func:`setOffsetX`

.. seealso:: :py:func:`offsetY`
%End

    double offsetY() const;
%Docstring
Gets the offset for grid lines in the y-direction. The units
are retrieved through the units() method.

:return: offset for vertical grid lines

.. seealso:: :py:func:`setOffsetY`

.. seealso:: :py:func:`offsetX`
%End


    void setStyle( const GridStyle style );
%Docstring
Sets the grid style, which controls how the grid is drawn
over the map's contents

:param style: desired grid style

.. seealso:: :py:func:`style`
%End

    GridStyle style() const;
%Docstring
Gets the grid's style, which controls how the grid is drawn
over the map's contents

:return: current grid style

.. seealso:: :py:func:`setStyle`
%End

    void setCrossLength( const double length );
%Docstring
Sets the length of the cross segments drawn for the grid. This is only used for grids
with QgsComposerMapGrid.Cross styles

:param length: cross length in millimeters

.. seealso:: :py:func:`crossLength`
%End

    double crossLength() const;
%Docstring
Retrieves the length of the cross segments drawn for the grid. This is only used for grids
with QgsComposerMapGrid.Cross styles

:return: cross length in millimeters

.. seealso:: :py:func:`setCrossLength`
%End

    void setGridLineWidth( const double width );
%Docstring
Sets width of grid lines. This is only used for grids with QgsComposerMapGrid.Solid
or QgsComposerMapGrid.Cross styles. For more control over grid line appearance, use
setLineSymbol instead.

:param width: grid line width

.. seealso:: :py:func:`setLineSymbol`

.. seealso:: :py:func:`setGridLineColor`
%End

    void setGridLineColor( const QColor &color );
%Docstring
Sets color of grid lines. This is only used for grids with QgsComposerMapGrid.Solid
or QgsComposerMapGrid.Cross styles. For more control over grid line appearance, use
setLineSymbol instead.

:param color: color of grid lines

.. seealso:: :py:func:`setLineSymbol`

.. seealso:: :py:func:`setGridLineWidth`
%End

    void setLineSymbol( QgsLineSymbol *symbol /Transfer/ );
%Docstring
Sets the line symbol used for drawing grid lines. This is only used for grids with
QgsComposerMapGrid.Solid or QgsComposerMapGrid.Cross styles.

:param symbol: line symbol for grid lines

.. seealso:: :py:func:`lineSymbol`

.. seealso:: :py:func:`setMarkerSymbol`

.. seealso:: :py:func:`setStyle`
%End


    QgsLineSymbol *lineSymbol();
%Docstring
Gets the line symbol used for drawing grid lines. This is only used for grids with
QgsComposerMapGrid.Solid or QgsComposerMapGrid.Cross styles.

:return: line symbol for grid lines

.. seealso:: :py:func:`setLineSymbol`

.. seealso:: :py:func:`markerSymbol`

.. seealso:: :py:func:`style`
%End

    void setMarkerSymbol( QgsMarkerSymbol *symbol /Transfer/ );
%Docstring
Sets the marker symbol used for drawing grid points. This is only used for grids with a
QgsComposerMapGrid.Markers style.

:param symbol: marker symbol for grid intersection points

.. seealso:: :py:func:`markerSymbol`

.. seealso:: :py:func:`setLineSymbol`

.. seealso:: :py:func:`setStyle`
%End


    QgsMarkerSymbol *markerSymbol();
%Docstring
Gets the marker symbol used for drawing grid points. This is only used for grids with a
QgsComposerMapGrid.Markers style.

:return: marker symbol for grid intersection points

.. seealso:: :py:func:`setMarkerSymbol`

.. seealso:: :py:func:`lineSymbol`

.. seealso:: :py:func:`style`
%End


    void setAnnotationEnabled( const bool enabled );
%Docstring
Sets whether annotations should be shown for the grid.

:param enabled: set to true to draw annotations for the grid

.. seealso:: :py:func:`annotationEnabled`
%End

    bool annotationEnabled() const;
%Docstring
Gets whether annotations are shown for the grid.

:return: true if annotations are drawn for the grid

.. seealso:: :py:func:`setAnnotationEnabled`
%End

    void setAnnotationFont( const QFont &font );
%Docstring
Sets the font used for drawing grid annotations

:param font: font for annotations

.. seealso:: :py:func:`annotationFont`
%End

    QFont annotationFont() const;
%Docstring
Gets the font used for drawing grid annotations

:return: font for annotations

.. seealso:: :py:func:`setAnnotationFont`
%End

    void setAnnotationFontColor( const QColor &color );
%Docstring
Sets the font color used for drawing grid annotations

:param color: font color for annotations

.. seealso:: :py:func:`annotationFontColor`
%End

    QColor annotationFontColor() const;
%Docstring
Gets the font color used for drawing grid annotations

:return: font color for annotations

.. seealso:: :py:func:`setAnnotationFontColor`
%End

    void setAnnotationPrecision( const int precision );
%Docstring
Sets the coordinate precision for grid annotations

:param precision: number of decimal places to show when drawing grid annotations

.. seealso:: :py:func:`annotationPrecision`
%End

    int annotationPrecision() const;
%Docstring
Returns the coordinate precision for grid annotations

:return: number of decimal places shown when drawing grid annotations

.. seealso:: :py:func:`setAnnotationPrecision`
%End

    void setAnnotationDisplay( const DisplayMode display, const BorderSide border );
%Docstring
Sets what types of grid annotations should be drawn for a specified side of the map frame,
or whether grid annotations should be disabled for the side.

:param display: display mode for annotations
:param border: side of map for annotations

.. seealso:: :py:func:`annotationDisplay`

.. versionadded:: 2.7
%End

    DisplayMode annotationDisplay( const BorderSide border ) const;
%Docstring
Gets the display mode for the grid annotations on a specified side of the map
frame. This property also specifies whether annotations have been disabled
from a side of the map frame.

:param border: side of map for annotations

:return: display mode for grid annotations

.. seealso:: :py:func:`setAnnotationDisplay`

.. versionadded:: 2.7
%End

    void setAnnotationPosition( const AnnotationPosition position, const BorderSide border );
%Docstring
Sets the position for the grid annotations on a specified side of the map
frame.

:param position: position to draw grid annotations
:param border: side of map for annotations

.. seealso:: :py:func:`annotationPosition`
%End

    AnnotationPosition annotationPosition( const BorderSide border ) const;
%Docstring
Gets the position for the grid annotations on a specified side of the map
frame.

:param border: side of map for annotations

:return: position that grid annotations are drawn in

.. seealso:: :py:func:`setAnnotationPosition`
%End

    void setAnnotationFrameDistance( const double distance );
%Docstring
Sets the distance between the map frame and annotations. Units are in millimeters.

:param distance: margin between map frame and annotations

.. seealso:: :py:func:`annotationFrameDistance`
%End

    double annotationFrameDistance() const;
%Docstring
Gets the distance between the map frame and annotations. Units are in millimeters.

:return: margin between map frame and annotations

.. seealso:: :py:func:`setAnnotationFrameDistance`
%End

    void setAnnotationDirection( const AnnotationDirection direction, const BorderSide border );
%Docstring
Sets the direction for drawing frame annotations.

:param direction: direction for frame annotations
:param border: side of map for annotations

.. seealso:: :py:func:`annotationDirection`
%End

    void setAnnotationDirection( const AnnotationDirection direction );
%Docstring
Sets the direction for drawing all frame annotations.

:param direction: direction for frame annotations

.. seealso:: :py:func:`annotationDirection`
%End

    AnnotationDirection annotationDirection( const BorderSide border ) const;
%Docstring
Gets the direction for drawing frame annotations.

:param border: side of map for annotations

:return: direction for frame annotations

.. seealso:: :py:func:`setAnnotationDirection`
%End

    void setAnnotationFormat( const AnnotationFormat format );
%Docstring
Sets the format for drawing grid annotations.

:param format: format for grid annotations

.. seealso:: :py:func:`annotationFormat`
%End

    AnnotationFormat annotationFormat() const;
%Docstring
Gets the format for drawing grid annotations.

:return: format for grid annotations

.. seealso:: :py:func:`setAnnotationFormat`
%End

    void setAnnotationExpression( const QString &expression );
%Docstring
Sets the expression used for drawing grid annotations. This is only used when annotationFormat()
is QgsComposerMapGrid.CustomFormat.

:param expression: expression for evaluating custom grid annotations

.. seealso:: :py:func:`annotationExpression`

.. versionadded:: 2.12
%End

    QString annotationExpression() const;
%Docstring
Returns the expression used for drawing grid annotations. This is only used when annotationFormat()
is QgsComposerMapGrid.CustomFormat.

:return: expression for evaluating custom grid annotations

.. seealso:: :py:func:`setAnnotationExpression`

.. versionadded:: 2.12
%End


    void setFrameStyle( const FrameStyle style );
%Docstring
Sets the grid frame style.

:param style: style for grid frame

.. seealso:: :py:func:`frameStyle`
%End

    FrameStyle frameStyle() const;
%Docstring
Gets the grid frame style.

:return: style for grid frame

.. seealso:: :py:func:`setFrameStyle`
%End

    void setFrameDivisions( const DisplayMode divisions, const BorderSide border );
%Docstring
Sets what type of grid divisions should be used for frames on a specified side of the map.

:param divisions: grid divisions for frame
:param border: side of map for frame

.. seealso:: :py:func:`frameDivisions`

.. versionadded:: 2.7
%End

    DisplayMode frameDivisions( const BorderSide border ) const;
%Docstring
Gets the type of grid divisions which are used for frames on a specified side of the map.

:param border: side of map for frame

:return: grid divisions for frame

.. seealso:: :py:func:`setFrameDivisions`

.. versionadded:: 2.7
%End

    void setFrameSideFlags( QgsComposerMapGrid::FrameSideFlags flags );
%Docstring
Sets flags for grid frame sides. Setting these flags controls which sides
of the map item the grid frame is drawn on.

:param flags: flags for grid frame sides

.. seealso:: :py:func:`setFrameSideFlag`

.. seealso:: :py:func:`frameSideFlags`

.. seealso:: :py:func:`testFrameSideFlag`
%End

    void setFrameSideFlag( const FrameSideFlag flag, bool on = true );
%Docstring
Sets whether the grid frame is drawn for a certain side of the map item.

:param flag: flag for grid frame side
:param on: set to true to draw grid frame on that side of the map

.. seealso:: :py:func:`setFrameSideFlags`

.. seealso:: :py:func:`frameSideFlags`

.. seealso:: :py:func:`testFrameSideFlag`
%End

    FrameSideFlags frameSideFlags() const;
%Docstring
Returns the flags which control which sides of the map item the grid frame
is drawn on.

:return: flags for side of map grid is drawn on

.. seealso:: :py:func:`setFrameSideFlags`

.. seealso:: :py:func:`setFrameSideFlag`

.. seealso:: :py:func:`testFrameSideFlag`
%End

    bool testFrameSideFlag( const FrameSideFlag flag ) const;
%Docstring
Tests whether the grid frame should be drawn on a specified side of the map
item.

:param flag: flag for grid frame side

:return: true if grid frame should be drawn for that side of the map

.. seealso:: :py:func:`setFrameSideFlags`

.. seealso:: :py:func:`setFrameSideFlag`

.. seealso:: :py:func:`frameSideFlags`
%End

    void setFrameWidth( const double width );
%Docstring
Sets the grid frame width. This property controls how wide the grid frame is.
The size of the line outlines drawn in the frame is controlled through the
setFramePenSize method.

:param width: width of grid frame in millimeters

.. seealso:: :py:func:`frameWidth`
%End

    double frameWidth() const;
%Docstring
Gets the grid frame width. This property controls how wide the grid frame is.
The size of the line outlines drawn in the frame can be retrieved via the
framePenSize method.

:return: width of grid frame in millimeters

.. seealso:: :py:func:`setFrameWidth`
%End

    void setFramePenSize( const double width );
%Docstring
Sets the width of the stroke drawn in the grid frame.

:param width: width of grid frame stroke

.. seealso:: :py:func:`framePenSize`

.. seealso:: :py:func:`setFramePenColor`
%End

    double framePenSize() const;
%Docstring
Retrieves the width of the stroke drawn in the grid frame.

:return: width of grid frame stroke

.. seealso:: :py:func:`setFramePenSize`

.. seealso:: :py:func:`framePenColor`
%End

    void setFramePenColor( const QColor &color );
%Docstring
Sets the color of the stroke drawn in the grid frame.

:param color: color of grid frame stroke

.. seealso:: :py:func:`framePenColor`

.. seealso:: :py:func:`setFramePenSize`

.. seealso:: :py:func:`setFrameFillColor1`

.. seealso:: :py:func:`setFrameFillColor2`
%End

    QColor framePenColor() const;
%Docstring
Retrieves the color of the stroke drawn in the grid frame.

:return: color of grid frame stroke

.. seealso:: :py:func:`setFramePenColor`

.. seealso:: :py:func:`framePenSize`

.. seealso:: :py:func:`frameFillColor1`

.. seealso:: :py:func:`frameFillColor2`
%End

    void setFrameFillColor1( const QColor &color );
%Docstring
Sets the first fill color used for the grid frame.

:param color: first fill color for grid frame

.. seealso:: :py:func:`frameFillColor1`

.. seealso:: :py:func:`setFramePenColor`

.. seealso:: :py:func:`setFrameFillColor2`
%End

    QColor frameFillColor1() const;
%Docstring
Retrieves the first fill color for the grid frame.

:return: first fill color for grid frame

.. seealso:: :py:func:`setFrameFillColor1`

.. seealso:: :py:func:`framePenColor`

.. seealso:: :py:func:`frameFillColor2`
%End

    void setFrameFillColor2( const QColor &color );
%Docstring
Sets the second fill color used for the grid frame.

:param color: second fill color for grid frame

.. seealso:: :py:func:`frameFillColor2`

.. seealso:: :py:func:`setFramePenColor`

.. seealso:: :py:func:`setFrameFillColor1`
%End

    QColor frameFillColor2() const;
%Docstring
Retrieves the second fill color for the grid frame.

:return: second fill color for grid frame

.. seealso:: :py:func:`setFrameFillColor2`

.. seealso:: :py:func:`framePenColor`

.. seealso:: :py:func:`frameFillColor1`
%End

    virtual QgsExpressionContext createExpressionContext() const;


      public:
};

QFlags<QgsComposerMapGrid::FrameSideFlag> operator|(QgsComposerMapGrid::FrameSideFlag f1, QFlags<QgsComposerMapGrid::FrameSideFlag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermapgrid.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
