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
 \see setNumberOfSegments()
 \see numberOfSegmentsLeft()
 :rtype: int
%End

    void setNumberOfSegments( int segments );
%Docstring
 Sets the number of ``segments`` included in the scalebar.
 \see numberOfSegments()
 \see setNumberOfSegmentsLeft()
%End

    int numberOfSegmentsLeft() const;
%Docstring
 Returns the number of segments included in the left part of the scalebar.
 \see setNumberOfSegmentsLeft()
 \see numberOfSegments()
 :rtype: int
%End

    void setNumberOfSegmentsLeft( int segments );
%Docstring
 Sets the number of ``segments`` included in the left part of the scalebar.
 \see numberOfSegmentsLeft()
 \see setNumberOfSegments()
%End

    double unitsPerSegment() const;
%Docstring
 Returns the number of scalebar units per segment.
 \see setUnitsPerSegment()
 :rtype: float
%End

    void setUnitsPerSegment( double units );
%Docstring
 Sets the number of scalebar ``units`` per segment.
 \see unitsPerSegment()
%End

    SegmentSizeMode segmentSizeMode() const;
%Docstring
 Returns the size mode for the scale bar segments.
 \see setSegmentSizeMode()
 \see minBarWidth()
 \see maxBarWidth()
 :rtype: SegmentSizeMode
%End

    void setSegmentSizeMode( SegmentSizeMode mode );
%Docstring
 Sets the size ``mode`` for scale bar segments.
 \see segmentSizeMode()
 \see setMinimumBarWidth()
 \see setMaximumBarWidth()
%End

    double minimumBarWidth() const;
