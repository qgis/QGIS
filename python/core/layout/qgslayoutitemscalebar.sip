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

    virtual int type() const;

    virtual QIcon icon() const;


    static QgsLayoutItemScaleBar *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new scale bar item for the specified ``layout``.

The caller takes responsibility for deleting the returned object.
%End
    virtual QgsLayoutSize minimumSize() const;


    int numberOfSegments() const;
%Docstring
Returns the number of segments included in the scalebar.

.. seealso:: :py:func:`setNumberOfSegments`

.. seealso:: :py:func:`numberOfSegmentsLeft`
%End

    void setNumberOfSegments( int segments );
%Docstring
Sets the number of ``segments`` included in the scalebar.

.. seealso:: :py:func:`numberOfSegments`

.. seealso:: :py:func:`setNumberOfSegmentsLeft`
%End

    int numberOfSegmentsLeft() const;
%Docstring
Returns the number of segments included in the left part of the scalebar.

.. seealso:: :py:func:`setNumberOfSegmentsLeft`

.. seealso:: :py:func:`numberOfSegments`
%End

    void setNumberOfSegmentsLeft( int segments );
%Docstring
Sets the number of ``segments`` included in the left part of the scalebar.

.. seealso:: :py:func:`numberOfSegmentsLeft`

.. seealso:: :py:func:`setNumberOfSegments`
%End

    double unitsPerSegment() const;
%Docstring
Returns the number of scalebar units per segment.

.. seealso:: :py:func:`setUnitsPerSegment`
%End

    void setUnitsPerSegment( double units );
%Docstring
Sets the number of scalebar ``units`` per segment.

.. seealso:: :py:func:`unitsPerSegment`
%End

    QgsScaleBarSettings::SegmentSizeMode segmentSizeMode() const;
%Docstring
Returns the size mode for the scale bar segments.

.. seealso:: :py:func:`setSegmentSizeMode`

.. seealso:: :py:func:`minBarWidth`

.. seealso:: :py:func:`maxBarWidth`
%End

    void setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeMode mode );
%Docstring
Sets the size ``mode`` for scale bar segments.

.. seealso:: :py:func:`segmentSizeMode`

.. seealso:: :py:func:`setMinimumBarWidth`

.. seealso:: :py:func:`setMaximumBarWidth`
%End

    double minimumBarWidth() const;
%Docstring
Returns the minimum width (in millimeters) for scale bar segments. This
property is only effective if the segmentSizeMode() is set
to SegmentSizeFitWidth.

.. seealso:: :py:func:`segmentSizeMode`

.. seealso:: :py:func:`setMinimumBarWidth`

.. seealso:: :py:func:`maximumBarWidth`
%End

    void setMinimumBarWidth( double minWidth );
%Docstring
Sets the minimum ``width`` (in millimeters) for scale bar segments. This
property is only effective if the segmentSizeMode() is set
to SegmentSizeFitWidth.

.. seealso:: :py:func:`minimumBarWidth`

.. seealso:: :py:func:`setMaximumBarWidth`

.. seealso:: :py:func:`setSegmentSizeMode`
%End

    double maximumBarWidth() const;
%Docstring
Returns the maximum width (in millimeters) for scale bar segments. This
property is only effective if the segmentSizeMode() is set
to SegmentSizeFitWidth.

.. seealso:: :py:func:`segmentSizeMode`

.. seealso:: :py:func:`setMaximumBarWidth`

.. seealso:: :py:func:`minimumBarWidth`
%End

    void setMaximumBarWidth( double maxWidth );
%Docstring
Sets the maximum ``width`` (in millimeters) for scale bar segments. This
property is only effective if the segmentSizeMode() is set
to SegmentSizeFitWidth.

.. seealso:: :py:func:`minimumBarWidth`

.. seealso:: :py:func:`setMinimumBarWidth`

.. seealso:: :py:func:`setSegmentSizeMode`
%End

    double mapUnitsPerScaleBarUnit() const;
