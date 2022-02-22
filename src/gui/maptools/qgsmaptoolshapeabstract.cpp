/***************************************************************************
                         qgsmaptoolshapeabstract.cpp
                         ----------------------
    begin                : January 2022
    copyright            : (C) 2022 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshapeabstract.h"
#include "qgsgeometryrubberband.h"

#include <QKeyEvent>


QgsMapToolShapeAbstract::~QgsMapToolShapeAbstract()
{
  clean();
}

void QgsMapToolShapeAbstract::keyPressEvent( QKeyEvent *e )
{
  e->ignore();
}

void QgsMapToolShapeAbstract::keyReleaseEvent( QKeyEvent *e )
{
  e->ignore();
}

void QgsMapToolShapeAbstract::clean()
{
  if ( mTempRubberBand )
  {
    delete mTempRubberBand;
    mTempRubberBand = nullptr;
  }

  mPoints.clear();
}

void QgsMapToolShapeAbstract::undo()
{
  if ( mPoints.count() > 0 )
    mPoints.removeLast();
}
