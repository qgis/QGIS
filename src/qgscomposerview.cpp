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

#include <qwmatrix.h>
#include <qevent.h>

#include "qgsrect.h"

#include "qgscomposer.h"
#include "qgscomposeritem.h"
#include "qgscomposerview.h"

#include "qgscomposermap.h"

// Note: |WRepaintNoErase|WResizeNoErase|WStaticContents doeen't make it faster
QgsComposerView::QgsComposerView( QgsComposer *composer, QWidget* parent, const char* name, WFlags f) :
                                  QCanvasView(parent,name,f|WRepaintNoErase|WResizeNoErase|WStaticContents)
{
    mComposer = composer;

    // TODO: nothing doe work -> necessary to call setFocus ()
    setEnabled ( true );
    setFocusPolicy ( QWidget::StrongFocus );
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
