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

#include "qgslinestring.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include "qgsmulticurve.h"
#include "qgsgeometry.h"

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
  QList<QgsLineString*> linesToProcess;

  const QgsMultiCurve* multiCurve = dynamic_cast< const QgsMultiCurve* >( mGeometry );
  if ( multiCurve )
  {
    for ( int i = 0; i < multiCurve->partCount(); ++i )
    {
      linesToProcess << static_cast<QgsLineString*>( multiCurve->geometryN( i )->clone() );
    }
  }

  const QgsCurve* curve = dynamic_cast< const QgsCurve* >( mGeometry );
  if ( curve )
  {
    linesToProcess << static_cast<QgsLineString*>( curve->segmentize() );
  }

  QgsMultiPolygonV2 *multipolygon = linesToProcess.size() > 1 ? new QgsMultiPolygonV2() : nullptr;
  QgsPolygonV2 *polygon = nullptr;

  if ( !linesToProcess.empty() )
  {
    Q_FOREACH ( QgsLineString* line, linesToProcess )
    {
      QTransform transform = QTransform::fromTranslate( x, y );

      QgsLineString* secondline = line->reversed();
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
