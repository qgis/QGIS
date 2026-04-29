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
#include <QGraphicsRectItem>
#include <QObject>

class QgsModelComponentGraphicItem;
class QgsModelArrowItem;

///@cond NOT_STABLE


/**
 * \ingroup gui
 * \brief A item for showing a "badge" on the midpoint of an arrow item.
 * \warning Not stable API
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsModelDesignerArrowBadgeItem : public QGraphicsRectItem SIP_SKIP
{
  public:
    QgsModelDesignerArrowBadgeItem( QgsModelArrowItem *link SIP_TRANSFERTHIS );

    /**
     * Sets the \a center point of the badge, in parent item coordinates.
     */
    void setCenter( const QPointF &center );

    /**
     * Returns the parent arrow item.
     */
    QgsModelArrowItem *arrow();

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

    /**
     * Sets the \a value to display in the badge.
     *
     * \see value()
     */
    void setValue( const QVariant &value );

    /**
     * Returns the value displayed in the badge.
     *
     * \see setValue()
     */
    QVariant value() const;

    /**
     * Returns the badge text to show for a specified \a value.
     */
    static QString textForValue( const QVariant &value );

  private:
    void resizeToContents();

    static constexpr int FONT_SIZE = 10; // Font size for the feature count text
    static constexpr double BORDER_RADIUS = 5;
    static constexpr double CONTENTS_MARGIN = 0.2;
    QVariant mValue;
};


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
      NoMarker
    };

    /**
     * Constructor for QgsModelArrowItem, with the specified \a parent item.
     *
     * The arrow will link \a startItem to \a endItem, joining the specified \a startEdge and \a startIndex
     * to \a endEdge and \a endIndex.
     */
    QgsModelArrowItem(
      QgsModelComponentGraphicItem *startItem,
      Qt::Edge startEdge,
      int startIndex,
      bool startIsOutgoing,
      Marker startMarker,
      QgsModelComponentGraphicItem *endItem,
      Qt::Edge endEdge,
      int endIndex,
      bool endIsIncoming,
      Marker endMarker
    );

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

    /**
     * Returns the item at the start of the arrow.
     *
     * \see endItem()
     */
    QgsModelComponentGraphicItem *startItem();

    /**
     * Returns the edge of the start item that arrow begins at.
     *
     * \see endEdge()
     */
    Qt::Edge startEdge() { return mStartEdge; }

    /**
     * Returns the index of the start item that arrow begins at.
     *
     * \see endIndex()
     */
    int startIndex() { return mStartIndex; }

    /**
     * Returns the item at the end of the arrow.
     *
     * \see startItem()
     */
    QgsModelComponentGraphicItem *endItem();

    /**
     * Returns the edge of the end item that arrow ends at.
     *
     * \see startEdge()
     */
    Qt::Edge endEdge() { return mEndEdge; }

    /**
     * Returns the index of the end item that arrow ends at.
     *
     * \see startIndex()
     */
    int endIndex() { return mEndIndex; }

    /**
     * Returns the optional badge item attached to the arrow.
     *
     * If setShowBadge() has not been called to show the item, the NULLPTR will be returned.
     */
    SIP_SKIP QgsModelDesignerArrowBadgeItem *badgeItem();

    /**
     * Sets whether the arrow's badge item should be shown.
     *
     * \see badgeItem();
     */
    SIP_SKIP void setShowBadge( bool visible );

  signals:
    /**
     * Emitted when the path is updated.
     *
     * \since QGIS 4.0
     */
    void painterPathUpdated();

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

    // Optional child badge item. Not created by default.
    QgsModelDesignerArrowBadgeItem *mBadgeItem = nullptr;

    QPointF mStartPoint;
    QPointF mEndPoint;

    QColor mColor;
};

///@endcond

#endif // QGSMODELARROWITEM_H
