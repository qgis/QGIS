/***************************************************************************
    qgsmaptoolvertexedit.cpp  - tool for adding, moving, deleting vertices
    ---------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolvertexedit.h"
#include "qgsmapcanvas.h"
#include "qgsmaprenderer.h"
#include "qgsmessageviewer.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsmaptopixel.h"
#include "qgsproject.h"
#include "qgscursors.h"
#include "qgsmessagebar.h"
#include "qgisapp.h"

#include <QCursor>
#include <QSettings>
#include <QPixmap>

QgsMapToolVertexEdit::QgsMapToolVertexEdit( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas )
{

}

QgsMapToolVertexEdit::~QgsMapToolVertexEdit()
{

}

void QgsMapToolVertexEdit::displaySnapToleranceWarning()
{
  QgisApp::instance()->messageBar()->pushMessage(
    tr( "Snap tolerance" ),
    tr( "Could not snap segment. Have you set the tolerance in Settings > Snapping Options?" ),
    QgsMessageBar::INFO,
    QgisApp::instance()->messageTimeout() );
}
