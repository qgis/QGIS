/***************************************************************************
    qgsmaptoolselect.cpp  -  map tool for selecting features
    ----------------------
    begin                : January 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsmaptoolselect.h"
#include "qgsmaptoolselectutils.h"
#include "qgsrubberband.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgspointxy.h"
#include "qgis.h"
#include "qgsapplication.h"
#include "qgslogger.h"

#include <QMouseEvent>
#include <QRect>
#include <QColor>


QgsMapToolSelect::QgsMapToolSelect( QgsMapCanvas *canvas )
  : QgsMapTool( canvas )
{
  mToolName = tr( "Select features" );

  mSelectionHandler = qgis::make_unique<QgsMapToolSelectionHandler>( canvas );
  connect( mSelectionHandler.get(), &QgsMapToolSelectionHandler::geometryChanged, this, &QgsMapToolSelect::selectFeatures );
  setSelectionMode( QgsMapToolSelectionHandler::SelectSimple );
}

void QgsMapToolSelect::setSelectionMode( QgsMapToolSelectionHandler::SelectionMode selectionMode )
{
  mSelectionHandler->setSelectionMode( selectionMode );
  if ( selectionMode == QgsMapToolSelectionHandler::SelectSimple )
    mCursor = QgsApplication::getThemeCursor( QgsApplication::Cursor::Select );
  else
    mCursor = Qt::ArrowCursor;
}

void QgsMapToolSelect::canvasPressEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasPressEvent( e );
}

void QgsMapToolSelect::canvasMoveEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasMoveEvent( e );
}

void QgsMapToolSelect::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  mSelectionHandler->canvasReleaseEvent( e );
}

void QgsMapToolSelect::keyPressEvent( QKeyEvent *e )
{
  if ( !e->isAutoRepeat() )
  {
    switch ( e->key() )
    {
      case Qt::Key_Shift:
      case Qt::Key_Control:
      case Qt::Key_Alt:
      case Qt::Key_Meta:
        //note -- if ctrl and shift are already depressed, pressing alt reports the "meta" key eventZ
        modifiersChanged( e->modifiers() & Qt::ControlModifier || e->key() == Qt::Key_Control,
                          e->modifiers() & Qt::ShiftModifier || e->key() == Qt::Key_Shift,
                          e->modifiers() & Qt::AltModifier || e->key() == Qt::Key_Alt ||
                          ( e->modifiers() & Qt::ControlModifier && e->modifiers() & Qt::ShiftModifier && e->key() == Qt::Key_Meta ) );
        break;

      default:
        break;
    }
  }

  QgsMapTool::keyPressEvent( e );
}

void QgsMapToolSelect::keyReleaseEvent( QKeyEvent *e )
{
  if ( mSelectionHandler->keyReleaseEvent( e ) )
    return;

  if ( !e->isAutoRepeat() )
  {
    switch ( e->key() )
    {
      case Qt::Key_Shift:
      case Qt::Key_Control:
      case Qt::Key_Alt:
      case Qt::Key_Meta:
        modifiersChanged( e->modifiers() & Qt::ControlModifier && e->key() != Qt::Key_Control,
                          e->modifiers() & Qt::ShiftModifier && e->key() != Qt::Key_Shift,
                          e->modifiers() & Qt::AltModifier && e->key() != Qt::Key_Alt &&
                          !( e->modifiers() & Qt::ControlModifier && e->modifiers() & Qt::ShiftModifier && e->key() == Qt::Key_Meta ) );
        break;

      default:
        break;
    }
  }

  QgsMapTool::keyReleaseEvent( e );
}

void QgsMapToolSelect::deactivate()
{
  mSelectionHandler->deactivate();
  QgsMapTool::deactivate();
}

void QgsMapToolSelect::selectFeatures( Qt::KeyboardModifiers modifiers )
{
  if ( mSelectionHandler->selectionMode() == QgsMapToolSelectionHandler::SelectSimple &&
       mSelectionHandler->selectedGeometry().type() == QgsWkbTypes::PointGeometry )
  {
    QgsVectorLayer *vlayer = QgsMapToolSelectUtils::getCurrentVectorLayer( mCanvas );
    QgsRectangle r = QgsMapToolSelectUtils::expandSelectRectangle( mSelectionHandler->selectedGeometry().asPoint(), mCanvas, vlayer );
    QgsMapToolSelectUtils::selectSingleFeature( mCanvas, QgsGeometry::fromRect( r ), modifiers );
  }
  else
    QgsMapToolSelectUtils::selectMultipleFeatures( mCanvas, mSelectionHandler->selectedGeometry(), modifiers );
}

void QgsMapToolSelect::modifiersChanged( bool ctrlModifier, bool shiftModifier, bool altModifier )
{
  if ( !ctrlModifier && !shiftModifier && !altModifier )
    emit modeChanged( GeometryIntersectsSetSelection );
  else if ( !ctrlModifier && !shiftModifier && altModifier )
    emit modeChanged( GeometryWithinSetSelection );
  else if ( !ctrlModifier && shiftModifier && !altModifier )
    emit modeChanged( GeometryIntersectsAddToSelection );
  else if ( !ctrlModifier && shiftModifier && altModifier )
    emit modeChanged( GeometryWithinAddToSelection );
  else if ( ctrlModifier && !shiftModifier && !altModifier )
    emit modeChanged( GeometryIntersectsSubtractFromSelection );
  else if ( ctrlModifier && !shiftModifier && altModifier )
    emit modeChanged( GeometryWithinSubtractFromSelection );
  else if ( ctrlModifier && shiftModifier && !altModifier )
    emit modeChanged( GeometryIntersectsIntersectWithSelection );
  else if ( ctrlModifier && shiftModifier && altModifier )
    emit modeChanged( GeometryWithinIntersectWithSelection );
}
