/***************************************************************************
                         qgsalgorithmconvertgeometrytype.cpp
                         ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmconvertgeometrytype.h"

///@cond PRIVATE

QString QgsConvertGeometryTypeAlgorithm::name() const
{
  return QStringLiteral( "convertgeometrytype" );
}

QString QgsConvertGeometryTypeAlgorithm::displayName() const
{
  return QObject::tr( "Convert geometry type" );
}

QStringList QgsConvertGeometryTypeAlgorithm::tags() const
{
  return QObject::tr( "polygon,line,point,centroids,nodes,convert,type,geometry" ).split( ',' );
}

QString QgsConvertGeometryTypeAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsConvertGeometryTypeAlgorithm::groupId() const
{
  return QStringLiteral( "vectorgeometry" );
}

QString QgsConvertGeometryTypeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm generates a new layer based on an existing one, with a different type of geometry.\n\n"
                      "Not all conversions are possible. For instance, a line layer can be converted to a "
                      "point layer, but a point layer cannot be converted to a line layer.\n\n"
                      "See the \"Polygonize\" or \"Lines to polygons\" algorithms for alternative options." );
}

QString QgsConvertGeometryTypeAlgorithm::shortDescription() const
{
  return QObject::tr( "Converts the geometries from a vector layer to a different geometry type." );
}

QgsConvertGeometryTypeAlgorithm *QgsConvertGeometryTypeAlgorithm::createInstance() const
{
  return new QgsConvertGeometryTypeAlgorithm();
}

void QgsConvertGeometryTypeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );

  QStringList geometryTypes = QStringList() << QObject::tr( "Centroids" )
                                            << QObject::tr( "Nodes" )
                                            << QObject::tr( "Linestrings" )
                                            << QObject::tr( "Multilinestrings" )
                                            << QObject::tr( "Polygons" );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "TYPE" ), QObject::tr( "New geometry type" ), geometryTypes ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Converted" ) ) );
}

QVariantMap QgsConvertGeometryTypeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
  {
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );
  }

  const int typeIndex = parameterAsEnum( parameters, QStringLiteral( "TYPE" ), context );
  Qgis::WkbType outputWkbType;

  if ( typeIndex == 0 ) // centroids
  {
    outputWkbType = Qgis::WkbType::Point;
  }
  else if ( typeIndex == 1 ) // nodes
  {
    outputWkbType = Qgis::WkbType::MultiPoint;
  }
  else if ( typeIndex == 2 ) // LineStrings
  {
    outputWkbType = Qgis::WkbType::LineString;
  }
  else if ( typeIndex == 3 ) // MultiLineStrings
  {
    outputWkbType = Qgis::WkbType::MultiLineString;
  }
  else if ( typeIndex == 4 ) // polygons
  {
    outputWkbType = Qgis::WkbType::Polygon;
  }

  // preserve Z/M values
  if ( QgsWkbTypes::hasM( source->wkbType() ) )
  {
    outputWkbType = QgsWkbTypes::addM( outputWkbType );
  }
  if ( QgsWkbTypes::hasZ( source->wkbType() ) )
  {
    outputWkbType = QgsWkbTypes::addZ( outputWkbType );
  }

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(), outputWkbType, source->sourceCrs() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );
  }

  QgsFeatureIterator features = source->getFeatures();
  const double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 0;

  QgsFeature f;
  long long i = 0;
  while ( features.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !f.hasGeometry() )
    {
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    }
    else
    {
      const QVector< QgsGeometry > geometries = convertGeometry( f.geometry(), typeIndex, outputWkbType );
      for ( const QgsGeometry &g : geometries )
      {
        QgsFeature feat;
        feat.setGeometry( g );
        feat.setAttributes( f.attributes() );
        if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
      }
    }

    i++;
    feedback->setProgress( i * step );
  }

  sink->finalize();

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), dest );
  return results;
}

const QVector< QgsGeometry > QgsConvertGeometryTypeAlgorithm::convertGeometry( const QgsGeometry &geom, const int typeIndex, const Qgis::WkbType outputWkbType )
{
  QVector< QgsGeometry > geometries;
  if ( typeIndex == 0 )
  {
    geometries << geom.centroid();
  }
  else
  {
    geometries = geom.coerceToType( outputWkbType, 0, 0, false );
  }

  return geometries;
}

///@endcond
