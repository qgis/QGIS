/***************************************************************************
                              qgsmaptooldeletevertex.h
                              ------------------------
  begin                : June 30, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLDELETEVERTEX_H
#define QGSMAPTOOLDELETEVERTEX_H

#include "qgsmaptoolvertexedit.h"

class QgsVertexMarker;
/**Map tool to delete vertices from line/polygon features*/

class QgsMapToolDeleteVertex: public QgsMapToolVertexEdit
{
    Q_OBJECT

  public:
    QgsMapToolDeleteVertex( QgsMapCanvas* canvas );
    virtual ~QgsMapToolDeleteVertex();

    void canvasMoveEvent( QMouseEvent * e );

    void canvasPressEvent( QMouseEvent * e );

    void canvasReleaseEvent( QMouseEvent * e );

    //! called when map tool is being deactivated
    void deactivate();

  private:
    QgsVertexMarker* mCross;

};

#endif
