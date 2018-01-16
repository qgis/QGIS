/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitempolygon.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsLayoutItemPolygon: QgsLayoutNodesItem
{
%Docstring
Layout item for node based polygon shapes.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitempolygon.h"
%End
  public:

    QgsLayoutItemPolygon( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutItemPolygon for the specified ``layout``.
%End

    QgsLayoutItemPolygon( const QPolygonF &polygon, QgsLayout *layout );
%Docstring
Constructor for QgsLayoutItemPolygon for the specified ``polygon``
and ``layout``.
%End

    static QgsLayoutItemPolygon *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new polygon item for the specified ``layout``.

The caller takes responsibility for deleting the returned object.
%End

    virtual int type() const;

    virtual QIcon icon() const;

    virtual QString displayName() const;


    QgsFillSymbol *symbol();
%Docstring
Returns the fill symbol used to draw the shape.

.. seealso:: :py:func:`setSymbol`
%End

    void setSymbol( QgsFillSymbol *symbol );
%Docstring
Sets the ``symbol`` used to draw the shape.
Ownership of ``symbol`` is not transferred.

.. seealso:: :py:func:`symbol`
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
 * src/core/layout/qgslayoutitempolygon.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
