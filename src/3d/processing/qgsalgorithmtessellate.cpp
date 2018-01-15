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
#include "qgstriangle.h"
///@cond PRIVATE

QString QgsTessellateAlgorithm::name() const
{
  return QStringLiteral( "tessellate" );
}

QString QgsTessellateAlgorithm::displayName() const
{
  return QObject::tr( "Tessellate" );
}

QgsProcessingAlgorithm::Flags QgsTessellateAlgorithm::flags() const
{
  return QgsProcessingFeatureBasedAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QStringList QgsTessellateAlgorithm::tags() const
{
  return QObject::tr( "3d,triangle" ).split( ',' );
}

QString QgsTessellateAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTessellateAlgorithm::outputName() const
{
  return QObject::tr( "Tessellated" );
}

QgsProcessing::SourceType QgsTessellateAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsWkbTypes::Type QgsTessellateAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  Q_UNUSED( inputWkbType );
  return QgsWkbTypes::MultiPolygonZ;
}

QString QgsTessellateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm tessellates a polygon geometry layer, dividing the geometries into triangular components." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The output layer consists of multipolygon geometries for each input feature, with each multipolygon consisting of multiple triangle component polygons." );
}

QList<int> QgsTessellateAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon;
}

QgsTessellateAlgorithm *QgsTessellateAlgorithm::createInstance() const
{
  return new QgsTessellateAlgorithm();
}

QgsFeature QgsTessellateAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    if ( QgsWkbTypes::geometryType( f.geometry().wkbType() ) != QgsWkbTypes::PolygonGeometry )
      f.clearGeometry();
    else
    {
      QgsRectangle bounds = f.geometry().boundingBox();
      QgsTessellator t( bounds.xMinimum(), bounds.yMinimum(), false );

      if ( f.geometry().isMultipart() )
      {
        const QgsMultiSurface *ms = qgsgeometry_cast< const QgsMultiSurface * >( f.geometry().constGet() );
        for ( int i = 0; i < ms->numGeometries(); ++i )
        {
          std::unique_ptr< QgsPolygon > p( qgsgeometry_cast< QgsPolygon * >( ms->geometryN( i )->segmentize() ) );
          t.addPolygon( *p, 0 );
        }
      }
      else
      {
        std::unique_ptr< QgsPolygon > p( qgsgeometry_cast< QgsPolygon * >( f.geometry().constGet()->segmentize() ) );
        t.addPolygon( *p, 0 );
      }
      QgsGeometry g( t.asMultiPolygon() );
      g.translate( bounds.xMinimum(), bounds.yMinimum() );
      f.setGeometry( g );
    }
  }
  return f;
}

///@endcond


