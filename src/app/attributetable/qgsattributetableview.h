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

    void setLayer( QgsVectorLayer* layer );

    void closeEvent( QCloseEvent *event );
    void keyPressEvent( QKeyEvent *event );
    void keyReleaseEvent( QKeyEvent *event );
    bool shiftPressed() { return mShiftPressed; }
    bool ctrlPressed() { return mCtrlPressed; }

  private:
    bool mShiftPressed;
    bool mCtrlPressed;

    QgsAttributeTableModel* mModel;
    QgsAttributeTableFilterModel* mFilterModel;
};

#endif
