/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerarrow.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerArrow: QgsComposerItem
{
%Docstring
An item that draws an arrow between two points.
%End

%TypeHeaderCode
#include "qgscomposerarrow.h"
%End
  public:

    enum MarkerMode
    {
      DefaultMarker,
      NoMarker,
      SVGMarker
    };

    QgsComposerArrow( QgsComposition *c /TransferThis/ );
%Docstring
Constructor

:param c: parent composition
%End

    QgsComposerArrow( QPointF startPoint, QPointF stopPoint, QgsComposition *c /TransferThis/ );
%Docstring
Constructor

:param startPoint: start point for line
:param stopPoint: end point for line
:param c: parent composition
%End

    ~QgsComposerArrow();

    virtual int type() const;
%Docstring
Return composer item type.
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint - draw on canvas
%End

    virtual void setSceneRect( const QRectF &rectangle );

%Docstring
Modifies position of start and endpoint and calls QgsComposerItem.setSceneRect
%End

    void setArrowHeadWidth( double width );
%Docstring
Sets the width of the arrow head in mm

:param width: width of arrow head

.. seealso:: :py:func:`arrowHeadWidth`
%End

    double arrowHeadWidth() const;
%Docstring
Returns the width of the arrow head in mm

:return: width of arrow head

.. seealso:: :py:func:`setArrowHeadWidth`
%End

    void setStartMarker( const QString &svgPath );
%Docstring
Sets the marker to draw at the start of the line

:param svgPath: file path for svg marker graphic to draw

.. seealso:: :py:func:`startMarker`

.. seealso:: :py:func:`setEndMarker`
%End

    QString startMarker() const;
%Docstring
Returns the marker drawn at the start of the line

:return: file path for svg marker graphic

.. seealso:: :py:func:`setStartMarker`

.. seealso:: :py:func:`endMarker`
%End

    void setEndMarker( const QString &svgPath );
%Docstring
Sets the marker to draw at the end of the line

:param svgPath: file path for svg marker graphic to draw

.. seealso:: :py:func:`endMarker`

.. seealso:: :py:func:`setStartMarker`
%End

    QString endMarker() const;
%Docstring
Returns the marker drawn at the end of the line

:return: file path for svg marker graphic

.. seealso:: :py:func:`setEndMarker`

.. seealso:: :py:func:`startMarker`
%End

    QColor arrowHeadStrokeColor() const;
%Docstring
Returns the color used to draw stroke around the the arrow head.

:return: arrow head stroke color

.. seealso:: :py:func:`arrowHeadFillColor`

.. seealso:: :py:func:`setArrowHeadStrokeColor`

.. versionadded:: 2.5
%End

    void setArrowHeadStrokeColor( const QColor &color );
%Docstring
Sets the color used to draw the stroke around the arrow head.

:param color: arrow head stroke color

.. seealso:: :py:func:`setArrowHeadFillColor`

.. seealso:: :py:func:`arrowHeadStrokeColor`

.. versionadded:: 2.5
%End

    QColor arrowHeadFillColor() const;
%Docstring
Returns the color used to fill the arrow head.

:return: arrow head fill color

.. seealso:: :py:func:`arrowHeadStrokeColor`

.. seealso:: :py:func:`setArrowHeadFillColor`

.. versionadded:: 2.5
%End

    void setArrowHeadFillColor( const QColor &color );
%Docstring
Sets the color used to fill the arrow head.

:param color: arrow head fill color

.. seealso:: :py:func:`arrowHeadFillColor`

.. seealso:: :py:func:`setArrowHeadStrokeColor`

.. versionadded:: 2.5
%End

    void setArrowHeadStrokeWidth( const double width );
%Docstring
Sets the pen width for the stroke of the arrow head

:param width: pen width for arrow head stroke

.. seealso:: :py:func:`arrowHeadStrokeWidth`

.. seealso:: :py:func:`setArrowHeadStrokeColor`

.. versionadded:: 2.5
%End

    double arrowHeadStrokeWidth() const;
%Docstring
Returns the pen width for the stroke of the arrow head

:return: pen width for arrow head stroke

.. seealso:: :py:func:`setArrowHeadStrokeWidth`

.. seealso:: :py:func:`arrowHeadStrokeColor`

.. versionadded:: 2.5
%End

    void setLineSymbol( QgsLineSymbol *symbol /Transfer/ );
%Docstring
Sets the line symbol used for drawing the line portion of the arrow

:param symbol: line symbol

.. seealso:: :py:func:`lineSymbol`

.. versionadded:: 2.5
%End

    QgsLineSymbol *lineSymbol();
%Docstring
Returns the line symbol used for drawing the line portion of the arrow

:return: line symbol

.. seealso:: :py:func:`setLineSymbol`

.. versionadded:: 2.5
%End

    MarkerMode markerMode() const;
%Docstring
Returns marker mode, which controls how the arrow endpoints are drawn

:return: marker mode

.. seealso:: :py:func:`setMarkerMode`
%End

    void setMarkerMode( MarkerMode mode );
%Docstring
Sets the marker mode, which controls how the arrow endpoints are drawn

:param mode: marker mode

.. seealso:: :py:func:`setMarkerMode`
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores state in DOM element

:param elem: is DOM element corresponding to 'Composer' tag
:param doc: document
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets state from DOM document

:param itemElem: is DOM node corresponding to item tag
:param doc: is the document to read
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerarrow.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
