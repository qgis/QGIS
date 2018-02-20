/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgslinesymbollayer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsSimpleLineSymbolLayer : QgsLineSymbolLayer
{

%TypeHeaderCode
#include "qgslinesymbollayer.h"
%End
  public:
    QgsSimpleLineSymbolLayer( const QColor &color = DEFAULT_SIMPLELINE_COLOR,
                              double width = DEFAULT_SIMPLELINE_WIDTH,
                              Qt::PenStyle penStyle = DEFAULT_SIMPLELINE_PENSTYLE );


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;


    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context );


    virtual void renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsSimpleLineSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual QString ogrFeatureStyle( double mmScaleFactor, double mapUnitScaleFactor ) const;


    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;



    Qt::PenStyle penStyle() const;
    void setPenStyle( Qt::PenStyle style );

    Qt::PenJoinStyle penJoinStyle() const;
    void setPenJoinStyle( Qt::PenJoinStyle style );

    Qt::PenCapStyle penCapStyle() const;
    void setPenCapStyle( Qt::PenCapStyle style );

    bool useCustomDashPattern() const;
    void setUseCustomDashPattern( bool b );

    void setCustomDashPatternUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for lengths used in the custom dash pattern.

:param unit: length units

.. seealso:: :py:func:`customDashPatternUnit`
%End

    QgsUnitTypes::RenderUnit customDashPatternUnit() const;
%Docstring
Returns the units for lengths used in the custom dash pattern.

.. seealso:: :py:func:`setCustomDashPatternUnit`
%End

    const QgsMapUnitScale &customDashPatternMapUnitScale() const;
    void setCustomDashPatternMapUnitScale( const QgsMapUnitScale &scale );

    QVector<qreal> customDashVector() const;
    void setCustomDashVector( const QVector<qreal> &vector );

    bool drawInsidePolygon() const;
    void setDrawInsidePolygon( bool drawInsidePolygon );

    virtual QVector<qreal> dxfCustomDashPattern( QgsUnitTypes::RenderUnit &unit ) const;

    virtual Qt::PenStyle dxfPenStyle() const;


    virtual double dxfWidth( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;

    virtual double dxfOffset( const QgsDxfExport &e, QgsSymbolRenderContext &context ) const;

    virtual QColor dxfColor( QgsSymbolRenderContext &context ) const;


  protected:




};



class QgsMarkerLineSymbolLayer : QgsLineSymbolLayer
{

%TypeHeaderCode
#include "qgslinesymbollayer.h"
%End
  public:
    QgsMarkerLineSymbolLayer( bool rotateMarker = DEFAULT_MARKERLINE_ROTATE,
                              double interval = DEFAULT_MARKERLINE_INTERVAL );

    enum Placement
    {
      Interval,
      Vertex,
      LastVertex,
      FirstVertex,
      CentralPoint,
      CurvePoint
    };


    static QgsSymbolLayer *create( const QgsStringMap &properties = QgsStringMap() ) /Factory/;
%Docstring
Create a new MarkerLineSymbolLayerV2

:param properties: A property map to deserialize saved information from properties()

:return: A new MarkerLineSymbolLayerV2
%End

    static QgsSymbolLayer *createFromSld( QDomElement &element ) /Factory/;
%Docstring
Create a new MarkerLineSymbolLayerV2 from SLD

:param element: An SLD XML DOM element

:return: A new MarkerLineSymbolLayerV2
%End


    virtual QString layerType() const;


    virtual void startRender( QgsSymbolRenderContext &context );


    virtual void stopRender( QgsSymbolRenderContext &context );


    virtual void renderPolyline( const QPolygonF &points, QgsSymbolRenderContext &context );


    virtual void renderPolygonStroke( const QPolygonF &points, QList<QPolygonF> *rings, QgsSymbolRenderContext &context );


    virtual QgsStringMap properties() const;


    virtual QgsMarkerLineSymbolLayer *clone() const /Factory/;


    virtual void toSld( QDomDocument &doc, QDomElement &element, const QgsStringMap &props ) const;


    virtual void setColor( const QColor &color );

    virtual QColor color() const;


    virtual QgsSymbol *subSymbol();

    virtual bool setSubSymbol( QgsSymbol *symbol /Transfer/ );


    virtual void setWidth( double width );

    virtual double width() const;


    virtual double estimateMaxBleed( const QgsRenderContext &context ) const;



    bool rotateMarker() const;
%Docstring
Shall the marker be rotated.

:return: True if the marker should be rotated.
%End

    void setRotateMarker( bool rotate );
%Docstring
Shall the marker be rotated.
%End

