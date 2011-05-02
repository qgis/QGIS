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
#include "qgsannotationitem.h"

class QgsMapToolAnnotation: public QgsMapTool
{
  public:
    QgsMapToolAnnotation( QgsMapCanvas* canvas );
    ~QgsMapToolAnnotation();

    void canvasPressEvent( QMouseEvent * e );
    void canvasReleaseEvent( QMouseEvent * e );
    void canvasMoveEvent( QMouseEvent * e );
    void canvasDoubleClickEvent( QMouseEvent * e );
    void keyPressEvent( QKeyEvent* e );

  protected:
    /**Creates a new item. To be implemented by subclasses. Returns 0 by default*/
    virtual QgsAnnotationItem* createItem( QMouseEvent* e );
    /**Creates an editor widget (caller takes ownership)*/
    QDialog* createItemEditor( QgsAnnotationItem* item );

  private:
    /**Returns the topmost annotation item at the position (or 0 if none)*/
    QgsAnnotationItem* itemAtPos( const QPointF& pos );
    QgsAnnotationItem* selectedItem();
    /**Returns a list of all annotationitems in the canvas*/
    QList<QgsAnnotationItem*> annotationItems();
    /**Switches visibility states of text items*/
    void toggleTextItemVisibilities();

    QgsAnnotationItem* mActiveItem;
    QgsAnnotationItem::MouseMoveAction mCurrentMoveAction;
    QPointF mLastMousePosition;

};

#endif // QGSMAPTOOLANNOTATION_H
