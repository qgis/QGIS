/***************************************************************************
     BeataView.h
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

#ifndef BEATAVIEW_H_
#define BEATAVIEW_H_

#include <QTableView>

class QgsVectorLayer;


class BeataView: public QTableView
{
//private slots:
    //void setRows(int rows);

  public:
    BeataView( QWidget* parent = NULL );
    virtual ~BeataView();

    void setLayer( QgsVectorLayer* layer );

    void closeEvent( QCloseEvent *event );
    void keyPressEvent( QKeyEvent *event );
    void keyReleaseEvent( QKeyEvent *event );

    //make those private
    bool shiftPressed;
    bool ctrlPressed;
};

#endif
