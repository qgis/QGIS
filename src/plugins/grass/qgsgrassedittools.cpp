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

#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsgrassedittools.h"
#include "qgsgrassedit.h"
#include "qgsgrassattributes.h"
#include "../../src/providers/grass/qgsgrassprovider.h"
#include "qgsvertexmarker.h"

extern "C" {
#include <grass/gis.h>
#include <grass/Vect.h>
}


QgsGrassEditTool::QgsGrassEditTool(QgsGrassEdit* edit)
  : QgsMapTool(edit->mCanvas), e(edit)
{
}

void QgsGrassEditTool::canvasPressEvent(QMouseEvent * event)
{
  QgsPoint point = toLayerCoords(e->layer(), event->pos());
  mouseClick(point,  event->button());

  // Set last click
  e->mLastPoint = point;

  e->statusBar()->message(e->mCanvasPrompt);

#ifdef QGISDEBUG
  std::cerr << "n_points = " << e->mEditPoints->n_points << std::endl;
#endif
}

void QgsGrassEditTool::canvasMoveEvent(QMouseEvent * event)
{
  QgsPoint point = toLayerCoords(e->layer(), event->pos());
  mouseMove(point);

  e->statusBar()->message(e->mCanvasPrompt);
}


// ------------------------------------------------------------------
// NEW POINT + NEW CENTROID
// ------------------------------------------------------------------


QgsGrassEditNewPoint::QgsGrassEditNewPoint(QgsGrassEdit* edit, bool newCentroid)
  : QgsGrassEditTool(edit), mNewCentroid(newCentroid)
{
  if (newCentroid)
    e->setCanvasPropmt( QObject::tr("New centroid"), "", "" );
  else
    e->setCanvasPropmt( QObject::tr("New point"), "", "" );

}


void QgsGrassEditNewPoint::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  if ( button != Qt::LeftButton)
    return;

  Vect_reset_line ( e->mEditPoints );
  e->snap ( point ); 
  Vect_append_point ( e->mEditPoints, point.x(), point.y(), 0.0 );

  int type;
  if (mNewCentroid) // new centroid or point ?
    type = GV_CENTROID;
  else
    type = GV_POINT;

  int line;
  line = e->writeLine ( type, e->mEditPoints );
  e->updateSymb();
  e->displayUpdated();

  if ( e->mAttributes ) 
  {
    e->mAttributes->setLine ( line );
    e->mAttributes->clear();
  }
  else
  {
    e->mAttributes = new QgsGrassAttributes ( e, e->mProvider, line, e->mIface->getMainWindow() );
  }
  for ( int i = 0; i < e->mCats->n_cats; i++ ) {
    e->addAttributes ( e->mCats->field[i], e->mCats->cat[i] );
  }
  e->mAttributes->show();
  e->mAttributes->raise();
}


// ------------------------------------------------------------------
// NEW LINE + NEW BOUNDARY
// ------------------------------------------------------------------

QgsGrassEditNewLine::QgsGrassEditNewLine(QgsGrassEdit* edit, bool newBoundary)
  : QgsGrassEditTool(edit), mNewBoundary(newBoundary)
{
  e->setCanvasPropmt( QObject::tr("New vertex"), "", "");
}

void QgsGrassEditNewLine::deactivate()
{
  // Delete last segment
  if ( e->mEditPoints->n_points > 1 ) {
    Vect_reset_line ( e->mPoints );
    Vect_append_points ( e->mPoints, e->mEditPoints, GV_FORWARD );
    e->displayDynamic ( e->mPoints );
  }
  e->setCanvasPropmt( QObject::tr("New vertex"), "", "");

  QgsGrassEditTool::deactivate(); // call default bahivour
}

