/***************************************************************************
    qgsmaptoolreverseline.cpp  - reverse a line geometry
    ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Loïc Bartoletti
    email                : loic dot bartoletti at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolreverseline.h"

#include "qgsfeatureiterator.h"
#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgstolerance.h"
#include "qgisapp.h"
#include "qgslinestring.h"
#include "qgsmultilinestring.h"
#include "qgsmapmouseevent.h"


QgsMapToolReverseLine::QgsMapToolReverseLine( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
{
  mToolName = tr( "Reverse line geometry" );
}

QgsMapToolReverseLine::~QgsMapToolReverseLine()
{
}

void QgsMapToolReverseLine::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
  //nothing to do
}

void QgsMapToolReverseLine::canvasPressEvent( QgsMapMouseEvent *e )
{
  mPressedFid = -1;
  mPressedPartNum = -1;

  QgsMapLayer *currentLayer = mCanvas->currentLayer();
  if ( !currentLayer )
    return;

  vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
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

  const QgsGeometry geomPart = partUnderPoint( e->pos(), mPressedFid, mPressedPartNum );

  if ( mPressedFid != -1 )
  {
    mRubberBand.reset( createRubberBand( vlayer->geometryType() ) );

    mRubberBand->setToGeometry( geomPart, vlayer );
    mRubberBand->show();
  }

}

void QgsMapToolReverseLine::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )

  if ( !vlayer || !vlayer->isEditable() )
  {
    return;
  }

  if ( mPressedFid == -1 )
    return;

  QgsFeature f;
  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mPressedFid ) ).nextFeature( f );
  QgsGeometry geom;

  if ( f.hasGeometry() )
  {
    if ( f.geometry().isMultipart() )
    {
      std::unique_ptr<QgsMultiCurve> line_reversed( static_cast<QgsMultiCurve * >( f.geometry().constGet()->clone() ) );
      std::unique_ptr<QgsCurve> line_part( line_reversed->curveN( mPressedPartNum )->clone() );
      std::unique_ptr<QgsCurve> line_part_reversed( line_part->reversed() );
      line_reversed->removeGeometry( mPressedPartNum );
      line_reversed->insertGeometry( line_part_reversed.release(), mPressedPartNum );

      geom = QgsGeometry( line_reversed.release() );

    }
    else
    {

      geom = QgsGeometry( static_cast< const QgsCurve * >( f.geometry().constGet() )->reversed() );

    }

    if ( !geom.isNull() )
    {
      vlayer->beginEditCommand( tr( "Reverse line" ) );
      vlayer->changeGeometry( f.id(), geom );
      vlayer->endEditCommand();
      vlayer->triggerRepaint();
      emit messageEmitted( tr( "Line reversed." ) );
    }
    else
    {
      emit messageEmitted( tr( "Couldn't reverse the selected part." ) );
    }
  }
  mRubberBand.reset();
}

QgsGeometry QgsMapToolReverseLine::partUnderPoint( QPoint point, QgsFeatureId &fid, int &partNum )
{
  QgsFeature f;
  const QgsGeometry geomPart;

  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::LineGeometry:
    {
      const QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( point, QgsPointLocator::Types( QgsPointLocator::Vertex | QgsPointLocator::Edge ) );
      if ( !match.isValid() )
        return geomPart;

      int snapVertex = match.vertexIndex();
      vlayer->getFeatures( QgsFeatureRequest().setFilterFid( match.featureId() ) ).nextFeature( f );
      const QgsGeometry g = f.geometry();
      if ( !g.isMultipart() )
      {
        fid = match.featureId();
        return g;
      }
      else if ( QgsWkbTypes::geometryType( g.wkbType() ) == QgsWkbTypes::LineGeometry )
      {
        QgsMultiPolylineXY mline = g.asMultiPolyline();
        for ( int part = 0; part < mline.count(); part++ )
        {
          if ( snapVertex < mline[part].count() )
          {
            fid = match.featureId();
            partNum = part;
            return QgsGeometry::fromPolylineXY( mline[part] );
          }
          snapVertex -= mline[part].count();
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
  return geomPart;
}

void QgsMapToolReverseLine::deactivate()
{
  QgsMapTool::deactivate();
}

