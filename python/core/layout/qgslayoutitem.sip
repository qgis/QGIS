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

#include <qgslayoutitemshape.h>
#include <qgslayoutitempage.h>
%End

%ConvertToSubClassCode
    // the conversions have to be static, because they're using multiple inheritance
    // (seen in PyQt4 .sip files for some QGraphicsItem classes)
    switch ( sipCpp->type() )
    {
      // really, these *should* use the constants from QgsLayoutItemRegistry, but sip doesn't like that!
      case QGraphicsItem::UserType + 101:
        sipType = sipType_QgsLayoutItemPage;
        *sipCppRet = static_cast<QgsLayoutItemPage *>( sipCpp );
        break;
      default:
        sipType = 0;
    }
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

    virtual int type() const = 0;
%Docstring
 Return correct graphics item type
.. seealso:: stringType()
 :rtype: int
%End

    virtual QString stringType() const = 0;
%Docstring
 Return the item type as a string.

 This string must be a unique, single word, character only representation of the item type, eg "LayoutScaleBar"
.. seealso:: type()
 :rtype: str
%End

    QString uuid() const;
%Docstring
 Returns the item identification string. This is a unique random string set for the item
 upon creation.
.. note::

   There is no corresponding setter for the uuid - it's created automatically.
.. seealso:: id()
.. seealso:: setId()
 :rtype: str
%End

    QString id() const;
%Docstring
 Returns the item's ID name. This is not necessarily unique, and duplicate ID names may exist
 for a layout.
.. seealso:: setId()
.. seealso:: uuid()
 :rtype: str
%End

    virtual void setId( const QString &id );
%Docstring
 Set the item's ``id`` name. This is not necessarily unique, and duplicate ID names may exist
 for a layout.
.. seealso:: id()
.. seealso:: uuid()
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

    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
 Stores the item state in a DOM element.
 \param parentElement parent DOM element (e.g. 'Layout' element)
 \param document DOM document
 \param context read write context
.. seealso:: readXml()
.. note::

   Subclasses should ensure that they call writePropertiesToElement() in their implementation.
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElement, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
 Sets the item state from a DOM element.
 \param itemElement is the DOM node corresponding to item (e.g. 'LayoutItem' element)
 \param document DOM document
 \param context read write context
.. seealso:: writeXml()
.. note::

   Subclasses should ensure that they call readPropertiesFromElement() in their implementation.
 :rtype: bool
%End

  public slots:

    virtual void refresh();

%Docstring
 Refreshes the item, causing a recalculation of any property overrides and
 recalculation of its position and size.
%End

    virtual void redraw();
%Docstring
 Triggers a redraw (update) of the item.
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

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;
%Docstring
 Stores item state within an XML DOM element.
 \param element is the DOM element to store the item's properties in
 \param document DOM document
 \param context read write context
.. seealso:: writeXml()
.. seealso:: readPropertiesFromElement()
.. note::

   derived classes must call this base implementation when overriding this method
 :rtype: bool
%End

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );
%Docstring
 Sets item state from a DOM element.
 \param element is the DOM element for the item
 \param document DOM document
 \param context read write context
.. seealso:: writePropertiesToElement()
.. seealso:: readXml()
.. note::

   derived classes must call this base implementation when overriding this method
 :rtype: bool
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitem.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