%Docstring
Returns the number of map units per scale bar unit used by the scalebar.

.. seealso:: :py:func:`setMapUnitsPerScaleBarUnit`
%End

    void setMapUnitsPerScaleBarUnit( double units );
%Docstring
Sets the number of map ``units`` per scale bar unit used by the scalebar.

.. seealso:: :py:func:`mapUnitsPerScaleBarUnit`
%End

    QString unitLabel() const;
%Docstring
Returns the label for units.

.. seealso:: :py:func:`setUnitLabel`
%End

    void setUnitLabel( const QString &label );
%Docstring
Sets the ``label`` for units.

.. seealso:: :py:func:`unitLabel`
%End

    QFont font() const;
%Docstring
Returns the font used for drawing text in the scalebar.

.. seealso:: :py:func:`setFont`
%End

    void setFont( const QFont &font );
%Docstring
Sets the ``font`` used for drawing text in the scalebar.

.. seealso:: :py:func:`setFont`
%End

    QColor fontColor() const;
%Docstring
Returns the color used for drawing text in the scalebar.

.. seealso:: :py:func:`setFontColor`

.. seealso:: :py:func:`font`
%End

    void setFontColor( const QColor &color );
%Docstring
Sets the ``color`` used for drawing text in the scalebar.

.. seealso:: :py:func:`fontColor`

.. seealso:: :py:func:`setFont`
%End

    QColor fillColor() const;
%Docstring
Returns the color used for fills in the scalebar.

.. seealso:: :py:func:`setFillColor`

.. seealso:: :py:func:`fillColor2`
%End

    void setFillColor( const QColor &color );
%Docstring
Sets the ``color`` used for fills in the scalebar.

.. seealso:: :py:func:`fillColor`

.. seealso:: :py:func:`setFillColor2`
%End

    QColor fillColor2() const;
%Docstring
Returns the secondary color used for fills in the scalebar.

.. seealso:: :py:func:`setFillColor2`

.. seealso:: :py:func:`fillColor`
%End

    void setFillColor2( const QColor &color );
%Docstring
Sets the secondary ``color`` used for fills in the scalebar.

.. seealso:: :py:func:`fillColor2`

.. seealso:: :py:func:`setFillColor2`
%End

    QColor lineColor() const;
%Docstring
Returns the color used for lines in the scalebar.

.. seealso:: :py:func:`setLineColor`
%End

    void setLineColor( const QColor &color );
%Docstring
Sets the ``color`` used for lines in the scalebar.

.. seealso:: :py:func:`lineColor`
%End

    double lineWidth() const;
%Docstring
Returns the line width in millimeters for lines in the scalebar.

.. seealso:: :py:func:`setLineWidth`
%End

    void setLineWidth( double width );
%Docstring
Sets the line ``width`` in millimeters for lines in the scalebar.

.. seealso:: :py:func:`lineWidth`
%End

    QPen pen() const;
%Docstring
Returns the pen used for drawing outlines in the scalebar.

.. seealso:: :py:func:`setPen`

.. seealso:: :py:func:`brush`
%End

    QBrush brush() const;
%Docstring
Returns the primary brush for the scalebar.

:return: QBrush used for filling the scalebar

.. seealso:: :py:func:`setBrush`

.. seealso:: :py:func:`brush2`

.. seealso:: :py:func:`pen`
%End

    QBrush brush2() const;
%Docstring
Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
as single and double box styles.

:return: QBrush used for secondary color areas

.. seealso:: :py:func:`setBrush2`

.. seealso:: :py:func:`brush`
%End

    double height() const;
%Docstring
Returns the scalebar height (in millimeters).

.. seealso:: :py:func:`setHeight`
%End

    void setHeight( double height );
%Docstring
Sets the scalebar ``height`` (in millimeters).

.. seealso:: :py:func:`height`
%End

    void setLinkedMap( QgsLayoutItemMap *map );
