/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempolyline.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemPolyline: QgsLayoutNodesItem
{
%Docstring
Layout item for node based polyline shapes.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitempolyline.h"
%End
  public:

    enum MarkerMode
    {
      NoMarker,
      ArrowHead,
      SvgMarker,
    };

    QgsLayoutItemPolyline( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutItemPolyline for the specified ``layout``.
%End

    QgsLayoutItemPolyline( const QPolygonF &polyline, QgsLayout *layout );
%Docstring
Constructor for QgsLayoutItemPolyline for the specified ``polyline``
and ``layout``.
%End

    static QgsLayoutItemPolyline *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new polyline item for the specified ``layout``.

The caller takes responsibility for deleting the returned object.
%End

    virtual int type() const;

    virtual QIcon icon() const;

    virtual QString displayName() const;


    QgsLineSymbol *symbol();
%Docstring
Returns the line symbol used to draw the shape.

.. seealso:: :py:func:`setSymbol`
%End

    void setSymbol( QgsLineSymbol *symbol );
%Docstring
Sets the ``symbol`` used to draw the shape.
Ownership of ``symbol`` is not transferred.

.. seealso:: :py:func:`symbol`
%End

    MarkerMode startMarker() const;
%Docstring
Returns the start marker mode, which controls what marker is drawn at the start of the line.

.. seealso:: :py:func:`setStartMarker`

.. seealso:: :py:func:`endMarker`
%End

    void setStartMarker( MarkerMode mode );
%Docstring
Sets the start marker ``mode``, which controls what marker is drawn at the start of the line.

.. seealso:: :py:func:`startMarker`

.. seealso:: :py:func:`setEndMarker`
%End

    MarkerMode endMarker() const;
%Docstring
Returns the end marker mode, which controls what marker is drawn at the end of the line.

.. seealso:: :py:func:`setEndMarker`

.. seealso:: :py:func:`startMarker`
%End

    void setEndMarker( MarkerMode mode );
%Docstring
Sets the end marker ``mode``, which controls what marker is drawn at the end of the line.

.. seealso:: :py:func:`endMarker`

.. seealso:: :py:func:`setStartMarker`
%End

    void setArrowHeadWidth( double width );
%Docstring
Sets the ``width`` of line arrow heads in mm.

.. seealso:: :py:func:`arrowHeadWidth`
%End

    double arrowHeadWidth() const;
%Docstring
Returns the width of line arrow heads in mm.

.. seealso:: :py:func:`setArrowHeadWidth`
%End

    void setStartSvgMarkerPath( const QString &path );
%Docstring
Sets the ``path`` to a SVG marker to draw at the start of the line.

.. seealso:: :py:func:`startSvgMarkerPath`

.. seealso:: :py:func:`setEndSvgMarkerPath`
%End

    QString startSvgMarkerPath() const;
%Docstring
Returns the path the an SVG marker drawn at the start of the line.

.. seealso:: :py:func:`setStartSvgMarkerPath`

.. seealso:: :py:func:`endSvgMarkerPath`
%End

    void setEndSvgMarkerPath( const QString &path );
%Docstring
Sets the ``path`` to a SVG marker to draw at the end of the line.

.. seealso:: :py:func:`endSvgMarkerPath`

.. seealso:: :py:func:`setStartSvgMarkerPath`
%End

    QString endSvgMarkerPath() const;
%Docstring
Returns the path the an SVG marker drawn at the end of the line.

.. seealso:: :py:func:`setEndSvgMarkerPath`

.. seealso:: :py:func:`startSvgMarkerPath`
%End

    QColor arrowHeadStrokeColor() const;
%Docstring
Returns the color used to draw the stroke around the the arrow head.

.. seealso:: :py:func:`arrowHeadFillColor`

.. seealso:: :py:func:`setArrowHeadStrokeColor`
%End

    void setArrowHeadStrokeColor( const QColor &color );
%Docstring
Sets the ``color`` used to draw the stroke around the arrow head.

.. seealso:: :py:func:`setArrowHeadFillColor`

.. seealso:: :py:func:`arrowHeadStrokeColor`
%End

    QColor arrowHeadFillColor() const;
%Docstring
Returns the color used to fill the arrow head.

.. seealso:: :py:func:`arrowHeadStrokeColor`

.. seealso:: :py:func:`setArrowHeadFillColor`
%End

    void setArrowHeadFillColor( const QColor &color );
%Docstring
Sets the ``color`` used to fill the arrow head.

.. seealso:: :py:func:`arrowHeadFillColor`

.. seealso:: :py:func:`setArrowHeadStrokeColor`
%End

    void setArrowHeadStrokeWidth( double width );
%Docstring
Sets the pen ``width`` in millimeters for the stroke of the arrow head

.. seealso:: :py:func:`arrowHeadStrokeWidth`

.. seealso:: :py:func:`setArrowHeadStrokeColor`
%End

    double arrowHeadStrokeWidth() const;
%Docstring
Returns the pen width in millimeters for the stroke of the arrow head.

.. seealso:: :py:func:`setArrowHeadStrokeWidth`

.. seealso:: :py:func:`arrowHeadStrokeColor`
%End

  protected:

    virtual bool _addNode( const int indexPoint, QPointF newPoint, const double radius );

    virtual bool _removeNode( const int nodeIndex );

    virtual void _draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual void _readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context );

    virtual void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const;

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


  protected slots:

    virtual void updateBoundingRect();


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempolyline.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
