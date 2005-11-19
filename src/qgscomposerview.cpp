/***************************************************************************
                         qgscomposerview.cpp
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
#include <iostream>

#include <qmatrix.h>
#include <qevent.h>

#include "qgsrect.h"

#include "qgscomposer.h"
#include "qgscomposeritem.h"
#include "qgscomposerview.h"

#include "qgscomposermap.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <QKeyEvent>

// Note: |WRepaintNoErase|WResizeNoErase|WStaticContents doeen't make it faster
QgsComposerView::QgsComposerView( QgsComposer *composer, QWidget* parent, const char* name, Qt::WFlags f) :
                                  Q3CanvasView(parent,name,f|Qt::WNoAutoErase|Qt::WResizeNoErase|Qt::WStaticContents)
{
    mComposer = composer;

    // TODO: nothing doe work -> necessary to call setFocus ()
    setEnabled ( true );
    setFocusPolicy ( Qt::StrongFocus );
    setFocusProxy ( 0 );
}

void QgsComposerView::contentsMousePressEvent(QMouseEvent* e)
{
    // TODO: find how to get focus without setFocus
    setFocus ();
    mComposer->composition()->contentsMousePressEvent(e);
}

void QgsComposerView::contentsMouseReleaseEvent(QMouseEvent* e)
{
    mComposer->composition()->contentsMouseReleaseEvent(e);
}

void QgsComposerView::contentsMouseMoveEvent(QMouseEvent* e)
{
    mComposer->composition()->contentsMouseMoveEvent(e);
}

void QgsComposerView::keyPressEvent ( QKeyEvent * e )
{
    mComposer->composition()->keyPressEvent ( e );
}
