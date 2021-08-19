/***************************************************************************
    qgsmaptooladdcircle.cpp  -  map tool for adding circle
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

#include "qgsmaptooladdcircle.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgssnapindicator.h"

QgsMapToolAddCircle::QgsMapToolAddCircle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddAbstract( parentTool, canvas, mode )
{
  mToolName = tr( "Add circle" );
}

void QgsMapToolAddCircle::deactivate()
{
  if ( !mParentTool || mCircle.isEmpty() )
  {
    return;
  }

  mParentTool->clearCurve();

  // keep z value from the first snapped point
  std::unique_ptr<QgsCircularString> lineString( mCircle.toCircularString() );
  for ( const QgsPoint &point : std::as_const( mPoints ) )
  {
    if ( QgsWkbTypes::hasZ( point.wkbType() ) &&
         point.z() != defaultZValue() )
    {
      lineString->dropZValue();
      lineString->addZValue( point.z() );
      break;
    }
  }

  mParentTool->addCurve( lineString.release() );
  clean();

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddCircle::clean()
{
  QgsMapToolAddAbstract::clean();
  mCircle = QgsCircle();
}
