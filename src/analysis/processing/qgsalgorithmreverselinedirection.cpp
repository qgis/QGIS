/***************************************************************************
                         qgsalgorithmreverselinedirection.cpp
                         ------------------------------------
    begin                : July 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmreverselinedirection.h"
#include "qgscurve.h"
#include "qgsgeometrycollection.h"

///@cond PRIVATE

QString QgsReverseLineDirectionAlgorithm ::name() const
{
  return QStringLiteral( "reverselinedirection" );
}

QString QgsReverseLineDirectionAlgorithm ::displayName() const
{
  return QObject::tr( "Reverse line direction" );
}

QStringList QgsReverseLineDirectionAlgorithm ::tags() const
{
  return QObject::tr( "swap,reverse,switch,flip,linestring,orientation" ).split( ',' );
}

QString QgsReverseLineDirectionAlgorithm ::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsReverseLineDirectionAlgorithm ::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsReverseLineDirectionAlgorithm ::outputName() const
{
  return QObject::tr( "Reversed" );
}

QString QgsReverseLineDirectionAlgorithm ::shortHelpString() const
{
  return QObject::tr( "This algorithm reverses the direction of curve or LineString geometries." );
}

QString QgsReverseLineDirectionAlgorithm::shortDescription() const
{
  return QObject::tr( "Reverses the direction of curve or LineString geometries." );
}

QgsReverseLineDirectionAlgorithm  *QgsReverseLineDirectionAlgorithm ::createInstance() const
{
  return new QgsReverseLineDirectionAlgorithm();
}

QgsProcessing::SourceType QgsReverseLineDirectionAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorLine;
}

QList<int> QgsReverseLineDirectionAlgorithm::inputLayerTypes() const
{
  return QList<int>() << QgsProcessing::TypeVectorLine;
}

QgsProcessingFeatureSource::Flag QgsReverseLineDirectionAlgorithm ::sourceFlags() const
{
  // this algorithm doesn't care about invalid geometries
  return QgsProcessingFeatureSource::FlagSkipGeometryValidityChecks;
}

QgsFeatureList QgsReverseLineDirectionAlgorithm ::processFeature( const QgsFeature &f, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QgsFeature feature = f;
  if ( feature.hasGeometry() )
  {
    const QgsGeometry geom = feature.geometry();
    if ( !geom.isMultipart() )
    {
      const QgsCurve *curve = qgsgeometry_cast< const QgsCurve * >( geom.constGet() );
      if ( curve )
      {
        std::unique_ptr< QgsCurve > reversed( curve->reversed() );
        if ( !reversed )
        {
          // can this even happen?
          throw QgsProcessingException( QObject::tr( "Error reversing line" ) );
        }
        const QgsGeometry outGeom( std::move( reversed ) );
        feature.setGeometry( outGeom );
      }
    }
    else
    {
      std::unique_ptr< QgsAbstractGeometry > dest( geom.constGet()->createEmptyWithSameType() );
      const QgsGeometryCollection *collection = qgsgeometry_cast< const QgsGeometryCollection * >( geom.constGet() );
      QgsGeometryCollection *destCollection = qgsgeometry_cast< QgsGeometryCollection * >( dest.get() );
      destCollection->reserve( collection->numGeometries() );
      for ( int i = 0; i < collection->numGeometries(); ++i )
      {
        const QgsCurve *curve = qgsgeometry_cast< const QgsCurve *>( collection->geometryN( i ) );
        if ( curve )
        {
          std::unique_ptr< QgsCurve > reversed( curve->reversed() );
          if ( !reversed )
          {
            // can this even happen?
            throw QgsProcessingException( QObject::tr( "Error reversing line" ) );
          }
          destCollection->addGeometry( reversed.release() );
        }
      }
      const QgsGeometry outGeom( std::move( dest ) );
      feature.setGeometry( outGeom );
    }
  }
  return QgsFeatureList() << feature;
}

///@endcond