    double interval() const;
%Docstring
Returns the interval between individual markers. Units are specified through intervalUnits().

.. seealso:: :py:func:`setInterval`

.. seealso:: :py:func:`intervalUnit`
%End

    void setInterval( double interval );
%Docstring
Sets the interval between individual markers.

:param interval: interval size. Units are specified through setIntervalUnits()

.. seealso:: :py:func:`interval`

.. seealso:: :py:func:`setIntervalUnits`
%End

    Placement placement() const;
%Docstring
The placement of the markers.
%End

    void setPlacement( Placement p );
%Docstring
The placement of the markers.
%End

    double offsetAlongLine() const;
%Docstring
Returns the offset along the line for the marker placement. For Interval placements, this is the distance
between the start of the line and the first marker. For FirstVertex and LastVertex placements, this is the
distance between the marker and the start of the line or the end of the line respectively.
This setting has no effect for Vertex or CentralPoint placements.

:return: The offset along the line. The unit for the offset is retrievable via offsetAlongLineUnit.

.. versionadded:: 2.3

.. seealso:: :py:func:`setOffsetAlongLine`

.. seealso:: :py:func:`offsetAlongLineUnit`

.. seealso:: :py:func:`placement`
%End

    void setOffsetAlongLine( double offsetAlongLine );
%Docstring
Sets the the offset along the line for the marker placement. For Interval placements, this is the distance
between the start of the line and the first marker. For FirstVertex and LastVertex placements, this is the
distance between the marker and the start of the line or the end of the line respectively.
This setting has no effect for Vertex or CentralPoint placements.

:param offsetAlongLine: Distance to offset markers along the line. The offset
unit is set via setOffsetAlongLineUnit.

.. versionadded:: 2.3

.. seealso:: :py:func:`offsetAlongLine`

.. seealso:: :py:func:`setOffsetAlongLineUnit`

.. seealso:: :py:func:`setPlacement`
%End

    QgsUnitTypes::RenderUnit offsetAlongLineUnit() const;
%Docstring
Returns the unit used for calculating the offset along line for markers.

:return: Offset along line unit type.

.. seealso:: :py:func:`setOffsetAlongLineUnit`

.. seealso:: :py:func:`offsetAlongLine`
%End

    void setOffsetAlongLineUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the unit used for calculating the offset along line for markers.

:param unit: Offset along line unit type.

.. seealso:: :py:func:`offsetAlongLineUnit`

.. seealso:: :py:func:`setOffsetAlongLine`
%End

    const QgsMapUnitScale &offsetAlongLineMapUnitScale() const;
%Docstring
Returns the map unit scale used for calculating the offset in map units along line for markers.

:return: Offset along line map unit scale.
%End

    void setOffsetAlongLineMapUnitScale( const QgsMapUnitScale &scale );
%Docstring
Sets the map unit scale used for calculating the offset in map units along line for markers.

:param scale: Offset along line map unit scale.
%End

    void setIntervalUnit( QgsUnitTypes::RenderUnit unit );
%Docstring
Sets the units for the interval between markers.

:param unit: interval units

.. seealso:: :py:func:`intervalUnit`

.. seealso:: :py:func:`setInterval`
%End

    QgsUnitTypes::RenderUnit intervalUnit() const;
%Docstring
Returns the units for the interval between markers.

.. seealso:: :py:func:`setIntervalUnit`

.. seealso:: :py:func:`interval`
%End

    void setIntervalMapUnitScale( const QgsMapUnitScale &scale );
    const QgsMapUnitScale &intervalMapUnitScale() const;

    virtual void setOutputUnit( QgsUnitTypes::RenderUnit unit );

    virtual QgsUnitTypes::RenderUnit outputUnit() const;


    virtual void setMapUnitScale( const QgsMapUnitScale &scale );

    virtual QgsMapUnitScale mapUnitScale() const;


    virtual QSet<QString> usedAttributes( const QgsRenderContext &context ) const;


    virtual void setDataDefinedProperty( QgsSymbolLayer::Property key, const QgsProperty &property );



  protected:

    void renderPolylineInterval( const QPolygonF &points, QgsSymbolRenderContext &context );
    void renderPolylineVertex( const QPolygonF &points, QgsSymbolRenderContext &context, Placement placement = Vertex );
    void renderPolylineCentral( const QPolygonF &points, QgsSymbolRenderContext &context );
    double markerAngle( const QPolygonF &points, bool isRing, int vertex );


  private:
    QgsMarkerLineSymbolLayer( const QgsMarkerLineSymbolLayer &other );
};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/symbology/qgslinesymbollayer.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
