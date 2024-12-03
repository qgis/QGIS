/***************************************************************************
                              qgsmaptoolannotation.h
                              -------------------------
  begin                : February 9, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLANNOTATION_H
#define QGSMAPTOOLANNOTATION_H

#include "qgsmaptool.h"
#include "qgis_app.h"
#include "qgsmapcanvasannotationitem.h"

class QgsAnnotation;

class APP_EXPORT QgsMapToolAnnotation : public QgsMapTool
{
    Q_OBJECT

  public:
    QgsMapToolAnnotation( QgsMapCanvas *canvas );

    Flags flags() const override;

    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void canvasReleaseEvent( QgsMapMouseEvent *e ) override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasDoubleClickEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;
    bool populateContextMenuWithEvent( QMenu *menu, QgsMapMouseEvent *event ) override;

  protected:
    /**
     * Creates a new item. To be implemented by subclasses.
     */
    virtual QgsAnnotation *createItem() const { return nullptr; }

    //! Creates an editor widget (caller takes ownership)
    QDialog *createItemEditor( QgsMapCanvasAnnotationItem *item );

  private:
    //! Returns the topmost annotation item at the position (or 0 if none)
    QgsMapCanvasAnnotationItem *itemAtPos( QPointF pos ) const;
    QgsMapCanvasAnnotationItem *selectedItem() const;
    //! Returns a list of all annotationitems in the canvas
    QList<QgsMapCanvasAnnotationItem *> annotationItems() const;

    QgsPointXY transformCanvasToAnnotation( QgsPointXY p, QgsAnnotation *annotation ) const;

    QgsMapCanvasAnnotationItem::MouseMoveAction mCurrentMoveAction = QgsMapCanvasAnnotationItem::NoAction;
    QPoint mLastMousePosition = QPoint( 0, 0 );
};

#endif // QGSMAPTOOLANNOTATION_H
