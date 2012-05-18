/***************************************************************************
    qgsundowidget.h
    ---------------------
    begin                : June 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
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

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDockWidget>
#include <QtGui/QGridLayout>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>
#include <QUndoView>
#include <QUndoStack>

class QgsMapCanvas;
class QgsMapLayer;

/**
 * Class that handles undo display fo undo commands
 */
class QgsUndoWidget : public QDockWidget
{
    Q_OBJECT
  public:
    QWidget *dockWidgetContents;
    QGridLayout *gridLayout;
    QSpacerItem *spacerItem;
    QPushButton *undoButton;
    QPushButton *redoButton;
    QSpacerItem *spacerItem1;

    QgsUndoWidget( QWidget * parent, QgsMapCanvas* mapCanvas );
    void setupUi( QDockWidget *UndoWidget );
    void retranslateUi( QDockWidget *UndoWidget );

    /**
     * Setting new undo stack for undo view
     */
    void setUndoStack( QUndoStack * undoStack );

    /**
     * Handles destroying of stack when active layer is changed
     */
    void destroyStack();

  public slots:
    /**
     * Changes undo stack which is displayed by undo view
     */
    void layerChanged( QgsMapLayer * layer );

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
    void indexChanged( int value );

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
    QUndoView * mUndoView;
    QUndoStack * mUndoStack;
    QgsMapCanvas* mMapCanvas;

};


#endif // QGSUNDOWIDGET_H

