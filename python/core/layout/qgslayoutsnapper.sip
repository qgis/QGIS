/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutsnapper.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsLayoutSnapper: QgsLayoutSerializableObject
{
%Docstring
 Manages snapping grids and preset snap lines in a layout, and handles
snapping points to the nearest grid coordinate/snap line when possible.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutsnapper.h"
%End
  public:

    QgsLayoutSnapper( QgsLayout *layout );
%Docstring
Constructor for QgsLayoutSnapper, attached to the specified ``layout``.
%End

    virtual QString stringType() const;
    virtual QgsLayout *layout();


    void setSnapTolerance( const int snapTolerance );
%Docstring
Sets the snap ``tolerance`` (in pixels) to use when snapping.

.. seealso:: :py:func:`snapTolerance`
%End

    int snapTolerance() const;
%Docstring
Returns the snap tolerance (in pixels) to use when snapping.

.. seealso:: :py:func:`setSnapTolerance`
%End

    bool snapToGrid() const;
%Docstring
Returns true if snapping to grid is enabled.

.. seealso:: :py:func:`setSnapToGrid`
%End

    void setSnapToGrid( bool enabled );
%Docstring
Sets whether snapping to grid is ``enabled``.

.. seealso:: :py:func:`snapToGrid`
%End

    bool snapToGuides() const;
%Docstring
Returns true if snapping to guides is enabled.

.. seealso:: :py:func:`setSnapToGuides`
%End

    void setSnapToGuides( bool enabled );
%Docstring
Sets whether snapping to guides is ``enabled``.

.. seealso:: :py:func:`snapToGuides`
%End

    bool snapToItems() const;
%Docstring
Returns true if snapping to items is enabled.

.. seealso:: :py:func:`setSnapToItems`
%End

    void setSnapToItems( bool enabled );
%Docstring
Sets whether snapping to items is ``enabled``.

.. seealso:: :py:func:`snapToItems`
%End

    QPointF snapPoint( QPointF point, double scaleFactor, bool &snapped /Out/, QGraphicsLineItem *horizontalSnapLine = 0,
                       QGraphicsLineItem *verticalSnapLine = 0,
                       const QList< QgsLayoutItem * > *ignoreItems = 0 ) const;
%Docstring
Snaps a layout coordinate ``point``. If ``point`` was snapped, ``snapped`` will be set to true.

The ``scaleFactor`` argument should be set to the transformation from
scalar transform from layout coordinates to pixels, i.e. the
graphics view transform().m11() value.

This method considers snapping to the grid, snap lines, etc.

If the ``horizontalSnapLine`` and ``verticalSnapLine`` arguments are specified, then the snapper
will automatically display and position these lines to indicate snapping positions to item bounds.

A list of items to ignore during the snapping can be specified via the ``ignoreItems`` list.

.. seealso:: :py:func:`snapRect`
%End

    QRectF snapRect( const QRectF &rect, double scaleFactor, bool &snapped /Out/, QGraphicsLineItem *horizontalSnapLine = 0,
                     QGraphicsLineItem *verticalSnapLine = 0,
                     const QList< QgsLayoutItem * > *ignoreItems = 0 ) const;
%Docstring
Snaps a layout coordinate ``rect``. If ``rect`` was snapped, ``snapped`` will be set to true.

Snapping occurs by moving the rectangle alone. The rectangle will not be resized
as a result of the snap operation.

The ``scaleFactor`` argument should be set to the transformation from
scalar transform from layout coordinates to pixels, i.e. the
graphics view transform().m11() value.

This method considers snapping to the grid, snap lines, etc.

If the ``horizontalSnapLine`` and ``verticalSnapLine`` arguments are specified, then the snapper
will automatically display and position these lines to indicate snapping positions to item bounds.

A list of items to ignore during the snapping can be specified via the ``ignoreItems`` list.

.. seealso:: :py:func:`snapPoint`
%End

    QPointF snapPointToGrid( QPointF point, double scaleFactor, bool &snappedX /Out/, bool &snappedY /Out/ ) const;
%Docstring
Snaps a layout coordinate ``point`` to the grid. If ``point``
was snapped horizontally, ``snappedX`` will be set to true. If ``point``
was snapped vertically, ``snappedY`` will be set to true.

The ``scaleFactor`` argument should be set to the transformation from
scalar transform from layout coordinates to pixels, i.e. the
graphics view transform().m11() value.

If snapToGrid() is disabled, this method will return the point
unchanged.

.. seealso:: :py:func:`snapPointsToGrid`
%End

    QPointF snapPointsToGrid( const QList< QPointF > &points, double scaleFactor, bool &snappedX /Out/, bool &snappedY /Out/ ) const;
%Docstring
Snaps a set of ``points`` to the grid. If the points
were snapped, ``snapped`` will be set to true.

The ``scaleFactor`` argument should be set to the transformation from
scalar transform from layout coordinates to pixels, i.e. the
graphics view transform().m11() value.

If snapToGrid() is disabled, this method will not attempt to snap the points.

The returned value is the smallest delta which the points need to be shifted by in order to align
one of the points to the grid.

.. seealso:: :py:func:`snapPointToGrid`
%End

    double snapPointToGuides( double original, Qt::Orientation orientation, double scaleFactor, bool &snapped /Out/ ) const;
%Docstring
Snaps an ``original`` layout coordinate to the guides. If the point
was snapped, ``snapped`` will be set to true.

The ``scaleFactor`` argument should be set to the transformation from
scalar transform from layout coordinates to pixels, i.e. the
graphics view transform().m11() value.

If snapToGuides() is disabled, this method will return the point
unchanged.

.. seealso:: :py:func:`snapPointsToGuides`
%End

    double snapPointsToGuides( const QList< double > &points, Qt::Orientation orientation, double scaleFactor, bool &snapped /Out/ ) const;
%Docstring
Snaps a set of ``points`` to the guides. If the points
were snapped, ``snapped`` will be set to true.

The ``scaleFactor`` argument should be set to the transformation from
scalar transform from layout coordinates to pixels, i.e. the
graphics view transform().m11() value.

If snapToGuides() is disabled, this method will not attempt to snap the points.

The returned value is the smallest delta which the points need to be shifted by in order to align
one of the points to a guide.

.. seealso:: :py:func:`snapPointToGuides`
%End

    double snapPointToItems( double original, Qt::Orientation orientation, double scaleFactor, const QList< QgsLayoutItem * > &ignoreItems, bool &snapped /Out/,
                             QGraphicsLineItem *snapLine = 0 ) const;
%Docstring
Snaps an ``original`` layout coordinate to the item bounds. If the point
was snapped, ``snapped`` will be set to true.

The ``scaleFactor`` argument should be set to the transformation from
scalar transform from layout coordinates to pixels, i.e. the
graphics view transform().m11() value.

If snapToItems() is disabled, this method will return the point
unchanged.

A list of items to ignore during the snapping can be specified via the ``ignoreItems`` list.

If ``snapLine`` is specified, the snapper will automatically show (or hide) the snap line
based on the result of the snap, and position it at the correct location for the snap.

.. seealso:: :py:func:`snapPointsToItems`
%End

    double snapPointsToItems( const QList< double > &points, Qt::Orientation orientation, double scaleFactor, const QList< QgsLayoutItem * > &ignoreItems, bool &snapped /Out/,
                              QGraphicsLineItem *snapLine = 0 ) const;
%Docstring
Snaps a set of ``points`` to the item bounds. If the points
were snapped, ``snapped`` will be set to true.

The ``scaleFactor`` argument should be set to the transformation from
scalar transform from layout coordinates to pixels, i.e. the
graphics view transform().m11() value.

If snapToItems() is disabled, this method will not attempt to snap the points.

The returned value is the smallest delta which the points need to be shifted by in order to align
one of the points to an item bound.

.. seealso:: :py:func:`snapPointToItems`
%End

    virtual bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const;

%Docstring
Stores the snapper's state in a DOM element. The ``parentElement`` should refer to the parent layout's DOM element.

.. seealso:: :py:func:`readXml`
%End

    virtual bool readXml( const QDomElement &gridElement, const QDomDocument &document, const QgsReadWriteContext &context );

%Docstring
Sets the snapper's state from a DOM element. snapperElement is the DOM node corresponding to the snapper.

.. seealso:: :py:func:`writeXml`
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutsnapper.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