%Docstring
Sets the ``map`` item linked to the scalebar.

.. seealso:: :py:func:`linkedMap`
%End

    QgsLayoutItemMap *linkedMap() const;
%Docstring
Returns the map item linked to the scalebar.

.. seealso:: :py:func:`setLinkedMap`
%End

    double labelBarSpace() const;
%Docstring
Returns the spacing (in millimeters) between labels and the scalebar.

.. seealso:: :py:func:`setLabelBarSpace`
%End

    void setLabelBarSpace( double space );
%Docstring
Sets the spacing (in millimeters) between labels and the scalebar.

.. seealso:: :py:func:`labelBarSpace`
%End

    double boxContentSpace() const;
%Docstring
Returns the spacing (margin) between the scalebar box and content in millimeters.

.. seealso:: :py:func:`setBoxContentSpace`
%End

    void setBoxContentSpace( double space );
%Docstring
Sets the ``space`` (margin) between the scalebar box and content in millimeters.

.. seealso:: :py:func:`boxContentSpace`
%End

    QgsScaleBarSettings::Alignment alignment() const;
%Docstring
Returns the scalebar alignment.

.. seealso:: :py:func:`setAlignment`
%End

    void setAlignment( QgsScaleBarSettings::Alignment alignment );
%Docstring
Sets the scalebar ``alignment``.

.. seealso:: :py:func:`alignment`
%End

    QgsUnitTypes::DistanceUnit units() const;
%Docstring
Returns the distance units used by the scalebar.

.. seealso:: :py:func:`setUnits`
%End

    void setUnits( QgsUnitTypes::DistanceUnit units );
%Docstring
Sets the distance ``units`` used by the scalebar.

.. seealso:: :py:func:`units`
%End

    Qt::PenJoinStyle lineJoinStyle() const;
%Docstring
Returns the join style used for drawing lines in the scalebar.

.. seealso:: :py:func:`setLineJoinStyle`
%End

    void setLineJoinStyle( Qt::PenJoinStyle style );
%Docstring
Sets the join ``style`` used when drawing the lines in the scalebar

.. seealso:: :py:func:`lineJoinStyle`
%End

    Qt::PenCapStyle lineCapStyle() const;
%Docstring
Returns the cap style used for drawing lines in the scalebar.

.. seealso:: :py:func:`setLineCapStyle`
%End

    void setLineCapStyle( Qt::PenCapStyle style );
%Docstring
Sets the cap ``style`` used when drawing the lines in the scalebar.

.. seealso:: :py:func:`lineCapStyle`
%End

    void applyDefaultSettings();
%Docstring
Applies the default scalebar settings to the scale bar.

.. seealso:: :py:func:`applyDefaultSize`
%End

    QgsUnitTypes::DistanceUnit guessUnits() const;
%Docstring
Attempts to guess the most reasonable unit choice for the scalebar, given
the current linked map's scale.

This method also considers the linked map's CRS, in order to determine if
metric or imperial units are more appropriate.
%End

    void applyDefaultSize( QgsUnitTypes::DistanceUnit units = QgsUnitTypes::DistanceMeters );
%Docstring
Applies the default size to the scale bar (scale bar 1/5 of map item width)

.. seealso:: :py:func:`applyDefaultSettings`
%End

    void resizeToMinimumWidth();
%Docstring
Resizes the scale bar to its minimum width, without changing the height.
%End

    void setStyle( const QString &name );
%Docstring
Sets the scale bar style by ``name``.

The ``name`` parameter gives the (untranslated) style name.
Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle',
'Line Ticks Down', 'Line Ticks Up', 'Numeric'

.. seealso:: :py:func:`style`
%End

    QString style() const;
%Docstring
Returns the scale bar style name.

.. seealso:: :py:func:`setStyle`
%End

    void update();
%Docstring
Adjusts the scale bar box size and updates the item.
%End

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );

    virtual void finalizeRestoreFromXml();

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
