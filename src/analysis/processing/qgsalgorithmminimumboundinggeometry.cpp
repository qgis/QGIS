/***************************************************************************
                         qgsalgorithmminimumboundinggeometry.cpp
                         ---------------------
    begin                : May 2025
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

#include "qgsalgorithmminimumboundinggeometry.h"

#include "qgsmultipoint.h"

///@cond PRIVATE

QString QgsMinimumBoundingGeometryAlgorithm::name() const
{
  return u"minimumboundinggeometry"_s;
}

QString QgsMinimumBoundingGeometryAlgorithm::displayName() const
{
  return QObject::tr( "Minimum bounding geometry" );
}

QStringList QgsMinimumBoundingGeometryAlgorithm::tags() const
{
  return QObject::tr( "bounding,box,bounds,envelope,minimum,oriented,rectangle,enclosing,circle,convex,hull,generalization" ).split( ',' );
}

QString QgsMinimumBoundingGeometryAlgorithm::group() const
{
  return QObject::tr( "Vector geometry" );
}

QString QgsMinimumBoundingGeometryAlgorithm::groupId() const
{
  return u"vectorgeometry"_s;
}

QString QgsMinimumBoundingGeometryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates geometries which enclose the features from an input layer.\n\n"
                      "Numerous enclosing geometry types are supported, including bounding "
                      "boxes (envelopes), oriented rectangles, circles and convex hulls.\n\n"
                      "Optionally, the features can be grouped by a field. If set, this "
                      "causes the output layer to contain one feature per grouped value with "
                      "a minimal geometry covering just the features with matching values." );
}

QString QgsMinimumBoundingGeometryAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates geometries which enclose the features from an input layer." );
}

QgsMinimumBoundingGeometryAlgorithm *QgsMinimumBoundingGeometryAlgorithm::createInstance() const
{
  return new QgsMinimumBoundingGeometryAlgorithm();
}

void QgsMinimumBoundingGeometryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorAnyGeometry ) ) );
  addParameter( new QgsProcessingParameterField( u"FIELD"_s, QObject::tr( "Field (optional, set if features should be grouped by class)" ), QVariant(), u"INPUT"_s, Qgis::ProcessingFieldParameterDataType::Any, false, true ) );

  QStringList geometryTypes = QStringList() << QObject::tr( "Envelope (Bounding Box)" )
                                            << QObject::tr( "Minimum Oriented Rectangle" )
                                            << QObject::tr( "Minimum Enclosing Circle" )
                                            << QObject::tr( "Convex Hull" );

  addParameter( new QgsProcessingParameterEnum( u"TYPE"_s, QObject::tr( "Geometry type" ), geometryTypes ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Bounding geometry" ), Qgis::ProcessingSourceType::VectorPolygon ) );
}

QVariantMap QgsMinimumBoundingGeometryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsProcessingFeatureSource> source( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !source )
  {
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );
  }

  const QString fieldName = parameterAsString( parameters, u"FIELD"_s, context );
  const int geometryType = parameterAsEnum( parameters, u"TYPE"_s, context );
  const bool useField = !fieldName.isEmpty();

  int fieldIndex = -1;

  QgsFields fields = QgsFields();
  fields.append( QgsField( u"id"_s, QMetaType::Type::Int, QString(), 20 ) );

  if ( useField )
  {
    // keep original field type, name and parameters
    fieldIndex = source->fields().lookupField( fieldName );
    if ( fieldIndex >= 0 )
    {
      fields.append( source->fields().at( fieldIndex ) );
    }
  }

  if ( geometryType == 0 )
  {
    // envelope
    fields.append( QgsField( u"width"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"height"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"perimeter"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  }
  else if ( geometryType == 1 )
  {
    // oriented rectangle
    fields.append( QgsField( u"width"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"height"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"angle"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"perimeter"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  }
  else if ( geometryType == 2 )
  {
    // circle
    fields.append( QgsField( u"radius"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  }
  else if ( geometryType == 3 )
  {
    // convex hull
    fields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
    fields.append( QgsField( u"perimeter"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  }

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, fields, Qgis::WkbType::Polygon, source->sourceCrs() ) );
  if ( !sink )
  {
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );
  }

  if ( fieldIndex >= 0 )
  {
    QHash<QVariant, QVector<QgsGeometry>> geometryHash;
    QHash<QVariant, QgsRectangle> boundsHash;

    double step = source->featureCount() > 0 ? 50 / source->featureCount() : 1;
    QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QList<int>() << fieldIndex ) );

    QgsFeature f;
    long long i = 0;
    while ( features.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( !f.hasGeometry() )
        continue;

      QVariant fieldValue = f.attribute( fieldIndex );
      if ( geometryType == 0 )
      {
        auto boundsHashIt = boundsHash.find( fieldValue );
        if ( boundsHashIt == boundsHash.end() )
        {
          boundsHash.insert( fieldValue, f.geometry().boundingBox() );
        }
        else
        {
          boundsHashIt.value().combineExtentWith( f.geometry().boundingBox() );
        }
      }
      else
      {
        auto geometryHashIt = geometryHash.find( fieldValue );
        if ( geometryHashIt == geometryHash.end() )
        {
          geometryHash.insert( fieldValue, QVector<QgsGeometry>() << f.geometry() );
        }
        else
        {
          geometryHashIt.value().append( f.geometry() );
        }
      }
      i++;
      feedback->setProgress( i * step );
    }

    // bounding boxes
    i = 0;
    if ( geometryType == 0 )
    {
      step = boundsHash.size() > 0 ? 50 / boundsHash.size() : 1;
      for ( auto it = boundsHash.constBegin(); it != boundsHash.constEnd(); ++it )
      {
        if ( feedback->isCanceled() )
          break;

        // envelope
        QgsFeature feature;
        QgsRectangle rect = it.value();
        feature.setGeometry( QgsGeometry::fromRect( rect ) );
        feature.setAttributes( QgsAttributes() << i << it.key() << rect.width() << rect.height() << rect.area() << rect.perimeter() );
        if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        i++;
        feedback->setProgress( 50 + i * step );
      }
    }
    else
    {
      step = geometryHash.size() > 0 ? 50 / geometryHash.size() : 1;
      for ( auto it = geometryHash.constBegin(); it != geometryHash.constEnd(); ++it )
      {
        if ( feedback->isCanceled() )
          break;

        // envelope
        QgsFeature feature = createFeature( feedback, i, geometryType, it.value(), it.key() );
        if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        i++;
        feedback->setProgress( 50 + i * step );
      }
    }
  }
  else
  {
    double step = source->featureCount() > 0 ? 80 / source->featureCount() : 1;
    QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setNoAttributes() );

    QVector<QgsGeometry> geometryQueue;
    geometryQueue.reserve( source->featureCount() );
    QgsRectangle bounds;

    QgsFeature f;
    long long i = 0;
    while ( features.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( !f.hasGeometry() )
        continue;

      if ( geometryType == 0 )
      {
        // bounding boxes, calculate on the fly for efficiency
        bounds.combineExtentWith( f.geometry().boundingBox() );
      }
      else
      {
        geometryQueue << f.geometry();
      }
      i++;
      feedback->setProgress( i * step );
    }

    if ( !feedback->isCanceled() )
    {
      QgsFeature feature;
      if ( geometryType == 0 )
      {
        feature.setGeometry( QgsGeometry::fromRect( bounds ) );
        feature.setAttributes( QgsAttributes() << 0 << bounds.width() << bounds.height() << bounds.area() << bounds.perimeter() );
      }
      else
      {
        feature = createFeature( feedback, 0, geometryType, geometryQueue );
      }

      if ( !sink->addFeature( feature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
  }
  sink->finalize();

  QVariantMap results;
  results.insert( u"OUTPUT"_s, dest );
  return results;
}

QgsFeature QgsMinimumBoundingGeometryAlgorithm::createFeature( QgsProcessingFeedback *feedback, const int featureId, const int featureType, QVector<QgsGeometry> geometries, QVariant classField )
{
  QgsAttributes attrs;
  attrs << featureId;
  if ( classField.isValid() )
  {
    attrs << classField;
  }

  auto multiPoint = std::make_unique<QgsMultiPoint>();

  for ( auto &g : geometries )
  {
    if ( feedback->isCanceled() )
      break;

    for ( auto it = g.constGet()->vertices_begin(); it != g.constGet()->vertices_end(); ++it )
    {
      if ( feedback->isCanceled() )
        break;

      multiPoint->addGeometry( ( *it ).clone() );
    }
  }

  QgsGeometry geometry( std::move( multiPoint ) );
  QgsGeometry outputGeometry;
  if ( featureType == 0 )
  {
    // envelope
    QgsRectangle rect = geometry.boundingBox();
    outputGeometry = QgsGeometry::fromRect( rect );
    attrs << rect.width() << rect.height() << rect.area() << rect.perimeter();
  }
  else if ( featureType == 1 )
  {
    // oriented rect
    double area, angle, width, height;
    outputGeometry = geometry.orientedMinimumBoundingBox( area, angle, width, height );
    attrs << width << height << angle << area << 2 * width + 2 * height;
  }
  else if ( featureType == 2 )
  {
    // circle
    QgsPointXY center;
    double radius;
    outputGeometry = geometry.minimalEnclosingCircle( center, radius, 72 );
    attrs << radius << M_PI * radius * radius;
  }
  else if ( featureType == 3 )
  {
    // convex hull
    outputGeometry = geometry.convexHull();
    attrs << outputGeometry.constGet()->area() << outputGeometry.constGet()->perimeter();
  }

  QgsFeature f;
  f.setGeometry( outputGeometry );
  f.setAttributes( attrs );
  return f;
}

///@endcond
