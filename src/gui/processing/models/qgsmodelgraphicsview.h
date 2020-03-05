/***************************************************************************
                             qgsmodelgraphicsview.h
                             -----------------------
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

#ifndef QGSMODELGRAPHICVIEW_H
#define QGSMODELGRAPHICVIEW_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsprocessingcontext.h"
#include <QGraphicsView>

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief QGraphicsView subclass representing the model designer.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelGraphicsView : public QGraphicsView
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsModelGraphicsView, with the specified \a parent widget.
     */
    QgsModelGraphicsView( QWidget *parent = nullptr );

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    void dragMoveEvent( QDragMoveEvent *event ) override;
    void wheelEvent( QWheelEvent *event ) override;
    void enterEvent( QEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;

  signals:

    /**
     * Emitted when an algorithm is dropped onto the view.
     */
    void algorithmDropped( const QString &algorithmId, const QPointF &pos );

    /**
     * Emitted when an input parameter is dropped onto the view.
     */
    void inputDropped( const QString &inputId, const QPointF &pos );

  private:
    QPoint mPreviousMousePos;

};

///@endcond

#endif // QGSMODELGRAPHICVIEW_H
