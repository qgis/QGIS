/***************************************************************************
                         qgsalgorithmclip.cpp
                         ---------------------
    begin                : April 2017
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

#include "qgsalgorithmclip.h"
#include "qgsgeometryengine.h"

///@cond PRIVATE

QString QgsClipAlgorithm::name() const
{
  return QStringLiteral( "clip" );
}

QString QgsClipAlgorithm::displayName() const
{
  return QObject::tr( "Clip" );
}

QStringList QgsClipAlgorithm::tags() const
{
  return QObject::tr( "clip,intersect,intersection,mask" ).split( ',' );
}

QString QgsClipAlgorithm::group() const
{
  return QObject::tr( "Vector overlay" );
}

QgsProcessingAlgorithm::Flags QgsClipAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

QString QgsClipAlgorithm::groupId() const
{
  return QStringLiteral( "vectoroverlay" );
}

void QgsClipAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Clip layer" ), QList< int >() << QgsProcessing::TypeVectorPolygon ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Clipped" ) ) );
}

QString QgsClipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm clips a vector layer using the polygons of an additional polygons layer. Only the parts of the features "
                      "in the input layer that falls within the polygons of the clipping layer will be added to the resulting layer.\n\n"
                      "The attributes of the features are not modified, although properties such as area or length of the features will "
                      "be modified by the clipping operation. If such properties are stored as attributes, those attributes will have to "
                      "be manually updated." );
}

QgsClipAlgorithm *QgsClipAlgorithm::createInstance() const
{
  return new QgsClipAlgorithm();
}

QVariantMap QgsClipAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > featureSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !featureSource )
    return QVariantMap();

  std::unique_ptr< QgsFeatureSource > maskSource( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !maskSource )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, featureSource->fields(), QgsWkbTypes::multiType( featureSource->wkbType() ), featureSource->sourceCrs() ) );

  if ( !sink )
    return QVariantMap();

  // first build up a list of clip geometries
  QVector< QgsGeometry > clipGeoms;
  QgsFeatureIterator it = maskSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QList< int >() ).setDestinationCrs( featureSource->sourceCrs(), context.transformContext() ) );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( f.hasGeometry() )
      clipGeoms << f.geometry();
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );

  if ( clipGeoms.isEmpty() )
    return outputs;

  // are we clipping against a single feature? if so, we can show finer progress reports
  bool singleClipFeature = false;
  QgsGeometry combinedClipGeom;
  if ( clipGeoms.length() > 1 )
  {
    combinedClipGeom = QgsGeometry::unaryUnion( clipGeoms );
    singleClipFeature = false;
  }
  else
  {
    combinedClipGeom = clipGeoms.at( 0 );
    singleClipFeature = true;
  }

  // use prepared geometries for faster intersection tests
  std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( combinedClipGeom.constGet() ) );
  engine->prepareGeometry();

  QgsFeatureIds testedFeatureIds;

  int i = -1;
  Q_FOREACH ( const QgsGeometry &clipGeom, clipGeoms )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeatureIterator inputIt = featureSource->getFeatures( QgsFeatureRequest().setFilterRect( clipGeom.boundingBox() ) );
    QgsFeatureList inputFeatures;
    QgsFeature f;
    while ( inputIt.nextFeature( f ) )
      inputFeatures << f;

    if ( inputFeatures.isEmpty() )
      continue;

    double step = 0;
    if ( singleClipFeature )
      step = 100.0 / inputFeatures.length();

    int current = 0;
    Q_FOREACH ( const QgsFeature &inputFeature, inputFeatures )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( !inputFeature.hasGeometry() )
        continue;

      if ( testedFeatureIds.contains( inputFeature.id() ) )
      {
        // don't retest a feature we have already checked
        continue;
      }
      testedFeatureIds.insert( inputFeature.id() );

      if ( !engine->intersects( inputFeature.geometry().constGet() ) )
        continue;

      QgsGeometry newGeometry;
      if ( !engine->contains( inputFeature.geometry().constGet() ) )
      {
        QgsGeometry currentGeometry = inputFeature.geometry();
        newGeometry = combinedClipGeom.intersection( currentGeometry );
        if ( newGeometry.wkbType() == QgsWkbTypes::Unknown || QgsWkbTypes::flatType( newGeometry.wkbType() ) == QgsWkbTypes::GeometryCollection )
        {
          QgsGeometry intCom = inputFeature.geometry().combine( newGeometry );
          QgsGeometry intSym = inputFeature.geometry().symDifference( newGeometry );
          newGeometry = intCom.difference( intSym );
        }
      }
      else
      {
        // clip geometry totally contains feature geometry, so no need to perform intersection
        newGeometry = inputFeature.geometry();
      }

      QgsFeature outputFeature;
      outputFeature.setGeometry( newGeometry );
      outputFeature.setAttributes( inputFeature.attributes() );
      sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );


      if ( singleClipFeature )
        feedback->setProgress( current * step );
    }

    if ( !singleClipFeature )
    {
      // coarse progress report for multiple clip geometries
      feedback->setProgress( 100.0 * static_cast< double >( i ) / clipGeoms.length() );
    }
  }

  return outputs;
}

///@endcond
