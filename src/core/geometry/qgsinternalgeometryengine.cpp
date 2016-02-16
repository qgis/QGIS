/***************************************************************************
  qgsinternalgeometryengine.cpp - QgsInternalGeometryEngine

 ---------------------
 begin                : 13.1.2016
 copyright            : (C) 2016 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsinternalgeometryengine.h"

#include "qgslinestringv2.h"
#include "qgsmultipolygonv2.h"
#include "qgspolygonv2.h"
#include "qgsmulticurvev2.h"

#include <QTransform>

QgsInternalGeometryEngine::QgsInternalGeometryEngine( const QgsGeometry& geometry )
    : mGeometry( geometry.geometry() )
{

}

/***************************************************************************
 * This class is considered CRITICAL and any change MUST be accompanied with
 * full unit tests.
 * See details in QEP #17
 ****************************************************************************/

QgsGeometry QgsInternalGeometryEngine::extrude( double x, double y )
{
  QList<QgsLineStringV2*> linesToProcess;

  const QgsMultiCurveV2* multiCurve = dynamic_cast< const QgsMultiCurveV2* >( mGeometry );
  if ( multiCurve )
  {
    for ( int i = 0; i < multiCurve->partCount(); ++i )
    {
      linesToProcess << static_cast<QgsLineStringV2*>( multiCurve->geometryN( i )->clone() );
    }
  }

  const QgsCurveV2* curve = dynamic_cast< const QgsCurveV2* >( mGeometry );
  if ( curve )
  {
    linesToProcess << static_cast<QgsLineStringV2*>( curve->segmentize() );
  }

  QgsMultiPolygonV2 *multipolygon = linesToProcess.size() > 1 ? new QgsMultiPolygonV2() : nullptr;
  QgsPolygonV2 *polygon = nullptr;

  if ( !linesToProcess.empty() )
  {
    Q_FOREACH ( QgsLineStringV2* line, linesToProcess )
    {
      QTransform transform = QTransform::fromTranslate( x, y );

      QgsLineStringV2* secondline = line->reversed();
      secondline->transform( transform );

      line->append( secondline );
      line->addVertex( line->pointN( 0 ) );

      polygon = new QgsPolygonV2();
      polygon->setExteriorRing( line );

      if ( multipolygon )
        multipolygon->addGeometry( polygon );

      delete secondline;
    }

    if ( multipolygon )
      return QgsGeometry( multipolygon );
    else
      return QgsGeometry( polygon );
  }

  return QgsGeometry();
}
