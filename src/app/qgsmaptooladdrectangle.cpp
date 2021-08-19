/***************************************************************************
    qgsmaptooladdrectangle.h  -  map tool for adding rectangle
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

#include "qgsmaptooladdrectangle.h"
#include "qgscompoundcurve.h"
#include "qgscurvepolygon.h"
#include "qgslinestring.h"
#include "qgspolygon.h"
#include "qgsgeometryrubberband.h"
#include "qgsgeometryutils.h"
#include "qgsmapcanvas.h"
#include "qgspoint.h"
#include "qgisapp.h"
#include "qgssnapindicator.h"

QgsMapToolAddRectangle::QgsMapToolAddRectangle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode )
  : QgsMapToolAddAbstract( parentTool, canvas, mode )
{
  mToolName = tr( "Add rectangle" );
}

void QgsMapToolAddRectangle::deactivate( )
{
  if ( !mParentTool || !mRectangle.isValid() )
  {
    return;
  }

  mParentTool->clearCurve( );

  // keep z value from the first snapped point
  std::unique_ptr<QgsLineString> lineString( mRectangle.toLineString() );
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

void QgsMapToolAddRectangle::clean()
{
  QgsMapToolAddAbstract::clean();
  mRectangle = QgsQuadrilateral();
}
