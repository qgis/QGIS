/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemshape.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutItemShape : QgsLayoutItem
{
%Docstring
 Layout item for basic filled shapes (e.g. rectangles, ellipses).

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemshape.h"
%End
  public:

    enum Shape
    {
      Ellipse,
      Rectangle,
      Triangle
    };


    explicit QgsLayoutItemShape( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutItemShape, with the specified parent ``layout``.
%End

    static QgsLayoutItemShape *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new shape item for the specified ``layout``.

The caller takes responsibility for deleting the returned object.
%End


    virtual int type() const;

    virtual QIcon icon() const;


    virtual QString displayName() const;


    QgsLayoutItemShape::Shape shapeType() const;
%Docstring
Returns the type of shape (e.g. rectangle, ellipse, etc).

.. seealso:: :py:func:`setShapeType`
%End

    void setShapeType( QgsLayoutItemShape::Shape type );
%Docstring
Sets the ``type`` of shape (e.g. rectangle, ellipse, etc).

.. seealso:: :py:func:`shapeType`
%End

    void setSymbol( QgsFillSymbol *symbol );
%Docstring
Sets the fill ``symbol`` used to draw the shape. Ownership is not transferred
and a clone of the symbol is made.

.. seealso:: :py:func:`symbol`
%End

    QgsFillSymbol *symbol();
%Docstring
Returns the fill symbol used to draw the shape.

.. seealso:: :py:func:`setSymbol`
%End

    void setCornerRadius( QgsLayoutMeasurement radius );
%Docstring
Sets the corner ``radius`` for rounded rectangle corners.

.. seealso:: :py:func:`cornerRadius`
%End

    QgsLayoutMeasurement cornerRadius() const;
%Docstring
Returns the corner radius for rounded rectangle corners.

.. seealso:: :py:func:`setCornerRadius`
%End

    virtual QRectF boundingRect() const;


    virtual double estimatedFrameBleed() const;


  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );


    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemshape.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
