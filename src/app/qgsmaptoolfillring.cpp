/***************************************************************************
    qgsmaptoolfillring.h  - map tool to cut rings in polygon and multipolygon
                            features and fill them with new feature
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolfillring.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsattributedialog.h"
#include <qgsapplication.h>

#include <QMouseEvent>

#include <limits>

QgsMapToolFillRing::QgsMapToolFillRing( QgsMapCanvas* canvas )
    : QgsMapToolCapture( canvas, QgsMapToolCapture::CapturePolygon )
{
}

QgsMapToolFillRing::~QgsMapToolFillRing()
{
}

void QgsMapToolFillRing::canvasMapReleaseEvent( QgsMapMouseEvent * e )
{
  //check if we operate on a vector layer
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  if ( !vlayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  //add point to list and to rubber band
  if ( e->button() == Qt::LeftButton )
  {
    int error = addVertex( e->mapPoint() );
    if ( error == 1 )
    {
      //current layer is not a vector layer
      return;
    }
    else if ( error == 2 )
    {
      //problem with coordinate transformation
      emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system" ), QgsMessageBar::WARNING );
      return;
    }

    startCapturing();
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( !isCapturing() )
      return;

    deleteTempRubberBand();

    closePolygon();

    vlayer->beginEditCommand( tr( "Ring added and filled" ) );
    int addRingReturnCode = vlayer->addRing( points() );
    if ( addRingReturnCode != 0 )
    {
      QString errorMessage;
      //todo: open message box to communicate errors
      if ( addRingReturnCode == 1 )
      {
        errorMessage = tr( "a problem with geometry type occured" );
      }
      else if ( addRingReturnCode == 2 )
      {
        errorMessage = tr( "the inserted Ring is not closed" );
      }
      else if ( addRingReturnCode == 3 )
      {
        errorMessage = tr( "the inserted Ring is not a valid geometry" );
      }
      else if ( addRingReturnCode == 4 )
      {
        errorMessage = tr( "the inserted Ring crosses existing rings" );
      }
      else if ( addRingReturnCode == 5 )
      {
        errorMessage = tr( "the inserted Ring is not contained in a feature" );
      }
      else
      {
        errorMessage = tr( "an unknown error occured" );
      }
      emit messageEmitted( tr( "could not add ring since %1." ).arg( errorMessage ), QgsMessageBar::CRITICAL );
      vlayer->destroyEditCommand();
    }
    else
    {
      // find parent feature and get it attributes
      double xMin, xMax, yMin, yMax;
      QgsRectangle bBox;

      xMin = std::numeric_limits<double>::max();
      xMax = -std::numeric_limits<double>::max();
      yMin = std::numeric_limits<double>::max();
      yMax = -std::numeric_limits<double>::max();

      for ( QList<QgsPoint>::const_iterator it = points().constBegin(); it != points().constEnd(); ++it )
      {
        if ( it->x() < xMin )
        {
          xMin = it->x();
        }
        if ( it->x() > xMax )
        {
          xMax = it->x();
        }
        if ( it->y() < yMin )
        {
          yMin = it->y();
        }
        if ( it->y() > yMax )
        {
          yMax = it->y();
        }
      }
      bBox.setXMinimum( xMin );
      bBox.setYMinimum( yMin );
      bBox.setXMaximum( xMax );
      bBox.setYMaximum( yMax );

      QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( bBox ).setFlags( QgsFeatureRequest::ExactIntersect ) );

      QgsFeature f;
      bool res = false;
      while ( fit.nextFeature( f ) )
      {
        //create QgsFeature with wkb representation
        QgsFeature* ft = new QgsFeature( vlayer->pendingFields(), 0 );

        QgsGeometry *g;
        g = QgsGeometry::fromPolygon( QgsPolygon() << points().toVector() );
        ft->setGeometry( g );
        ft->setAttributes( f.attributes() );

        if ( QgsApplication::keyboardModifiers() == Qt::ControlModifier )
        {
          res = vlayer->addFeature( *ft );
        }
        else
        {
          QgsAttributeDialog *dialog = new QgsAttributeDialog( vlayer, ft, false, NULL, true );
          dialog->setIsAddDialog( true );
          res = dialog->exec(); // will also add the feature
        }

        if ( res )
        {
          vlayer->endEditCommand();
        }
        else
        {
          delete ft;
          vlayer->destroyEditCommand();
        }
        res = false;
      }
    }
    stopCapturing();
  }
}
