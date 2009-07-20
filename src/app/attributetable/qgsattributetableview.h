/***************************************************************************
     QgsAttributeTableView.h
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLEVIEW_H
#define QGSATTRIBUTETABLEVIEW_H

#include <QTableView>

class QgsAttributeTableModel;
class QgsAttributeTableFilterModel;

class QgsVectorLayer;


class QgsAttributeTableView: public QTableView
{
  public:
    QgsAttributeTableView( QWidget* parent = NULL );
    virtual ~QgsAttributeTableView();

    /**
     * Sets the layer
     * @param layer layer pointer
     */
    void setLayer( QgsVectorLayer* layer );

    /**
     * Saves geometry to the settings on close
     * @param event not used
     */
    void closeEvent( QCloseEvent *event );
    /**
     * Handles Ctrl or Shift key press
     * @param event the key pressed
     */
    void keyPressEvent( QKeyEvent *event );
    /**
     * Handles Ctrl or Shift key release
     * @param event the key released
     */
    void keyReleaseEvent( QKeyEvent *event );
    /**
     * Returns true if shift was pressed
     */
    bool shiftPressed() { return mShiftPressed; }
    /**
     * Returns true if ctrl was pressed
     */
    bool ctrlPressed() { return mCtrlPressed; }

  private:
    bool mShiftPressed;
    bool mCtrlPressed;

    QgsAttributeTableModel* mModel;
    QgsAttributeTableFilterModel* mFilterModel;
};

#endif
