/***************************************************************************
    qgsmaptooladdellipse.cpp  -  map tool for adding ellipse
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

#include "qgsmaptooladdellipse.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgssnapindicator.h"

QgsMapToolAddEllipse::QgsMapToolAddEllipse( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddAbstract( parentTool, canvas, mode )
{
  mToolName = tr( "Add ellipse" );
}

void QgsMapToolAddEllipse::deactivate()
{
  if ( !mParentTool || mEllipse.isEmpty() )
  {
    return;
  }

  mParentTool->clearCurve();

  // keep z value from the first snapped point
  std::unique_ptr<QgsLineString> ls( mEllipse.toLineString( segments() ) );
  for ( const QgsPoint &point : std::as_const( mPoints ) )
  {
    if ( QgsWkbTypes::hasZ( point.wkbType() ) &&
         point.z() != defaultZValue() )
    {
      ls->dropZValue();
      ls->addZValue( point.z() );
      break;
    }
  }

  mParentTool->addCurve( ls.release() );
  clean();

  QgsMapToolCapture::deactivate();
}

void QgsMapToolAddEllipse::clean()
{
  QgsMapToolAddAbstract::clean();
  mEllipse = QgsEllipse();
}
