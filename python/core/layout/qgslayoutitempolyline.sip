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
 :rtype: QgsLayoutItemPolyline
%End

    virtual int type() const;

    virtual QString stringType() const;

    virtual QString displayName() const;


    QgsLineSymbol *symbol();
%Docstring
 Returns the line symbol used to draw the shape.
.. seealso:: setSymbol()
 :rtype: QgsLineSymbol
%End

    void setSymbol( QgsLineSymbol *symbol );
%Docstring
 Sets the ``symbol`` used to draw the shape.
 Ownership of ``symbol`` is not transferred.
.. seealso:: symbol()
%End

  protected:

    virtual bool _addNode( const int indexPoint, QPointF newPoint, const double radius );

    virtual bool _removeNode( const int nodeIndex );

    virtual void _draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual void _readXmlStyle( const QDomElement &elmt, const QgsReadWriteContext &context );

    virtual void _writeXmlStyle( QDomDocument &doc, QDomElement &elmt, const QgsReadWriteContext &context ) const;


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempolyline.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
