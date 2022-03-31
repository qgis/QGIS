/***************************************************************************
                          qgsmaptoolprofilecurve.cpp
                          ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaptoolprofilecurve.h"
#include "qgsmapcanvas.h"

QgsMapToolProfileCurve::QgsMapToolProfileCurve( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapToolCapture( canvas, cadDockWidget, CaptureMode::CaptureLine )
{
  connect( canvas, &QgsMapCanvas::mapToolSet, this, [ = ]( QgsMapTool * tool, QgsMapTool * )
  {
    if ( tool != this )
      mPreviousTool = tool;
  } );
  mPreviousTool = canvas->mapTool();
}

QgsMapToolProfileCurve::~QgsMapToolProfileCurve() = default;

QgsMapToolCapture::Capabilities QgsMapToolProfileCurve::capabilities() const
{
  return QgsMapToolCapture::SupportsCurves;
}

bool QgsMapToolProfileCurve::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Streaming:
      return true;

    case Qgis::CaptureTechnique::Shape:
      return false;
  }
  BUILTIN_UNREACHABLE
}

void QgsMapToolProfileCurve::keyPressEvent( QKeyEvent *e )
{
  QgsMapToolCapture::keyPressEvent( e );

  if ( e->key() == Qt::Key_Escape )
  {
    canvas()->setMapTool( mPreviousTool );
  }
}

void QgsMapToolProfileCurve::lineCaptured( const QgsCurve *line )
{
  const QgsGeometry geom( line->clone() );
  emit curveCaptured( geom );

  canvas()->setMapTool( mPreviousTool );
}
