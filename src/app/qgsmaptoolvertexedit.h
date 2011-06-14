/***************************************************************************
    qgsmaptoolvertexedit.h  - tool for adding, moving, deleting vertices
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

#ifndef QGSMAPTOOLVERTEXEDIT_H
#define QGSMAPTOOLVERTEXEDIT_H

#include "qgsmapcanvassnapper.h"
#include "qgsmaptooledit.h"
#include "qgsgeometry.h"

/**Base class for vertex manipulation tools.
 Inherited by QgsMapToolMoveVertex, QgsMapToolAddVertex,
QgsMapToolDeleteVertex*/
class QgsMapToolVertexEdit: public QgsMapToolEdit
{
    Q_OBJECT

  public:

    QgsMapToolVertexEdit( QgsMapCanvas* canvas );

    virtual ~QgsMapToolVertexEdit();

  protected:

    /**Snapping results that are collected during the mouse press event
     (search for vertices/segments to manipulate)*/
    QList<QgsSnappingResult> mRecentSnappingResults;

    //! Displays a warning about the snap tolerance settings
    void displaySnapToleranceWarning();
};

#endif
