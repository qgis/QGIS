/***************************************************************************
                              qgsmaptoolmovevertex.h
                              ---------------------
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

#ifndef QGSMAPTOOLADDVERTEX_H
#define QGSMAPTOOLADDVERTEX_H

#include "qgsmaptoolvertexedit.h"

class QgsRubberBand;

/**Map tool to add vertices to line/polygon features*/
class QgsMapToolAddVertex: public QgsMapToolVertexEdit
{
  Q_OBJECT

  public:
    QgsMapToolAddVertex( QgsMapCanvas* canvas );
    virtual ~QgsMapToolAddVertex();

    void canvasMoveEvent( QMouseEvent * e );

    void canvasPressEvent( QMouseEvent * e );

    void canvasReleaseEvent( QMouseEvent * e );

    //! called when map tool is being deactivated
    void deactivate();

  private:
    QgsRubberBand* mRubberBand;
};

#endif
