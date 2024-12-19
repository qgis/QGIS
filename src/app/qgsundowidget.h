/***************************************************************************
    qgsundowidget.h
    ---------------------
    begin                : June 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSUNDOWIDGET_H
#define QGSUNDOWIDGET_H

#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QGridLayout>
#include <QPushButton>
#include <QSpacerItem>
#include <QWidget>
#include <QUndoView>
#include <QUndoStack>
#include <QPointer>

#include "qgspanelwidget.h"
#include "qgis_app.h"

class QgsMapCanvas;
class QgsMapLayer;

/**
 * Class that handles undo display for undo commands
 */
class APP_EXPORT QgsUndoWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:

    QgsUndoWidget( QWidget *parent, QgsMapCanvas *mapCanvas );

    /**
     * Setting new undo stack for undo view
     */
    void setUndoStack( QUndoStack *undoStack );

    /**
     * Show or hide the undo/redo buttons on the widget.
     * \param show Show or hide the undo/redo buttons.
     */
    void setButtonsVisible( bool show );

    /**
     * Handles destroying of stack when active layer is changed
     */
    void unsetStack();

    //! Access to dock's contents
    QWidget *dockContents() { return mDockWidgetContents; }

  public slots:

    /**
     * Slot to handle undo changed signal
     */
    void undoChanged( bool value );

    /**
     * Slot to handle redo changed signal
     */
    void redoChanged( bool value );

    /**
     * Slot to handle index changed signal
     */
    void indexChanged( int curIndx );

    /**
     * Undo operation called from button push
     */
    void undo();

    /**
     * Redo operation called from button push
     */
    void redo();

  signals:
    void undoStackChanged();

  private:
    QUndoView *mUndoView = nullptr;
    QPointer< QUndoStack > mUndoStack;
    QgsMapCanvas *mMapCanvas = nullptr;

    int mPreviousIndex = 0;
    int mPreviousCount = 0;

    QWidget *mDockWidgetContents = nullptr;
    QGridLayout *mGridLayout = nullptr;
    QPushButton *mUndoButton = nullptr;
    QPushButton *mRedoButton = nullptr;
};


#endif // QGSUNDOWIDGET_H

