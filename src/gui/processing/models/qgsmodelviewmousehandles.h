/***************************************************************************
                             qgsmodelviewmousehandles.h
                             -----------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMODELVIEWMOUSEHANDLES_H
#define QGSMODELVIEWMOUSEHANDLES_H

#define SIP_NO_FILE

#include "qgsgraphicsviewmousehandles.h"
#include <QPointer>
#include <memory>

#include "qgis_gui.h"

class QgsModelGraphicsView;
class QgsModelGraphicsScene;
class QInputEvent;

class QgsLayoutItem;

///@cond PRIVATE

/**
 * \ingroup gui
 * \brief Handles drawing of selection outlines and mouse handles in a QgsModelGraphicsView
 *
 * Also is responsible for mouse interactions such as resizing and moving selected items.
 *
 * \note not available in Python bindings
 *
 * \since QGIS 3.14
*/
class GUI_EXPORT QgsModelViewMouseHandles : public QgsGraphicsViewMouseHandles
{
    Q_OBJECT
  public:
    QgsModelViewMouseHandles( QgsModelGraphicsView *view );

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

  protected:
    void setViewportCursor( Qt::CursorShape cursor ) override;
    QList<QGraphicsItem *> sceneItemsAtPoint( QPointF scenePoint ) override;
    QList<QGraphicsItem *> selectedSceneItems( bool includeLockedItems = true ) const override;
    QRectF itemRect( QGraphicsItem *item ) const override;
    QRectF storedItemRect( QGraphicsItem *item ) const override;
    void moveItem( QGraphicsItem *item, double deltaX, double deltaY ) override;
    void previewItemMove( QGraphicsItem *item, double deltaX, double deltaY ) override;
    void setItemRect( QGraphicsItem *item, QRectF rect ) override;
    QRectF previewSetItemRect( QGraphicsItem *item, QRectF rect ) override;
    void startMacroCommand( const QString &text ) override;
    void endMacroCommand() override;
    QPointF snapPoint( QPointF originalPoint, SnapGuideMode mode, bool snapHorizontal = true, bool snapVertical = true ) override;

  public slots:

    //! Sets up listeners to sizeChanged signal for all selected items
    void selectionChanged();

  private:
    QgsModelGraphicsScene *modelScene() const;

    QPointer<QgsModelGraphicsView> mView;
};

///@endcond PRIVATE

#endif // QGSMODELVIEWMOUSEHANDLES_H