void QgsGrassEditNewLine::activate()
{
  std::cerr << "QgsGrassEditNewLine::activate()" << std::endl;

  // Display dynamic segment
  if ( e->mEditPoints->n_points > 0 ) {
    Vect_reset_line ( e->mPoints );
    Vect_append_points ( e->mPoints, e->mEditPoints, GV_FORWARD );
    QgsPoint point = toMapCoords( e->mCanvas->mouseLastXY() );
    Vect_append_point ( e->mPoints, point.x(), point.y(), 0.0 );
    e->displayDynamic ( e->mPoints );
  }

  QgsGrassEditTool::activate(); // call default bahivour
}

void QgsGrassEditNewLine::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  switch ( button ) {
    case Qt::LeftButton:
      if ( e->mEditPoints->n_points > 2 )
      {
        e->snap ( point, e->mEditPoints->x[0], e->mEditPoints->y[0] ); 
      }
      else
      {
        e->snap ( point ); 
      }
      Vect_append_point ( e->mEditPoints, point.x(), point.y(), 0.0 );

      // Draw
      Vect_reset_line ( e->mPoints );
      Vect_append_points ( e->mPoints, e->mEditPoints, GV_FORWARD );
      e->displayDynamic ( e->mPoints );
      break;
    case Qt::MidButton:
      if ( e->mEditPoints->n_points > 0 ) {
        e->mEditPoints->n_points--;
        Vect_reset_line ( e->mPoints );
        Vect_append_points ( e->mPoints, e->mEditPoints, GV_FORWARD );
        QgsPoint point = toMapCoords( e->mCanvas->mouseLastXY() );
        Vect_append_point ( e->mPoints, point.x(), point.y(), 0.0 );
        e->displayDynamic ( e->mPoints );
      }
      break;
    case Qt::RightButton:
      e->eraseDynamic();
      if ( e->mEditPoints->n_points > 1 ) {
        int type;

        if ( mNewBoundary ) // boundary or line?
          type = GV_BOUNDARY;
        else
          type = GV_LINE;

        int line;
        line = e->writeLine ( type, e->mEditPoints );
        e->updateSymb();
        e->displayUpdated();

        if ( e->mAttributes ) 
        {
          e->mAttributes->setLine ( line );
          e->mAttributes->clear();
        }
        else
        {
          e->mAttributes = new QgsGrassAttributes ( e, e->mProvider, line, e->mIface->getMainWindow() );
        }
        for ( int i = 0; i < e->mCats->n_cats; i++ ) {
          e->addAttributes ( e->mCats->field[i], e->mCats->cat[i] );
        }
        e->mAttributes->show();
        e->mAttributes->raise();
      }
      Vect_reset_line ( e->mEditPoints );
      break;
  }

  if ( e->mEditPoints->n_points == 0 ) {
    e->setCanvasPropmt( QObject::tr("New point"), "", "");
  } else if ( e->mEditPoints->n_points == 1 ) {
    e->setCanvasPropmt( QObject::tr("New point"), QObject::tr("Undo last point"), "" );
  } else if ( e->mEditPoints->n_points > 1 ) {
    e->setCanvasPropmt( QObject::tr("New point"), QObject::tr("Undo last point"), QObject::tr("Close line"));
  }
}

