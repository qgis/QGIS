/***************************************************************************
                         qgsalgorithmclimb.cpp
                         ---------------------
    begin                : February 2025
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

#include "qgsalgorithmclimb.h"

///@cond PRIVATE

QString QgsClimbAlgorithm::name() const
{
  return u"climbalongline"_s;
}

QString QgsClimbAlgorithm::displayName() const
{
  return QObject::tr( "Climb along line" );
}

QStringList QgsClimbAlgorithm::tags() const
{
  return QObject::tr( "line,climb,descent,elevation" ).split( ',' );
}

QString QgsClimbAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsClimbAlgorithm::groupId() const
{
  return u"vectoranalysis"_s;
}

QString QgsClimbAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the total climb and descent along line geometries.\n\n"
                      "Input layer must have Z values present. If Z values are not available, the \"Drape\" (set Z "
                      "value from raster) algorithm may be used to add Z values from a DEM layer.\n\n"
                      "The output layer is a copy of the input layer with additional fields that contain the total "
                      "climb, total descent, the minimum elevation and the maximum elevation for each line geometry."
                      "If the input layer contains fields with the same names as these added fields, they will be "
                      "renamed (field names will be altered to \"name_2\", \"name_3\", etc, finding the first "
                      "non-duplicate name)." );
}

QString QgsClimbAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the total climb and descent along line geometries with Z values." );
}

QgsClimbAlgorithm *QgsClimbAlgorithm::createInstance() const
{
  return new QgsClimbAlgorithm();
}

void QgsClimbAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Line layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Climb layer" ) ) );
  // TODO QGIS 5.0 harmonize output names with the rest of algorithms (use underscore to separate words)
  addOutput( new QgsProcessingOutputNumber( u"TOTALCLIMB"_s, QObject::tr( "Total climb" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"TOTALDESCENT"_s, QObject::tr( "Total descent" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MINELEVATION"_s, QObject::tr( "Minimum elevation" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MAXELEVATION"_s, QObject::tr( "Maximum elevation" ) ) );
}

QVariantMap QgsClimbAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
  {
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );
  }

  if ( !QgsWkbTypes::hasZ( source->wkbType() ) )
  {
    throw QgsProcessingException( QObject::tr( "The layer does not have Z values. If you have a DEM, use the Drape algorithm to extract Z values." ) );
  }

  QgsFields outputFields = source->fields();
  QgsFields newFields;
  newFields.append( QgsField( u"climb"_s, QMetaType::Type::Double ) );
  newFields.append( QgsField( u"descent"_s, QMetaType::Type::Double ) );
  newFields.append( QgsField( u"minelev"_s, QMetaType::Type::Double ) );
  newFields.append( QgsField( u"maxelev"_s, QMetaType::Type::Double ) );
  outputFields = QgsProcessingUtils::combineFields( outputFields, newFields );

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, outputFields, source->wkbType(), source->sourceCrs() ) );
  if ( !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  double totalClimb = 0;
  double totalDescent = 0;
  double minElevation = std::numeric_limits<double>::max();
  double maxElevation = -std::numeric_limits<double>::max();

  QStringList noGeometry;
  QStringList noZValue;

  QgsFeatureIterator it = source->getFeatures();
  double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  int i = 0;

  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !f.hasGeometry() )
    {
      noGeometry.append( QObject::tr( "Feature: %1" ).arg( f.id() ) );
      continue;
    }

    double climb = 0;
    double descent = 0;
    double minElev = std::numeric_limits<double>::max();
    double maxElev = -std::numeric_limits<double>::max();
    // Handle multipart geometries
    const QgsGeometry g = f.geometry();
    int partNumber = 0;
    for ( auto partIt = g.const_parts_begin(); partIt != g.const_parts_end(); ++partIt, ++partNumber )
    {
      bool first = true;
      double z = 0;
      double previousZ = 0;
      const QgsAbstractGeometry *part = *partIt;
      int vertexNumber = 0;
      for ( auto it = part->vertices_begin(); it != part->vertices_end(); ++it, ++vertexNumber )
      {
        z = QgsPoint( *it ).z();
        if ( std::isnan( z ) )
        {
          noZValue.append( QObject::tr( "Feature: %1, part: %2, point: %3" ).arg( f.id(), partNumber, vertexNumber ) );
          continue;
        }
        if ( first )
        {
          previousZ = z;
          minElev = z;
          maxElev = z;
          first = false;
        }
        else
        {
          double diff = z - previousZ;
          if ( diff > 0 )
          {
            climb += diff;
          }
          else
          {
            descent -= diff;
          }
          minElev = std::min( minElev, z );
          maxElev = std::max( maxElev, z );
        }
        previousZ = z;
      }
      totalClimb += climb;
      totalDescent += descent;
    }

    QgsAttributes attrs = f.attributes();
    attrs << climb << descent << minElev << maxElev;
    f.setAttributes( attrs );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
    {
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
    minElevation = std::min( minElevation, minElev );
    maxElevation = std::max( maxElevation, maxElev );

    feedback->setProgress( i * step );
    i++;
  }

  sink->finalize();

  if ( !noGeometry.empty() )
  {
    feedback->pushInfo( QObject::tr( "The following features do not have geometry: %1" ).arg( noGeometry.join( ", "_L1 ) ) );
  }
  if ( !noZValue.empty() )
  {
    feedback->pushInfo( QObject::tr( "The following points do not have Z value: %1" ).arg( noZValue.join( ", "_L1 ) ) );
  }

  QVariantMap results;
  results.insert( u"OUTPUT"_s, dest );
  results.insert( u"TOTALCLIMB"_s, totalClimb );
  results.insert( u"TOTALDESCENT"_s, totalDescent );
  results.insert( u"MINELEVATION"_s, minElevation );
  results.insert( u"MAXELEVATION"_s, maxElevation );
  return results;
}

///@endcond
