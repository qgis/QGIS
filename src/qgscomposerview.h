/***************************************************************************
                         qgscomposerview.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSCOMPOSERVIEW_H
#define QGSCOMPOSERVIEW_H

#include <qcanvas.h>

class QgsComposer;

class QgsComposerView: public QCanvasView
{
    Q_OBJECT
	
public:
    QgsComposerView (QgsComposer *composer, QWidget* parent=0, const char* name=0, WFlags f=0);

protected:
    void contentsMousePressEvent(QMouseEvent*);
    void contentsMouseReleaseEvent(QMouseEvent*);
    void contentsMouseMoveEvent(QMouseEvent*);

    void keyPressEvent ( QKeyEvent * e );

private:
    QgsComposer *mComposer;

};

#endif