void QgsGrassEditNewLine::mouseMove(QgsPoint & newPoint)
{
  if ( e->mEditPoints->n_points > 0 ) {
    // Draw the line with new segment
    Vect_reset_line ( e->mPoints );
    Vect_append_points ( e->mPoints, e->mEditPoints, GV_FORWARD );
    Vect_append_point ( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
    e->displayDynamic ( e->mPoints );
  }
}


// ------------------------------------------------------------------
// MOVE VERTEX
// ------------------------------------------------------------------

QgsGrassEditMoveVertex::QgsGrassEditMoveVertex(QgsGrassEdit* edit)
  : QgsGrassEditTool(edit)
{
  e->setCanvasPropmt( QObject::tr("Select vertex"), "", "" );
}
    
void QgsGrassEditMoveVertex::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  double thresh = e->threshold();

  switch ( button ) {
    case Qt::LeftButton:
      // Move previously selected vertex 
      if ( e->mSelectedLine > 0 ) {
        e->eraseDynamic();
        e->eraseElement ( e->mSelectedLine );

        // Move vertex
        int type = e->mProvider->readLine ( e->mPoints, e->mCats, e->mSelectedLine );
        e->snap ( point ); 
        e->mPoints->x[e->mSelectedPart] = point.x();
        e->mPoints->y[e->mSelectedPart] = point.y();

        Vect_line_prune ( e->mPoints );
        e->mProvider->rewriteLine ( e->mSelectedLine, type, e->mPoints, e->mCats );
        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line ( e->mEditPoints );

        e->setCanvasPropmt( QObject::tr("Select vertex"), "", "" );
      } else {
        // Select new line
        e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_LINES, thresh );

        if ( e->mSelectedLine ) { // highlite
          e->mProvider->readLine ( e->mEditPoints, NULL, e->mSelectedLine );
          e->displayElement ( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

          double xl, yl; // nearest point on the line

          // Note first segment is 1!
          e->mSelectedPart = Vect_line_distance ( e->mEditPoints, point.x(), point.y(), 0.0, 0, 
            &xl, &yl, NULL, NULL, NULL, NULL );

          double dist1 = Vect_points_distance ( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart-1], 
            e->mEditPoints->y[e->mSelectedPart-1], 0.0, 0);
          double dist2 = Vect_points_distance ( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart], 
            e->mEditPoints->y[e->mSelectedPart], 0.0, 0);

          if ( dist1 < dist2 ) e->mSelectedPart--;

          e->setCanvasPropmt( QObject::tr("Select new position"), "", "Release vertex" );
        }
      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement ( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      Vect_reset_line ( e->mEditPoints );

      e->setCanvasPropmt( QObject::tr("Select vertex"), "", "" );
      break;

    case Qt::MidButton:
      break;
  }

}

