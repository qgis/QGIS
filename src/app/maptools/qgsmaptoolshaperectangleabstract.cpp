/***************************************************************************
    qgsmaptoolshaperectangleabstract.h  -  map tool for adding rectangle
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lituus at free dot fr
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolshaperectangleabstract.h"

#include "qgisapp.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgslinestring.h"
#include "qgsmaptoolcapture.h"
#include "qgspoint.h"
#include "qgspolygon.h"

#include "moc_qgsmaptoolshaperectangleabstract.cpp"

void QgsMapToolShapeRectangleAbstract::addRectangleToParentTool()
{
  if ( !mParentTool || !mRectangle.isValid() )
  {
    return;
  }

  mParentTool->clearCurve();

  // keep z value from the first snapped point
  std::unique_ptr<QgsLineString> lineString( mRectangle.toLineString() );
  for ( const QgsPoint &point : std::as_const( mPoints ) )
  {
    if ( QgsWkbTypes::hasZ( point.wkbType() ) && point.z() != mParentTool->defaultZValue() )
    {
      lineString->dropZValue();
      lineString->addZValue( point.z() );
      break;
    }
  }

  mParentTool->addCurve( lineString.release() );
}

void QgsMapToolShapeRectangleAbstract::clean()
{
  mRectangle = QgsQuadrilateral();
  QgsMapToolShapeAbstract::clean();
}
