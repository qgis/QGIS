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
#include "moc_qgsmaptoolshapecircleabstract.cpp"
#include "qgsmaptoolcapture.h"
#include "qgsmapcanvas.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"

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

  // Check whether to draw the circle as a polygon or a circular string
  bool drawAsPolygon = false;
  if ( QgsMapLayer *layer = mParentTool->layer() )
  {
    const QgsCoordinateReferenceSystem layerCrs = layer->crs();
    const QgsCoordinateReferenceSystem mapCrs = mParentTool->canvas()->mapSettings().destinationCrs();
    drawAsPolygon = layerCrs != mapCrs;
  }
  if ( drawAsPolygon )
  {
    const int segments = QgsSettingsRegistryCore::settingsDigitizingOffsetQuadSeg->value() * 12;
    std::unique_ptr<QgsLineString> ls( mCircle.toLineString( segments ) );
    mParentTool->addCurve( ls.release() );
  }
  else
  {
    std::unique_ptr<QgsCircularString> ls( mCircle.toCircularString() );
    mParentTool->addCurve( ls.release() );
  }
}