void QgsGrassEditMoveVertex::mouseMove(QgsPoint & newPoint)
{
  if ( e->mSelectedLine > 0 ) {
    // Transform coordinates
    Vect_reset_line ( e->mPoints );
    if ( e->mSelectedPart == 0 ) {
      Vect_append_point ( e->mPoints, e->mEditPoints->x[1], e->mEditPoints->y[1], 0.0 );
      Vect_append_point ( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
    } else if ( e->mSelectedPart == e->mEditPoints->n_points-1 ) {
      Vect_append_point ( e->mPoints, e->mEditPoints->x[e->mSelectedPart-1], 
        e->mEditPoints->y[e->mSelectedPart-1], 0.0 );
      Vect_append_point ( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
    } else {
      Vect_append_point ( e->mPoints, e->mEditPoints->x[e->mSelectedPart-1], 
        e->mEditPoints->y[e->mSelectedPart-1], 0.0 );
      Vect_append_point ( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
      Vect_append_point ( e->mPoints, e->mEditPoints->x[e->mSelectedPart+1], 
        e->mEditPoints->y[e->mSelectedPart+1], 0.0 );
    }
    for (int i = 0; i < e->mPoints->n_points; i++ ) {
      std::cerr << e->mPoints->x[i] << " " << e->mPoints->y[i] << std::endl;
    }

    e->displayDynamic ( e->mPoints );
  }
}


// ------------------------------------------------------------------
// ADD VERTEX
// ------------------------------------------------------------------

QgsGrassEditAddVertex::QgsGrassEditAddVertex(QgsGrassEdit* edit)
  : QgsGrassEditTool(edit)
{
  e->setCanvasPropmt( QObject::tr("Select line segment"), "", "" );
}
    
void QgsGrassEditAddVertex::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  double thresh = e->threshold();

  switch ( button ) {
    case Qt::LeftButton:
      // Add vertex to previously selected line
      if ( e->mSelectedLine > 0 ) {
        e->eraseDynamic();
        e->eraseElement ( e->mSelectedLine );

        // Move vertex
        int type = e->mProvider->readLine ( e->mPoints, e->mCats, e->mSelectedLine );

        if ( e->mAddVertexEnd && e->mSelectedPart == e->mEditPoints->n_points-1 ) {
          e->snap ( point ); 
          Vect_append_point ( e->mPoints, point.x(), point.y(), 0.0 );
        } else {
          Vect_line_insert_point ( e->mPoints, e->mSelectedPart, point.x(), point.y(), 0.0 );
        }

        Vect_line_prune ( e->mPoints );
        e->mProvider->rewriteLine ( e->mSelectedLine, type, e->mPoints, e->mCats );
        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line ( e->mEditPoints );

        e->setCanvasPropmt( QObject::tr("Select line segment"), "", "" );
      } else {
        // Select new line
        e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_LINES, thresh );

        if ( e->mSelectedLine ) { // highlite
          e->mProvider->readLine ( e->mEditPoints, NULL, e->mSelectedLine );
          e->displayElement ( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

          double xl, yl; // nearest point on the line

          // Note first segment is 1!
          e->mSelectedPart = Vect_line_distance ( e->mEditPoints, point.x(), point.y(), 0.0, 0, 
            &xl, &yl, NULL, NULL, NULL, NULL );

          double dist1 = Vect_points_distance ( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart-1], 
            e->mEditPoints->y[e->mSelectedPart-1], 0.0, 0);
          double dist2 = Vect_points_distance ( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart], 
            e->mEditPoints->y[e->mSelectedPart], 0.0, 0);

          double maxdist = (dist1 + dist2)/4;

          if ( e->mSelectedPart == 1 && dist1 < maxdist ) {
            e->mSelectedPart = 0;
            e->mAddVertexEnd = true;
          } else if ( e->mSelectedPart == e->mEditPoints->n_points-1 && dist2 < maxdist ) {
            e->mAddVertexEnd = true;
          } else {
            e->mAddVertexEnd = false;
          }

          e->setCanvasPropmt( QObject::tr("New vertex position"), "", QObject::tr("Release") );
        } else {
          e->setCanvasPropmt( QObject::tr("Select line segment"), "", "" );
        }
      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement ( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      Vect_reset_line ( e->mEditPoints );

      e->setCanvasPropmt( QObject::tr("Select line segment"), "", "" );
      break;

    case Qt::MidButton:
      break;
  }

}

void QgsGrassEditAddVertex::mouseMove(QgsPoint & newPoint)
{
  if ( e->mSelectedLine > 0 ) {
    Vect_reset_line ( e->mPoints );
    if ( e->mAddVertexEnd ) {
      Vect_append_point ( e->mPoints, e->mEditPoints->x[e->mSelectedPart], 
        e->mEditPoints->y[e->mSelectedPart], 0.0 );
      Vect_append_point ( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
    } else {
      Vect_append_point ( e->mPoints, e->mEditPoints->x[e->mSelectedPart-1], 
        e->mEditPoints->y[e->mSelectedPart-1], 0.0 );
      Vect_append_point ( e->mPoints, newPoint.x(), newPoint.y(), 0.0 );
      Vect_append_point ( e->mPoints, e->mEditPoints->x[e->mSelectedPart], 
        e->mEditPoints->y[e->mSelectedPart], 0.0 );
    }
    for (int i = 0; i < e->mPoints->n_points; i++ ) {
      std::cerr << e->mPoints->x[i] << " " << e->mPoints->y[i] << std::endl;
    }

    e->displayDynamic ( e->mPoints );
  }
}

// ------------------------------------------------------------------
// DELETE VERTEX
// ------------------------------------------------------------------

QgsGrassEditDeleteVertex::QgsGrassEditDeleteVertex(QgsGrassEdit* edit)
  : QgsGrassEditTool(edit)
{
  e->setCanvasPropmt( QObject::tr("Select vertex"), "", "" );
}
    
void QgsGrassEditDeleteVertex::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  double thresh = e->threshold();

  switch ( button ) {
    case Qt::LeftButton:
      // Delete previously selected vertex 
      if ( e->mSelectedLine > 0 ) {
        e->eraseDynamic();
        e->eraseElement ( e->mSelectedLine );

        // Move vertex
        int type = e->mProvider->readLine ( e->mPoints, e->mCats, e->mSelectedLine );
        Vect_line_delete_point ( e->mPoints, e->mSelectedPart );

        if ( e->mPoints->n_points < 2 ) // delete line
        {
          e->mProvider->deleteLine ( e->mSelectedLine );

          // Check orphan records
          for ( int i = 0 ; i < e->mCats->n_cats; i++ ) {
            e->checkOrphan ( e->mCats->field[i], e->mCats->cat[i] );
          }
        }
        else 
        {
          e->mProvider->rewriteLine ( e->mSelectedLine, type, e->mPoints, e->mCats );
        }

        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line ( e->mEditPoints );

        e->setCanvasPropmt( QObject::tr("Select vertex"), "", "" );
      } else {
        // Select new/next line
        e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_LINES, thresh );

        if ( e->mSelectedLine ) { // highlite
          e->mProvider->readLine ( e->mEditPoints, NULL, e->mSelectedLine );

          e->displayElement ( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

          double xl, yl; // nearest point on the line

          // Note first segment is 1!
          e->mSelectedPart = Vect_line_distance ( e->mEditPoints, point.x(), point.y(), 0.0, 0, 
            &xl, &yl, NULL, NULL, NULL, NULL );

          double dist1 = Vect_points_distance ( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart-1], 
            e->mEditPoints->y[e->mSelectedPart-1], 0.0, 0);
          double dist2 = Vect_points_distance ( xl, yl, 0.0, e->mEditPoints->x[e->mSelectedPart], 
            e->mEditPoints->y[e->mSelectedPart], 0.0, 0);

          if ( dist1 < dist2 ) e->mSelectedPart--;

          e->displayDynamic ( e->mEditPoints->x[e->mSelectedPart], e->mEditPoints->y[e->mSelectedPart], 
            QgsVertexMarker::ICON_BOX, e->mSize );

          e->setCanvasPropmt( QObject::tr("Delete vertex"), "", QObject::tr("Release vertex") );
        } else {
          e->setCanvasPropmt( QObject::tr("Select vertex"), "", "" );
        }
      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement ( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      Vect_reset_line ( e->mEditPoints );

      e->setCanvasPropmt( QObject::tr("Select vertex"), "", "" );
      break;

    case Qt::MidButton:
      break;
  }
}

// ------------------------------------------------------------------
// MOVE LINE
// ------------------------------------------------------------------

QgsGrassEditMoveLine::QgsGrassEditMoveLine(QgsGrassEdit* edit)
  : QgsGrassEditTool(edit)
{
  e->setCanvasPropmt( QObject::tr("Select element"), "", "" );
}
    
void QgsGrassEditMoveLine::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  double thresh = e->threshold();

  switch ( button ) {
    case Qt::LeftButton:
      // Move previously selected line 
      if ( e->mSelectedLine > 0 ) {
        e->eraseDynamic();
        e->eraseElement ( e->mSelectedLine );

        // Transform coordinates
        int type = e->mProvider->readLine ( e->mPoints, e->mCats, e->mSelectedLine );
        for ( int i = 0; i < e->mPoints->n_points; i++ ) {
          e->mPoints->x[i] += point.x() - e->mLastPoint.x(); 
          e->mPoints->y[i] += point.y() - e->mLastPoint.y(); 
        }

        e->mProvider->rewriteLine ( e->mSelectedLine, type, e->mPoints, e->mCats );
        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line ( e->mEditPoints );

        e->setCanvasPropmt( QObject::tr("Select element"), "", "" );
      } else {
        // Select new/next line
        e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_POINT|GV_CENTROID, thresh );

        if ( e->mSelectedLine == 0 ) 
          e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_LINE|GV_BOUNDARY, thresh );

        if ( e->mSelectedLine ) { // highlite
          e->mProvider->readLine ( e->mEditPoints, NULL, e->mSelectedLine );
          e->displayElement ( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );
          e->setCanvasPropmt( QObject::tr("New location"), "", QObject::tr("Release selected") );
        } else { 
          e->setCanvasPropmt( QObject::tr("Select element"), "", "" );
        }
      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement ( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      e->setCanvasPropmt( QObject::tr("Select element"), "", "" );
      break;

    case Qt::MidButton:
      break;
  }
}

void QgsGrassEditMoveLine::mouseMove(QgsPoint & newPoint)
{
  // Move previously selected line 
  if ( e->mSelectedLine > 0 ) {
    // Transform coordinates
    Vect_reset_line ( e->mPoints );
    Vect_append_points ( e->mPoints, e->mEditPoints, GV_FORWARD );

    for ( int i = 0; i < e->mPoints->n_points; i++ ) {
      e->mPoints->x[i] += newPoint.x() - e->mLastPoint.x(); 
      e->mPoints->y[i] += newPoint.y() - e->mLastPoint.y(); 
    }

    e->displayDynamic ( e->mPoints );
  }
}


// ------------------------------------------------------------------
// DELETE LINE
// ------------------------------------------------------------------

QgsGrassEditDeleteLine::QgsGrassEditDeleteLine(QgsGrassEdit* edit)
  : QgsGrassEditTool(edit)
{
  e->setCanvasPropmt( QObject::tr("Select element"), "", "" );
}
    
void QgsGrassEditDeleteLine::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  double thresh = e->threshold();

  switch ( button ) {
    case Qt::LeftButton:
      // Delete previously selected line 
      if ( e->mSelectedLine > 0 ) {
        e->eraseElement ( e->mSelectedLine );
        e->mProvider->readLine ( NULL, e->mCats, e->mSelectedLine );
        e->mProvider->deleteLine ( e->mSelectedLine );

        // Check orphan records
        for ( int i = 0 ; i < e->mCats->n_cats; i++ ) {
          e->checkOrphan ( e->mCats->field[i], e->mCats->cat[i] );
        }

        e->updateSymb();
        e->displayUpdated();
      }

      // Select new/next line
      e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_POINT|GV_CENTROID, thresh );

      if ( e->mSelectedLine == 0 ) 
        e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_LINE|GV_BOUNDARY, thresh );

      if ( e->mSelectedLine ) { // highlite, propmt
        e->displayElement ( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );
        e->setCanvasPropmt( QObject::tr("Delete selected / select next"), "", QObject::tr("Release selected") );
      } else {
        e->setCanvasPropmt( QObject::tr("Select element"), "", "" );
      }
      break;

    case Qt::RightButton:
      e->displayElement ( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      e->setCanvasPropmt( QObject::tr("Select element"), "", "" );
      break;

    case Qt::MidButton:
      break;
  }
}

