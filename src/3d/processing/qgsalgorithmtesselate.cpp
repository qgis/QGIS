/***************************************************************************
                         qgsalgorithmtesselate.cpp
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

#include "qgsalgorithmtesselate.h"
#include "qgstessellator.h"
#include "qgsmultipolygon.h"
#include "qgstriangle.h"
///@cond PRIVATE

QString QgsTesselateAlgorithm::name() const
{
  return QStringLiteral( "tesselate" );
}

QString QgsTesselateAlgorithm::displayName() const
{
  return QObject::tr( "Tesselate geometry" );
}

QStringList QgsTesselateAlgorithm::tags() const
{
  return QObject::tr( "3d,triangle" ).split( ',' );
}

QString QgsTesselateAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsTesselateAlgorithm::outputName() const
{
  return QObject::tr( "Tesselated" );
}

QgsProcessing::SourceType QgsTesselateAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsWkbTypes::Type QgsTesselateAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  Q_UNUSED( inputWkbType );
  return QgsWkbTypes::MultiPolygonZ;
}

QString QgsTesselateAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm tesselates a polygon geometry layer, dividing the geometries into triangular components." )
         + QStringLiteral( "\n\n" )
         + QObject::tr( "The output layer consists of multipolygon geometries for each input feature, with each multipolygon consisting of multiple triangle component polygons." );
}

QList<int> QgsTesselateAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorPolygon;
}

QgsTesselateAlgorithm *QgsTesselateAlgorithm::createInstance() const
{
  return new QgsTesselateAlgorithm();
}

QgsPoint getPointFromData( QVector< float >::const_iterator &it )
{
  // tesselator geometry is x, z, -y
  double x = *it;
  ++it;
  double z = *it;
  ++it;
  double y = -( *it );
  ++it;
  return QgsPoint( x, y, z );
}

void tesselatePolygon( const QgsPolygon *polygon, QgsMultiPolygon *destination )
{
  QgsTessellator t( 0, 0, false );
  t.addPolygon( *polygon, 0 );

  QVector<float> data = t.data();
  for ( auto it = data.constBegin(); it != data.constEnd(); )
  {
    QgsPoint p1 = getPointFromData( it );
    QgsPoint p2 = getPointFromData( it );
    QgsPoint p3 = getPointFromData( it );
    destination->addGeometry( new QgsTriangle( p1, p2, p3 ) );
  }
}

QgsFeature QgsTesselateAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    if ( QgsWkbTypes::geometryType( f.geometry().wkbType() ) != QgsWkbTypes::PolygonGeometry )
      f.clearGeometry();
    else
    {
      std::unique_ptr< QgsMultiPolygon > mp = qgis::make_unique< QgsMultiPolygon >();
      if ( f.geometry().isMultipart() )
      {
        const QgsMultiSurface *ms = qgsgeometry_cast< const QgsMultiSurface * >( f.geometry().constGet() );
        for ( int i = 0; i < ms->numGeometries(); ++i )
        {
          std::unique_ptr< QgsPolygon > p( qgsgeometry_cast< QgsPolygon * >( ms->geometryN( i )->segmentize() ) );
          tesselatePolygon( p.get(), mp.get() );
        }
      }
      else
      {
        std::unique_ptr< QgsPolygon > p( qgsgeometry_cast< QgsPolygon * >( f.geometry().constGet()->segmentize() ) );
        tesselatePolygon( p.get(), mp.get() );
      }
      f.setGeometry( QgsGeometry( std::move( mp ) ) );
    }
  }
  return f;
}

///@endcond


