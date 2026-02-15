/***************************************************************************
                    qgsmaptoolselectannotationmousehandles.h
                             -----------------------
    begin                : November 2025
    copyright            : (C) 2025 by Mathieu Pellrin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMAPTOOLSELECTANNOTATIONMOUSEHANDLES_H
#define QGSMAPTOOLSELECTANNOTATIONMOUSEHANDLES_H

// We don't want to expose this in the public API

#include <memory>

#include "qgis_gui.h"
#include "qgsgraphicsviewmousehandles.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolselectannotation.h"

#include <QPointer>

#define SIP_NO_FILE

class QGraphicsView;
class QInputEvent;
class QgsMapToolSelectAnnotation;

///@cond PRIVATE

/**
 * \ingroup gui
 * \brief Handles drawing of selection outlines and mouse handles for annotations in a QgsMapCanvas
 *
 * Also is responsible for mouse interactions such as resizing and moving selected annotation items.
 *
 * \note not available in Python bindings
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsMapToolSelectAnnotationMouseHandles : public QgsGraphicsViewMouseHandles
{
    Q_OBJECT
  public:
    QgsMapToolSelectAnnotationMouseHandles( QgsMapToolSelectAnnotation *mapTool, QgsMapCanvas *canvas );

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

  protected:
    void setViewportCursor( Qt::CursorShape cursor ) override;
    QList<QGraphicsItem *> sceneItemsAtPoint( QPointF scenePoint ) override;
    QList<QGraphicsItem *> selectedSceneItems( bool includeLockedItems = true ) const override;
    QRectF itemRect( QGraphicsItem *item ) const override;
    void moveItem( QGraphicsItem *item, double deltaX, double deltaY ) override;
    void rotateItem( QGraphicsItem *item, double deltaDegree, double deltaCenterX, double deltaCenterY ) override;
    void setItemRect( QGraphicsItem *item, QRectF rect ) override;

  public slots:

    void selectionChanged();

  private:
    QPointer<QgsMapToolSelectAnnotation> mMapTool;
    QPointer<QgsMapCanvas> mCanvas;
};

///@endcond PRIVATE

#endif // QGSMAPTOOLSELECTANNOTATIONMOUSEHANDLES_H
