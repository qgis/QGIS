/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemmapgrid.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutItemMapGridStack : QgsLayoutItemMapItemStack
{
%Docstring
 A collection of grids which is drawn above the map content in a
QgsLayoutItemMap. The grid stack controls which grids are drawn and the
order they are drawn in.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsLayoutItemMapGrid`
%End

%TypeHeaderCode
#include "qgslayoutitemmapgrid.h"
%End
  public:

    QgsLayoutItemMapGridStack( QgsLayoutItemMap *map );
%Docstring
Constructor for QgsLayoutItemMapGridStack, attached to the specified ``map``.
%End

    void addGrid( QgsLayoutItemMapGrid *grid /Transfer/ );
%Docstring
Adds a new map ``grid`` to the stack and takes ownership of the grid.
The grid will be added to the end of the stack, and rendered
above any existing map grids already present in the stack.

.. note::

   After adding a grid to the stack, updateBoundingRect() and update()
should be called for the QgsLayoutItemMap to prevent rendering artifacts.

.. seealso:: :py:func:`removeGrid`
%End

    void removeGrid( const QString &gridId );
%Docstring
Removes a grid with matching ``gridId`` from the stack and deletes the corresponding :py:class:`QgsLayoutItemMapGrid`.

.. note::

   After removing a grid from the stack, updateBoundingRect() and update()
should be called for the QgsLayoutItemMap to prevent rendering artifacts.

.. seealso:: :py:func:`addGrid`
%End

    void moveGridUp( const QString &gridId );
%Docstring
Moves a grid with matching ``gridId`` up the stack, causing it to be rendered above other grids.

.. note::

   After moving a grid within the stack, update() should be
called for the QgsLayoutItemMap to redraw the map with the new grid stack order.

.. seealso:: :py:func:`moveGridDown`
%End

    void moveGridDown( const QString &gridId );
%Docstring
Moves a grid with matching ``gridId`` down the stack, causing it to be rendered below other grids.

.. note::

   After moving a grid within the stack, update() should be
called for the QgsLayoutItemMap to redraw the map with the new grid stack order.

.. seealso:: :py:func:`moveGridUp`
%End

    QgsLayoutItemMapGrid *grid( const QString &gridId ) const;
%Docstring
Returns a reference to a grid with matching ``gridId`` within the stack.

.. seealso:: :py:func:`constGrid`
%End

    QgsLayoutItemMapGrid *grid( int index ) const;
%Docstring
Returns a reference to a grid at the specified ``index`` within the stack.

.. seealso:: :py:func:`constGrid`
%End

    QgsLayoutItemMapGrid &operator[]( int index );

    QList< QgsLayoutItemMapGrid * > asList() const;
%Docstring
Returns a list of QgsLayoutItemMapGrids contained by the stack.
%End

    virtual bool readXml( const QDomElement &elem, const QDomDocument &doc, const QgsReadWriteContext &context );


    double maxGridExtension() const;
%Docstring
Calculates the maximum distance grids within the stack extend
beyond the QgsLayoutItemMap's item rect.

.. seealso:: :py:func:`calculateMaxGridExtension`
%End

    void calculateMaxGridExtension( double &top, double &right, double &bottom, double &left ) const;
%Docstring
Calculates the maximum distance grids within the stack extend beyond the
QgsLayoutItemMap's item rect. This method calculates the distance for each side of the
map item separately.

.. seealso:: :py:func:`maxGridExtension`
%End
};


class QgsLayoutItemMapGrid : QgsLayoutItemMapItem
{
%Docstring
 An individual grid which is drawn above the map content in a
:py:class:`QgsLayoutItemMap`.

.. versionadded:: 3.0

.. seealso:: :py:class:`QgsLayoutItemMapGridStack`
%End

%TypeHeaderCode
#include "qgslayoutitemmapgrid.h"
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
    typedef QFlags<QgsLayoutItemMapGrid::FrameSideFlag> FrameSideFlags;


    enum AnnotationCoordinate
    {
      Longitude,
      Latitude
    };

    QgsLayoutItemMapGrid( const QString &name, QgsLayoutItemMap *map );
%Docstring
Constructor for QgsLayoutItemMapGrid.

:param name: friendly display name for grid
:param map: QgsLayoutItemMap the grid is attached to
%End

    virtual void draw( QPainter *painter );

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context );


    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the ``crs`` for the grid.

.. seealso:: :py:func:`crs`
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Retrieves the CRS for the grid.

.. seealso:: :py:func:`setCrs`
%End

    void setBlendMode( const QPainter::CompositionMode mode );
%Docstring
Sets the blending ``mode`` used for drawing the grid.

.. seealso:: :py:func:`blendMode`
%End

    QPainter::CompositionMode blendMode() const;
%Docstring
Retrieves the blending mode used for drawing the grid.

.. seealso:: :py:func:`setBlendMode`
%End

    virtual bool usesAdvancedEffects() const;


    double maxExtension() const;
%Docstring
Calculates the maximum distance the grid extends beyond the QgsLayoutItemMap's
item rect (in layout units).
%End

    void calculateMaxExtension( double &top, double &right, double &bottom, double &left ) const;
%Docstring
Calculates the maximum distance the grid extends beyond the
QgsLayoutItemMap's item rect. This method calculates the distance for each side of the
map item separately.

.. seealso:: :py:func:`maxExtension`
%End


    void setUnits( GridUnit unit );
%Docstring
Sets the ``unit`` to use for grid measurements such as the interval
and offset for grid lines.

.. seealso:: :py:func:`units`
%End

    GridUnit units() const;
%Docstring
Returns the units used for grid measurements such as the interval
and offset for grid lines.

.. seealso:: :py:func:`setUnits`
%End

    void setIntervalX( double interval );
%Docstring
Sets the ``interval`` between grid lines in the x-direction. The units
are controlled through the setUnits method

.. seealso:: :py:func:`setIntervalY`

.. seealso:: :py:func:`intervalX`
%End

    double intervalX() const;
%Docstring
Returns the interval between grid lines in the x-direction. The units
are retrieved through the units() method.

.. seealso:: :py:func:`setIntervalX`

.. seealso:: :py:func:`intervalY`
%End

    void setIntervalY( double interval );
%Docstring
Sets the ``interval`` between grid lines in the y-direction. The units
are controlled through the setUnits method

.. seealso:: :py:func:`setIntervalX`

.. seealso:: :py:func:`intervalY`
%End

    double intervalY() const;
%Docstring
Returns the interval between grid lines in the y-direction. The units
are retrieved through the units() method.

.. seealso:: :py:func:`setIntervalY`

.. seealso:: :py:func:`intervalX`
%End

    void setOffsetX( const double offset );
%Docstring
Sets the ``offset`` for grid lines in the x-direction. The units
are controlled through the setUnits method.

.. seealso:: :py:func:`setOffsetY`

.. seealso:: :py:func:`offsetX`
%End

    double offsetX() const;
%Docstring
Returns the offset for grid lines in the x-direction. The units
are retrieved through the units() method.

.. seealso:: :py:func:`setOffsetX`

.. seealso:: :py:func:`offsetY`
%End

    void setOffsetY( const double offset );
%Docstring
Sets the ``offset`` for grid lines in the y-direction. The units
are controlled through the setUnits method.

.. seealso:: :py:func:`setOffsetX`

.. seealso:: :py:func:`offsetY`
%End

    double offsetY() const;
%Docstring
Returns the offset for grid lines in the y-direction. The units
are retrieved through the units() method.

.. seealso:: :py:func:`setOffsetY`

.. seealso:: :py:func:`offsetX`
%End


    void setStyle( const GridStyle style );
%Docstring
Sets the grid ``style``, which controls how the grid is drawn
over the map's contents.

.. seealso:: :py:func:`style`
%End

    GridStyle style() const;
%Docstring
Returns the grid's style, which controls how the grid is drawn
over the map's contents.

.. seealso:: :py:func:`setStyle`
%End

    void setCrossLength( const double length );
%Docstring
Sets the ``length`` (in layout units) of the cross segments drawn for the grid. This is only used for grids
with QgsLayoutItemMapGrid.Cross styles.

.. seealso:: :py:func:`crossLength`
%End

    double crossLength() const;
%Docstring
Retrieves the length (in layout units) of the cross segments drawn for the grid. This is only used for grids
with QgsLayoutItemMapGrid.Cross styles.

.. seealso:: :py:func:`setCrossLength`
%End

    void setGridLineWidth( const double width );
%Docstring
Sets the ``width`` of grid lines (in layout units). This is only used for grids with QgsLayoutItemMapGrid.Solid
or QgsLayoutItemMapGrid.Cross styles. For more control over grid line appearance, use
setLineSymbol instead.

.. seealso:: :py:func:`setLineSymbol`

.. seealso:: :py:func:`setGridLineColor`
%End

    void setGridLineColor( const QColor &color );
%Docstring
Sets the ``color`` of grid lines. This is only used for grids with QgsLayoutItemMapGrid.Solid
or QgsLayoutItemMapGrid.Cross styles. For more control over grid line appearance, use
setLineSymbol instead.

.. seealso:: :py:func:`setLineSymbol`

.. seealso:: :py:func:`setGridLineWidth`
%End

    void setLineSymbol( QgsLineSymbol *symbol /Transfer/ );
%Docstring
Sets the line ``symbol`` used for drawing grid lines. This is only used for grids with
QgsLayoutItemMapGrid.Solid or QgsLayoutItemMapGrid.Cross styles.
Ownership of ``symbol`` is transferred to the grid.

.. seealso:: :py:func:`lineSymbol`

.. seealso:: :py:func:`setMarkerSymbol`

.. seealso:: :py:func:`setStyle`
%End

    QgsLineSymbol *lineSymbol();
%Docstring
Returns the line symbol used for drawing grid lines. This is only used for grids with
QgsLayoutItemMapGrid.Solid or QgsLayoutItemMapGrid.Cross styles.

.. seealso:: :py:func:`setLineSymbol`

.. seealso:: :py:func:`markerSymbol`

.. seealso:: :py:func:`style`
%End

    void setMarkerSymbol( QgsMarkerSymbol *symbol /Transfer/ );
%Docstring
Sets the marker ``symbol`` used for drawing grid points. This is only used for grids with a
QgsLayoutItemMapGrid.Markers style.
Ownership of ``symbol`` is transferred to the grid.

.. seealso:: :py:func:`markerSymbol`

.. seealso:: :py:func:`setLineSymbol`

.. seealso:: :py:func:`setStyle`
%End

    QgsMarkerSymbol *markerSymbol();
%Docstring
Returns the marker symbol used for drawing grid points. This is only used for grids with a
QgsLayoutItemMapGrid.Markers style.

.. seealso:: :py:func:`setMarkerSymbol`

.. seealso:: :py:func:`lineSymbol`

.. seealso:: :py:func:`style`
%End


    void setAnnotationEnabled( const bool enabled );
%Docstring
Sets whether annotations should be shown for the grid.

.. seealso:: :py:func:`annotationEnabled`
%End

    bool annotationEnabled() const;
%Docstring
Returns whether annotations are shown for the grid.

.. seealso:: :py:func:`setAnnotationEnabled`
%End

    void setAnnotationFont( const QFont &font );
%Docstring
Sets the ``font`` used for drawing grid annotations.

.. seealso:: :py:func:`annotationFont`
%End

    QFont annotationFont() const;
%Docstring
Returns the font used for drawing grid annotations.

.. seealso:: :py:func:`setAnnotationFont`
%End

    void setAnnotationFontColor( const QColor &color );
%Docstring
Sets the font ``color`` used for drawing grid annotations.

.. seealso:: :py:func:`annotationFontColor`
%End

    QColor annotationFontColor() const;
%Docstring
Returns the font color used for drawing grid annotations.

.. seealso:: :py:func:`setAnnotationFontColor`
%End

    void setAnnotationPrecision( const int precision );
%Docstring
Sets the coordinate ``precision`` for grid annotations.
The ``precision`` indicates the number of decimal places to show when drawing grid annotations.

.. seealso:: :py:func:`annotationPrecision`
%End

    int annotationPrecision() const;
%Docstring
Returns the coordinate precision for grid annotations, which is the
number of decimal places shown when drawing grid annotations.

.. seealso:: :py:func:`setAnnotationPrecision`
%End

    void setAnnotationDisplay( const DisplayMode display, const BorderSide border );
%Docstring
Sets what types of grid annotations should be drawn for a specified side of the map frame,
or whether grid annotations should be disabled for the side.

:param display: display mode for annotations
:param border: side of map for annotations

.. seealso:: :py:func:`annotationDisplay`
%End

    DisplayMode annotationDisplay( const BorderSide border ) const;
%Docstring
Returns the display mode for the grid annotations on a specified side of the map
frame. This property also specifies whether annotations have been disabled
from a side of the map frame.

:param border: side of map for annotations

:return: display mode for grid annotations

.. seealso:: :py:func:`setAnnotationDisplay`
%End

    void setAnnotationPosition( const AnnotationPosition position, const BorderSide side );
%Docstring
Sets the ``position`` for the grid annotations on a specified ``side`` of the map
frame.

.. seealso:: :py:func:`annotationPosition`
%End

    AnnotationPosition annotationPosition( const BorderSide side ) const;
%Docstring
Returns the position for the grid annotations on a specified ``side`` of the map
frame.

.. seealso:: :py:func:`setAnnotationPosition`
%End

    void setAnnotationFrameDistance( const double distance );
%Docstring
Sets the ``distance`` between the map frame and annotations. Units are layout units.

.. seealso:: :py:func:`annotationFrameDistance`
%End

    double annotationFrameDistance() const;
%Docstring
Returns the distance between the map frame and annotations. Units are in layout units.

.. seealso:: :py:func:`setAnnotationFrameDistance`
%End

    void setAnnotationDirection( const AnnotationDirection direction, const BorderSide side );
%Docstring
Sets the ``direction`` for drawing frame annotations for the specified map ``side``.

.. seealso:: :py:func:`annotationDirection`
%End

    void setAnnotationDirection( const AnnotationDirection direction );
%Docstring
Sets the ``direction`` for drawing all frame annotations.

.. seealso:: :py:func:`annotationDirection`
%End

    AnnotationDirection annotationDirection( const BorderSide border ) const;
%Docstring
Returns the direction for drawing frame annotations, on the specified ``side``
of the map.

.. seealso:: :py:func:`setAnnotationDirection`
%End

    void setAnnotationFormat( const AnnotationFormat format );
%Docstring
Sets the ``format`` for drawing grid annotations.

.. seealso:: :py:func:`annotationFormat`
%End

    AnnotationFormat annotationFormat() const;
%Docstring
Returns the format for drawing grid annotations.

.. seealso:: :py:func:`setAnnotationFormat`
%End

    void setAnnotationExpression( const QString &expression );
%Docstring
Sets the ``expression`` used for drawing grid annotations. This is only used when annotationFormat()
is QgsLayoutItemMapGrid.CustomFormat.

.. seealso:: :py:func:`annotationExpression`
%End

    QString annotationExpression() const;
%Docstring
Returns the expression used for drawing grid annotations. This is only used when annotationFormat()
is QgsLayoutItemMapGrid.CustomFormat.

.. seealso:: :py:func:`setAnnotationExpression`
%End


    void setFrameStyle( const FrameStyle style );
%Docstring
Sets the grid frame ``style``.

.. seealso:: :py:func:`frameStyle`
%End

    FrameStyle frameStyle() const;
%Docstring
Returns the grid frame style.

.. seealso:: :py:func:`setFrameStyle`
%End

    void setFrameDivisions( const DisplayMode divisions, const BorderSide side );
%Docstring
Sets what type of grid ``divisions`` should be used for frames on a specified ``side`` of the map.

.. seealso:: :py:func:`frameDivisions`
%End

    DisplayMode frameDivisions( const BorderSide side ) const;
%Docstring
Returns the type of grid divisions which are used for frames on a specified ``side`` of the map.

.. seealso:: :py:func:`setFrameDivisions`
%End

    void setFrameSideFlags( QgsLayoutItemMapGrid::FrameSideFlags flags );
%Docstring
Sets ``flags`` for grid frame sides. Setting these flags controls which sides
of the map item the grid frame is drawn on.

.. seealso:: :py:func:`setFrameSideFlag`

.. seealso:: :py:func:`frameSideFlags`

.. seealso:: :py:func:`testFrameSideFlag`
%End

    void setFrameSideFlag( const QgsLayoutItemMapGrid::FrameSideFlag flag, bool on = true );
%Docstring
Sets whether the grid frame is drawn for a certain side of the map item.

:param flag: flag for grid frame side
:param on: set to true to draw grid frame on that side of the map

.. seealso:: :py:func:`setFrameSideFlags`

.. seealso:: :py:func:`frameSideFlags`

.. seealso:: :py:func:`testFrameSideFlag`
%End

    QgsLayoutItemMapGrid::FrameSideFlags frameSideFlags() const;
%Docstring
Returns the flags which control which sides of the map item the grid frame
is drawn on.

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
Sets the grid frame ``width`` (in layout units). This property controls how wide the grid frame is.
The size of the line outlines drawn in the frame is controlled through the
setFramePenSize method.

.. seealso:: :py:func:`frameWidth`
%End

    double frameWidth() const;
%Docstring
Gets the grid frame width in layout units. This property controls how wide the grid frame is.
The size of the line outlines drawn in the frame can be retrieved via the
framePenSize method.

.. seealso:: :py:func:`setFrameWidth`
%End

    void setFramePenSize( const double width );
%Docstring
Sets the ``width`` of the stroke drawn in the grid frame.

.. seealso:: :py:func:`framePenSize`

.. seealso:: :py:func:`setFramePenColor`
%End

    double framePenSize() const;
%Docstring
Retrieves the width of the stroke drawn in the grid frame.

.. seealso:: :py:func:`setFramePenSize`

.. seealso:: :py:func:`framePenColor`
%End

    void setFramePenColor( const QColor &color );
%Docstring
Sets the ``color`` of the stroke drawn in the grid frame.

.. seealso:: :py:func:`framePenColor`

.. seealso:: :py:func:`setFramePenSize`

.. seealso:: :py:func:`setFrameFillColor1`

.. seealso:: :py:func:`setFrameFillColor2`
%End

    QColor framePenColor() const;
%Docstring
Retrieves the color of the stroke drawn in the grid frame.

.. seealso:: :py:func:`setFramePenColor`

.. seealso:: :py:func:`framePenSize`

.. seealso:: :py:func:`frameFillColor1`

.. seealso:: :py:func:`frameFillColor2`
%End

    void setFrameFillColor1( const QColor &color );
%Docstring
Sets the first fill ``color`` used for the grid frame.

.. seealso:: :py:func:`frameFillColor1`

.. seealso:: :py:func:`setFramePenColor`

.. seealso:: :py:func:`setFrameFillColor2`
%End

    QColor frameFillColor1() const;
%Docstring
Retrieves the first fill color for the grid frame.

.. seealso:: :py:func:`setFrameFillColor1`

.. seealso:: :py:func:`framePenColor`

.. seealso:: :py:func:`frameFillColor2`
%End

    void setFrameFillColor2( const QColor &color );
%Docstring
Sets the second fill ``color`` used for the grid frame.

.. seealso:: :py:func:`frameFillColor2`

.. seealso:: :py:func:`setFramePenColor`

.. seealso:: :py:func:`setFrameFillColor1`
%End

    QColor frameFillColor2() const;
%Docstring
Retrieves the second fill color for the grid frame.

.. seealso:: :py:func:`setFrameFillColor2`

.. seealso:: :py:func:`framePenColor`

.. seealso:: :py:func:`frameFillColor1`
%End

    virtual QgsExpressionContext createExpressionContext() const;


      public:
};

QFlags<QgsLayoutItemMapGrid::FrameSideFlag> operator|(QgsLayoutItemMapGrid::FrameSideFlag f1, QFlags<QgsLayoutItemMapGrid::FrameSideFlag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemmapgrid.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
