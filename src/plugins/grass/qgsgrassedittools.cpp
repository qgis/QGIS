/***************************************************************************
    qgsgrassedittools.cpp -    GRASS Edit tools
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassedittools.h"
#include "qgsgrassattributes.h"
#include "qgsgrassedit.h"
#include "qgsgrassprovider.h"

#include "qgisinterface.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"

#include <QMouseEvent>

extern "C"
{
#include <grass/Vect.h>
}


QgsGrassEditTool::QgsGrassEditTool( QgsGrassEdit* edit )
    : QgsMapTool( edit->mCanvas ), e( edit )
{
}

void QgsGrassEditTool::canvasPressEvent( QMouseEvent * event )
{
  QgsPoint point = toLayerCoordinates( e->layer(), event->pos() );
  mouseClick( point,  event->button() );

  // Set last click
  e->mLastPoint = point;

  e->statusBar()->showMessage( e->mCanvasPrompt );

  QgsDebugMsg( QString( "n_points = %1" ).arg( e->mEditPoints->n_points ) );
}

void QgsGrassEditTool::canvasMoveEvent( QMouseEvent * event )
{
  QgsPoint point = toLayerCoordinates( e->layer(), event->pos() );
  mouseMove( point );

  e->statusBar()->showMessage( e->mCanvasPrompt );
}


// ------------------------------------------------------------------
// NEW POINT + NEW CENTROID
// ------------------------------------------------------------------


QgsGrassEditNewPoint::QgsGrassEditNewPoint( QgsGrassEdit* edit, bool newCentroid )
    : QgsGrassEditTool( edit ), mNewCentroid( newCentroid )
{
  if ( newCentroid )
    e->setCanvasPrompt( tr( "New centroid" ), "", "" );
  else
    e->setCanvasPrompt( tr( "New point" ), "", "" );

}


void QgsGrassEditNewPoint::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  if ( button != Qt::LeftButton )
    return;

  Vect_reset_line( e->mEditPoints );
  e->snap( point );
  Vect_append_point( e->mEditPoints, point.x(), point.y(), 0.0 );

  int type;
  if ( mNewCentroid ) // new centroid or point ?
    type = GV_CENTROID;
  else
    type = GV_POINT;

  int line;
  line = e->writeLine( type, e->mEditPoints );
  e->updateSymb();
  e->displayUpdated();

  if ( e->mAttributes )
  {
    e->mAttributes->setLine( line );
    e->mAttributes->clear();
  }
  else
  {
    e->mAttributes = new QgsGrassAttributes( e, e->mProvider, line, e->mIface->mainWindow() );
  }
  for ( int i = 0; i < e->mCats->n_cats; i++ )
  {
    e->addAttributes( e->mCats->field[i], e->mCats->cat[i] );
  }
  e->mAttributes->show();
  e->mAttributes->raise();
}


// ------------------------------------------------------------------
// NEW LINE + NEW BOUNDARY
// ------------------------------------------------------------------

QgsGrassEditNewLine::QgsGrassEditNewLine( QgsGrassEdit* edit, bool newBoundary )
    : QgsGrassEditTool( edit ), mNewBoundary( newBoundary )
{
  e->setCanvasPrompt( tr( "New vertex" ), "", "" );
}

void QgsGrassEditNewLine::deactivate()
{
  // Delete last segment
  if ( e->mEditPoints->n_points > 1 )
  {
    Vect_reset_line( e->mPoints );
    Vect_append_points( e->mPoints, e->mEditPoints, GV_FORWARD );
    e->displayDynamic( e->mPoints );
  }
  e->setCanvasPrompt( tr( "New vertex" ), "", "" );

  QgsGrassEditTool::deactivate(); // call default bahivour
}

void QgsGrassEditNewLine::activate()
{
  QgsDebugMsg( "entered." );

  // Display dynamic segment
  if ( e->mEditPoints->n_points > 0 )
  {
    Vect_reset_line( e->mPoints );
    Vect_append_points( e->mPoints, e->mEditPoints, GV_FORWARD );
    QgsPoint point = toMapCoordinates( e->mCanvas->mouseLastXY() );
    Vect_append_point( e->mPoints, point.x(), point.y(), 0.0 );
    e->displayDynamic( e->mPoints );
  }

  QgsGrassEditTool::activate(); // call default bahivour
}

void QgsGrassEditNewLine::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  switch ( button )
  {
    case Qt::LeftButton:
      if ( e->mEditPoints->n_points > 2 )
      {
        e->snap( point, e->mEditPoints->x[0], e->mEditPoints->y[0] );
      }
      else
      {
        e->snap( point );
      }
      Vect_append_point( e->mEditPoints, point.x(), point.y(), 0.0 );

      // Draw
      Vect_reset_line( e->mPoints );
      Vect_append_points( e->mPoints, e->mEditPoints, GV_FORWARD );
      e->displayDynamic( e->mPoints );
      break;
    case Qt::MidButton:
      if ( e->mEditPoints->n_points > 0 )
      {
        e->mEditPoints->n_points--;
        Vect_reset_line( e->mPoints );
        Vect_append_points( e->mPoints, e->mEditPoints, GV_FORWARD );
        QgsPoint point = toMapCoordinates( e->mCanvas->mouseLastXY() );
        Vect_append_point( e->mPoints, point.x(), point.y(), 0.0 );
        e->displayDynamic( e->mPoints );
      }
      break;
    case Qt::RightButton:
      e->eraseDynamic();
      if ( e->mEditPoints->n_points > 1 )
      {
        int type;

        if ( mNewBoundary ) // boundary or line?
          type = GV_BOUNDARY;
        else
          type = GV_LINE;

        int line;
        line = e->writeLine( type, e->mEditPoints );
        e->updateSymb();
        e->displayUpdated();

        if ( e->mAttributes )
        {
          e->mAttributes->setLine( line );
          e->mAttributes->clear();
        }
        else
        {
          e->mAttributes = new QgsGrassAttributes( e, e->mProvider, line, e->mIface->mainWindow() );
        }
        for ( int i = 0; i < e->mCats->n_cats; i++ )
        {
          e->addAttributes( e->mCats->field[i], e->mCats->cat[i] );
        }
        e->mAttributes->show();
        e->mAttributes->raise();
      }
      Vect_reset_line( e->mEditPoints );
      break;

    default:
      // ignore others
      break;
  }

  if ( e->mEditPoints->n_points == 0 )
  {
    e->setCanvasPrompt( tr( "New point" ), "", "" );
  }
  else if ( e->mEditPoints->n_points == 1 )
  {
    e->setCanvasPrompt( tr( "New point" ), tr( "Undo last point" ), "" );
  }
  else if ( e->mEditPoints->n_points > 1 )
  {
    e->setCanvasPrompt( tr( "New point" ), tr( "Undo last point" ), tr( "Close line" ) );
  }
}

void QgsGrassEditNewLine::mouseMove( QgsPoint & newPoint )
{
  if ( e->mEditPoints->n_points > 0 )
  {
    // Draw the line with new segment
    Vect_reset_line( e->mPoints );
    Vect_append_points( e->mPoints, e->mEditPoints, GV_FORWARD );
    Vect_append_point( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
    e->displayDynamic( e->mPoints );
  }
}


// ------------------------------------------------------------------
// MOVE VERTEX
// ------------------------------------------------------------------

QgsGrassEditMoveVertex::QgsGrassEditMoveVertex( QgsGrassEdit* edit )
    : QgsGrassEditTool( edit )
{
  e->setCanvasPrompt( tr( "Select vertex" ), "", "" );
}

void QgsGrassEditMoveVertex::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  double thresh = e->threshold();

  switch ( button )
  {
    case Qt::LeftButton:
      // Move previously selected vertex
      if ( e->mSelectedLine > 0 )
      {
        e->eraseDynamic();
        e->eraseElement( e->mSelectedLine );

        // Move vertex
        int type = e->mProvider->readLine( e->mPoints, e->mCats, e->mSelectedLine );
        e->snap( point );
        e->mPoints->x[e->mSelectedPart] = point.x();
        e->mPoints->y[e->mSelectedPart] = point.y();

        Vect_line_prune( e->mPoints );
        e->mProvider->rewriteLine( e->mSelectedLine, type, e->mPoints, e->mCats );
        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line( e->mEditPoints );

        e->setCanvasPrompt( tr( "Select vertex" ), "", "" );
      }
      else
      {
        // Select new line
        e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_LINES, thresh );

        if ( e->mSelectedLine )   // highlite
        {
          e->mProvider->readLine( e->mEditPoints, NULL, e->mSelectedLine );
          e->displayElement( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

          double xl, yl; // nearest point on the line

          // Note first segment is 1!
          e->mSelectedPart = Vect_line_distance( e->mEditPoints, point.x(), point.y(), 0.0, 0,
                                                 &xl, &yl, NULL, NULL, NULL, NULL );

          double dist1 = Vect_points_distance( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart-1],
                                               e->mEditPoints->y[e->mSelectedPart-1], 0.0, 0 );
          double dist2 = Vect_points_distance( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart],
                                               e->mEditPoints->y[e->mSelectedPart], 0.0, 0 );

          if ( dist1 < dist2 ) e->mSelectedPart--;

          e->setCanvasPrompt( tr( "Select new position" ), "", "Release vertex" );
        }
      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      Vect_reset_line( e->mEditPoints );

      e->setCanvasPrompt( tr( "Select vertex" ), "", "" );
      break;

    default:
      // ignore others
      break;
  }

}

void QgsGrassEditMoveVertex::mouseMove( QgsPoint & newPoint )
{
  if ( e->mSelectedLine > 0 )
  {
    // Transform coordinates
    Vect_reset_line( e->mPoints );
    if ( e->mSelectedPart == 0 )
    {
      Vect_append_point( e->mPoints, e->mEditPoints->x[1], e->mEditPoints->y[1], 0.0 );
      Vect_append_point( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
    }
    else if ( e->mSelectedPart == e->mEditPoints->n_points - 1 )
    {
      Vect_append_point( e->mPoints, e->mEditPoints->x[e->mSelectedPart-1],
                         e->mEditPoints->y[e->mSelectedPart-1], 0.0 );
      Vect_append_point( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
    }
    else
    {
      Vect_append_point( e->mPoints, e->mEditPoints->x[e->mSelectedPart-1],
                         e->mEditPoints->y[e->mSelectedPart-1], 0.0 );
      Vect_append_point( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
      Vect_append_point( e->mPoints, e->mEditPoints->x[e->mSelectedPart+1],
                         e->mEditPoints->y[e->mSelectedPart+1], 0.0 );
    }
    for ( int i = 0; i < e->mPoints->n_points; i++ )
    {
      QgsDebugMsg( QString( "%1 %2" ).arg( e->mPoints->x[i] ).arg( e->mPoints->y[i] ) );
    }

    e->displayDynamic( e->mPoints );
  }
}


// ------------------------------------------------------------------
// ADD VERTEX
// ------------------------------------------------------------------

QgsGrassEditAddVertex::QgsGrassEditAddVertex( QgsGrassEdit* edit )
    : QgsGrassEditTool( edit )
{
  e->setCanvasPrompt( tr( "Select line segment" ), "", "" );
}

void QgsGrassEditAddVertex::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  double thresh = e->threshold();

  switch ( button )
  {
    case Qt::LeftButton:
      // Add vertex to previously selected line
      if ( e->mSelectedLine > 0 )
      {
        e->eraseDynamic();
        e->eraseElement( e->mSelectedLine );

        // Move vertex
        int type = e->mProvider->readLine( e->mPoints, e->mCats, e->mSelectedLine );

        if ( e->mAddVertexEnd && e->mSelectedPart == e->mEditPoints->n_points - 1 )
        {
          e->snap( point );
          Vect_append_point( e->mPoints, point.x(), point.y(), 0.0 );
        }
        else
        {
          Vect_line_insert_point( e->mPoints, e->mSelectedPart, point.x(), point.y(), 0.0 );
        }

        Vect_line_prune( e->mPoints );
        e->mProvider->rewriteLine( e->mSelectedLine, type, e->mPoints, e->mCats );
        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line( e->mEditPoints );

        e->setCanvasPrompt( tr( "Select line segment" ), "", "" );
      }
      else
      {
        // Select new line
        e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_LINES, thresh );

        if ( e->mSelectedLine )   // highlite
        {
          e->mProvider->readLine( e->mEditPoints, NULL, e->mSelectedLine );
          e->displayElement( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

          double xl, yl; // nearest point on the line

          // Note first segment is 1!
          e->mSelectedPart = Vect_line_distance( e->mEditPoints, point.x(), point.y(), 0.0, 0,
                                                 &xl, &yl, NULL, NULL, NULL, NULL );

          double dist1 = Vect_points_distance( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart-1],
                                               e->mEditPoints->y[e->mSelectedPart-1], 0.0, 0 );
          double dist2 = Vect_points_distance( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart],
                                               e->mEditPoints->y[e->mSelectedPart], 0.0, 0 );

          double maxdist = ( dist1 + dist2 ) / 4;

          if ( e->mSelectedPart == 1 && dist1 < maxdist )
          {
            e->mSelectedPart = 0;
            e->mAddVertexEnd = true;
          }
          else if ( e->mSelectedPart == e->mEditPoints->n_points - 1 && dist2 < maxdist )
          {
            e->mAddVertexEnd = true;
          }
          else
          {
            e->mAddVertexEnd = false;
          }

          e->setCanvasPrompt( tr( "New vertex position" ), "", tr( "Release" ) );
        }
        else
        {
          e->setCanvasPrompt( tr( "Select line segment" ), "", "" );
        }
      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      Vect_reset_line( e->mEditPoints );

      e->setCanvasPrompt( tr( "Select line segment" ), "", "" );
      break;

    default:
      // ignore others
      break;
  }

}

void QgsGrassEditAddVertex::mouseMove( QgsPoint & newPoint )
{
  if ( e->mSelectedLine > 0 )
  {
    Vect_reset_line( e->mPoints );
    if ( e->mAddVertexEnd )
    {
      Vect_append_point( e->mPoints, e->mEditPoints->x[e->mSelectedPart],
                         e->mEditPoints->y[e->mSelectedPart], 0.0 );
      Vect_append_point( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
    }
    else
    {
      Vect_append_point( e->mPoints, e->mEditPoints->x[e->mSelectedPart-1],
                         e->mEditPoints->y[e->mSelectedPart-1], 0.0 );
      Vect_append_point( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
      Vect_append_point( e->mPoints, e->mEditPoints->x[e->mSelectedPart],
                         e->mEditPoints->y[e->mSelectedPart], 0.0 );
    }
    for ( int i = 0; i < e->mPoints->n_points; i++ )
    {
      QgsDebugMsg( QString( "%1 %2" ).arg( e->mPoints->x[i] ).arg( e->mPoints->y[i] ) );
    }

    e->displayDynamic( e->mPoints );
  }
}

// ------------------------------------------------------------------
// DELETE VERTEX
// ------------------------------------------------------------------

QgsGrassEditDeleteVertex::QgsGrassEditDeleteVertex( QgsGrassEdit* edit )
    : QgsGrassEditTool( edit )
{
  e->setCanvasPrompt( tr( "Select vertex" ), "", "" );
}

void QgsGrassEditDeleteVertex::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  double thresh = e->threshold();

  switch ( button )
  {
    case Qt::LeftButton:
      // Delete previously selected vertex
      if ( e->mSelectedLine > 0 )
      {
        e->eraseDynamic();
        e->eraseElement( e->mSelectedLine );

        // Move vertex
        int type = e->mProvider->readLine( e->mPoints, e->mCats, e->mSelectedLine );
        Vect_line_delete_point( e->mPoints, e->mSelectedPart );

        if ( e->mPoints->n_points < 2 ) // delete line
        {
          e->mProvider->deleteLine( e->mSelectedLine );

          // Check orphan records
          for ( int i = 0 ; i < e->mCats->n_cats; i++ )
          {
            e->checkOrphan( e->mCats->field[i], e->mCats->cat[i] );
          }
        }
        else
        {
          e->mProvider->rewriteLine( e->mSelectedLine, type, e->mPoints, e->mCats );
        }

        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line( e->mEditPoints );

        e->setCanvasPrompt( tr( "Select vertex" ), "", "" );
      }
      else
      {
        // Select new/next line
        e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_LINES, thresh );

        if ( e->mSelectedLine )   // highlite
        {
          e->mProvider->readLine( e->mEditPoints, NULL, e->mSelectedLine );

          e->displayElement( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

          double xl, yl; // nearest point on the line

          // Note first segment is 1!
          e->mSelectedPart = Vect_line_distance( e->mEditPoints, point.x(), point.y(), 0.0, 0,
                                                 &xl, &yl, NULL, NULL, NULL, NULL );

          double dist1 = Vect_points_distance( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart-1],
                                               e->mEditPoints->y[e->mSelectedPart-1], 0.0, 0 );
          double dist2 = Vect_points_distance( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart],
                                               e->mEditPoints->y[e->mSelectedPart], 0.0, 0 );

          if ( dist1 < dist2 ) e->mSelectedPart--;

          e->displayDynamic( e->mEditPoints->x[e->mSelectedPart], e->mEditPoints->y[e->mSelectedPart],
                             QgsVertexMarker::ICON_BOX, e->mSize );

          e->setCanvasPrompt( tr( "Delete vertex" ), "", tr( "Release vertex" ) );
        }
        else
        {
          e->setCanvasPrompt( tr( "Select vertex" ), "", "" );
        }
      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      Vect_reset_line( e->mEditPoints );

      e->setCanvasPrompt( tr( "Select vertex" ), "", "" );
      break;

    default:
      // ignore others
      break;
  }
}

// ------------------------------------------------------------------
// MOVE LINE
// ------------------------------------------------------------------

QgsGrassEditMoveLine::QgsGrassEditMoveLine( QgsGrassEdit* edit )
    : QgsGrassEditTool( edit )
{
  e->setCanvasPrompt( tr( "Select element" ), "", "" );
}

void QgsGrassEditMoveLine::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  double thresh = e->threshold();

  switch ( button )
  {
    case Qt::LeftButton:
      // Move previously selected line
      if ( e->mSelectedLine > 0 )
      {
        e->eraseDynamic();
        e->eraseElement( e->mSelectedLine );

        // Transform coordinates
        int type = e->mProvider->readLine( e->mPoints, e->mCats, e->mSelectedLine );
        for ( int i = 0; i < e->mPoints->n_points; i++ )
        {
          e->mPoints->x[i] += point.x() - e->mLastPoint.x();
          e->mPoints->y[i] += point.y() - e->mLastPoint.y();
        }

        e->mProvider->rewriteLine( e->mSelectedLine, type, e->mPoints, e->mCats );
        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line( e->mEditPoints );

        e->setCanvasPrompt( tr( "Select element" ), "", "" );
      }
      else
      {
        // Select new/next line
        e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_POINT | GV_CENTROID, thresh );

        if ( e->mSelectedLine == 0 )
          e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_LINE | GV_BOUNDARY, thresh );

        if ( e->mSelectedLine )   // highlite
        {
          e->mProvider->readLine( e->mEditPoints, NULL, e->mSelectedLine );
          e->displayElement( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );
          e->setCanvasPrompt( tr( "New location" ), "", tr( "Release selected" ) );
        }
        else
        {
          e->setCanvasPrompt( tr( "Select element" ), "", "" );
        }
      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      e->setCanvasPrompt( tr( "Select element" ), "", "" );
      break;

    default:
      // ignore others
      break;
  }
}

void QgsGrassEditMoveLine::mouseMove( QgsPoint & newPoint )
{
  // Move previously selected line
  if ( e->mSelectedLine > 0 )
  {
    // Transform coordinates
    Vect_reset_line( e->mPoints );
    Vect_append_points( e->mPoints, e->mEditPoints, GV_FORWARD );

    for ( int i = 0; i < e->mPoints->n_points; i++ )
    {
      e->mPoints->x[i] += newPoint.x() - e->mLastPoint.x();
      e->mPoints->y[i] += newPoint.y() - e->mLastPoint.y();
    }

    e->displayDynamic( e->mPoints );
  }
}


// ------------------------------------------------------------------
// DELETE LINE
// ------------------------------------------------------------------

QgsGrassEditDeleteLine::QgsGrassEditDeleteLine( QgsGrassEdit* edit )
    : QgsGrassEditTool( edit )
{
  e->setCanvasPrompt( tr( "Select element" ), "", "" );
}

void QgsGrassEditDeleteLine::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  double thresh = e->threshold();

  switch ( button )
  {
    case Qt::LeftButton:
      // Delete previously selected line
      if ( e->mSelectedLine > 0 )
      {
        e->eraseElement( e->mSelectedLine );
        e->mProvider->readLine( NULL, e->mCats, e->mSelectedLine );
        e->mProvider->deleteLine( e->mSelectedLine );

        // Check orphan records
        for ( int i = 0 ; i < e->mCats->n_cats; i++ )
        {
          e->checkOrphan( e->mCats->field[i], e->mCats->cat[i] );
        }

        e->updateSymb();
        e->displayUpdated();
      }

      // Select new/next line
      e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_POINT | GV_CENTROID, thresh );

      if ( e->mSelectedLine == 0 )
        e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_LINE | GV_BOUNDARY, thresh );

      if ( e->mSelectedLine )   // highlite, propmt
      {
        e->displayElement( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );
        e->setCanvasPrompt( tr( "Delete selected / select next" ), "", tr( "Release selected" ) );
      }
      else
      {
        e->setCanvasPrompt( tr( "Select element" ), "", "" );
      }
      break;

    case Qt::RightButton:
      e->displayElement( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      e->setCanvasPrompt( tr( "Select element" ), "", "" );
      break;

    default:
      // ignore others
      break;
  }
}

// ------------------------------------------------------------------
// SPLIT LINE
// ------------------------------------------------------------------

QgsGrassEditSplitLine::QgsGrassEditSplitLine( QgsGrassEdit* edit )
    : QgsGrassEditTool( edit )
{
  e->setCanvasPrompt( tr( "Select position on line" ), "", "" );
}

void QgsGrassEditSplitLine::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  double thresh = e->threshold();

  switch ( button )
  {
    case Qt::LeftButton:
      // Split previously selected line
      if ( e->mSelectedLine > 0 )
      {
        e->eraseDynamic();
        e->eraseElement( e->mSelectedLine );

        int type = e->mProvider->readLine( e->mPoints, e->mCats, e->mSelectedLine );

        double xl, yl;
        Vect_line_distance( e->mPoints, e->mLastPoint.x(), e->mLastPoint.y(), 0.0, 0,
                            &xl, &yl, NULL, NULL, NULL, NULL );

        e->mPoints->n_points = e->mSelectedPart;
        Vect_append_point( e->mPoints, xl, yl, 0.0 );
        e->mProvider->rewriteLine( e->mSelectedLine, type, e->mPoints, e->mCats );
        e->updateSymb();
        e->displayUpdated();

        Vect_reset_line( e->mPoints );
        Vect_append_point( e->mPoints, xl, yl, 0.0 );
        for ( int i = e->mSelectedPart; i < e->mEditPoints->n_points; i++ )
        {
          Vect_append_point( e->mPoints, e->mEditPoints->x[i], e->mEditPoints->y[i], 0.0 );
        }

        e->mProvider->writeLine( type, e->mPoints, e->mCats );

        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line( e->mEditPoints );
        e->setCanvasPrompt( tr( "Select position on line" ), "", "" );
      }
      else
      {
        // Select new/next line
        e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_LINES, thresh );

        if ( e->mSelectedLine )   // highlite
        {
          e->mProvider->readLine( e->mEditPoints, NULL, e->mSelectedLine );

          e->displayElement( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

          double xl, yl; // nearest point on the line

          // Note first segment is 1!
          e->mSelectedPart = Vect_line_distance( e->mEditPoints, point.x(), point.y(), 0.0, 0,
                                                 &xl, &yl, NULL, NULL, NULL, NULL );

          e->displayDynamic( xl, yl, QgsVertexMarker::ICON_X, e->mSize );

          e->setCanvasPrompt( tr( "Split the line" ), "", tr( "Release the line" ) );
        }
        else
        {
          e->setCanvasPrompt( tr( "Select point on line" ), "", "" );
        }

      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      Vect_reset_line( e->mEditPoints );

      e->setCanvasPrompt( tr( "Select point on line" ), "", "" );
      break;

    default:
      // ignore others
      break;
  }
}


// ------------------------------------------------------------------
// EDIT ATTRIBUTES
// ------------------------------------------------------------------

QgsGrassEditAttributes::QgsGrassEditAttributes( QgsGrassEdit* edit )
    : QgsGrassEditTool( edit )
{
  e->setCanvasPrompt( tr( "Select element" ), "", "" );
}

void QgsGrassEditAttributes::mouseClick( QgsPoint & point, Qt::MouseButton button )
{
  double thresh = e->threshold();

  // Redraw previously selected line
  if ( e->mSelectedLine > 0 )
  {
    e->displayElement( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
  }

  // Select new/next line
  e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_POINT | GV_CENTROID, thresh );

  if ( e->mSelectedLine == 0 )
    e->mSelectedLine = e->mProvider->findLine( point.x(), point.y(), GV_LINE | GV_BOUNDARY, thresh );

  QgsDebugMsg( QString( "mSelectedLine = %1" ).arg( e->mSelectedLine ) );

  if ( e->mAttributes )
  {
    e->mAttributes->setLine( 0 );
    e->mAttributes->clear();
    e->mAttributes->raise();
    // Just to disable new button 
    e->mAttributes->setCategoryMode( QgsGrassEdit::CAT_MODE_NOCAT, QString() );
  }

  if ( e->mSelectedLine > 0 )   // highlite
  {
    e->displayElement( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

    e->mProvider->readLine( NULL, e->mCats, e->mSelectedLine );

    if ( !e->mAttributes )
    {
      e->mAttributes = new QgsGrassAttributes( e, e->mProvider, e->mSelectedLine, e->mIface->mainWindow() );
    }
    else
    {
      e->mAttributes->setLine( e->mSelectedLine );
    }
    for ( int i = 0; i < e->mCats->n_cats; i++ )
    {
      e->addAttributes( e->mCats->field[i], e->mCats->cat[i] );
    }
    e->mAttributes->show();
    e->mAttributes->raise();
    e->mAttributes->setCategoryMode( static_cast<QgsGrassEdit::CatMode>( e->mCatModeBox->currentIndex() ), e->mCatEntry->text() );
  }
}
