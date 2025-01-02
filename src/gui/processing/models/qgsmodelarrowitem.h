/***************************************************************************
                             qgsmodelarrowitem.h
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELARROWITEM_H
#define QGSMODELARROWITEM_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QGraphicsPathItem>
#include <QObject>

class QgsModelComponentGraphicItem;

///@cond NOT_STABLE


/**
 * \ingroup gui
 * \brief A link arrow item for use in the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelArrowItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT
  public:
    enum Marker
    {
      Circle,
      ArrowHead,
    };

    /**
     * Constructor for QgsModelArrowItem, with the specified \a parent item.
     *
     * The arrow will link \a startItem to \a endItem, joining the specified \a startEdge and \a startIndex
     * to \a endEdge and \a endIndex.
     */
    QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Qt::Edge startEdge, int startIndex, bool startIsOutgoing, Marker startMarker, QgsModelComponentGraphicItem *endItem, Qt::Edge endEdge, int endIndex, bool endIsIncoming, Marker endMarker );

    /**
     * Constructor for QgsModelArrowItem, with the specified \a parent item.
     *
     * The arrow will link \a startItem to \a endItem, joining the specified \a startEdge and \a startIndex
     * to an automatic point on \a endItem.
     */
    QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Qt::Edge startEdge, int startIndex, Marker startMarker, QgsModelComponentGraphicItem *endItem, Marker endMarker );

    /**
     * Constructor for QgsModelArrowItem, with the specified \a parent item.
     *
     * The arrow will link \a startItem to \a endItem, joining an automatic point on \a startItem to the specified
     * \a endEdge and \a endIndex.
     */
    QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Marker startMarker, QgsModelComponentGraphicItem *endItem, Qt::Edge endEdge, int endIndex, Marker endMarker );

    /**
     * Constructor for QgsModelArrowItem, with the specified \a parent item.
     *
     * The arrow will link \a startItem to \a endItem, joining an automatic points on both items.
     */
    QgsModelArrowItem( QgsModelComponentGraphicItem *startItem, Marker startMarker, QgsModelComponentGraphicItem *endItem, Marker endMarker );

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

    /**
     * Sets the pen \a style to use for rendering the arrow line.
     */
    void setPenStyle( Qt::PenStyle style );

  public slots:

    /**
     * Updates the cached path linking the two items.
     */
    void updatePath();

  private:
    QPointF bezierPointForCurve( const QPointF &point, Qt::Edge edge, bool incoming, bool hasSpecificDirectionalFlow ) const;

    void drawArrowHead( QPainter *painter, const QPointF &point, const QPointF &vector );

    QgsModelComponentGraphicItem *mStartItem = nullptr;
    Qt::Edge mStartEdge = Qt::LeftEdge;
    int mStartIndex = -1;
    bool mStartIsOutgoing = true;
    Marker mStartMarker;

    QgsModelComponentGraphicItem *mEndItem = nullptr;
    Qt::Edge mEndEdge = Qt::LeftEdge;
    int mEndIndex = -1;
    bool mEndIsIncoming = false;
    Marker mEndMarker;

    QPointF mStartPoint;
    QPointF mEndPoint;

    QColor mColor;
};

///@endcond

#endif // QGSMODELARROWITEM_H