// ------------------------------------------------------------------
// SPLIT LINE
// ------------------------------------------------------------------

QgsGrassEditSplitLine::QgsGrassEditSplitLine(QgsGrassEdit* edit)
  : QgsGrassEditTool(edit)
{
  e->setCanvasPropmt( QObject::tr("Select position on line"), "", "" );
}
    
void QgsGrassEditSplitLine::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  double thresh = e->threshold();

  switch ( button ) {
    case Qt::LeftButton:
      // Split previously selected line 
      if ( e->mSelectedLine > 0 ) {
        e->eraseDynamic();
        e->eraseElement ( e->mSelectedLine );

        int type = e->mProvider->readLine ( e->mPoints, e->mCats, e->mSelectedLine );

        double xl, yl;
        Vect_line_distance ( e->mPoints, e->mLastPoint.x(), e->mLastPoint.y(), 0.0, 0, 
          &xl, &yl, NULL, NULL, NULL, NULL );

        e->mPoints->n_points = e->mSelectedPart;
        Vect_append_point ( e->mPoints, xl, yl, 0.0 );
        e->mProvider->rewriteLine ( e->mSelectedLine, type, e->mPoints, e->mCats );
        e->updateSymb();
        e->displayUpdated();

        Vect_reset_line ( e->mPoints );
        Vect_append_point ( e->mPoints, xl, yl, 0.0 );
        for ( int i = e->mSelectedPart; i < e->mEditPoints->n_points; i++ ) {
          Vect_append_point ( e->mPoints, e->mEditPoints->x[i], e->mEditPoints->y[i], 0.0 );
        }

        e->mProvider->writeLine ( type, e->mPoints, e->mCats );

        e->updateSymb();
        e->displayUpdated();

        e->mSelectedLine = 0;
        Vect_reset_line ( e->mEditPoints );
        e->setCanvasPropmt( QObject::tr("Select position on line"), "", "" );
      } else {
        // Select new/next line
        e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_LINES, thresh );

        if ( e->mSelectedLine ) { // highlite
          e->mProvider->readLine ( e->mEditPoints, NULL, e->mSelectedLine );

          e->displayElement ( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

          double xl, yl; // nearest point on the line

          // Note first segment is 1!
          e->mSelectedPart = Vect_line_distance ( e->mEditPoints, point.x(), point.y(), 0.0, 0, 
            &xl, &yl, NULL, NULL, NULL, NULL );

          e->displayDynamic ( xl, yl, QgsVertexMarker::ICON_X, e->mSize );

          e->setCanvasPropmt( QObject::tr("Split the line"), "", QObject::tr("Release the line") );
        } else {
          e->setCanvasPropmt( QObject::tr("Select point on line"), "", "" );
        }

      }
      break;

    case Qt::RightButton:
      e->eraseDynamic();
      e->displayElement ( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
      e->mSelectedLine = 0;
      Vect_reset_line ( e->mEditPoints );

      e->setCanvasPropmt( QObject::tr("Select point on line"), "", "" );
      break;

    case Qt::MidButton:
      break;
  }
}


