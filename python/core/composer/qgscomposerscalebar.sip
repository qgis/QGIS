/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerscalebar.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerScaleBar: QgsComposerItem
{
%Docstring
 A scale bar item that can be added to a map composition.
%End

%TypeHeaderCode
#include "qgscomposerscalebar.h"
%End
  public:

    QgsComposerScaleBar( QgsComposition *composition /TransferThis/ );
    ~QgsComposerScaleBar();

    virtual int type() const;
%Docstring
Return correct graphics item type.
 :rtype: int
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint
%End

    int numSegments() const;
%Docstring
 :rtype: int
%End
    void setNumSegments( int nSegments );

    int numSegmentsLeft() const;
%Docstring
 :rtype: int
%End
    void setNumSegmentsLeft( int nSegmentsLeft );

    double numUnitsPerSegment() const;
%Docstring
 :rtype: float
%End
    void setNumUnitsPerSegment( double units );

    QgsScaleBarSettings::SegmentSizeMode segmentSizeMode() const;
%Docstring
 Returns the size mode for scale bar segments.
.. seealso:: setSegmentSizeMode
.. seealso:: minBarWidth
.. seealso:: maxBarWidth
.. versionadded:: 2.9
 :rtype: QgsScaleBarSettings.SegmentSizeMode
%End

    void setSegmentSizeMode( QgsScaleBarSettings::SegmentSizeMode mode );
%Docstring
 Sets the size mode for scale bar segments.
 \param mode size mode
.. seealso:: segmentSizeMode
.. seealso:: setMinBarWidth
.. seealso:: setMaxBarWidth
.. versionadded:: 2.9
%End

    double minBarWidth() const;
%Docstring
 Returns the minimum size (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
.. seealso:: segmentSizeMode
.. seealso:: setMinBarWidth
.. seealso:: maxBarWidth
.. versionadded:: 2.9
 :rtype: float
%End

    void setMinBarWidth( double minWidth );
%Docstring
 Sets the minimum size (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
 \param minWidth minimum width in millimeters
.. seealso:: minBarWidth
.. seealso:: setMaxBarWidth
.. seealso:: setSegmentSizeMode
.. versionadded:: 2.9
%End

    double maxBarWidth() const;
%Docstring
 Returns the maximum size (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
.. seealso:: segmentSizeMode
.. seealso:: setMaxBarWidth
.. seealso:: minBarWidth
.. versionadded:: 2.9
 :rtype: float
%End

    void setMaxBarWidth( double maxWidth );
%Docstring
 Sets the maximum size (in millimeters) for scale bar segments. This
 property is only effective if the segmentSizeMode() is set
 to SegmentSizeFitWidth.
 \param maxWidth maximum width in millimeters
.. seealso:: minBarWidth
.. seealso:: setMaxBarWidth
.. seealso:: setSegmentSizeMode
.. versionadded:: 2.9
%End

    double numMapUnitsPerScaleBarUnit() const;
%Docstring
 :rtype: float
%End
    void setNumMapUnitsPerScaleBarUnit( double d );

    QString unitLabeling() const;
%Docstring
 :rtype: str
%End
    void setUnitLabeling( const QString &label );

    QFont font() const;
%Docstring
 :rtype: QFont
%End
    void setFont( const QFont &font );

    QColor fontColor() const;
%Docstring
 Returns the color used for drawing text in the scalebar.
 :return: font color for scalebar.
.. seealso:: setFontColor
.. seealso:: font
 :rtype: QColor
%End

    void setFontColor( const QColor &c );
%Docstring
 Sets the color used for drawing text in the scalebar.
 \param c font color for scalebar.
.. seealso:: fontColor
.. seealso:: setFont
%End

    QColor fillColor() const;
%Docstring
 Returns the color used for fills in the scalebar.
.. seealso:: setFillColor()
.. seealso:: fillColor2()
.. versionadded:: 3.0
 :rtype: QColor
%End

    void setFillColor( const QColor &color );
%Docstring
 Sets the color used for fills in the scalebar.
.. seealso:: fillColor()
.. seealso:: setFillColor2()
.. versionadded:: 3.0
%End

    QColor fillColor2() const;
%Docstring
 Returns the secondary color used for fills in the scalebar.
.. seealso:: setFillColor2()
.. seealso:: fillColor()
.. versionadded:: 3.0
 :rtype: QColor
%End

    void setFillColor2( const QColor &color );
%Docstring
 Sets the secondary color used for fills in the scalebar.
.. seealso:: fillColor2()
.. seealso:: setFillColor2()
.. versionadded:: 3.0
%End

    QColor lineColor() const;
%Docstring
 Returns the color used for lines in the scalebar.
.. seealso:: setLineColor()
.. versionadded:: 3.0
 :rtype: QColor
%End

    void setLineColor( const QColor &color );
%Docstring
 Sets the color used for lines in the scalebar.
.. seealso:: lineColor()
.. versionadded:: 3.0
%End

    double lineWidth() const;
%Docstring
 Returns the line width in millimeters for lines in the scalebar.
.. seealso:: setLineWidth()
.. versionadded:: 3.0
 :rtype: float
%End

    void setLineWidth( double width );
%Docstring
 Sets the line width in millimeters for lines in the scalebar.
.. seealso:: lineWidth()
.. versionadded:: 3.0
%End

    QPen pen() const;
%Docstring
 Returns the pen used for drawing the scalebar.
 :return: QPen used for drawing the scalebar outlines.
.. seealso:: setPen
.. seealso:: brush
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
 :rtype: float
%End
    void setHeight( double h );

    void setComposerMap( QgsComposerMap *map );
%Docstring
 Sets the ``map`` item linked to the scalebar.
%End

    QgsComposerMap *composerMap() const;
%Docstring
 Returns the map item linked to the scalebar.
 :rtype: QgsComposerMap
%End

    double labelBarSpace() const;
%Docstring
 :rtype: float
%End
    void setLabelBarSpace( double space );

    double boxContentSpace() const;
%Docstring
 :rtype: float
%End
    void setBoxContentSpace( double space );

    QgsScaleBarSettings::Alignment alignment() const;
%Docstring
 Returns the alignment of the scalebar.
 :rtype: QgsScaleBarSettings.Alignment
%End

    void setAlignment( QgsScaleBarSettings::Alignment alignment );
%Docstring
 Sets the ``alignment`` of the scalebar.
%End

    QgsUnitTypes::DistanceUnit units() const;
%Docstring
 Returns the scalebar distance units.
 :rtype: QgsUnitTypes.DistanceUnit
%End

    void setUnits( QgsUnitTypes::DistanceUnit u );
%Docstring
 Sets the scalebar distance units.
%End

    Qt::PenJoinStyle lineJoinStyle() const;
%Docstring
 Returns the join style used for drawing lines in the scalebar
 :return: Join style for lines
.. versionadded:: 2.3
.. seealso:: setLineJoinStyle
 :rtype: Qt.PenJoinStyle
%End

    void setLineJoinStyle( Qt::PenJoinStyle style );
%Docstring
 Sets join style used when drawing the lines in the scalebar
 \param style Join style for lines
 :return: nothing
.. versionadded:: 2.3
.. seealso:: lineJoinStyle
%End

    Qt::PenCapStyle lineCapStyle() const;
%Docstring
 Returns the cap style used for drawing lines in the scalebar
 :return: Cap style for lines
.. versionadded:: 2.3
.. seealso:: setLineCapStyle
 :rtype: Qt.PenCapStyle
%End

    void setLineCapStyle( Qt::PenCapStyle style );
%Docstring
 Sets cap style used when drawing the lines in the scalebar
 \param style Cap style for lines
 :return: nothing
.. versionadded:: 2.3
.. seealso:: lineCapStyle
%End

    void applyDefaultSettings();
%Docstring
Apply default settings
%End
    void applyDefaultSize( QgsUnitTypes::DistanceUnit u = QgsUnitTypes::DistanceMeters );
%Docstring
Apply default size (scale bar 1/5 of map item width)
%End

    void setStyle( const QString &styleName );
%Docstring
 Sets style by name
\param styleName (untranslated) style name. Possibilities are: 'Single Box', 'Double Box', 'Line Ticks Middle', 'Line Ticks Down', 'Line Ticks Up', 'Numeric'*
%End

    QString style() const;
%Docstring
Returns style name
 :rtype: str
%End

    void adjustBoxSize();
%Docstring
Sets box size suitable to content
%End

    void update();
%Docstring
Adjusts box size and calls QgsComposerItem.update()
%End


    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
 Stores state in Dom element
 \param elem is Dom element corresponding to 'Composer' tag
 \param doc Dom document
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
 Sets state from Dom document
 \param itemElem is Dom node corresponding to item tag
 \param doc is Dom document
 :rtype: bool
%End

    void correctXPositionAlignment( double width, double widthAfter );
%Docstring
Moves scalebar position to the left / right depending on alignment and change in item width
%End

    virtual void setSceneRect( const QRectF &rectangle );


  public slots:
    void updateSegmentSize();
    void invalidateCurrentMap();
%Docstring
Sets mCompositionMap to 0 if the map is deleted
%End
    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = 0 );

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerscalebar.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
