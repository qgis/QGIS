/***************************************************************************
                         qgsnativealgorithms.cpp
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

#include "qgsnativealgorithms.h"
#include "qgsfeatureiterator.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingutils.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"
#include "qgswkbtypes.h"

#include <functional>

///@cond PRIVATE

QgsNativeAlgorithms::QgsNativeAlgorithms( QObject *parent )
  : QgsProcessingProvider( parent )
{}

QIcon QgsNativeAlgorithms::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/providerQgis.svg" ) );
}

QString QgsNativeAlgorithms::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "providerQgis.svg" ) );
}

QString QgsNativeAlgorithms::id() const
{
  return QStringLiteral( "native" );
}

QString QgsNativeAlgorithms::name() const
{
  return tr( "QGIS (native c++)" );
}

bool QgsNativeAlgorithms::supportsNonFileBasedOutput() const
{
  return true;
}

void QgsNativeAlgorithms::loadAlgorithms()
{
  addAlgorithm( new QgsBufferAlgorithm() );
  addAlgorithm( new QgsCentroidAlgorithm() );
  addAlgorithm( new QgsClipAlgorithm() );
  addAlgorithm( new QgsDissolveAlgorithm() );
  addAlgorithm( new QgsCollectAlgorithm() );
  addAlgorithm( new QgsExtractByAttributeAlgorithm() );
  addAlgorithm( new QgsExtractByExpressionAlgorithm() );
  addAlgorithm( new QgsMultipartToSinglepartAlgorithm() );
  addAlgorithm( new QgsSubdivideAlgorithm() );
  addAlgorithm( new QgsTransformAlgorithm() );
  addAlgorithm( new QgsRemoveNullGeometryAlgorithm() );
  addAlgorithm( new QgsBoundingBoxAlgorithm() );
  addAlgorithm( new QgsOrientedMinimumBoundingBoxAlgorithm() );
  addAlgorithm( new QgsMinimumEnclosingCircleAlgorithm() );
  addAlgorithm( new QgsConvexHullAlgorithm() );
  addAlgorithm( new QgsPromoteToMultipartAlgorithm() );
  addAlgorithm( new QgsSelectByLocationAlgorithm() );
  addAlgorithm( new QgsExtractByLocationAlgorithm() );
  addAlgorithm( new QgsFixGeometriesAlgorithm() );
  addAlgorithm( new QgsMergeLinesAlgorithm() );
  addAlgorithm( new QgsSaveSelectedFeatures() );
  addAlgorithm( new QgsSmoothAlgorithm() );
  addAlgorithm( new QgsSimplifyAlgorithm() );
  addAlgorithm( new QgsExtractByExtentAlgorithm() );
  addAlgorithm( new QgsExtentToLayerAlgorithm() );
  addAlgorithm( new QgsLineIntersectionAlgorithm() );
  addAlgorithm( new QgsSplitWithLinesAlgorithm() );
  addAlgorithm( new QgsMeanCoordinatesAlgorithm() );
  addAlgorithm( new QgsRasterLayerUniqueValuesReportAlgorithm() );
  addAlgorithm( new QgsJoinByAttributeAlgorithm() );
  addAlgorithm( new QgsJoinWithLinesAlgorithm() );
  addAlgorithm( new QgsAssignProjectionAlgorithm() );
}

void QgsSaveSelectedFeatures::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Selected features" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsSaveSelectedFeatures::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new layer with all the selected features in a given vector layer.\n\n"
                      "If the selected layer has no selected features, the newly created layer will be empty." );
}

QgsSaveSelectedFeatures *QgsSaveSelectedFeatures::createInstance() const
{
  return new QgsSaveSelectedFeatures();
}

QVariantMap QgsSaveSelectedFeatures::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *selectLayer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, selectLayer->fields(), selectLayer->wkbType(), selectLayer->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();


  int count = selectLayer->selectedFeatureCount();
  int current = 0;
  double step = count > 0 ? 100.0 / count : 1;

  QgsFeatureIterator it = selectLayer->getSelectedFeatures();;
  QgsFeature feat;
  while ( it.nextFeature( feat ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    sink->addFeature( feat, QgsFeatureSink::FastInsert );

    feedback->setProgress( current++ * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

void QgsCentroidAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Centroids" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsCentroidAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new point layer, with points representing the centroid of the geometries in an input layer.\n\n"
                      "The attributes associated to each point in the output layer are the same ones associated to the original features." );
}

QgsCentroidAlgorithm *QgsCentroidAlgorithm::createInstance() const
{
  return new QgsCentroidAlgorithm();
}

QgsFeature QgsCentroidAlgorithm::processFeature( const QgsFeature &f, QgsProcessingFeedback *feedback )
{
  QgsFeature feature = f;
  if ( feature.hasGeometry() )
  {
    feature.setGeometry( feature.geometry().centroid() );
    if ( !feature.geometry() )
    {
      feedback->pushInfo( QObject::tr( "Error calculating centroid for feature %1" ).arg( feature.id() ) );
    }
  }
  return feature;
}
//
// QgsBufferAlgorithm
//

void QgsBufferAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), QgsProcessingParameterNumber::Double, 10 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer, 5, false, 1 ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "END_CAP_STYLE" ), QObject::tr( "End cap style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Flat" ) << QObject::tr( "Square" ), false ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "JOIN_STYLE" ), QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MITER_LIMIT" ), QObject::tr( "Miter limit" ), QgsProcessingParameterNumber::Double, 2, false, 1 ) );

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DISSOLVE" ), QObject::tr( "Dissolve result" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Buffered" ), QgsProcessing::TypeVectorPolygon ) );
}

QString QgsBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a buffer area for all the features in an input layer, using a fixed or dynamic distance.\n\n"
                      "The segments parameter controls the number of line segments to use to approximate a quarter circle when creating rounded offsets.\n\n"
                      "The end cap style parameter controls how line endings are handled in the buffer.\n\n"
                      "The join style parameter specifies whether round, miter or beveled joins should be used when offsetting corners in a line.\n\n"
                      "The miter limit parameter is only applicable for miter join styles, and controls the maximum distance from the offset curve to use when creating a mitered join." );
}

QgsBufferAlgorithm *QgsBufferAlgorithm::createInstance() const
{
  return new QgsBufferAlgorithm();
}

QVariantMap QgsBufferAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(), QgsWkbTypes::Polygon, source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  // fixed parameters
  bool dissolve = parameterAsBool( parameters, QStringLiteral( "DISSOLVE" ), context );
  int segments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  QgsGeometry::EndCapStyle endCapStyle = static_cast< QgsGeometry::EndCapStyle >( 1 + parameterAsInt( parameters, QStringLiteral( "END_CAP_STYLE" ), context ) );
  QgsGeometry::JoinStyle joinStyle = static_cast< QgsGeometry::JoinStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  double miterLimit = parameterAsDouble( parameters, QStringLiteral( "MITER_LIMIT" ), context );
  double bufferDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  bool dynamicBuffer = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  const QgsProcessingParameterDefinition *distanceParamDef = parameterDefinition( QStringLiteral( "DISTANCE" ) );

  long count = source->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  QList< QgsGeometry > bufferedGeometriesForDissolve;
  QgsAttributes dissolveAttrs;

  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }
    if ( dissolveAttrs.isEmpty() )
      dissolveAttrs = f.attributes();

    QgsFeature out = f;
    if ( out.hasGeometry() )
    {
      if ( dynamicBuffer )
      {
        context.expressionContext().setFeature( f );
        bufferDistance = QgsProcessingParameters::parameterAsDouble( distanceParamDef, parameters, context );
      }

      QgsGeometry outputGeometry = f.geometry().buffer( bufferDistance, segments, endCapStyle, joinStyle, miterLimit );
      if ( !outputGeometry )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating buffer for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), QgsMessageLog::WARNING );
      }
      if ( dissolve )
        bufferedGeometriesForDissolve << outputGeometry;
      else
        out.setGeometry( outputGeometry );
    }

    if ( !dissolve )
      sink->addFeature( out, QgsFeatureSink::FastInsert );

    feedback->setProgress( current * step );
    current++;
  }

  if ( dissolve )
  {
    QgsGeometry finalGeometry = QgsGeometry::unaryUnion( bufferedGeometriesForDissolve );
    QgsFeature f;
    f.setGeometry( finalGeometry );
    f.setAttributes( dissolveAttrs );
    sink->addFeature( f, QgsFeatureSink::FastInsert );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


void QgsDissolveAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Unique ID fields" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Dissolved" ) ) );
}

QString QgsDissolveAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon or line vector layer and combines their geometries into new geometries. One or more attributes can "
                      "be specified to dissolve only geometries belonging to the same class (having the same value for the specified attributes), alternatively "
                      "all geometries can be dissolved.\n\n"
                      "All output geometries will be converted to multi geometries. "
                      "In case the input is a polygon layer, common boundaries of adjacent polygons being dissolved will get erased." );
}

QgsDissolveAlgorithm *QgsDissolveAlgorithm::createInstance() const
{
  return new QgsDissolveAlgorithm();
}

QVariantMap QgsCollectorAlgorithm::processCollection( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback,
    const std::function<QgsGeometry( const QList< QgsGeometry >& )> &collector, int maxQueueLength )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(), QgsWkbTypes::multiType( source->wkbType() ), source->sourceCrs() ) );

  if ( !sink )
    return QVariantMap();

  QStringList fields = parameterAsFields( parameters, QStringLiteral( "FIELD" ), context );

  long count = source->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  if ( fields.isEmpty() )
  {
    // dissolve all - not using fields
    bool firstFeature = true;
    // we dissolve geometries in blocks using unaryUnion
    QList< QgsGeometry > geomQueue;
    QgsFeature outputFeature;

    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( firstFeature )
      {
        outputFeature = f;
        firstFeature = false;
      }

      if ( f.hasGeometry() && f.geometry() )
      {
        geomQueue.append( f.geometry() );
        if ( maxQueueLength > 0 && geomQueue.length() > maxQueueLength )
        {
          // queue too long, combine it
          QgsGeometry tempOutputGeometry = collector( geomQueue );
          geomQueue.clear();
          geomQueue << tempOutputGeometry;
        }
      }

      feedback->setProgress( current * step );
      current++;
    }

    outputFeature.setGeometry( collector( geomQueue ) );
    sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
  }
  else
  {
    QList< int > fieldIndexes;
    Q_FOREACH ( const QString &field, fields )
    {
      int index = source->fields().lookupField( field );
      if ( index >= 0 )
        fieldIndexes << index;
    }

    QHash< QVariant, QgsAttributes > attributeHash;
    QHash< QVariant, QList< QgsGeometry > > geometryHash;

    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QVariantList indexAttributes;
      Q_FOREACH ( int index, fieldIndexes )
      {
        indexAttributes << f.attribute( index );
      }

      if ( !attributeHash.contains( indexAttributes ) )
      {
        // keep attributes of first feature
        attributeHash.insert( indexAttributes, f.attributes() );
      }

      if ( f.hasGeometry() && f.geometry() )
      {
        geometryHash[ indexAttributes ].append( f.geometry() );
      }
    }

    int numberFeatures = attributeHash.count();
    QHash< QVariant, QgsAttributes >::const_iterator attrIt = attributeHash.constBegin();
    for ( ; attrIt != attributeHash.constEnd(); ++attrIt )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QgsFeature outputFeature;
      if ( geometryHash.contains( attrIt.key() ) )
      {
        QgsGeometry geom = collector( geometryHash.value( attrIt.key() ) );
        if ( !geom.isMultipart() )
        {
          geom.convertToMultiType();
        }
        outputFeature.setGeometry( geom );
      }
      outputFeature.setAttributes( attrIt.value() );
      sink->addFeature( outputFeature, QgsFeatureSink::FastInsert );

      feedback->setProgress( current * 100.0 / numberFeatures );
      current++;
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

QVariantMap QgsDissolveAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return processCollection( parameters, context, feedback, []( const QList< QgsGeometry > &parts )->QgsGeometry
  {
    return QgsGeometry::unaryUnion( parts );
  }, 10000 );
}

QVariantMap QgsCollectAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  return processCollection( parameters, context, feedback, []( const QList< QgsGeometry > &parts )->QgsGeometry
  {
    return QgsGeometry::collectGeometry( parts );
  } );
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
  QList< QgsGeometry > clipGeoms;
  QgsFeatureIterator it = maskSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QList< int >() ).setDestinationCrs( featureSource->sourceCrs() ) );
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
  std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( combinedClipGeom.geometry() ) );
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

      if ( !engine->intersects( inputFeature.geometry().geometry() ) )
        continue;

      QgsGeometry newGeometry;
      if ( !engine->contains( inputFeature.geometry().geometry() ) )
      {
        QgsGeometry currentGeometry = inputFeature.geometry();
        newGeometry = combinedClipGeom.intersection( currentGeometry );
        if ( newGeometry.wkbType() == QgsWkbTypes::Unknown || QgsWkbTypes::flatType( newGeometry.geometry()->wkbType() ) == QgsWkbTypes::GeometryCollection )
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


void QgsTransformAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "EPSG:4326" ) ) );
}

QString QgsTransformAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reprojects a vector layer. It creates a new layer with the same features "
                      "as the input one, but with geometries reprojected to a new CRS.\n\n"
                      "Attributes are not modified by this algorithm." );
}

QgsTransformAlgorithm *QgsTransformAlgorithm::createInstance() const
{
  return new QgsTransformAlgorithm();
}

bool QgsTransformAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDestCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );
  return true;
}

QgsFeature QgsTransformAlgorithm::processFeature( const QgsFeature &f, QgsProcessingFeedback * )
{
  QgsFeature feature = f;
  if ( !mCreatedTransform )
  {
    mCreatedTransform = true;
    mTransform = QgsCoordinateTransform( sourceCrs(), mDestCrs );
  }

  if ( feature.hasGeometry() )
  {
    QgsGeometry g = feature.geometry();
    if ( g.transform( mTransform ) == 0 )
    {
      feature.setGeometry( g );
    }
    else
    {
      feature.clearGeometry();
    }
  }
  return feature;
}


QString QgsAssignProjectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm assigns a new projection to a vector layer. It creates a new layer with the exact same features "
                      "and geometries as the input one, but assigned to a new CRS. E.g. the geometries are not reprojected, they are just assigned "
                      "to a different CRS. This algorithm can be used to repair layers which have been assigned an incorrect projection.\n\n"
                      "Attributes are not modified by this algorithm." );
}

QgsAssignProjectionAlgorithm *QgsAssignProjectionAlgorithm::createInstance() const
{
  return new QgsAssignProjectionAlgorithm();
}

void QgsAssignProjectionAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "Assigned CRS" ), QStringLiteral( "EPSG:4326" ) ) );
}

bool QgsAssignProjectionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mDestCrs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  return true;
}

QgsFeature QgsAssignProjectionAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback * )
{
  return feature;
}



void QgsSubdivideAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_NODES" ), QObject::tr( "Maximum nodes in parts" ), QgsProcessingParameterNumber::Integer,
                256, false, 8, 100000 ) );
}

QString QgsSubdivideAlgorithm::shortHelpString() const
{
  return QObject::tr( "Subdivides the geometry. The returned geometry will be a collection containing subdivided parts "
                      "from the original geometry, where no part has more then the specified maximum number of nodes.\n\n"
                      "This is useful for dividing a complex geometry into less complex parts, which are better able to be spatially "
                      "indexed and faster to perform further operations such as intersects on. The returned geometry parts may "
                      "not be valid and may contain self-intersections.\n\n"
                      "Curved geometries will be segmentized before subdivision." );
}

QgsSubdivideAlgorithm *QgsSubdivideAlgorithm::createInstance() const
{
  return new QgsSubdivideAlgorithm();
}

QgsWkbTypes::Type QgsSubdivideAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return QgsWkbTypes::multiType( inputWkbType );
}

QgsFeature QgsSubdivideAlgorithm::processFeature( const QgsFeature &f, QgsProcessingFeedback *feedback )
{
  QgsFeature feature = f;
  if ( feature.hasGeometry() )
  {
    feature.setGeometry( feature.geometry().subdivide( mMaxNodes ) );
    if ( !feature.geometry() )
    {
      feedback->reportError( QObject::tr( "Error calculating subdivision for feature %1" ).arg( feature.id() ) );
    }
  }
  return feature;
}

bool QgsSubdivideAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mMaxNodes = parameterAsInt( parameters, QStringLiteral( "MAX_NODES" ), context );
  return true;
}


void QgsMultipartToSinglepartAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Single parts" ) ) );
}

QString QgsMultipartToSinglepartAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer with multipart geometries and generates a new one in which all geometries contain "
                      "a single part. Features with multipart geometries are divided in as many different features as parts the geometry "
                      "contain, and the same attributes are used for each of them." );
}

QgsMultipartToSinglepartAlgorithm *QgsMultipartToSinglepartAlgorithm::createInstance() const
{
  return new QgsMultipartToSinglepartAlgorithm();
}

QVariantMap QgsMultipartToSinglepartAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QgsWkbTypes::Type sinkType = QgsWkbTypes::singleType( source->wkbType() );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(),
                                          sinkType, source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  long count = source->featureCount();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    QgsFeature out = f;
    if ( out.hasGeometry() )
    {
      QgsGeometry inputGeometry = f.geometry();
      if ( inputGeometry.isMultipart() )
      {
        Q_FOREACH ( const QgsGeometry &g, inputGeometry.asGeometryCollection() )
        {
          out.setGeometry( g );
          sink->addFeature( out, QgsFeatureSink::FastInsert );
        }
      }
      else
      {
        sink->addFeature( out, QgsFeatureSink::FastInsert );
      }
    }
    else
    {
      // feature with null geometry
      sink->addFeature( out, QgsFeatureSink::FastInsert );
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


void QgsExtractByExpressionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Expression" ), QVariant(), QStringLiteral( "INPUT" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Matching features" ) ) );
  QgsProcessingParameterFeatureSink *failOutput = new QgsProcessingParameterFeatureSink( QStringLiteral( "FAIL_OUTPUT" ),  QObject::tr( "Non-matching" ),
      QgsProcessing::TypeVectorAnyGeometry, QVariant(), true );
  failOutput->setCreateByDefault( false );
  addParameter( failOutput );
}

QString QgsExtractByExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an input layer. "
                      "The criteria for adding features to the resulting layer is based on a QGIS expression.\n\n"
                      "For more information about expressions see the <a href =\"{qgisdocs}/user_manual/working_with_vector/expression.html\">user manual</a>" );
}

QgsExtractByExpressionAlgorithm *QgsExtractByExpressionAlgorithm::createInstance() const
{
  return new QgsExtractByExpressionAlgorithm();
}

QVariantMap QgsExtractByExpressionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString expressionString = parameterAsExpression( parameters, QStringLiteral( "EXPRESSION" ), context );

  QString matchingSinkId;
  std::unique_ptr< QgsFeatureSink > matchingSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, matchingSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );
  if ( !matchingSink )
    return QVariantMap();

  QString nonMatchingSinkId;
  std::unique_ptr< QgsFeatureSink > nonMatchingSink( parameterAsSink( parameters, QStringLiteral( "FAIL_OUTPUT" ), context, nonMatchingSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );

  QgsExpression expression( expressionString );
  if ( expression.hasParserError() )
  {
    throw QgsProcessingException( expression.parserErrorString() );
  }

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context );

  long count = source->featureCount();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  if ( !nonMatchingSink )
  {
    // not saving failing features - so only fetch good features
    QgsFeatureRequest req;
    req.setFilterExpression( expressionString );
    req.setExpressionContext( expressionContext );

    QgsFeatureIterator it = source->getFeatures( req );
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      matchingSink->addFeature( f, QgsFeatureSink::FastInsert );

      feedback->setProgress( current * step );
      current++;
    }
  }
  else
  {
    // saving non-matching features, so we need EVERYTHING
    expressionContext.setFields( source->fields() );
    expression.prepare( &expressionContext );

    QgsFeatureIterator it = source->getFeatures();
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      expressionContext.setFeature( f );
      if ( expression.evaluate( &expressionContext ).toBool() )
      {
        matchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }
      else
      {
        nonMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }

      feedback->setProgress( current * step );
      current++;
    }
  }


  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), matchingSinkId );
  if ( nonMatchingSink )
    outputs.insert( QStringLiteral( "FAIL_OUTPUT" ), nonMatchingSinkId );
  return outputs;
}


void QgsExtractByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Selection attribute" ), QVariant(), QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "OPERATOR" ), QObject::tr( "Operator" ), QStringList()
                << QObject::tr( "=" )
                << QObject::trUtf8( "≠" )
                << QObject::tr( ">" )
                << QObject::tr( ">=" )
                << QObject::tr( "<" )
                << QObject::tr( "<=" )
                << QObject::tr( "begins with" )
                << QObject::tr( "contains" )
                << QObject::tr( "is null" )
                << QObject::tr( "is not null" )
                << QObject::tr( "does not contain" ) ) );
  addParameter( new QgsProcessingParameterString( QStringLiteral( "VALUE" ), QObject::tr( "Value" ), QVariant(), false, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted (attribute)" ) ) );
  QgsProcessingParameterFeatureSink *failOutput = new QgsProcessingParameterFeatureSink( QStringLiteral( "FAIL_OUTPUT" ),  QObject::tr( "Extracted (non-matching)" ),
      QgsProcessing::TypeVectorAnyGeometry, QVariant(), true );
  failOutput->setCreateByDefault( false );
  addParameter( failOutput );
}

QString QgsExtractByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "  This algorithm creates a new vector layer that only contains matching features from an input layer. "
                      "The criteria for adding features to the resulting layer is defined based on the values "
                      "of an attribute from the input layer." );
}

QgsExtractByAttributeAlgorithm *QgsExtractByAttributeAlgorithm::createInstance() const
{
  return new QgsExtractByAttributeAlgorithm();
}

QVariantMap QgsExtractByAttributeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  Operation op = static_cast< Operation >( parameterAsEnum( parameters, QStringLiteral( "OPERATOR" ), context ) );
  QString value = parameterAsString( parameters, QStringLiteral( "VALUE" ), context );

  QString matchingSinkId;
  std::unique_ptr< QgsFeatureSink > matchingSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, matchingSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );
  if ( !matchingSink )
    return QVariantMap();

  QString nonMatchingSinkId;
  std::unique_ptr< QgsFeatureSink > nonMatchingSink( parameterAsSink( parameters, QStringLiteral( "FAIL_OUTPUT" ), context, nonMatchingSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );


  int idx = source->fields().lookupField( fieldName );
  QVariant::Type fieldType = source->fields().at( idx ).type();

  if ( fieldType != QVariant::String && ( op == BeginsWith || op == Contains || op == DoesNotContain ) )
  {
    QString method;
    switch ( op )
    {
      case BeginsWith:
        method = QObject::tr( "begins with" );
        break;
      case Contains:
        method = QObject::tr( "contains" );
        break;
      case DoesNotContain:
        method = QObject::tr( "does not contain" );
        break;

      default:
        break;
    }

    throw QgsProcessingException( QObject::tr( "Operator '%1' can be used only with string fields." ).arg( method ) );
  }

  QString fieldRef = QgsExpression::quotedColumnRef( fieldName );
  QString quotedVal = QgsExpression::quotedValue( value );
  QString expr;
  switch ( op )
  {
    case Equals:
      expr = QStringLiteral( "%1 = %3" ).arg( fieldRef, quotedVal );
      break;
    case  NotEquals:
      expr = QStringLiteral( "%1 != %3" ).arg( fieldRef, quotedVal );
      break;
    case GreaterThan:
      expr = QStringLiteral( "%1 > %3" ).arg( fieldRef, quotedVal );
      break;
    case GreaterThanEqualTo:
      expr = QStringLiteral( "%1 >= %3" ).arg( fieldRef, quotedVal );
      break;
    case LessThan:
      expr = QStringLiteral( "%1 < %3" ).arg( fieldRef, quotedVal );
      break;
    case LessThanEqualTo:
      expr = QStringLiteral( "%1 <= %3" ).arg( fieldRef, quotedVal );
      break;
    case BeginsWith:
      expr = QStringLiteral( "%1 LIKE '%2%'" ).arg( fieldRef, value );
      break;
    case Contains:
      expr = QStringLiteral( "%1 LIKE '%%2%'" ).arg( fieldRef, value );
      break;
    case IsNull:
      expr = QStringLiteral( "%1 IS NULL" ).arg( fieldRef );
      break;
    case IsNotNull:
      expr = QStringLiteral( "%1 IS NOT NULL" ).arg( fieldRef );
      break;
    case DoesNotContain:
      expr = QStringLiteral( "%1 NOT LIKE '%%2%'" ).arg( fieldRef, value );
      break;
  }

  QgsExpression expression( expr );
  if ( expression.hasParserError() )
  {
    throw QgsProcessingException( expression.parserErrorString() );
  }

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context );

  long count = source->featureCount();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  if ( !nonMatchingSink )
  {
    // not saving failing features - so only fetch good features
    QgsFeatureRequest req;
    req.setFilterExpression( expr );
    req.setExpressionContext( expressionContext );

    QgsFeatureIterator it = source->getFeatures( req );
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      matchingSink->addFeature( f, QgsFeatureSink::FastInsert );

      feedback->setProgress( current * step );
      current++;
    }
  }
  else
  {
    // saving non-matching features, so we need EVERYTHING
    expressionContext.setFields( source->fields() );
    expression.prepare( &expressionContext );

    QgsFeatureIterator it = source->getFeatures();
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      expressionContext.setFeature( f );
      if ( expression.evaluate( &expressionContext ).toBool() )
      {
        matchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }
      else
      {
        nonMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }

      feedback->setProgress( current * step );
      current++;
    }
  }


  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), matchingSinkId );
  if ( nonMatchingSink )
    outputs.insert( QStringLiteral( "FAIL_OUTPUT" ), nonMatchingSinkId );
  return outputs;
}


void QgsRemoveNullGeometryAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Non null geometries" ),
                QgsProcessing::TypeVectorAnyGeometry, QVariant(), true ) );
  QgsProcessingParameterFeatureSink *nullOutput = new QgsProcessingParameterFeatureSink( QStringLiteral( "NULL_OUTPUT" ),  QObject::tr( "Null geometries" ),
      QgsProcessing::TypeVector, QVariant(), true );
  nullOutput->setCreateByDefault( false );
  addParameter( nullOutput );
}

QString QgsRemoveNullGeometryAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm removes any features which do not have a geometry from a vector layer. "
                      "All other features will be copied unchanged.\n\n"
                      "Optionally, the features with null geometries can be saved to a separate output." );
}

QgsRemoveNullGeometryAlgorithm *QgsRemoveNullGeometryAlgorithm::createInstance() const
{
  return new QgsRemoveNullGeometryAlgorithm();
}

QVariantMap QgsRemoveNullGeometryAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString nonNullSinkId;
  std::unique_ptr< QgsFeatureSink > nonNullSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, nonNullSinkId, source->fields(),
      source->wkbType(), source->sourceCrs() ) );

  QString nullSinkId;
  std::unique_ptr< QgsFeatureSink > nullSink( parameterAsSink( parameters, QStringLiteral( "NULL_OUTPUT" ), context, nullSinkId, source->fields() ) );

  long count = source->featureCount();

  double step = count > 0 ? 100.0 / count : 1;
  int current = 0;

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( f.hasGeometry() && nonNullSink )
    {
      nonNullSink->addFeature( f, QgsFeatureSink::FastInsert );
    }
    else if ( !f.hasGeometry() && nullSink )
    {
      nullSink->addFeature( f, QgsFeatureSink::FastInsert );
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  if ( nonNullSink )
    outputs.insert( QStringLiteral( "OUTPUT" ), nonNullSinkId );
  if ( nullSink )
    outputs.insert( QStringLiteral( "NULL_OUTPUT" ), nullSinkId );
  return outputs;
}


QString QgsBoundingBoxAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the bounding box (envelope) for each feature in an input layer." ) +
         QStringLiteral( "\n\n" )  +
         QObject::tr( "See the 'Minimum bounding geometry' algorithm for a bounding box calculation which covers the whole layer or grouped subsets of features." );
}

QgsBoundingBoxAlgorithm *QgsBoundingBoxAlgorithm::createInstance() const
{
  return new QgsBoundingBoxAlgorithm();
}

QgsFields QgsBoundingBoxAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "width" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "height" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "area" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "perimeter" ), QVariant::Double, QString(), 20, 6 ) );
  return fields;
}

QgsFeature QgsBoundingBoxAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsRectangle bounds = f.geometry().boundingBox();
    QgsGeometry outputGeometry = QgsGeometry::fromRect( bounds );
    f.setGeometry( outputGeometry );
    QgsAttributes attrs = f.attributes();
    attrs << bounds.width()
          << bounds.height()
          << bounds.area()
          << bounds.perimeter();
    f.setAttributes( attrs );
  }
  return f;
}

QString QgsOrientedMinimumBoundingBoxAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the minimum area rotated rectangle which covers each feature in an input layer." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Minimum bounding geometry' algorithm for a oriented bounding box calculation which covers the whole layer or grouped subsets of features." );
}

QgsOrientedMinimumBoundingBoxAlgorithm *QgsOrientedMinimumBoundingBoxAlgorithm::createInstance() const
{
  return new QgsOrientedMinimumBoundingBoxAlgorithm();
}

QgsFields QgsOrientedMinimumBoundingBoxAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "width" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "height" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "angle" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "area" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "perimeter" ), QVariant::Double, QString(), 20, 6 ) );
  return fields;
}

QgsFeature QgsOrientedMinimumBoundingBoxAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double area = 0;
    double angle = 0;
    double width = 0;
    double height = 0;
    QgsGeometry outputGeometry = f.geometry().orientedMinimumBoundingBox( area, angle, width, height );
    f.setGeometry( outputGeometry );
    QgsAttributes attrs = f.attributes();
    attrs << width
          << height
          << angle
          << area
          << 2 * width + 2 * height;
    f.setAttributes( attrs );
  }
  return f;
}


void QgsMinimumEnclosingCircleAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEGMENTS" ), QObject::tr( "Number of segments in circles" ), QgsProcessingParameterNumber::Integer,
                72, false, 8, 100000 ) );
}

QString QgsMinimumEnclosingCircleAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the minimum enclosing circle which covers each feature in an input layer." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Minimum bounding geometry' algorithm for a minimal enclosing circle calculation which covers the whole layer or grouped subsets of features." );
}

QgsMinimumEnclosingCircleAlgorithm *QgsMinimumEnclosingCircleAlgorithm::createInstance() const
{
  return new QgsMinimumEnclosingCircleAlgorithm();
}

QgsFields QgsMinimumEnclosingCircleAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "radius" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "area" ), QVariant::Double, QString(), 20, 6 ) );
  return fields;
}

bool QgsMinimumEnclosingCircleAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSegments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  return true;
}

QgsFeature QgsMinimumEnclosingCircleAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    double radius = 0;
    QgsPointXY center;
    QgsGeometry outputGeometry = f.geometry().minimalEnclosingCircle( center, radius, mSegments );
    f.setGeometry( outputGeometry );
    QgsAttributes attrs = f.attributes();
    attrs << radius
          << M_PI *radius *radius;
    f.setAttributes( attrs );
  }
  return f;
}

QString QgsConvexHullAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the convex hull for each feature in an input layer." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Minimum bounding geometry' algorithm for a convex hull calculation which covers the whole layer or grouped subsets of features." );
}

QgsConvexHullAlgorithm *QgsConvexHullAlgorithm::createInstance() const
{
  return new QgsConvexHullAlgorithm();
}

QgsFields QgsConvexHullAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  QgsFields fields = inputFields;
  fields.append( QgsField( QStringLiteral( "area" ), QVariant::Double, QString(), 20, 6 ) );
  fields.append( QgsField( QStringLiteral( "perimeter" ), QVariant::Double, QString(), 20, 6 ) );
  return fields;
}

QgsFeature QgsConvexHullAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry outputGeometry = f.geometry().convexHull();
    if ( !outputGeometry )
      feedback->reportError( outputGeometry.lastError() );
    f.setGeometry( outputGeometry );
    if ( outputGeometry )
    {
      QgsAttributes attrs = f.attributes();
      attrs << outputGeometry.geometry()->area()
            << outputGeometry.geometry()->perimeter();
      f.setAttributes( attrs );
    }
  }
  return f;
}


QString QgsPromoteToMultipartAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer with singlepart geometries and generates a new one in which all geometries are "
                      "multipart. Input features which are already multipart features will remain unchanged." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "This algorithm can be used to force geometries to multipart types in order to be compatibility with data providers "
                      "with strict singlepart/multipart compatibility checks." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Collect geometries' or 'Aggregate' algorithms for alternative options." );
}

QgsPromoteToMultipartAlgorithm *QgsPromoteToMultipartAlgorithm::createInstance() const
{
  return new QgsPromoteToMultipartAlgorithm();
}

QgsWkbTypes::Type QgsPromoteToMultipartAlgorithm::outputWkbType( QgsWkbTypes::Type inputWkbType ) const
{
  return QgsWkbTypes::multiType( inputWkbType );
}

QgsFeature QgsPromoteToMultipartAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() && !f.geometry().isMultipart() )
  {
    QgsGeometry g = f.geometry();
    g.convertToMultiType();
    f.setGeometry( g );
  }
  return f;
}


void QgsCollectAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Unique ID fields" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Collected" ) ) );
}

QString QgsCollectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer and collects its geometries into new multipart geometries. One or more attributes can "
                      "be specified to collect only geometries belonging to the same class (having the same value for the specified attributes), alternatively "
                      "all geometries can be collected." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "All output geometries will be converted to multi geometries, even those with just a single part. "
                      "This algorithm does not dissolve overlapping geometries - they will be collected together without modifying the shape of each geometry part." ) +
         QStringLiteral( "\n\n" ) +
         QObject::tr( "See the 'Promote to multipart' or 'Aggregate' algorithms for alternative options." );
}

QgsCollectAlgorithm *QgsCollectAlgorithm::createInstance() const
{
  return new QgsCollectAlgorithm();
}



void QgsSelectByLocationAlgorithm::initAlgorithm( const QVariantMap & )
{
  QStringList methods = QStringList() << QObject::tr( "creating new selection" )
                        << QObject::tr( "adding to current selection" )
                        << QObject::tr( "select within current selection" )
                        << QObject::tr( "removing from current selection" );

  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Select features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addPredicateParameter();
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INTERSECT" ),
                QObject::tr( "By comparing to the features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ),
                QObject::tr( "Modify current selection by" ),
                methods, false, 0 ) );
}

QString QgsSelectByLocationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a selection in a vector layer. The criteria for selecting "
                      "features is based on the spatial relationship between each feature and the features in an additional layer." );
}

QgsSelectByLocationAlgorithm *QgsSelectByLocationAlgorithm::createInstance() const
{
  return new QgsSelectByLocationAlgorithm();
}

QVariantMap QgsSelectByLocationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsVectorLayer *selectLayer = parameterAsVectorLayer( parameters, QStringLiteral( "INPUT" ), context );
  QgsVectorLayer::SelectBehavior method = static_cast< QgsVectorLayer::SelectBehavior >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  std::unique_ptr< QgsFeatureSource > intersectSource( parameterAsSource( parameters, QStringLiteral( "INTERSECT" ), context ) );
  const QList< int > selectedPredicates = parameterAsEnums( parameters, QStringLiteral( "PREDICATE" ), context );

  QgsFeatureIds selectedIds;
  auto addToSelection = [&]( const QgsFeature & feature )
  {
    selectedIds.insert( feature.id() );
  };
  process( selectLayer, intersectSource.get(), selectedPredicates, addToSelection, true, feedback );

  selectLayer->selectByIds( selectedIds, method );
  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), parameters.value( QStringLiteral( "INPUT" ) ) );
  return results;
}

void QgsLocationBasedAlgorithm::process( QgsFeatureSource *targetSource,
    QgsFeatureSource *intersectSource,
    const QList< int > &selectedPredicates,
    const std::function < void( const QgsFeature & ) > &handleFeatureFunction,
    bool onlyRequireTargetIds,
    QgsFeedback *feedback )
{
  // build a list of 'reversed' predicates, because in this function
  // we actually test the reverse of what the user wants (allowing us
  // to prepare geometries and optimise the algorithm)
  QList< Predicate > predicates;
  for ( int i : selectedPredicates )
  {
    predicates << reversePredicate( static_cast< Predicate >( i ) );
  }

  QgsFeatureIds disjointSet;
  if ( predicates.contains( Disjoint ) )
    disjointSet = targetSource->allFeatureIds();

  QgsFeatureIds foundSet;
  QgsFeatureRequest request = QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ).setDestinationCrs( targetSource->sourceCrs() );
  QgsFeatureIterator fIt = intersectSource->getFeatures( request );
  double step = intersectSource->featureCount() > 0 ? 100.0 / intersectSource->featureCount() : 1;
  int current = 0;
  QgsFeature f;
  std::unique_ptr< QgsGeometryEngine > engine;
  while ( fIt.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
      break;

    if ( !f.hasGeometry() )
      continue;

    engine.reset();

    QgsRectangle bbox = f.geometry().boundingBox();
    request = QgsFeatureRequest().setFilterRect( bbox );
    if ( onlyRequireTargetIds )
      request.setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( QgsAttributeList() );

    QgsFeatureIterator testFeatureIt = targetSource->getFeatures( request );
    QgsFeature testFeature;
    while ( testFeatureIt.nextFeature( testFeature ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( foundSet.contains( testFeature.id() ) )
      {
        // already added this one, no need for further tests
        continue;
      }
      if ( predicates.count() == 1 && predicates.at( 0 ) == Disjoint && !disjointSet.contains( testFeature.id() ) )
      {
        // calculating only the disjoint set, and we've already eliminated this feature so no need for further tests
        continue;
      }

      if ( !engine )
      {
        engine.reset( QgsGeometry::createGeometryEngine( f.geometry().geometry() ) );
        engine->prepareGeometry();
      }

      for ( Predicate predicate : qgsAsConst( predicates ) )
      {
        bool isMatch = false;
        switch ( predicate )
        {
          case Intersects:
            isMatch = engine->intersects( testFeature.geometry().geometry() );
            break;
          case Contains:
            isMatch = engine->contains( testFeature.geometry().geometry() );
            break;
          case Disjoint:
            if ( engine->intersects( testFeature.geometry().geometry() ) )
            {
              disjointSet.remove( testFeature.id() );
            }
            break;
          case IsEqual:
            isMatch = engine->isEqual( testFeature.geometry().geometry() );
            break;
          case Touches:
            isMatch = engine->touches( testFeature.geometry().geometry() );
            break;
          case Overlaps:
            isMatch = engine->overlaps( testFeature.geometry().geometry() );
            break;
          case Within:
            isMatch = engine->within( testFeature.geometry().geometry() );
            break;
          case Crosses:
            isMatch = engine->crosses( testFeature.geometry().geometry() );
            break;
        }
        if ( isMatch )
        {
          foundSet.insert( testFeature.id() );
          handleFeatureFunction( testFeature );
        }
      }

    }

    current += 1;
    feedback->setProgress( current * step );
  }

  if ( predicates.contains( Disjoint ) )
  {
    disjointSet = disjointSet.subtract( foundSet );
    QgsFeatureRequest disjointReq = QgsFeatureRequest().setFilterFids( disjointSet );
    if ( onlyRequireTargetIds )
      disjointReq.setSubsetOfAttributes( QgsAttributeList() ).setFlags( QgsFeatureRequest::NoGeometry );
    QgsFeatureIterator disjointIt = targetSource->getFeatures( disjointReq );
    QgsFeature f;
    while ( disjointIt.nextFeature( f ) )
    {
      handleFeatureFunction( f );
    }
  }
}

void QgsLocationBasedAlgorithm::addPredicateParameter()
{
  std::unique_ptr< QgsProcessingParameterEnum > predicateParam( new QgsProcessingParameterEnum( QStringLiteral( "PREDICATE" ),
      QObject::tr( "Where the features (geometric predicate)" ),
      predicateOptionsList(), true, QVariant::fromValue( QList< int >() << 0 ) ) );

  QVariantMap predicateMetadata;
  QVariantMap widgetMetadata;
  widgetMetadata.insert( QStringLiteral( "class" ), QStringLiteral( "processing.gui.wrappers.EnumWidgetWrapper" ) );
  widgetMetadata.insert( QStringLiteral( "useCheckBoxes" ), true );
  widgetMetadata.insert( QStringLiteral( "columns" ), 2 );
  predicateMetadata.insert( QStringLiteral( "widget_wrapper" ), widgetMetadata );
  predicateParam->setMetadata( predicateMetadata );

  addParameter( predicateParam.release() );
}

QgsLocationBasedAlgorithm::Predicate QgsLocationBasedAlgorithm::reversePredicate( QgsLocationBasedAlgorithm::Predicate predicate ) const
{
  switch ( predicate )
  {
    case Intersects:
      return Intersects;
    case Contains:
      return Within;
    case Disjoint:
      return Disjoint;
    case IsEqual:
      return IsEqual;
    case Touches:
      return Touches;
    case Overlaps:
      return Overlaps;
    case Within:
      return Contains;
    case Crosses:
      return Crosses;
  }
  // no warnings
  return Intersects;
}

QStringList QgsLocationBasedAlgorithm::predicateOptionsList() const
{
  return QStringList() << QObject::tr( "intersect" )
         << QObject::tr( "contain" )
         << QObject::tr( "disjoint" )
         << QObject::tr( "equal" )
         << QObject::tr( "touch" )
         << QObject::tr( "overlap" )
         << QObject::tr( "are within" )
         << QObject::tr( "cross" );
}



void QgsExtractByLocationAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterVectorLayer( QStringLiteral( "INPUT" ), QObject::tr( "Extract features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addPredicateParameter();
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INTERSECT" ),
                QObject::tr( "By comparing to the features from" ),
                QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted (location)" ) ) );
}

QString QgsExtractByLocationAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an "
                      "input layer. The criteria for adding features to the resulting layer is defined "
                      "based on the spatial relationship between each feature and the features in an additional layer." );
}

QgsExtractByLocationAlgorithm *QgsExtractByLocationAlgorithm::createInstance() const
{
  return new QgsExtractByLocationAlgorithm();
}

QVariantMap QgsExtractByLocationAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  std::unique_ptr< QgsFeatureSource > intersectSource( parameterAsSource( parameters, QStringLiteral( "INTERSECT" ), context ) );
  const QList< int > selectedPredicates = parameterAsEnums( parameters, QStringLiteral( "PREDICATE" ), context );
  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, input->fields(), input->wkbType(), input->sourceCrs() ) );

  if ( !sink )
    return QVariantMap();

  auto addToSink = [&]( const QgsFeature & feature )
  {
    QgsFeature f = feature;
    sink->addFeature( f, QgsFeatureSink::FastInsert );
  };
  process( input.get(), intersectSource.get(), selectedPredicates, addToSink, false, feedback );

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), dest );
  return results;
}


QString QgsFixGeometriesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm attempts to create a valid representation of a given invalid geometry without "
                      "losing any of the input vertices. Already-valid geometries are returned without further intervention. "
                      "Always outputs multi-geometry layer.\n\n"
                      "NOTE: M values will be dropped from the output." );
}

QgsFixGeometriesAlgorithm *QgsFixGeometriesAlgorithm::createInstance() const
{
  return new QgsFixGeometriesAlgorithm();
}

QgsFeature QgsFixGeometriesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback )
{
  if ( !feature.hasGeometry() )
    return feature;

  QgsFeature outputFeature = feature;

  QgsGeometry outputGeometry = outputFeature.geometry().makeValid();
  if ( !outputGeometry )
  {
    feedback->pushInfo( QObject::tr( "makeValid failed for feature %1 " ).arg( feature.id() ) );
    outputFeature.clearGeometry();
    return outputFeature;
  }

  if ( outputGeometry.wkbType() == QgsWkbTypes::Unknown ||
       QgsWkbTypes::flatType( outputGeometry.geometry()->wkbType() ) == QgsWkbTypes::GeometryCollection )
  {
    // keep only the parts of the geometry collection with correct type
    const QList< QgsGeometry > tmpGeometries = outputGeometry.asGeometryCollection();
    QList< QgsGeometry > matchingParts;
    for ( const QgsGeometry &g : tmpGeometries )
    {
      if ( g.type() == feature.geometry().type() )
        matchingParts << g;
    }
    if ( !matchingParts.empty() )
      outputGeometry = QgsGeometry::collectGeometry( matchingParts );
    else
      outputGeometry = QgsGeometry();
  }

  outputGeometry.convertToMultiType();
  outputFeature.setGeometry( outputGeometry );
  return outputFeature;
}


QString QgsMergeLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm joins all connected parts of MultiLineString geometries into single LineString geometries.\n\n"
                      "If any parts of the input MultiLineString geometries are not connected, the resultant "
                      "geometry will be a MultiLineString containing any lines which could be merged and any non-connected line parts." );
}

QgsMergeLinesAlgorithm *QgsMergeLinesAlgorithm::createInstance() const
{
  return new QgsMergeLinesAlgorithm();
}

QgsFeature QgsMergeLinesAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback )
{
  if ( !feature.hasGeometry() )
    return feature;

  QgsFeature out = feature;
  QgsGeometry outputGeometry = feature.geometry().mergeLines();
  if ( !outputGeometry )
    feedback->reportError( QObject::tr( "Error merging lines for feature %1" ).arg( feature.id() ) );

  out.setGeometry( outputGeometry );
  return out;
}

QString QgsSmoothAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm smooths the geometries in a line or polygon layer. It creates a new layer with the "
                      "same features as the ones in the input layer, but with geometries containing a higher number of vertices "
                      "and corners in the geometries smoothed out.\n\n"
                      "The iterations parameter dictates how many smoothing iterations will be applied to each "
                      "geometry. A higher number of iterations results in smoother geometries with the cost of "
                      "greater number of nodes in the geometries.\n\n"
                      "The offset parameter controls how \"tightly\" the smoothed geometries follow the original geometries. "
                      "Smaller values results in a tighter fit, and larger values will create a looser fit.\n\n"
                      "The maximum angle parameter can be used to prevent smoothing of "
                      "nodes with large angles. Any node where the angle of the segments to either "
                      "side is larger than this will not be smoothed. For example, setting the maximum "
                      "angle to 90 degrees or lower would preserve right angles in the geometry." );
}

QgsSmoothAlgorithm *QgsSmoothAlgorithm::createInstance() const
{
  return new QgsSmoothAlgorithm();
}

void QgsSmoothAlgorithm::initParameters( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "ITERATIONS" ),
                QObject::tr( "Iterations" ), QgsProcessingParameterNumber::Integer,
                1, false, 1, 10 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "OFFSET" ),
                QObject::tr( "Offset" ), QgsProcessingParameterNumber::Double,
                0.25, false, 0.0, 0.5 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_ANGLE" ),
                QObject::tr( "Maximum node angle to smooth" ), QgsProcessingParameterNumber::Double,
                180.0, false, 0.0, 180.0 ) );
}

bool QgsSmoothAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mIterations = parameterAsInt( parameters, QStringLiteral( "ITERATIONS" ), context );
  mOffset = parameterAsDouble( parameters, QStringLiteral( "OFFSET" ), context );
  mMaxAngle = parameterAsDouble( parameters, QStringLiteral( "MAX_ANGLE" ), context );
  return true;
}

QgsFeature QgsSmoothAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback *feedback )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry outputGeometry = f.geometry().smooth( mIterations, mOffset, -1, mMaxAngle );
    if ( !outputGeometry )
    {
      feedback->reportError( QObject::tr( "Error smoothing geometry %1" ).arg( feature.id() ) );
    }
    f.setGeometry( outputGeometry );
  }
  return f;
}

QString QgsSimplifyAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm simplifies the geometries in a line or polygon layer. It creates a new layer "
                      "with the same features as the ones in the input layer, but with geometries containing a lower number of vertices.\n\n"
                      "The algorithm gives a choice of simplification methods, including distance based "
                      "(the \"Douglas-Peucker\" algorithm), area based (\"Visvalingam\" algorithm) and snapping geometries to a grid." );
}

QgsSimplifyAlgorithm *QgsSimplifyAlgorithm::createInstance() const
{
  return new QgsSimplifyAlgorithm();
}

void QgsSimplifyAlgorithm::initParameters( const QVariantMap & )
{
  QStringList methods;
  methods << QObject::tr( "Distance (Douglas-Peucker)" )
          << QObject::tr( "Snap to grid" )
          << QObject::tr( "Area (Visvalingam)" );

  addParameter( new QgsProcessingParameterEnum(
                  QStringLiteral( "METHOD" ),
                  QObject::tr( "Simplification method" ),
                  methods, false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "TOLERANCE" ),
                QObject::tr( "Tolerance" ), QgsProcessingParameterNumber::Double,
                1.0, false, 0, 10000000.0 ) );
}

bool QgsSimplifyAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mTolerance = parameterAsDouble( parameters, QStringLiteral( "TOLERANCE" ), context );
  mMethod = static_cast< QgsMapToPixelSimplifier::SimplifyAlgorithm >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  if ( mMethod != QgsMapToPixelSimplifier::Distance )
    mSimplifier.reset( new QgsMapToPixelSimplifier( QgsMapToPixelSimplifier::SimplifyGeometry, mTolerance, mMethod ) );

  return true;
}

QgsFeature QgsSimplifyAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingFeedback * )
{
  QgsFeature f = feature;
  if ( f.hasGeometry() )
  {
    QgsGeometry inputGeometry = f.geometry();
    QgsGeometry outputGeometry;
    if ( mMethod == QgsMapToPixelSimplifier::Distance )
    {
      outputGeometry = inputGeometry.simplify( mTolerance );
    }
    else
    {
      outputGeometry = mSimplifier->simplify( inputGeometry );
    }
    f.setGeometry( outputGeometry );
  }
  return f;
}




void QgsExtractByExtentAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "CLIP" ), QObject::tr( "Clip features to extent" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extracted" ) ) );
}

QString QgsExtractByExtentAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains features which fall within a specified extent. "
                      "Any features which intersect the extent will be included.\n\n"
                      "Optionally, feature geometries can also be clipped to the extent. If this option is selected, then the output "
                      "geometries will automatically be converted to multi geometries to ensure uniform output geometry types." );
}

QgsExtractByExtentAlgorithm *QgsExtractByExtentAlgorithm::createInstance() const
{
  return new QgsExtractByExtentAlgorithm();
}

QVariantMap QgsExtractByExtentAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > featureSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !featureSource )
    return QVariantMap();

  QgsRectangle extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, featureSource->sourceCrs() );
  bool clip = parameterAsBool( parameters, QStringLiteral( "CLIP" ), context );

  // if clipping, we force multi output
  QgsWkbTypes::Type outType = clip ? QgsWkbTypes::multiType( featureSource->wkbType() ) : featureSource->wkbType();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, featureSource->fields(), outType, featureSource->sourceCrs() ) );

  if ( !sink )
    return QVariantMap();

  QgsGeometry clipGeom = parameterAsExtentGeometry( parameters, QStringLiteral( "EXTENT" ), context, featureSource->sourceCrs() );

  double step = featureSource->featureCount() > 0 ? 100.0 / featureSource->featureCount() : 1;
  QgsFeatureIterator inputIt = featureSource->getFeatures( QgsFeatureRequest().setFilterRect( extent ).setFlags( QgsFeatureRequest::ExactIntersect ) );
  QgsFeature f;
  int i = -1;
  while ( inputIt.nextFeature( f ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( clip )
    {
      QgsGeometry g = f.geometry().intersection( clipGeom );
      g.convertToMultiType();
      f.setGeometry( g );
    }

    sink->addFeature( f, QgsFeatureSink::FastInsert );
    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


void QgsExtentToLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterExtent( QStringLiteral( "INPUT" ), QObject::tr( "Extent" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Extent" ), QgsProcessing::TypeVectorPolygon ) );
}

QString QgsExtentToLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that contains a single feature with geometry matching an extent parameter.\n\n"
                      "It can be used in models to convert an extent into a layer which can be used for other algorithms which require "
                      "a layer based input." );
}

QgsExtentToLayerAlgorithm *QgsExtentToLayerAlgorithm::createInstance() const
{
  return new QgsExtentToLayerAlgorithm();
}

QVariantMap QgsExtentToLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsCoordinateReferenceSystem crs = parameterAsExtentCrs( parameters, QStringLiteral( "INPUT" ), context );
  QgsGeometry geom = parameterAsExtentGeometry( parameters, QStringLiteral( "INPUT" ), context );

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "id" ), QVariant::Int ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields, QgsWkbTypes::Polygon, crs ) );
  if ( !sink )
    return QVariantMap();

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( geom );
  sink->addFeature( f, QgsFeatureSink::FastInsert );

  feedback->setProgress( 100 );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

void QgsLineIntersectionAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INTERSECT" ),
                QObject::tr( "Intersect layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );

  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "INPUT_FIELDS" ),
                  QObject::tr( "Input fields to keep (leave empty to keep all fields)" ), QVariant(),
                  QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any,
                  true, true ) );
  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "INTERSECT_FIELDS" ),
                  QObject::tr( "Intersect fields to keep (leave empty to keep all fields)" ), QVariant(),
                  QStringLiteral( "INTERSECT" ), QgsProcessingParameterField::Any,
                  true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Intersections" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsLineIntersectionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates point features where the lines in the Intersect layer intersect the lines in the Input layer." );
}

QgsLineIntersectionAlgorithm *QgsLineIntersectionAlgorithm::createInstance() const
{
  return new QgsLineIntersectionAlgorithm();
}

QVariantMap QgsLineIntersectionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > sourceA( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !sourceA )
    return QVariantMap();

  std::unique_ptr< QgsFeatureSource > sourceB( parameterAsSource( parameters, QStringLiteral( "INTERSECT" ), context ) );
  if ( !sourceB )
    return QVariantMap();

  const QStringList fieldsA = parameterAsFields( parameters, QStringLiteral( "INPUT_FIELDS" ), context );
  const QStringList fieldsB = parameterAsFields( parameters, QStringLiteral( "INTERSECT_FIELDS" ), context );

  QgsFields outFieldsA;
  QgsAttributeList fieldsAIndices;

  if ( fieldsA.empty() )
  {
    outFieldsA = sourceA->fields();
    for ( int i = 0; i < outFieldsA.count(); ++i )
    {
      fieldsAIndices << i;
    }
  }
  else
  {
    for ( const QString &field : fieldsA )
    {
      int index = sourceA->fields().lookupField( field );
      if ( index >= 0 )
      {
        fieldsAIndices << index;
        outFieldsA.append( sourceA->fields().at( index ) );
      }
    }
  }

  QgsFields outFieldsB;
  QgsAttributeList fieldsBIndices;

  if ( fieldsB.empty() )
  {
    outFieldsB = sourceB->fields();
    for ( int i = 0; i < outFieldsB.count(); ++i )
    {
      fieldsBIndices << i;
    }
  }
  else
  {
    for ( const QString &field : fieldsB )
    {
      int index = sourceB->fields().lookupField( field );
      if ( index >= 0 )
      {
        fieldsBIndices << index;
        outFieldsB.append( sourceB->fields().at( index ) );
      }
    }
  }

  QgsFields outFields = QgsProcessingUtils::combineFields( outFieldsA, outFieldsB );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outFields, QgsWkbTypes::Point,  sourceA->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  QgsSpatialIndex spatialIndex( sourceB->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ).setDestinationCrs( sourceA->sourceCrs() ) ), feedback );
  QgsFeature outFeature;
  QgsFeatureIterator features = sourceA->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( fieldsAIndices ) );
  double step = sourceA->featureCount() > 0 ? 100.0 / sourceA->featureCount() : 1;
  int i = 0;
  QgsFeature inFeatureA;
  while ( features.nextFeature( inFeatureA ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !inFeatureA.hasGeometry() )
      continue;

    QgsGeometry inGeom = inFeatureA.geometry();
    QgsFeatureIds lines = spatialIndex.intersects( inGeom.boundingBox() ).toSet();
    if ( !lines.empty() )
    {
      // use prepared geometries for faster intersection tests
      std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( inGeom.geometry() ) );
      engine->prepareGeometry();

      QgsFeatureRequest request = QgsFeatureRequest().setFilterFids( lines );
      request.setDestinationCrs( sourceA->sourceCrs() );
      request.setSubsetOfAttributes( fieldsBIndices );

      QgsFeature inFeatureB;
      QgsFeatureIterator featuresB = sourceB->getFeatures( request );
      while ( featuresB.nextFeature( inFeatureB ) )
      {
        if ( feedback->isCanceled() )
        {
          break;
        }

        QgsGeometry tmpGeom = inFeatureB.geometry();
        if ( engine->intersects( tmpGeom.geometry() ) )
        {
          QgsMultiPoint points;
          QgsGeometry intersectGeom = inGeom.intersection( tmpGeom );
          QgsAttributes outAttributes;
          for ( int a : qgsAsConst( fieldsAIndices ) )
          {
            outAttributes.append( inFeatureA.attribute( a ) );
          }
          for ( int b : qgsAsConst( fieldsBIndices ) )
          {
            outAttributes.append( inFeatureB.attribute( b ) );
          }
          if ( intersectGeom.type() == QgsWkbTypes::PointGeometry )
          {
            if ( intersectGeom.isMultipart() )
            {
              points = intersectGeom.asMultiPoint();
            }
            else
            {
              points.append( intersectGeom.asPoint() );
            }

            for ( const QgsPointXY &j : qgsAsConst( points ) )
            {
              outFeature.setGeometry( QgsGeometry::fromPoint( j ) );
              outFeature.setAttributes( outAttributes );
              sink->addFeature( outFeature, QgsFeatureSink::FastInsert );
            }
          }
        }
      }
    }

    feedback->setProgress( i * step );

  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


void QgsSplitWithLinesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorLine << QgsProcessing::TypeVectorPolygon ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "LINES" ),
                QObject::tr( "Split layer" ), QList< int >() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Split" ) ) );
}

QString QgsSplitWithLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm splits the lines or polygons in one layer using the lines in another layer to define the breaking points. "
                      "Intersection between geometries in both layers are considered as split points.\n\n"
                      "Output will contain multi geometries for split features." );
}

QgsSplitWithLinesAlgorithm *QgsSplitWithLinesAlgorithm::createInstance() const
{
  return new QgsSplitWithLinesAlgorithm();
}

QVariantMap QgsSplitWithLinesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  std::unique_ptr< QgsFeatureSource > linesSource( parameterAsSource( parameters, QStringLiteral( "LINES" ), context ) );
  if ( !linesSource )
    return QVariantMap();

  bool sameLayer = parameters.value( QStringLiteral( "INPUT" ) ) == parameters.value( QStringLiteral( "LINES" ) );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, source->fields(),
                                          QgsWkbTypes::multiType( source->wkbType() ),  source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  QgsSpatialIndex spatialIndex;
  QMap< QgsFeatureId, QgsGeometry > splitGeoms;
  QgsFeatureRequest request;
  request.setSubsetOfAttributes( QgsAttributeList() );
  request.setDestinationCrs( source->sourceCrs() );

  QgsFeatureIterator splitLines = linesSource->getFeatures( request );
  QgsFeature aSplitFeature;
  while ( splitLines.nextFeature( aSplitFeature ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    splitGeoms.insert( aSplitFeature.id(), aSplitFeature.geometry() );
    spatialIndex.insertFeature( aSplitFeature );
  }

  QgsFeature outFeat;
  QgsFeatureIterator features = source->getFeatures();

  double step = source->featureCount() > 0 ? 100.0 / source->featureCount() : 1;
  int i = 0;
  QgsFeature inFeatureA;
  while ( features.nextFeature( inFeatureA ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    if ( !inFeatureA.hasGeometry() )
    {
      sink->addFeature( inFeatureA, QgsFeatureSink::FastInsert );
      continue;
    }

    QgsGeometry inGeom = inFeatureA.geometry();
    outFeat.setAttributes( inFeatureA.attributes() );

    QList< QgsGeometry > inGeoms = inGeom.asGeometryCollection();

    const QgsFeatureIds lines = spatialIndex.intersects( inGeom.boundingBox() ).toSet();
    if ( !lines.empty() ) // has intersection of bounding boxes
    {
      QList< QgsGeometry > splittingLines;

      // use prepared geometries for faster intersection tests
      std::unique_ptr< QgsGeometryEngine > engine;

      for ( QgsFeatureId line : lines )
      {
        // check if trying to self-intersect
        if ( sameLayer && inFeatureA.id() == line )
          continue;

        QgsGeometry splitGeom = splitGeoms.value( line );
        if ( !engine )
        {
          engine.reset( QgsGeometry::createGeometryEngine( inGeom.geometry() ) );
          engine->prepareGeometry();
        }

        if ( engine->intersects( splitGeom.geometry() ) )
        {
          QList< QgsGeometry > splitGeomParts = splitGeom.asGeometryCollection();
          splittingLines.append( splitGeomParts );
        }
      }

      if ( !splittingLines.empty() )
      {
        for ( const QgsGeometry &splitGeom : qgsAsConst( splittingLines ) )
        {
          QList<QgsPointXY> splitterPList;
          QList< QgsGeometry > outGeoms;

          // use prepared geometries for faster intersection tests
          std::unique_ptr< QgsGeometryEngine > splitGeomEngine( QgsGeometry::createGeometryEngine( splitGeom.geometry() ) );
          splitGeomEngine->prepareGeometry();
          while ( !inGeoms.empty() )
          {
            if ( feedback->isCanceled() )
            {
              break;
            }

            QgsGeometry inGeom = inGeoms.takeFirst();
            if ( !inGeom )
              continue;

            if ( splitGeomEngine->intersects( inGeom.geometry() ) )
            {
              QgsGeometry before = inGeom;
              if ( splitterPList.empty() )
              {
                const QgsCoordinateSequence sequence = splitGeom.geometry()->coordinateSequence();
                for ( const QgsRingSequence &part : sequence )
                {
                  for ( const QgsPointSequence &ring : part )
                  {
                    for ( const QgsPoint &pt : ring )
                    {
                      splitterPList << QgsPointXY( pt );
                    }
                  }
                }
              }

              QList< QgsGeometry > newGeometries;
              QList<QgsPointXY> topologyTestPoints;
              QgsGeometry::OperationResult result = inGeom.splitGeometry( splitterPList, newGeometries, false, topologyTestPoints );

              // splitGeometry: If there are several intersections
              // between geometry and splitLine, only the first one is considered.
              if ( result == QgsGeometry::Success ) // split occurred
              {
                if ( inGeom.equals( before ) )
                {
                  // bug in splitGeometry: sometimes it returns 0 but
                  // the geometry is unchanged
                  outGeoms.append( inGeom );
                }
                else
                {
                  inGeoms.append( inGeom );
                  inGeoms.append( newGeometries );
                }
              }
              else
              {
                outGeoms.append( inGeom );
              }
            }
            else
            {
              outGeoms.append( inGeom );
            }

          }
          inGeoms = outGeoms;
        }
      }
    }

    QList< QgsGeometry > parts;
    for ( const QgsGeometry &aGeom : qgsAsConst( inGeoms ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      bool passed = true;
      if ( QgsWkbTypes::geometryType( aGeom.wkbType() ) == QgsWkbTypes::LineGeometry )
      {
        int numPoints = aGeom.geometry()->nCoordinates();

        if ( numPoints <= 2 )
        {
          if ( numPoints == 2 )
            passed = !static_cast< QgsCurve * >( aGeom.geometry() )->isClosed(); // tests if vertex 0 = vertex 1
          else
            passed = false; // sometimes splitting results in lines of zero length
        }
      }

      if ( passed )
        parts.append( aGeom );
    }

    if ( !parts.empty() )
    {
      outFeat.setGeometry( QgsGeometry::collectGeometry( parts ) );
      sink->addFeature( outFeat, QgsFeatureSink::FastInsert );
    }

    feedback->setProgress( i * step );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


void QgsMeanCoordinatesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int >() << QgsProcessing::TypeVectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "WEIGHT" ), QObject::tr( "Weight field" ),
                QVariant(), QStringLiteral( "INPUT" ),
                QgsProcessingParameterField::Numeric, false, true ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "UID" ),
                QObject::tr( "Unique ID field" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, false, true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Mean coordinates" ), QgsProcessing::TypeVectorPoint ) );
}

QString QgsMeanCoordinatesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a point layer with the center of mass of geometries in an input layer.\n\n"
                      "An attribute can be specified as containing weights to be applied to each feature when computing the center of mass.\n\n"
                      "If an attribute is selected in the <Unique ID field> parameter, features will be grouped according "
                      "to values in this field. Instead of a single point with the center of mass of the whole layer, "
                      "the output layer will contain a center of mass for the features in each category." );
}

QgsMeanCoordinatesAlgorithm *QgsMeanCoordinatesAlgorithm::createInstance() const
{
  return new QgsMeanCoordinatesAlgorithm();
}

QVariantMap QgsMeanCoordinatesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString weightFieldName = parameterAsString( parameters, QStringLiteral( "WEIGHT" ), context );
  QString uniqueFieldName = parameterAsString( parameters, QStringLiteral( "UID" ), context );

  QgsAttributeList attributes;
  int weightIndex = -1;
  if ( !weightFieldName.isEmpty() )
  {
    weightIndex = source->fields().lookupField( weightFieldName );
    if ( weightIndex >= 0 )
      attributes.append( weightIndex );
  }

  int uniqueFieldIndex = -1;
  if ( !uniqueFieldName.isEmpty() )
  {
    uniqueFieldIndex = source->fields().lookupField( uniqueFieldName );
    if ( uniqueFieldIndex >= 0 )
      attributes.append( uniqueFieldIndex );
  }

  QgsFields fields;
  fields.append( QgsField( QStringLiteral( "MEAN_X" ), QVariant::Double, QString(), 24, 15 ) );
  fields.append( QgsField( QStringLiteral( "MEAN_Y" ), QVariant::Double, QString(), 24, 15 ) );
  if ( uniqueFieldIndex >= 0 )
  {
    QgsField uniqueField = source->fields().at( uniqueFieldIndex );
    fields.append( uniqueField );
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          QgsWkbTypes::Point, source->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  QgsFeatureIterator features = source->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( attributes ) );

  double step = source->featureCount() > 0 ? 50.0 / source->featureCount() : 1;
  int i = 0;
  QgsFeature feat;

  QHash< QVariant, QList< double > > means;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );
    if ( !feat.hasGeometry() )
      continue;


    QVariant featureClass;
    if ( uniqueFieldIndex >= 0 )
    {
      featureClass = feat.attribute( uniqueFieldIndex );
    }
    else
    {
      featureClass = QStringLiteral( "#####singleclass#####" );
    }

    double weight = 1;
    if ( weightIndex >= 0 )
    {
      bool ok = false;
      weight = feat.attribute( weightIndex ).toDouble( &ok );
      if ( !ok )
        weight = 1.0;
    }

    if ( weight < 0 )
    {
      throw QgsProcessingException( QObject::tr( "Negative weight value found. Please fix your data and try again." ) );
    }

    QList< double > values = means.value( featureClass );
    double cx = 0;
    double cy = 0;
    double totalWeight = 0;
    if ( !values.empty() )
    {
      cx = values.at( 0 );
      cy = values.at( 1 );
      totalWeight = values.at( 2 );
    }

    QgsVertexId vid;
    QgsPoint pt;
    const QgsAbstractGeometry *g = feat.geometry().geometry();
    // NOTE - should this be including the duplicate nodes for closed rings? currently it is,
    // but I suspect that the expected behavior would be to NOT include these
    while ( g->nextVertex( vid, pt ) )
    {
      cx += pt.x() * weight;
      cy += pt.y() * weight;
      totalWeight += weight;
    }

    means[featureClass] = QList< double >() << cx << cy << totalWeight;
  }

  i = 0;
  step = !means.empty() ? 50.0 / means.count() : 1;
  for ( auto it = means.constBegin(); it != means.constEnd(); ++it )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 50 + i * step );
    if ( qgsDoubleNear( it.value().at( 2 ), 0 ) )
      continue;

    QgsFeature outFeat;
    double cx = it.value().at( 0 ) / it.value().at( 2 );
    double cy = it.value().at( 1 ) / it.value().at( 2 );

    QgsPointXY meanPoint( cx, cy );
    outFeat.setGeometry( QgsGeometry::fromPoint( meanPoint ) );

    QgsAttributes attributes;
    attributes << cx << cy;
    if ( uniqueFieldIndex >= 0 )
      attributes.append( it.key() );

    outFeat.setAttributes( attributes );
    sink->addFeature( outFeat, QgsFeatureSink::FastInsert );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


void QgsRasterLayerUniqueValuesReportAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_HTML_FILE" ),
                QObject::tr( "Unique values report" ), QObject::tr( "HTML files (*.html)" ), QVariant(), true ) );

  addOutput( new QgsProcessingOutputHtml( QStringLiteral( "OUTPUT_HTML_FILE" ), QObject::tr( "Unique values report" ) ) );

  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_PIXEL_COUNT" ), QObject::tr( "Total pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NODATA_PIXEL_COUNT" ), QObject::tr( "NODATA pixel count" ) ) );
}

QString QgsRasterLayerUniqueValuesReportAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm returns the count and area of each unique value in a given raster layer." );
}

QgsRasterLayerUniqueValuesReportAlgorithm *QgsRasterLayerUniqueValuesReportAlgorithm::createInstance() const
{
  return new QgsRasterLayerUniqueValuesReportAlgorithm();
}

bool QgsRasterLayerUniqueValuesReportAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );
  int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );

  if ( !layer )
    return false;

  mInterface.reset( layer->dataProvider()->clone() );
  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = layer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = layer->rasterUnitsPerPixelY();
  mSource = layer->source();

  return true;
}

QVariantMap QgsRasterLayerUniqueValuesReportAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT_HTML_FILE" ), context );

  QHash< double, qgssize > uniqueValues;
  qgssize noDataCount = 0;

  qgssize layerSize = static_cast< qgssize >( mLayerWidth ) * static_cast< qgssize >( mLayerHeight );
  int maxWidth = 4000;
  int maxHeight = 4000;
  int nbBlocksWidth = std::ceil( 1.0 * mLayerWidth / maxWidth );
  int nbBlocksHeight = std::ceil( 1.0 * mLayerHeight / maxHeight );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  QgsRasterIterator iter( mInterface.get() );
  iter.setMaximumTileWidth( maxWidth );
  iter.setMaximumTileHeight( maxHeight );
  iter.startRasterRead( band, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRasterBlock *rasterBlock = nullptr;
  while ( iter.readNextRasterPart( band, iterCols, iterRows, &rasterBlock, iterLeft, iterTop ) )
  {
    feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( mHasNoDataValue && rasterBlock->isNoData( row, column ) )
        {
          noDataCount += 1;
        }
        else
        {
          double value = rasterBlock->value( row, column );
          uniqueValues[ value ]++;
        }
      }
    }
    delete rasterBlock;
  }

  QMap< double, qgssize > sortedUniqueValues;
  for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
  {
    sortedUniqueValues.insert( it.key(), it.value() );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "EXTENT" ), mExtent.toString() );
  outputs.insert( QStringLiteral( "CRS_AUTHID" ), mCrs.authid() );
  outputs.insert( QStringLiteral( "WIDTH_IN_PIXELS" ), mLayerWidth );
  outputs.insert( QStringLiteral( "HEIGHT_IN_PIXELS" ), mLayerHeight );
  outputs.insert( QStringLiteral( "TOTAL_PIXEL_COUNT" ), layerSize );
  outputs.insert( QStringLiteral( "NODATA_PIXEL_COUNT" ), noDataCount );

  if ( !outputFile.isEmpty() )
  {
    QFile file( outputFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QString areaUnit = QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::distanceToAreaUnit( mCrs.mapUnits() ) );
      double pixelArea = mRasterUnitsPerPixelX * mRasterUnitsPerPixelY;

      QTextStream out( &file );
      out << QString( "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n" );
      out << QString( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Analyzed file" ) ).arg( mSource ).arg( QObject::tr( "band" ) ).arg( band );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Extent" ) ).arg( mExtent.toString() );
      out << QObject::tr( "<p>%1: %2 (%3)</p>\n" ).arg( QObject::tr( "Projection" ) ).arg( mCrs.description() ).arg( mCrs.authid() );
      out << QObject::tr( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Width in pixels" ) ).arg( mLayerWidth ).arg( QObject::tr( "units per pixel" ) ).arg( mRasterUnitsPerPixelX );
      out << QObject::tr( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Height in pixels" ) ).arg( mLayerHeight ).arg( QObject::tr( "units per pixel" ) ).arg( mRasterUnitsPerPixelY );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Total pixel count" ) ).arg( layerSize );
      if ( mHasNoDataValue )
        out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "NODATA pixel count" ) ).arg( noDataCount );
      out << QString( "<table><tr><td>%1</td><td>%2</td><td>%3 (%4)</td></tr>\n" ).arg( QObject::tr( "Value" ) ).arg( QObject::tr( "Pixel count" ) ).arg( QObject::tr( "Area" ) ).arg( areaUnit );

      for ( double key : sortedUniqueValues.keys() )
      {
        double area = sortedUniqueValues[key] * pixelArea;
        out << QString( "<tr><td>%1</td><td>%2</td><td>%3</td></tr>\n" ).arg( key ).arg( sortedUniqueValues[key] ).arg( QString::number( area, 'g', 16 ) );
      }
      out << QString( "</table>\n</body></html>" );
      outputs.insert( QStringLiteral( "OUTPUT_HTML_FILE" ), outputFile );
    }
  }

  return outputs;
}



void QgsJoinByAttributeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ), QList< int>() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ),
                QObject::tr( "Table field" ), QVariant(), QStringLiteral( "INPUT" ) ) );

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_2" ),
                QObject::tr( "Input layer 2" ), QList< int>() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD_2" ),
                QObject::tr( "Table field 2" ), QVariant(), QStringLiteral( "INPUT_2" ) ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELDS_TO_COPY" ),
                QObject::tr( "Layer 2 fields to copy (leave empty to copy all fields)" ),
                QVariant(), QStringLiteral( "INPUT_2" ), QgsProcessingParameterField::Any,
                true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Joined layer" ) ) );
}

QString QgsJoinByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes an input vector layer and creates a new vector layer that is an extended version of the "
                      "input one, with additional attributes in its attribute table.\n\n"
                      "The additional attributes and their values are taken from a second vector layer. An attribute is selected "
                      "in each of them to define the join criteria." );
}

QgsJoinByAttributeAlgorithm *QgsJoinByAttributeAlgorithm::createInstance() const
{
  return new QgsJoinByAttributeAlgorithm();
}

QVariantMap QgsJoinByAttributeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr< QgsFeatureSource > input( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  std::unique_ptr< QgsFeatureSource > input2( parameterAsSource( parameters, QStringLiteral( "INPUT_2" ), context ) );
  if ( !input || !input2 )
    return QVariantMap();

  QString field1Name = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  QString field2Name = parameterAsString( parameters, QStringLiteral( "FIELD_2" ), context );
  const QStringList fieldsToCopy = parameterAsFields( parameters, QStringLiteral( "FIELDS_TO_COPY" ), context );

  int joinField1Index = input->fields().lookupField( field1Name );
  int joinField2Index = input2->fields().lookupField( field2Name );
  if ( joinField1Index < 0 || joinField2Index < 0 )
    return QVariantMap();

  QgsFields outFields2;
  QgsAttributeList fields2Indices;
  if ( fieldsToCopy.empty() )
  {
    outFields2 = input2->fields();
    for ( int i = 0; i < outFields2.count(); ++i )
    {
      fields2Indices << i;
    }
  }
  else
  {
    for ( const QString &field : fieldsToCopy )
    {
      int index = input2->fields().lookupField( field );
      if ( index >= 0 )
      {
        fields2Indices << index;
        outFields2.append( input2->fields().at( index ) );
      }
    }
  }

  QgsAttributeList fields2Fetch = fields2Indices;
  fields2Fetch << joinField2Index;

  QgsFields outFields = QgsProcessingUtils::combineFields( input->fields(), outFields2 );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, outFields,
                                          input->wkbType(), input->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();


  // cache attributes of input2
  QHash< QVariant, QgsAttributes > input2AttributeCache;
  QgsFeatureIterator features = input2->getFeatures( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setSubsetOfAttributes( fields2Fetch ) );
  double step = input2->featureCount() > 0 ? 50.0 / input2->featureCount() : 1;
  int i = 0;
  QgsFeature feat;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    if ( input2AttributeCache.contains( feat.attribute( joinField2Index ) ) )
      continue;

    // only keep selected attributes
    QgsAttributes attributes;
    for ( int j = 0; j < feat.attributes().count(); ++j )
    {
      if ( ! fields2Indices.contains( j ) )
        continue;
      attributes << feat.attribute( j );
    }

    input2AttributeCache.insert( feat.attribute( joinField2Index ), attributes );
  }

  // Create output vector layer with additional attribute
  step = input->featureCount() > 0 ? 50.0 / input->featureCount() : 1;
  features = input->getFeatures();
  i = 0;
  while ( features.nextFeature( feat ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 50 + i * step );

    QgsAttributes attrs = feat.attributes();
    attrs.append( input2AttributeCache.value( feat.attribute( joinField1Index ) ) );
    feat.setAttributes( attrs );
    sink->addFeature( feat, QgsFeatureSink::FastInsert );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


void QgsJoinWithLinesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "HUBS" ),
                QObject::tr( "Hub layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "HUB_FIELD" ),
                QObject::tr( "Hub ID field" ), QVariant(), QStringLiteral( "HUBS" ) ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "HUB_FIELDS" ),
                QObject::tr( "Hub layer fields to copy (leave empty to copy all fields)" ),
                QVariant(), QStringLiteral( "HUBS" ), QgsProcessingParameterField::Any,
                true, true ) );

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "SPOKES" ),
                QObject::tr( "Spoke layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "SPOKE_FIELD" ),
                QObject::tr( "Spoke ID field" ), QVariant(), QStringLiteral( "SPOKES" ) ) );

  addParameter( new QgsProcessingParameterField( QStringLiteral( "SPOKE_FIELDS" ),
                QObject::tr( "Spoke layer fields to copy (leave empty to copy all fields)" ),
                QVariant(), QStringLiteral( "SPOKES" ), QgsProcessingParameterField::Any,
                true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Hub lines" ), QgsProcessing::TypeVectorLine ) );
}

QString QgsJoinWithLinesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates hub and spoke diagrams by connecting lines from points on the Spoke layer to matching points in the Hub layer.\n\n"
                      "Determination of which hub goes with each point is based on a match between the Hub ID field on the hub points and the Spoke ID field on the spoke points.\n\n"
                      "If input layers are not point layers, a point on the surface of the geometries will be taken as the connecting location." );
}

QgsJoinWithLinesAlgorithm *QgsJoinWithLinesAlgorithm::createInstance() const
{
  return new QgsJoinWithLinesAlgorithm();
}

QVariantMap QgsJoinWithLinesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( parameters.value( QStringLiteral( "SPOKES" ) ) == parameters.value( QStringLiteral( "HUBS" ) ) )
    throw QgsProcessingException( QObject::tr( "Same layer given for both hubs and spokes" ) );

  std::unique_ptr< QgsFeatureSource > hubSource( parameterAsSource( parameters, QStringLiteral( "HUBS" ), context ) );
  std::unique_ptr< QgsFeatureSource > spokeSource( parameterAsSource( parameters, QStringLiteral( "SPOKES" ), context ) );
  if ( !hubSource || !spokeSource )
    return QVariantMap();

  QString fieldHubName = parameterAsString( parameters, QStringLiteral( "HUB_FIELD" ), context );
  int fieldHubIndex = hubSource->fields().lookupField( fieldHubName );
  const QStringList hubFieldsToCopy = parameterAsFields( parameters, QStringLiteral( "HUB_FIELDS" ), context );

  QString fieldSpokeName = parameterAsString( parameters, QStringLiteral( "SPOKE_FIELD" ), context );
  int fieldSpokeIndex = spokeSource->fields().lookupField( fieldSpokeName );
  const QStringList spokeFieldsToCopy = parameterAsFields( parameters, QStringLiteral( "SPOKE_FIELDS" ), context );

  if ( fieldHubIndex < 0 || fieldSpokeIndex < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid ID field" ) );

  QgsFields hubOutFields;
  QgsAttributeList hubFieldIndices;
  if ( hubFieldsToCopy.empty() )
  {
    hubOutFields = hubSource->fields();
    for ( int i = 0; i < hubOutFields.count(); ++i )
    {
      hubFieldIndices << i;
    }
  }
  else
  {
    for ( const QString &field : hubFieldsToCopy )
    {
      int index = hubSource->fields().lookupField( field );
      if ( index >= 0 )
      {
        hubFieldIndices << index;
        hubOutFields.append( hubSource->fields().at( index ) );
      }
    }
  }

  QgsAttributeList hubFields2Fetch = hubFieldIndices;
  hubFields2Fetch << fieldHubIndex;

  QgsFields spokeOutFields;
  QgsAttributeList spokeFieldIndices;
  if ( spokeFieldsToCopy.empty() )
  {
    spokeOutFields = spokeSource->fields();
    for ( int i = 0; i < spokeOutFields.count(); ++i )
    {
      spokeFieldIndices << i;
    }
  }
  else
  {
    for ( const QString &field : spokeFieldsToCopy )
    {
      int index = spokeSource->fields().lookupField( field );
      if ( index >= 0 )
      {
        spokeFieldIndices << index;
        spokeOutFields.append( spokeSource->fields().at( index ) );
      }
    }
  }

  QgsAttributeList spokeFields2Fetch = spokeFieldIndices;
  spokeFields2Fetch << fieldSpokeIndex;


  QgsFields fields = QgsProcessingUtils::combineFields( hubOutFields, spokeOutFields );

  QgsWkbTypes::Type outType = QgsWkbTypes::LineString;
  bool hasZ = false;
  if ( QgsWkbTypes::hasZ( hubSource->wkbType() ) || QgsWkbTypes::hasZ( spokeSource->wkbType() ) )
  {
    outType = QgsWkbTypes::addZ( outType );
    hasZ = true;
  }
  bool hasM = false;
  if ( QgsWkbTypes::hasM( hubSource->wkbType() ) || QgsWkbTypes::hasM( spokeSource->wkbType() ) )
  {
    outType = QgsWkbTypes::addM( outType );
    hasM = true;
  }

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest, fields,
                                          outType, hubSource->sourceCrs() ) );
  if ( !sink )
    return QVariantMap();

  auto getPointFromFeature = [hasZ, hasM]( const QgsFeature & feature )->QgsPoint
  {
    QgsPoint p;
    if ( feature.geometry().type() == QgsWkbTypes::PointGeometry && !feature.geometry().isMultipart() )
      p = *static_cast< QgsPoint *>( feature.geometry().geometry() );
    else
      p = *static_cast< QgsPoint *>( feature.geometry().pointOnSurface().geometry() );
    if ( hasZ && !p.is3D() )
      p.addZValue( 0 );
    if ( hasM && !p.isMeasure() )
      p.addMValue( 0 );
    return p;
  };

  QgsFeatureIterator hubFeatures = hubSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( hubFields2Fetch ) );
  double step = hubSource->featureCount() > 0 ? 100.0 / hubSource->featureCount() : 1;
  int i = 0;
  QgsFeature hubFeature;
  while ( hubFeatures.nextFeature( hubFeature ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( i * step );

    if ( !hubFeature.hasGeometry() )
      continue;

    QgsPoint hubPoint = getPointFromFeature( hubFeature );

    // only keep selected attributes
    QgsAttributes hubAttributes;
    for ( int j = 0; j < hubFeature.attributes().count(); ++j )
    {
      if ( !hubFieldIndices.contains( j ) )
        continue;
      hubAttributes << hubFeature.attribute( j );
    }

    QgsFeatureRequest spokeRequest = QgsFeatureRequest().setDestinationCrs( hubSource->sourceCrs() );
    spokeRequest.setSubsetOfAttributes( spokeFields2Fetch );
    spokeRequest.setFilterExpression( QgsExpression::createFieldEqualityExpression( fieldSpokeName, hubFeature.attribute( fieldHubIndex ) ) );

    QgsFeatureIterator spokeFeatures = spokeSource->getFeatures( spokeRequest );
    QgsFeature spokeFeature;
    while ( spokeFeatures.nextFeature( spokeFeature ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }
      if ( !spokeFeature.hasGeometry() )
        continue;

      QgsPoint spokePoint = getPointFromFeature( spokeFeature );
      QgsGeometry line( new QgsLineString( QVector< QgsPoint >() << hubPoint << spokePoint ) );

      QgsFeature outFeature;
      QgsAttributes outAttributes = hubAttributes;

      // only keep selected attributes
      QgsAttributes spokeAttributes;
      for ( int j = 0; j < spokeFeature.attributes().count(); ++j )
      {
        if ( !spokeFieldIndices.contains( j ) )
          continue;
        spokeAttributes << spokeFeature.attribute( j );
      }

      outAttributes.append( spokeAttributes );
      outFeature.setAttributes( outAttributes );
      outFeature.setGeometry( line );
      sink->addFeature( outFeature, QgsFeatureSink::FastInsert );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


///@endcond