// ------------------------------------------------------------------
// EDIT ATTRIBUTES
// ------------------------------------------------------------------

QgsGrassEditAttributes::QgsGrassEditAttributes(QgsGrassEdit* edit)
  : QgsGrassEditTool(edit)
{
  e->setCanvasPropmt( QObject::tr("Select element"), "", "" );
}
    
void QgsGrassEditAttributes::mouseClick(QgsPoint & point, Qt::ButtonState button)
{
  double thresh = e->threshold();

  // Redraw previously selected line 
  if ( e->mSelectedLine > 0 ) {
    e->displayElement ( e->mSelectedLine, e->mSymb[e->mLineSymb[e->mSelectedLine]], e->mSize );
  }

  // Select new/next line
  e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_POINT|GV_CENTROID, thresh );

  if ( e->mSelectedLine == 0 ) 
    e->mSelectedLine = e->mProvider->findLine ( point.x(), point.y(), GV_LINE|GV_BOUNDARY, thresh );

#ifdef QGISDEBUG
  std::cerr << "mSelectedLine = " << e->mSelectedLine << std::endl;
#endif

  if ( e->mAttributes ) 
  {
    e->mAttributes->setLine ( 0 );
    e->mAttributes->clear();
    e->mAttributes->raise();
  }

  if ( e->mSelectedLine ) { // highlite
    e->displayElement ( e->mSelectedLine, e->mSymb[QgsGrassEdit::SYMB_HIGHLIGHT], e->mSize );

    e->mProvider->readLine ( NULL, e->mCats, e->mSelectedLine );

    if ( !e->mAttributes ) 
    {
      e->mAttributes = new QgsGrassAttributes ( e, e->mProvider, e->mSelectedLine, e->mIface->getMainWindow() );
    }
    else
    {
      e->mAttributes->setLine ( e->mSelectedLine );
    }
    for ( int i = 0; i < e->mCats->n_cats; i++ ) {
      e->addAttributes ( e->mCats->field[i], e->mCats->cat[i] );
    }
    e->mAttributes->show();
    e->mAttributes->raise();
  }
}
