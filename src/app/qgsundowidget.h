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

#include "qgspanelwidget.h"
#include "qgis_app.h"

class QgsMapCanvas;
class QgsMapLayer;

/**
 * Class that handles undo display fo undo commands
 */
class APP_EXPORT QgsUndoWidget : public QgsPanelWidget
{
    Q_OBJECT
  public:
    QWidget *dockWidgetContents = nullptr;
    QGridLayout *gridLayout = nullptr;
    QSpacerItem *spacerItem = nullptr;
    QPushButton *undoButton = nullptr;
    QPushButton *redoButton = nullptr;
    QSpacerItem *spacerItem1 = nullptr;

    QgsUndoWidget( QWidget *parent, QgsMapCanvas *mapCanvas );
    void setupUi( QWidget *UndoWidget );
    void retranslateUi( QWidget *UndoWidget );

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
    void destroyStack();

    //! Access to dock's contents
    QWidget *dockContents() { return dockWidgetContents; }

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
    QUndoStack *mUndoStack = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;

    int mPreviousIndex;
    int mPreviousCount;
};


#endif // QGSUNDOWIDGET_H

