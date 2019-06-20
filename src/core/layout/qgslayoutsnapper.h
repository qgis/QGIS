/***************************************************************************
                             qgslayoutsnapper.h
                             -------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTSNAPPER_H
#define QGSLAYOUTSNAPPER_H

#include "qgis_core.h"
#include "qgslayoutmeasurement.h"
#include "qgslayoutpoint.h"
#include "qgslayoutguidecollection.h"
#include "qgslayoutserializableobject.h"
#include <QPen>

class QgsLayout;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \class QgsLayoutSnapper
 * \brief Manages snapping grids and preset snap lines in a layout, and handles
 * snapping points to the nearest grid coordinate/snap line when possible.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutSnapper: public QgsLayoutSerializableObject
{

  public:

    /**
     * Constructor for QgsLayoutSnapper, attached to the specified \a layout.
     */
    QgsLayoutSnapper( QgsLayout *layout );

    QString stringType() const override { return QStringLiteral( "LayoutSnapper" ); }
    QgsLayout *layout() override;

    /**
     * Sets the snap \a tolerance (in pixels) to use when snapping.
     * \see snapTolerance()
     */
    void setSnapTolerance( int snapTolerance );

    /**
     * Returns the snap tolerance (in pixels) to use when snapping.
     * \see setSnapTolerance()
     */
    int snapTolerance() const { return mTolerance; }

    /**
     * Returns TRUE if snapping to grid is enabled.
     * \see setSnapToGrid()
     */
    bool snapToGrid() const { return mSnapToGrid; }

    /**
     * Sets whether snapping to grid is \a enabled.
     * \see snapToGrid()
     */
    void setSnapToGrid( bool enabled );

    /**
     * Returns TRUE if snapping to guides is enabled.
     * \see setSnapToGuides()
     */
    bool snapToGuides() const { return mSnapToGuides; }

    /**
     * Sets whether snapping to guides is \a enabled.
     * \see snapToGuides()
     */
    void setSnapToGuides( bool enabled );

    /**
     * Returns TRUE if snapping to items is enabled.
     * \see setSnapToItems()
     */
    bool snapToItems() const { return mSnapToItems; }

    /**
     * Sets whether snapping to items is \a enabled.
     * \see snapToItems()
     */
    void setSnapToItems( bool enabled );

    /**
     * Snaps a layout coordinate \a point. If \a point was snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * This method considers snapping to the grid, snap lines, etc.
     *
     * If the \a horizontalSnapLine and \a verticalSnapLine arguments are specified, then the snapper
     * will automatically display and position these lines to indicate snapping positions to item bounds.
     *
     * A list of items to ignore during the snapping can be specified via the \a ignoreItems list.

     * \see snapRect()
     */
    QPointF snapPoint( QPointF point, double scaleFactor, bool &snapped SIP_OUT, QGraphicsLineItem *horizontalSnapLine = nullptr,
                       QGraphicsLineItem *verticalSnapLine = nullptr,
                       const QList< QgsLayoutItem * > *ignoreItems = nullptr ) const;

    /**
     * Snaps a layout coordinate \a rect. If \a rect was snapped, \a snapped will be set to TRUE.
     *
     * Snapping occurs by moving the rectangle alone. The rectangle will not be resized
     * as a result of the snap operation.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * This method considers snapping to the grid, snap lines, etc.
     *
     * If the \a horizontalSnapLine and \a verticalSnapLine arguments are specified, then the snapper
     * will automatically display and position these lines to indicate snapping positions to item bounds.
     *
     * A list of items to ignore during the snapping can be specified via the \a ignoreItems list.
     *
     * \see snapPoint()
     */
    QRectF snapRect( const QRectF &rect, double scaleFactor, bool &snapped SIP_OUT, QGraphicsLineItem *horizontalSnapLine = nullptr,
                     QGraphicsLineItem *verticalSnapLine = nullptr,
                     const QList< QgsLayoutItem * > *ignoreItems = nullptr ) const;

    /**
     * Snaps a layout coordinate \a point to the grid. If \a point
     * was snapped horizontally, \a snappedX will be set to TRUE. If \a point
     * was snapped vertically, \a snappedY will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGrid() is disabled, this method will return the point
     * unchanged.
     *
     * \see snapPointsToGrid()
     */
    QPointF snapPointToGrid( QPointF point, double scaleFactor, bool &snappedX SIP_OUT, bool &snappedY SIP_OUT ) const;

    /**
     * Snaps a set of \a points to the grid. If the points
     * were snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGrid() is disabled, this method will not attempt to snap the points.
     *
     * The returned value is the smallest delta which the points need to be shifted by in order to align
     * one of the points to the grid.
     *
     * \see snapPointToGrid()
     */
    QPointF snapPointsToGrid( const QList< QPointF > &points, double scaleFactor, bool &snappedX SIP_OUT, bool &snappedY SIP_OUT ) const;

    /**
     * Snaps an \a original layout coordinate to the guides. If the point
     * was snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGuides() is disabled, this method will return the point
     * unchanged.
     *
     * \see snapPointsToGuides()
     */
    double snapPointToGuides( double original, Qt::Orientation orientation, double scaleFactor, bool &snapped SIP_OUT ) const;

    /**
     * Snaps a set of \a points to the guides. If the points
     * were snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToGuides() is disabled, this method will not attempt to snap the points.
     *
     * The returned value is the smallest delta which the points need to be shifted by in order to align
     * one of the points to a guide.
     *
     * \see snapPointToGuides()
     */
    double snapPointsToGuides( const QList< double > &points, Qt::Orientation orientation, double scaleFactor, bool &snapped SIP_OUT ) const;

    /**
     * Snaps an \a original layout coordinate to the item bounds. If the point
     * was snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToItems() is disabled, this method will return the point
     * unchanged.
     *
     * A list of items to ignore during the snapping can be specified via the \a ignoreItems list.
     *
     * If \a snapLine is specified, the snapper will automatically show (or hide) the snap line
     * based on the result of the snap, and position it at the correct location for the snap.
     *
     * \see snapPointsToItems()
     */
    double snapPointToItems( double original, Qt::Orientation orientation, double scaleFactor, const QList< QgsLayoutItem * > &ignoreItems, bool &snapped SIP_OUT,
                             QGraphicsLineItem *snapLine = nullptr ) const;

    /**
     * Snaps a set of \a points to the item bounds. If the points
     * were snapped, \a snapped will be set to TRUE.
     *
     * The \a scaleFactor argument should be set to the transformation from
     * scalar transform from layout coordinates to pixels, i.e. the
     * graphics view transform().m11() value.
     *
     * If snapToItems() is disabled, this method will not attempt to snap the points.
     *
     * The returned value is the smallest delta which the points need to be shifted by in order to align
     * one of the points to an item bound.
     *
     * \see snapPointToItems()
     */
    double snapPointsToItems( const QList< double > &points, Qt::Orientation orientation, double scaleFactor, const QList< QgsLayoutItem * > &ignoreItems, bool &snapped SIP_OUT,
                              QGraphicsLineItem *snapLine = nullptr ) const;

    /**
     * Stores the snapper's state in a DOM element. The \a parentElement should refer to the parent layout's DOM element.
     * \see readXml()
     */
    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const override;

    /**
     * Sets the snapper's state from a DOM element. snapperElement is the DOM node corresponding to the snapper.
     * \see writeXml()
     */
    bool readXml( const QDomElement &gridElement, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

    // Used for 'collapsing' undo commands
    enum UndoCommand
    {
      UndoTolerance = 1,
      UndoSnapToGrid,
      UndoSnapToGuides,
    };

    QgsLayout *mLayout = nullptr;

    int mTolerance = 5;
    bool mSnapToGrid = false;
    bool mSnapToGuides = true;
    bool mSnapToItems = true;

    friend class QgsLayoutSnapperUndoCommand;

};

#endif //QGSLAYOUTSNAPPER_H
