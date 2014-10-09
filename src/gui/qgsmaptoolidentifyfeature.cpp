/***************************************************************************
    qgsmaptoolidentifyfeature.cpp
     --------------------------------------
    Date                 : 22.5.2014
    Copyright            : (C) 2014 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMouseEvent>

#include "qgscursors.h"
#include "qgsmaptoolidentifyfeature.h"
#include "qgsmapcanvas.h"

QgsMapToolIdentifyFeature::QgsMapToolIdentifyFeature( QgsMapCanvas* canvas, QgsVectorLayer* vl )
    : QgsMapToolIdentify( canvas )
    , mCanvas( canvas )
    , mLayer( vl )
{
  mToolName = tr( "Identify feature" );

  // set cursor
  QPixmap cursorPixmap = QPixmap(( const char ** ) cross_hair_cursor );
  mCursor = QCursor( cursorPixmap, 1, 1 );
}

QgsMapToolIdentifyFeature::~QgsMapToolIdentifyFeature()
{
}

void QgsMapToolIdentifyFeature::canvasReleaseEvent( QMouseEvent* e )
{

  QgsPoint point = mCanvas->getCoordinateTransform()->toMapCoordinates( e->x(), e->y() );

  QList<IdentifyResult> results;
  if ( !identifyVectorLayer( &results, mLayer, point ) )
    return;

  // TODO: display a menu when several features identified

  emit featureIdentified( results[0].mFeature );
  emit featureIdentified( results[0].mFeature.id() );
}

void QgsMapToolIdentifyFeature::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Escape )
  {
    mCanvas->unsetMapTool( this );
  }
}
