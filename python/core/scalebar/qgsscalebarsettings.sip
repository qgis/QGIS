/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgsscalebarsettings.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsScaleBarSettings
{
%Docstring
The QgsScaleBarSettings class stores the appearance and layout settings
for scalebar drawing with QgsScaleBarRenderer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsscalebarsettings.h"
%End
  public:

    enum Alignment
    {
      AlignLeft,
      AlignMiddle,
      AlignRight,
    };

    enum SegmentSizeMode
    {
      SegmentSizeFixed,
      SegmentSizeFitWidth
    };

    QgsScaleBarSettings();
%Docstring
Constructor for QgsScaleBarSettings.
%End

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

    SegmentSizeMode segmentSizeMode() const;
%Docstring
Returns the size mode for the scale bar segments.

.. seealso:: :py:func:`setSegmentSizeMode`

.. seealso:: :py:func:`minBarWidth`

.. seealso:: :py:func:`maxBarWidth`
%End

    void setSegmentSizeMode( SegmentSizeMode mode );
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

    void setMinimumBarWidth( double width );
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

    void setMaximumBarWidth( double width );
%Docstring
Sets the maximum ``width`` (in millimeters) for scale bar segments. This
property is only effective if the segmentSizeMode() is set
to SegmentSizeFitWidth.

.. seealso:: :py:func:`minimumBarWidth`

.. seealso:: :py:func:`setMinimumBarWidth`

.. seealso:: :py:func:`setSegmentSizeMode`
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

    void setPen( const QPen &pen );
%Docstring
Sets the pen used for drawing outlines in the scalebar.

.. seealso:: :py:func:`pen`
%End

    QBrush brush() const;
%Docstring
Returns the primary brush used for filling the scalebar.

.. seealso:: :py:func:`setBrush`

.. seealso:: :py:func:`brush2`

.. seealso:: :py:func:`pen`
%End

    void setBrush( const QBrush &brush );
%Docstring
Sets the primary brush used for filling the scalebar.

.. seealso:: :py:func:`brush`
%End

    QBrush brush2() const;
%Docstring
Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
as single and double box styles.

.. seealso:: :py:func:`setBrush2`

.. seealso:: :py:func:`brush`
%End

    void setBrush2( const QBrush &brush );
%Docstring
Sets the secondary brush used for filling the scalebar.

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

    Alignment alignment() const;
%Docstring
Returns the scalebar alignment.

.. seealso:: :py:func:`setAlignment`
%End

    void setAlignment( Alignment alignment );
%Docstring
Sets the scalebar ``alignment``.

.. seealso:: :py:func:`alignment`
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

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgsscalebarsettings.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
