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

#include <QMatrix>
#include <QEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include "qgsrect.h"
#include "qgscomposer.h"
#include "qgscomposeritem.h"
#include "qgscomposerview.h"
#include "qgscomposermap.h"

// Note: |WRepaintNoErase|WResizeNoErase|WStaticContents doeen't make it faster
QgsComposerView::QgsComposerView( QgsComposer *composer, QWidget* parent, const char* name, Qt::WFlags f) :
                                  QGraphicsView(parent)
//,name,f|Qt::WNoAutoErase|Qt::WResizeNoErase|Qt::WStaticContents
{
    mComposer = composer;

    // TODO: nothing doe work -> necessary to call setFocus ()
    setEnabled ( true );
    setFocusPolicy ( Qt::StrongFocus );
    setFocusProxy ( 0 );

    setResizeAnchor ( QGraphicsView::AnchorViewCenter );

}

void QgsComposerView::mousePressEvent(QMouseEvent* e)
{
    // TODO: find how to get focus without setFocus
    setFocus ();
    mComposer->composition()->mousePressEvent(e);
}

void QgsComposerView::mouseReleaseEvent(QMouseEvent* e)
{
    mComposer->composition()->mouseReleaseEvent(e);
}

void QgsComposerView::mouseMoveEvent(QMouseEvent* e)
{
    mComposer->composition()->mouseMoveEvent(e);
}

void QgsComposerView::keyPressEvent ( QKeyEvent * e )
{
    mComposer->composition()->keyPressEvent ( e );
}

void QgsComposerView::resizeEvent ( QResizeEvent *  )
{
/* BUG: When QT adds scrollbars because we're zooming in, it causes a resizeEvent.
 *  If we call zoomFull(), we reset the view size, which keeps us from zooming in.
 *  Really, we should do something like re-center the window.
*/
    //mComposer->zoomFull();
#ifdef QGISDEBUG
  std::cout << "resize anchor: " << resizeAnchor() << std::endl;
#endif
}

//TODO: add mouse wheel event forwarding

