/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemscalebar.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutItemScaleBar: QgsLayoutItem
{
%Docstring
 A layout item subclass for scale bars.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemscalebar.h"
%End
  public:

    QgsLayoutItemScaleBar( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemScaleBar, with the specified parent ``layout``.
%End
    ~QgsLayoutItemScaleBar();

    virtual int type() const;

    virtual QString stringType() const;


    static QgsLayoutItemScaleBar *create( QgsLayout *layout ) /Factory/;
%Docstring
 Returns a new scale bar item for the specified ``layout``.

 The caller takes responsibility for deleting the returned object.
 :rtype: QgsLayoutItemScaleBar
%End

    int numberOfSegments() const;
%Docstring
 Returns the number of segments included in the scalebar.
.. seealso:: setNumberOfSegments()
.. seealso:: numberOfSegmentsLeft()
 :rtype: int
%End

    void setNumberOfSegments( int segments );
%Docstring
 Sets the number of ``segments`` included in the scalebar.
.. seealso:: numberOfSegments()
.. seealso:: setNumberOfSegmentsLeft()
%End

    int numberOfSegmentsLeft() const;
%Docstring
 Returns the number of segments included in the left part of the scalebar.
.. seealso:: setNumberOfSegmentsLeft()
.. seealso:: numberOfSegments()
 :rtype: int
%End

    void setNumberOfSegmentsLeft( int segments );
%Docstring
 Sets the number of ``segments`` included in the left part of the scalebar.
.. seealso:: numberOfSegmentsLeft()
.. seealso:: setNumberOfSegments()
%End

    double unitsPerSegment() const;
%Docstring
 Returns the number of scalebar units per segment.
.. seealso:: setUnitsPerSegment()
 :rtype: float
%End

    void setUnitsPerSegment( double units );
%Docstring
 Sets the number of scalebar ``units`` per segment.
.. seealso:: unitsPerSegment()
%End

    QgsScaleBarSettings::SegmentSizeMode segmentSizeMode() const;
%Docstring
 Returns the size mode for the scale bar segments.
.. seealso:: setSegmentSizeMode()
.. seealso:: minBarWidth()
.. seealso:: maxBarWidth()
 :rtype: QgsScaleBarSettings.SegmentSizeMode
%End

    void setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeMode mode );
%Docstring
 Sets the size ``mode`` for scale bar segments.
.. seealso:: segmentSizeMode()
.. seealso:: setMinimumBarWidth()
.. seealso:: setMaximumBarWidth()
%End

    double minimumBarWidth() const;
%Docstring
 Returns the minimum width (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
.. seealso:: segmentSizeMode()
.. seealso:: setMinimumBarWidth()
.. seealso:: maximumBarWidth()
 :rtype: float
%End

    void setMinimumBarWidth( double minWidth );
%Docstring
 Sets the minimum ``width`` (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
.. seealso:: minimumBarWidth()
.. seealso:: setMaximumBarWidth()
.. seealso:: setSegmentSizeMode()
%End

    double maximumBarWidth() const;
%Docstring
 Returns the maximum width (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
.. seealso:: segmentSizeMode()
.. seealso:: setMaximumBarWidth()
.. seealso:: minimumBarWidth()
 :rtype: float
%End

    void setMaximumBarWidth( double maxWidth );
%Docstring
 Sets the maximum ``width`` (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
.. seealso:: minimumBarWidth()
.. seealso:: setMinimumBarWidth()
.. seealso:: setSegmentSizeMode()
%End

    double mapUnitsPerScaleBarUnit() const;
%Docstring
 Returns the number of map units per scale bar unit used by the scalebar.
.. seealso:: setMapUnitsPerScaleBarUnit()
 :rtype: float
%End

    void setMapUnitsPerScaleBarUnit( double units );
%Docstring
 Sets the number of map ``units`` per scale bar unit used by the scalebar.
.. seealso:: mapUnitsPerScaleBarUnit()
%End

    QString unitLabel() const;
%Docstring
 Returns the label for units.
.. seealso:: setUnitLabel()
 :rtype: str
%End

    void setUnitLabel( const QString &label );
%Docstring
 Sets the ``label`` for units.
.. seealso:: unitLabel()
%End

    QFont font() const;
%Docstring
 Returns the font used for drawing text in the scalebar.
.. seealso:: setFont()
 :rtype: QFont
%End

    void setFont( const QFont &font );
%Docstring
 Sets the ``font`` used for drawing text in the scalebar.
.. seealso:: setFont()
%End

    QColor fontColor() const;
%Docstring
 Returns the color used for drawing text in the scalebar.
.. seealso:: setFontColor()
.. seealso:: font()
 :rtype: QColor
%End

    void setFontColor( const QColor &color );
%Docstring
 Sets the ``color`` used for drawing text in the scalebar.
.. seealso:: fontColor()
.. seealso:: setFont()
%End

    QColor fillColor() const;
%Docstring
 Returns the color used for fills in the scalebar.
.. seealso:: setFillColor()
.. seealso:: fillColor2()
 :rtype: QColor
%End

    void setFillColor( const QColor &color );
%Docstring
 Sets the ``color`` used for fills in the scalebar.
.. seealso:: fillColor()
.. seealso:: setFillColor2()
%End

    QColor fillColor2() const;
%Docstring
 Returns the secondary color used for fills in the scalebar.
.. seealso:: setFillColor2()
.. seealso:: fillColor()
 :rtype: QColor
%End

    void setFillColor2( const QColor &color );
%Docstring
 Sets the secondary ``color`` used for fills in the scalebar.
.. seealso:: fillColor2()
.. seealso:: setFillColor2()
%End

    QColor lineColor() const;
%Docstring
 Returns the color used for lines in the scalebar.
.. seealso:: setLineColor()
 :rtype: QColor
%End

    void setLineColor( const QColor &color );
%Docstring
 Sets the ``color`` used for lines in the scalebar.
.. seealso:: lineColor()
%End

    double lineWidth() const;
%Docstring
 Returns the line width in millimeters for lines in the scalebar.
.. seealso:: setLineWidth()
 :rtype: float
%End

    void setLineWidth( double width );
%Docstring
 Sets the line ``width`` in millimeters for lines in the scalebar.
.. seealso:: lineWidth()
%End

    QPen pen() const;
%Docstring
 Returns the pen used for drawing outlines in the scalebar.
.. seealso:: setPen()
.. seealso:: brush()
 :rtype: QPen
%End

    QBrush brush() const;
%Docstring
 Returns the primary brush for the scalebar.
 :return: QBrush used for filling the scalebar
.. seealso:: setBrush
.. seealso:: brush2
.. seealso:: pen
 :rtype: QBrush
%End

    QBrush brush2() const;
%Docstring
 Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
 as single and double box styles.
 :return: QBrush used for secondary color areas
.. seealso:: setBrush2
.. seealso:: brush
 :rtype: QBrush
%End

    double height() const;
%Docstring
 Returns the scalebar height (in millimeters).
.. seealso:: setHeight()
 :rtype: float
%End

    void setHeight( double height );
%Docstring
 Sets the scalebar ``height`` (in millimeters).
.. seealso:: height()
%End

    void setMap( QgsLayoutItemMap *map );
%Docstring
 Sets the ``map`` item linked to the scalebar.
.. seealso:: map()
%End

    QgsLayoutItemMap *map() const;
%Docstring
 Returns the map item linked to the scalebar.
.. seealso:: setMap()
 :rtype: QgsLayoutItemMap
%End

    double labelBarSpace() const;
%Docstring
 Returns the spacing (in millimeters) between labels and the scalebar.
.. seealso:: setLabelBarSpace()
 :rtype: float
%End

    void setLabelBarSpace( double space );
%Docstring
 Sets the spacing (in millimeters) between labels and the scalebar.
.. seealso:: labelBarSpace()
%End

    double boxContentSpace() const;
%Docstring
 Returns the spacing (margin) between the scalebar box and content in millimeters.
.. seealso:: setBoxContentSpace()
 :rtype: float
%End

    void setBoxContentSpace( double space );
%Docstring
 Sets the ``space`` (margin) between the scalebar box and content in millimeters.
.. seealso:: boxContentSpace()
%End

    QgsScaleBarSettings::Alignment alignment() const;
%Docstring
 Returns the scalebar alignment.
.. seealso:: setAlignment()
 :rtype: QgsScaleBarSettings.Alignment
%End

    void setAlignment( QgsScaleBarSettings::Alignment alignment );
%Docstring
 Sets the scalebar ``alignment``.
.. seealso:: alignment()
%End

    QgsUnitTypes::DistanceUnit units() const;
%Docstring
 Returns the distance units used by the scalebar.
.. seealso:: setUnits()
 :rtype: QgsUnitTypes.DistanceUnit
%End

    void setUnits( QgsUnitTypes::DistanceUnit units );
%Docstring
 Sets the distance ``units`` used by the scalebar.
.. seealso:: units()
%End

    Qt::PenJoinStyle lineJoinStyle() const;
%Docstring
 Returns the join style used for drawing lines in the scalebar.
.. seealso:: setLineJoinStyle()
 :rtype: Qt.PenJoinStyle
%End

    void setLineJoinStyle( Qt::PenJoinStyle style );
%Docstring
 Sets the join ``style`` used when drawing the lines in the scalebar
.. seealso:: lineJoinStyle()
%End

    Qt::PenCapStyle lineCapStyle() const;
%Docstring
 Returns the cap style used for drawing lines in the scalebar.
.. seealso:: setLineCapStyle()
 :rtype: Qt.PenCapStyle
%End

    void setLineCapStyle( Qt::PenCapStyle style );
%Docstring
 Sets the cap ``style`` used when drawing the lines in the scalebar.
.. seealso:: lineCapStyle()
%End

    void applyDefaultSettings();
%Docstring
 Applies the default scalebar settings to the scale bar.
.. seealso:: applyDefaultSize()
%End

    void applyDefaultSize( QgsUnitTypes::DistanceUnit units = QgsUnitTypes::DistanceMeters );
%Docstring
 Applies the default size to the scale bar (scale bar 1/5 of map item width)
.. seealso:: applyDefaultSettings()
%End

    void setStyle( const QString &name );
%Docstring
 Sets the scale bar style by ``name``.

 The ``name`` parameter gives the (untranslated) style name.
 Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle',
 'Line Ticks Down', 'Line Ticks Up', 'Numeric'

.. seealso:: style()
%End

    QString style() const;
%Docstring
 Returns the scale bar style name.
.. seealso:: setStyle()
 :rtype: str
%End

    void adjustBoxSize();
%Docstring
 Sets the scale bar box size to a size suitable for the scalebar content.
%End

    void update();
%Docstring
 Adjusts the scale bar box size and updates the item.
%End
    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );


  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemscalebar.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
