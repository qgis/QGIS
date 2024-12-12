/***************************************************************************
                         qgsalgorithmtessellate.cpp
                         ---------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsalgorithmtessellate.h"
#include "qgstessellator.h"
#include "qgsmultipolygon.h"
#include "qgspolygon.h"
#include <geos_c.h>

///@cond PRIVATE

QString QgsTessellateAlgorithm::name() const
{
  return QStringLiteral( "tessellate" );
}

QString QgsTessellateAlgorithm::displayName() const
{
  return QObject::tr( "Tessellate" );
}

QStringList QgsTessellateAlgorithm::tags() const
{
  return QObject::tr( "3d,triangle" ).split( ',' );
}

QString QgsTessellateAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTessellateAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsTessellateAlgorithm::outputName() const
{
  return QObject::tr( "Tessellated" );
}

Qgis::ProcessingSourceType QgsTessellateAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

Qgis::WkbType QgsTessellateAlgorithm::outputWkbType( Qgis::WkbType inputWkbType ) const
{
  return QgsWkbTypes::hasZ( inputWkbType ) ? Qgis::WkbType::MultiPolygonZ : Qgis::WkbType::MultiPolygon;
}

QString QgsTessellateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm tessellates a polygon geometry layer, dividing the geometries into triangular components." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The output layer consists of multipolygon geometries for each input feature, with each multipolygon consisting of multiple triangle component polygons." );
}

QList<int> QgsTessellateAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

QgsTessellateAlgorithm *QgsTessellateAlgorithm::createInstance() const
{
  return new QgsTessellateAlgorithm();
}

QgsFeatureList QgsTessellateAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    if ( QgsWkbTypes::geometryType( f.geometry().wkbType() ) != Qgis::GeometryType::Polygon )
      f.clearGeometry();
    else
    {
      const QgsGeometry inputGeometry = f.geometry();
      bool tessellationComplete = false;
#if ( GEOS_VERSION_MAJOR == 3 && GEOS_VERSION_MINOR >= 11 ) || GEOS_VERSION_MAJOR > 3
      if ( !inputGeometry.constGet()->is3D() )
      {
        // on supported GEOS versions we prefer to use GEOS GEOSConstrainedDelaunayTriangulation
        // for 2D triangulation, as it's more stable and tolerant to situations like polygon
        // holes touching an exterior ring vs the poly2tri based tessellation
        const QgsGeometry triangulation = inputGeometry.constrainedDelaunayTriangulation();
        if ( triangulation.isEmpty() && !inputGeometry.isEmpty() )
        {
          if ( !triangulation.lastError().isEmpty() )
          {
            feedback->reportError( QObject::tr( "Feature ID %1 could not be tessellated: %2" ).arg( f.id() ).arg( triangulation.lastError() ) );
          }
          else
          {
            feedback->reportError( QObject::tr( "Feature ID %1 could not be tessellated" ).arg( f.id() ) );
          }
        }
        else
        {
          f.setGeometry( triangulation );
        }
        tessellationComplete = true;
      }
#endif

      if ( !tessellationComplete )
      {
        // 3D case, or 2D case with unsupported GEOS version -- use less stable poly2tri backend
        const QgsRectangle bounds = f.geometry().boundingBox();
        QgsTessellator t( bounds, false );
        t.setOutputZUp( true );

        if ( f.geometry().isMultipart() )
        {
          const QgsMultiSurface *ms = qgsgeometry_cast<const QgsMultiSurface *>( f.geometry().constGet() );
          for ( int i = 0; i < ms->numGeometries(); ++i )
          {
            const std::unique_ptr<QgsPolygon> p( qgsgeometry_cast<QgsPolygon *>( ms->geometryN( i )->segmentize() ) );
            t.addPolygon( *p, 0 );
          }
        }
        else
        {
          const std::unique_ptr<QgsPolygon> p( qgsgeometry_cast<QgsPolygon *>( f.geometry().constGet()->segmentize() ) );
          t.addPolygon( *p, 0 );
        }
        QgsGeometry g( t.asMultiPolygon() );
        if ( !g.isEmpty() )
        {
          if ( !t.error().isEmpty() )
          {
            feedback->reportError( QObject::tr( "Feature ID %1 was only partially tessellated: %2" ).arg( f.id() ).arg( t.error() ) );
          }

          g.translate( bounds.xMinimum(), bounds.yMinimum() );
        }
        else
        {
          if ( !t.error().isEmpty() )
            feedback->reportError( QObject::tr( "Feature ID %1 could not be tessellated: %2" ).arg( f.id() ).arg( t.error() ) );
          else
            feedback->reportError( QObject::tr( "Feature ID %1 could not be divided into triangular components." ).arg( f.id() ) );
        }
        if ( !inputGeometry.constGet()->is3D() )
          g.get()->dropZValue();

        f.setGeometry( g );
      }
    }
  }
  return QgsFeatureList() << f;
}

///@endcond