%Docstring
 Returns the minimum width (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
 \see segmentSizeMode()
 \see setMinimumBarWidth()
 \see maximumBarWidth()
 :rtype: float
%End

    void setMinimumBarWidth( double width );
%Docstring
 Sets the minimum ``width`` (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
 \see minimumBarWidth()
 \see setMaximumBarWidth()
 \see setSegmentSizeMode()
%End

    double maximumBarWidth() const;
%Docstring
 Returns the maximum width (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
 \see segmentSizeMode()
 \see setMaximumBarWidth()
 \see minimumBarWidth()
 :rtype: float
%End

    void setMaximumBarWidth( double width );
%Docstring
 Sets the maximum ``width`` (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
 \see minimumBarWidth()
 \see setMinimumBarWidth()
 \see setSegmentSizeMode()
%End

    QgsUnitTypes::DistanceUnit units() const;
%Docstring
 Returns the distance units used by the scalebar.
 \see setUnits()
 :rtype: QgsUnitTypes.DistanceUnit
%End

    void setUnits( QgsUnitTypes::DistanceUnit units );
%Docstring
 Sets the distance ``units`` used by the scalebar.
 \see units()
%End

    double mapUnitsPerScaleBarUnit() const;
%Docstring
 Returns the number of map units per scale bar unit used by the scalebar.
 \see setMapUnitsPerScaleBarUnit()
 :rtype: float
%End

    void setMapUnitsPerScaleBarUnit( double units );
%Docstring
 Sets the number of map ``units`` per scale bar unit used by the scalebar.
 \see mapUnitsPerScaleBarUnit()
%End

    QString unitLabel() const;
%Docstring
 Returns the label for units.
 \see setUnitLabel()
 :rtype: str
%End

    void setUnitLabel( const QString &label );
%Docstring
 Sets the ``label`` for units.
 \see unitLabel()
%End

    QFont font() const;
%Docstring
 Returns the font used for drawing text in the scalebar.
 \see setFont()
 :rtype: QFont
%End

    void setFont( const QFont &font );
%Docstring
 Sets the ``font`` used for drawing text in the scalebar.
 \see setFont()
%End

    QColor fontColor() const;
%Docstring
 Returns the color used for drawing text in the scalebar.
 \see setFontColor()
 \see font()
 :rtype: QColor
%End

    void setFontColor( const QColor &color );
%Docstring
 Sets the ``color`` used for drawing text in the scalebar.
 \see fontColor()
 \see setFont()
%End

    QColor fillColor() const;
%Docstring
 Returns the color used for fills in the scalebar.
 \see setFillColor()
 \see fillColor2()
 :rtype: QColor
%End

    void setFillColor( const QColor &color );
%Docstring
 Sets the ``color`` used for fills in the scalebar.
 \see fillColor()
 \see setFillColor2()
%End

    QColor fillColor2() const;
%Docstring
 Returns the secondary color used for fills in the scalebar.
 \see setFillColor2()
 \see fillColor()
 :rtype: QColor
%End

    void setFillColor2( const QColor &color );
%Docstring
 Sets the secondary ``color`` used for fills in the scalebar.
 \see fillColor2()
 \see setFillColor2()
%End

    QColor lineColor() const;
%Docstring
 Returns the color used for lines in the scalebar.
 \see setLineColor()
 :rtype: QColor
%End

    void setLineColor( const QColor &color );
%Docstring
 Sets the ``color`` used for lines in the scalebar.
 \see lineColor()
%End

    double lineWidth() const;
%Docstring
 Returns the line width in millimeters for lines in the scalebar.
 \see setLineWidth()
 :rtype: float
%End

    void setLineWidth( double width );
%Docstring
 Sets the line ``width`` in millimeters for lines in the scalebar.
 \see lineWidth()
%End

    QPen pen() const;
%Docstring
 Returns the pen used for drawing outlines in the scalebar.
 \see setPen()
 \see brush()
 :rtype: QPen
%End

    void setPen( const QPen &pen );
%Docstring
 Sets the pen used for drawing outlines in the scalebar.
 \see pen()
%End

    QBrush brush() const;
%Docstring
 Returns the primary brush used for filling the scalebar.
 \see setBrush()
 \see brush2()
 \see pen()
 :rtype: QBrush
%End

    void setBrush( const QBrush &brush );
%Docstring
 Sets the primary brush used for filling the scalebar.
 \see brush()
%End

    QBrush brush2() const;
%Docstring
 Returns the secondary brush for the scalebar. This is used for alternating color style scalebars, such
 as single and double box styles.
 \see setBrush2()
 \see brush()
 :rtype: QBrush
%End

    void setBrush2( const QBrush &brush );
%Docstring
 Sets the secondary brush used for filling the scalebar.
 \see brush()
%End

    double height() const;
%Docstring
 Returns the scalebar height (in millimeters).
 \see setHeight()
 :rtype: float
%End

    void setHeight( double height );
%Docstring
 Sets the scalebar ``height`` (in millimeters).
 \see height()
%End

    double labelBarSpace() const;
%Docstring
 Returns the spacing (in millimeters) between labels and the scalebar.
 \see setLabelBarSpace()
 :rtype: float
%End

    void setLabelBarSpace( double space );
%Docstring
 Sets the spacing (in millimeters) between labels and the scalebar.
 \see labelBarSpace()
%End

    double boxContentSpace() const;
%Docstring
 Returns the spacing (margin) between the scalebar box and content in millimeters.
 \see setBoxContentSpace()
 :rtype: float
%End

    void setBoxContentSpace( double space );
%Docstring
 Sets the ``space`` (margin) between the scalebar box and content in millimeters.
 \see boxContentSpace()
%End

    Alignment alignment() const;
%Docstring
 Returns the scalebar alignment.
 \see setAlignment()
 :rtype: Alignment
%End

    void setAlignment( Alignment alignment );
%Docstring
 Sets the scalebar ``alignment``.
 \see alignment()
%End

    Qt::PenJoinStyle lineJoinStyle() const;
%Docstring
 Returns the join style used for drawing lines in the scalebar.
 \see setLineJoinStyle()
 :rtype: Qt.PenJoinStyle
%End

    void setLineJoinStyle( Qt::PenJoinStyle style );
%Docstring
 Sets the join ``style`` used when drawing the lines in the scalebar
 \see lineJoinStyle()
%End

    Qt::PenCapStyle lineCapStyle() const;
%Docstring
 Returns the cap style used for drawing lines in the scalebar.
 \see setLineCapStyle()
 :rtype: Qt.PenCapStyle
%End

    void setLineCapStyle( Qt::PenCapStyle style );
%Docstring
 Sets the cap ``style`` used when drawing the lines in the scalebar.
 \see lineCapStyle()
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/scalebar/qgsscalebarsettings.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
