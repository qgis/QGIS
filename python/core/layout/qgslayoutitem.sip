/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitem.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItem : QgsLayoutObject, QGraphicsRectItem
{
%Docstring
 Base class for graphical items within a QgsLayout.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitem.h"
%End
  public:

    enum ReferencePoint
    {
      UpperLeft,
      UpperMiddle,
      UpperRight,
      MiddleLeft,
      Middle,
      MiddleRight,
      LowerLeft,
      LowerMiddle,
      LowerRight,
    };

    explicit QgsLayoutItem( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItem, with the specified parent ``layout``.
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Handles preparing a paint surface for the layout item and painting the item's
 content. Derived classes must not override this method, but instead implement
 the pure virtual method QgsLayoutItem.draw.
%End

    void setReferencePoint( const ReferencePoint &point );
%Docstring
 Sets the reference ``point`` for positioning of the layout item. This point is also
 fixed during resizing of the item, and any size changes will be performed
 so that the position of the reference point within the layout remains unchanged.
.. seealso:: referencePoint()
%End

    ReferencePoint referencePoint() const;
%Docstring
 Returns the reference point for positioning of the layout item. This point is also
 fixed during resizing of the item, and any size changes will be performed
 so that the position of the reference point within the layout remains unchanged.
.. seealso:: setReferencePoint()
 :rtype: ReferencePoint
%End

    QgsLayoutSize fixedSize() const;
%Docstring
 Returns the fixed size of the item, if applicable, or an empty size if item can be freely
 resized.
.. seealso:: setFixedSize()
.. seealso:: minimumSize()
 :rtype: QgsLayoutSize
%End

    virtual QgsLayoutSize minimumSize() const;
%Docstring
 Returns the minimum allowed size of the item, if applicable, or an empty size if item can be freely
 resized.
.. seealso:: setMinimumSize()
.. seealso:: fixedSize()
 :rtype: QgsLayoutSize
%End

    virtual void attemptResize( const QgsLayoutSize &size );
%Docstring
 Attempts to resize the item to a specified target ``size``. Note that the final size of the
 item may not match the specified target size, as items with a fixed or minimum
 size will place restrictions on the allowed item size. Data defined item size overrides
 will also override the specified target size.
.. seealso:: minimumSize()
.. seealso:: fixedSize()
.. seealso:: attemptMove()
.. seealso:: sizeWithUnits()
%End

    virtual void attemptMove( const QgsLayoutPoint &point );
%Docstring
 Attempts to move the item to a specified ``point``. This method respects the item's
 reference point, in that the item will be moved so that its current reference
 point is placed at the specified target point.
 Note that the final position of the item may not match the specified target position,
 as data defined item position may override the specified value.
.. seealso:: attemptResize()
.. seealso:: referencePoint()
.. seealso:: positionWithUnits()
%End

    QgsLayoutPoint positionWithUnits() const;
%Docstring
 Returns the item's current position, including units. The position returned
 is the position of the item's reference point, which may not necessarily be the top
 left corner of the item.
.. seealso:: attemptMove()
.. seealso:: referencePoint()
.. seealso:: sizeWithUnits()
 :rtype: QgsLayoutPoint
%End

    QgsLayoutSize sizeWithUnits() const;
%Docstring
 Returns the item's current size, including units.
.. seealso:: attemptResize()
.. seealso:: positionWithUnits()
 :rtype: QgsLayoutSize
%End

    double itemRotation() const;
%Docstring
 :rtype: float
%End

  public slots:

    virtual void refresh();

%Docstring
 Refreshes the item, causing a recalculation of any property overrides and
 recalculation of its position and size.
%End

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );
%Docstring
 Refreshes a data defined ``property`` for the item by reevaluating the property's value
 and redrawing the item with this new value. If ``property`` is set to
 QgsLayoutObject.AllProperties then all data defined properties for the item will be
 refreshed.
%End

    virtual void setItemRotation( const double rotation );
%Docstring
 Sets the layout item's ``rotation``, in degrees clockwise. This rotation occurs around the center of the item.
.. seealso:: itemRotation()
.. seealso:: rotateItem()
%End

    virtual void rotateItem( const double angle, const QPointF &transformOrigin );
%Docstring
 Rotates the item by a specified ``angle`` in degrees clockwise around a specified reference point.
.. seealso:: setItemRotation()
.. seealso:: itemRotation()
%End

  protected:

    virtual void drawDebugRect( QPainter *painter );
%Docstring
 Draws a debugging rectangle of the item's current bounds within the specified
 painter.
 @param painter destination QPainter
%End

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 ) = 0;
%Docstring
 Draws the item's contents using the specified render ``context``.
 Note that the context's painter has been scaled so that painter units are pixels.
 Use the QgsRenderContext methods to convert from millimeters or other units to the painter's units.
%End

    virtual void setFixedSize( const QgsLayoutSize &size );
%Docstring
 Sets a fixed ``size`` for the layout item, which prevents it from being freely
 resized. Set an empty size if item can be freely resized.
.. seealso:: fixedSize()
.. seealso:: setMinimumSize()
%End

    virtual void setMinimumSize( const QgsLayoutSize &size );
%Docstring
 Sets the minimum allowed ``size`` for the layout item. Set an empty size if item can be freely
 resized.
.. seealso:: minimumSize()
.. seealso:: setFixedSize()
%End

    void refreshItemSize();
%Docstring
 Refreshes an item's size by rechecking it against any possible item fixed
 or minimum sizes.
.. seealso:: setFixedSize()
.. seealso:: setMinimumSize()
.. seealso:: refreshItemPosition()
%End

    void refreshItemPosition();
%Docstring
 Refreshes an item's position by rechecking it against any possible overrides
 such as data defined positioning.
.. seealso:: refreshItemSize()
%End

    void refreshItemRotation();
%Docstring
 Refreshes an item's rotation by rechecking it against any possible overrides
 such as data defined rotation.
.. seealso:: refreshItemSize()
.. seealso:: refreshItemPosition()
%End

    QPointF adjustPointForReferencePosition( const QPointF &point, const QSizeF &size, const ReferencePoint &reference ) const;
%Docstring
 Adjusts the specified ``point`` at which a ``reference`` position of the item
 sits and returns the top left corner of the item, if reference point where placed at the specified position.
 :rtype: QPointF
%End

    QPointF positionAtReferencePoint( const ReferencePoint &reference ) const;
%Docstring
 Returns the current position (in layout units) of a ``reference`` point for the item.
 :rtype: QPointF
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitem.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
