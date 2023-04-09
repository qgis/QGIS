/***************************************************************************
    qgsmaptoolshaperectangleabstract.h  -  map tool for adding rectangle
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

#include "qgsmaptoolshaperectangleabstract.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgsmaptoolcapture.h"


void QgsMapToolShapeRectangleAbstract::addRectangleToParentTool( )
{
  if ( !mParentTool || !mRectangle.isValid() )
  {
    return;
  }

  mParentTool->clearCurve( );

  // keep z value from the first snapped point
  std::unique_ptr<QgsLineString> lineString( mRectangle.toLineString() );
  auto match = std::find_if( mPoints.constBegin(), mPoints.constEnd(), [this]( const QgsPoint & point ) { return QgsWkbTypes::hasZ( point.wkbType() ) && point.z() != mParentTool->defaultZValue(); } );
  if ( match != mPoints.constEnd() )
  {
    lineString->dropZValue();
    lineString->addZValue( match->z() );
  }

  mParentTool->addCurve( lineString.release() );
}

void QgsMapToolShapeRectangleAbstract::clean()
{
  mRectangle = QgsQuadrilateral();
  QgsMapToolShapeAbstract::clean();
}
