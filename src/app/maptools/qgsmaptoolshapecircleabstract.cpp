/***************************************************************************
    qgsmaptoolshapecircleabstract.cpp  -  map tool for adding circle
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshapecircleabstract.h"
#include "qgsmaptoolcapture.h"

void QgsMapToolShapeCircleAbstract::clean()
{
  mCircle = QgsCircle();
  QgsMapToolShapeAbstract::clean();
}

void QgsMapToolShapeCircleAbstract::addCircleToParentTool()
{
  if ( !mParentTool || mCircle.isEmpty() )
    return;

  mParentTool->clearCurve();

  std::unique_ptr<QgsCircularString> lineString( mCircle.toCircularString() );

  mParentTool->addCurve( lineString.release() );
}
