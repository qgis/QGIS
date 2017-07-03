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

    QgsLayoutItem( QgsLayout *layout );

    virtual int type() const = 0;
%Docstring
Return correct graphics item type.
 :rtype: int
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Handles preparing a paint surface for the layout item and painting the item's
 content. Derived classes must not override this method, but instead implement
 the pure virtual method QgsLayoutItem.draw.
%End

  protected:

    virtual void drawDebugRect( QPainter *painter );
%Docstring
 Draws a debugging rectangle of the item's current bounds within the specified
 painter.
 @param painter destination QPainter
%End

    virtual void draw( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) = 0;
%Docstring
 Draws the item's contents on a specified ``painter``.
%End

};




/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitem.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
