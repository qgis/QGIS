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
 Base class for layout items which are basic shapes (e.g. rectangles, ellipses).
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemshape.h"
%End
  public:

    void setSymbol( QgsFillSymbol *symbol );
%Docstring
 Sets the fill ``symbol`` used to draw the shape. Ownership is not transferred
 and a clone of the symbol is made.
.. seealso:: symbol()
%End

    QgsFillSymbol *symbol();
%Docstring
 Returns the fill symbol used to draw the shape.
.. seealso:: setSymbol()
 :rtype: QgsFillSymbol
%End

  protected:

    explicit QgsLayoutItemShape( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemShape, with the specified parent ``layout``.
%End

};


class QgsLayoutItemRectangularShape : QgsLayoutItemShape
{
%Docstring
 A rectangular shape item for layouts.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemshape.h"
%End
  public:

    explicit QgsLayoutItemRectangularShape( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemRectangularShape, with the specified parent ``layout``.
%End
    virtual int type() const;

    static QgsLayoutItemRectangularShape *create( QgsLayout *layout, const QVariantMap &settings ) /Factory/;
%Docstring
 Returns a new rectangular item for the specified ``layout``.

 The caller takes responsibility for deleting the returned object.
 :rtype: QgsLayoutItemRectangularShape
%End

    void setCornerRadius( QgsLayoutMeasurement radius );
%Docstring
 Sets the corner ``radius`` for rounded rectangle corners.
.. seealso:: cornerRadius()
%End

    QgsLayoutMeasurement cornerRadius() const;
%Docstring
 Returns the corner radius for rounded rectangle corners.
.. seealso:: setCornerRadius()
 :rtype: QgsLayoutMeasurement
%End

  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );


};

class QgsLayoutItemEllipseShape : QgsLayoutItemShape
{
%Docstring
 A ellipse shape item for layouts.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemshape.h"
%End
  public:

    explicit QgsLayoutItemEllipseShape( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemEllipseShape, with the specified parent ``layout``.
%End
    virtual int type() const;

    static QgsLayoutItemEllipseShape *create( QgsLayout *layout, const QVariantMap &settings ) /Factory/;
%Docstring
 Returns a new ellipse item for the specified ``layout``.

 The caller takes responsibility for deleting the returned object.
 :rtype: QgsLayoutItemEllipseShape
%End

  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );


};

class QgsLayoutItemTriangleShape : QgsLayoutItemShape
{
%Docstring
 A triangle shape item for layouts.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemshape.h"
%End
  public:

    explicit QgsLayoutItemTriangleShape( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemTriangleShape, with the specified parent ``layout``.
%End
    virtual int type() const;

    static QgsLayoutItemTriangleShape *create( QgsLayout *layout, const QVariantMap &settings ) /Factory/;
%Docstring
 Returns a new triangle item for the specified ``layout``.

 The caller takes responsibility for deleting the returned object.
 :rtype: QgsLayoutItemTriangleShape
%End

  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemshape.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
