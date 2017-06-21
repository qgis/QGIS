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
#include "qgsgeometry.h"
#include "qgsgeometryengine.h"
#include "qgswkbtypes.h"

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
  addAlgorithm( new QgsExtractByAttributeAlgorithm() );
  addAlgorithm( new QgsExtractByExpressionAlgorithm() );
  addAlgorithm( new QgsMultipartToSinglepartAlgorithm() );
  addAlgorithm( new QgsSubdivideAlgorithm() );
  addAlgorithm( new QgsTransformAlgorithm() );
}



QgsCentroidAlgorithm::QgsCentroidAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Centroids" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Centroids" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
}

QString QgsCentroidAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new point layer, with points representing the centroid of the geometries in an input layer.\n\n"
                      "The attributes associated to each point in the output layer are the same ones associated to the original features." );
}

QVariantMap QgsCentroidAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LAYER" ), context, source->fields(), QgsWkbTypes::Point, source->sourceCrs(), dest ) );
  if ( !sink )
    return QVariantMap();

  long count = source->featureCount();
  if ( count <= 0 )
    return QVariantMap();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = 100.0 / count;
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
      out.setGeometry( f.geometry().centroid() );
      if ( !out.geometry() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating centroid for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), QgsMessageLog::WARNING );
      }
    }
    sink->addFeature( out );

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT_LAYER" ), dest );
  return outputs;
}

//
// QgsBufferAlgorithm
//

QgsBufferAlgorithm::QgsBufferAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "DISTANCE" ), QObject::tr( "Distance" ), QgsProcessingParameterNumber::Double, 10 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "SEGMENTS" ), QObject::tr( "Segments" ), QgsProcessingParameterNumber::Integer, 5, false, 1 ) );

  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "END_CAP_STYLE" ), QObject::tr( "End cap style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Flat" ) << QObject::tr( "Square" ), false ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "JOIN_STYLE" ), QObject::tr( "Join style" ), QStringList() << QObject::tr( "Round" ) << QObject::tr( "Miter" ) << QObject::tr( "Bevel" ), false ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MITRE_LIMIT" ), QObject::tr( "Miter limit" ), QgsProcessingParameterNumber::Double, 2, false, 1 ) );

  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "DISSOLVE" ), QObject::tr( "Dissolve result" ), false ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Buffered" ), QgsProcessingParameterDefinition::TypeVectorPolygon ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT_LAYER" ), QObject::tr( "Buffered" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
}

QString QgsBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a buffer area for all the features in an input layer, using a fixed or dynamic distance.\n\n"
                      "The segments parameter controls the number of line segments to use to approximate a quarter circle when creating rounded offsets.\n\n"
                      "The end cap style parameter controls how line endings are handled in the buffer.\n\n"
                      "The join style parameter specifies whether round, mitre or beveled joins should be used when offsetting corners in a line.\n\n"
                      "The mitre limit parameter is only applicable for mitre join styles, and controls the maximum distance from the offset curve to use when creating a mitred join." );
}

QVariantMap QgsBufferAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT_LAYER" ), context, source->fields(), QgsWkbTypes::Polygon, source->sourceCrs(), dest ) );
  if ( !sink )
    return QVariantMap();

  // fixed parameters
  bool dissolve = parameterAsBool( parameters, QStringLiteral( "DISSOLVE" ), context );
  int segments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  QgsGeometry::EndCapStyle endCapStyle = static_cast< QgsGeometry::EndCapStyle >( 1 + parameterAsInt( parameters, QStringLiteral( "END_CAP_STYLE" ), context ) );
  QgsGeometry::JoinStyle joinStyle = static_cast< QgsGeometry::JoinStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  double miterLimit = parameterAsDouble( parameters, QStringLiteral( "MITRE_LIMIT" ), context );
  double bufferDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  bool dynamicBuffer = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  const QgsProcessingParameterDefinition *distanceParamDef = parameterDefinition( QStringLiteral( "DISTANCE" ) );

  long count = source->featureCount();
  if ( count <= 0 )
    return QVariantMap();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = 100.0 / count;
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
      sink->addFeature( out );

    feedback->setProgress( current * step );
    current++;
  }

  if ( dissolve )
  {
    QgsGeometry finalGeometry = QgsGeometry::unaryUnion( bufferedGeometriesForDissolve );
    QgsFeature f;
    f.setGeometry( finalGeometry );
    f.setAttributes( dissolveAttrs );
    sink->addFeature( f );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT_LAYER" ), dest );
  return outputs;
}


QgsDissolveAlgorithm::QgsDissolveAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterTableField( QStringLiteral( "FIELD" ), QObject::tr( "Unique ID fields" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterTableField::Any, true, true ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Dissolved" ) ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Dissolved" ) ) );
}

QString QgsDissolveAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon or line vector layer and combines their geometries into new geometries. One or more attributes can "
                      "be specified to dissolve only geometries belonging to the same class (having the same value for the specified attributes), alternatively "
                      "all geometries can be dissolved.\n\n"
                      "If the geometries to be dissolved are spatially separated from each other the output will be multi geometries. "
                      "In case the input is a polygon layer, common boundaries of adjacent polygons being dissolved will get erased." );
}

QVariantMap QgsDissolveAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, source->fields(), QgsWkbTypes::multiType( source->wkbType() ), source->sourceCrs(), dest ) );

  if ( !sink )
    return QVariantMap();

  QStringList fields = parameterAsFields( parameters, QStringLiteral( "FIELD" ), context );

  long count = source->featureCount();
  if ( count <= 0 )
    return QVariantMap();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = 100.0 / count;
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
        if ( geomQueue.length() > 10000 )
        {
          // queue too long, combine it
          QgsGeometry tempOutputGeometry = QgsGeometry::unaryUnion( geomQueue );
          geomQueue.clear();
          geomQueue << tempOutputGeometry;
        }
      }

      feedback->setProgress( current * step );
      current++;
    }

    outputFeature.setGeometry( QgsGeometry::unaryUnion( geomQueue ) );
    sink->addFeature( outputFeature );
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

      if ( f.hasGeometry() && f.geometry() )
      {
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
        geometryHash[ indexAttributes ].append( f.geometry() );
      }
    }

    int numberFeatures = attributeHash.count();
    QHash< QVariant, QList< QgsGeometry > >::const_iterator geomIt = geometryHash.constBegin();
    for ( ; geomIt != geometryHash.constEnd(); ++geomIt )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      QgsFeature outputFeature;
      outputFeature.setGeometry( QgsGeometry::unaryUnion( geomIt.value() ) );
      outputFeature.setAttributes( attributeHash.value( geomIt.key() ) );
      sink->addFeature( outputFeature );

      feedback->setProgress( current * 100.0 / numberFeatures );
      current++;
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}

QgsClipAlgorithm::QgsClipAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "OVERLAY" ), QObject::tr( "Clip layer" ), QList< int >() << QgsProcessingParameterDefinition::TypeVectorPolygon ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Clipped" ) ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Clipped" ) ) );
}

QString QgsClipAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm clips a vector layer using the polygons of an additional polygons layer. Only the parts of the features "
                      "in the input layer that falls within the polygons of the clipping layer will be added to the resulting layer.\n\n"
                      "The attributes of the features are not modified, although properties such as area or length of the features will "
                      "be modified by the clipping operation. If such properties are stored as attributes, those attributes will have to "
                      "be manually updated." );
}

QVariantMap QgsClipAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > featureSource( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !featureSource )
    return QVariantMap();

  std::unique_ptr< QgsFeatureSource > maskSource( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !maskSource )
    return QVariantMap();

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, featureSource->fields(), QgsWkbTypes::multiType( featureSource->wkbType() ), featureSource->sourceCrs(), dest ) );

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

  if ( clipGeoms.isEmpty() )
    return QVariantMap();

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

      if ( !engine->intersects( *inputFeature.geometry().geometry() ) )
        continue;

      QgsGeometry newGeometry;
      if ( !engine->contains( *inputFeature.geometry().geometry() ) )
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
      sink->addFeature( outputFeature );


      if ( singleClipFeature )
        feedback->setProgress( current * step );
    }

    if ( !singleClipFeature )
    {
      // coarse progress report for multiple clip geometries
      feedback->setProgress( 100.0 * static_cast< double >( i ) / clipGeoms.length() );
    }
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


QgsTransformAlgorithm::QgsTransformAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "TARGET_CRS" ), QObject::tr( "Target CRS" ), QStringLiteral( "EPSG:4326" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Reprojected" ) ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Reprojected" ) ) );
}

QString QgsTransformAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reprojects a vector layer. It creates a new layer with the same features "
                      "as the input one, but with geometries reprojected to a new CRS.\n\n"
                      "Attributes are not modified by this algorithm." );
}

QVariantMap QgsTransformAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QgsCoordinateReferenceSystem targetCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, source->fields(), source->wkbType(), targetCrs, dest ) );
  if ( !sink )
    return QVariantMap();

  long count = source->featureCount();
  if ( count <= 0 )
    return QVariantMap();

  QgsFeature f;
  QgsFeatureRequest req;
  // perform reprojection in the iterators...
  req.setDestinationCrs( targetCrs );

  QgsFeatureIterator it = source->getFeatures( req );

  double step = 100.0 / count;
  int current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    sink->addFeature( f );
    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


QgsSubdivideAlgorithm::QgsSubdivideAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "MAX_NODES" ), QObject::tr( "Maximum nodes in parts" ), QgsProcessingParameterNumber::Integer,
                256, false, 8, 100000 ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Subdivided" ) ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Subdivided" ) ) );
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

QVariantMap QgsSubdivideAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  int maxNodes = parameterAsInt( parameters, QStringLiteral( "MAX_NODES" ), context );
  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, source->fields(),
                                          QgsWkbTypes::multiType( source->wkbType() ), source->sourceCrs(), dest ) );
  if ( !sink )
    return QVariantMap();

  long count = source->featureCount();
  if ( count <= 0 )
    return QVariantMap();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = 100.0 / count;
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
      out.setGeometry( f.geometry().subdivide( maxNodes ) );
      if ( !out.geometry() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating subdivision for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), QgsMessageLog::WARNING );
      }
    }
    sink->addFeature( out );

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}



QgsMultipartToSinglepartAlgorithm::QgsMultipartToSinglepartAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Single parts" ) ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Single parts" ) ) );
}

QString QgsMultipartToSinglepartAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a vector layer with multipart geometries and generates a new one in which all geometries contain "
                      "a single part. Features with multipart geometries are divided in as many different features as parts the geometry "
                      "contain, and the same attributes are used for each of them." );
}

QVariantMap QgsMultipartToSinglepartAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QgsWkbTypes::Type sinkType = QgsWkbTypes::singleType( source->wkbType() );

  QString dest;
  std::unique_ptr< QgsFeatureSink > sink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, source->fields(),
                                          sinkType, source->sourceCrs(), dest ) );
  if ( !sink )
    return QVariantMap();

  long count = source->featureCount();
  if ( count <= 0 )
    return QVariantMap();

  QgsFeature f;
  QgsFeatureIterator it = source->getFeatures();

  double step = 100.0 / count;
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
          sink->addFeature( out );
        }
      }
      else
      {
        sink->addFeature( out );
      }
    }
    else
    {
      // feature with null geometry
      sink->addFeature( out );
    }

    feedback->setProgress( current * step );
    current++;
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest );
  return outputs;
}


QgsExtractByExpressionAlgorithm::QgsExtractByExpressionAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterExpression( QStringLiteral( "EXPRESSION" ), QObject::tr( "Expression" ), QVariant(), QStringLiteral( "INPUT" ) ) );

  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Matching features" ) ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ),  QObject::tr( "Matching (expression)" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "FAIL_OUTPUT" ),  QObject::tr( "Non-matching" ),
                QgsProcessingParameterDefinition::TypeVectorAny, QVariant(), true ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "FAIL_OUTPUT" ),  QObject::tr( "Non-matching (expression)" ) ) );
}

QString QgsExtractByExpressionAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new vector layer that only contains matching features from an input layer. "
                      "The criteria for adding features to the resulting layer is based on a QGIS expression.\n\n"
                      "For more information about expressions see the <a href =\"{qgisdocs}/user_manual/working_with_vector/expression.html\">user manual</a>" );
}

QVariantMap QgsExtractByExpressionAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString expressionString = parameterAsExpression( parameters, QStringLiteral( "EXPRESSION" ), context );

  QString matchingSinkId;
  std::unique_ptr< QgsFeatureSink > matchingSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, source->fields(),
      source->wkbType(), source->sourceCrs(), matchingSinkId ) );
  if ( !matchingSink )
    return QVariantMap();

  QString nonMatchingSinkId;
  std::unique_ptr< QgsFeatureSink > nonMatchingSink( parameterAsSink( parameters, QStringLiteral( "FAIL_OUTPUT" ), context, source->fields(),
      source->wkbType(), source->sourceCrs(), nonMatchingSinkId ) );

  QgsExpression expression( expressionString );
  if ( expression.hasParserError() )
  {
    // raise GeoAlgorithmExecutionException(expression.parserErrorString())
    return QVariantMap();
  }

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context );

  long count = source->featureCount();
  if ( count <= 0 )
    return QVariantMap();

  double step = 100.0 / count;
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

      matchingSink->addFeature( f );

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
        matchingSink->addFeature( f );
      }
      else
      {
        nonMatchingSink->addFeature( f );
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


QgsExtractByAttributeAlgorithm::QgsExtractByAttributeAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterTableField( QStringLiteral( "FIELD" ), QObject::tr( "Selection attribute" ), QVariant(), QStringLiteral( "INPUT" ) ) );
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
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ),  QObject::tr( "Matching (attribute)" ) ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "FAIL_OUTPUT" ),  QObject::tr( "Extracted (non-matching)" ),
                QgsProcessingParameterDefinition::TypeVectorAny, QVariant(), true ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "FAIL_OUTPUT" ),  QObject::tr( "Non-matching (attribute)" ) ) );
}

QString QgsExtractByAttributeAlgorithm::shortHelpString() const
{
  return QObject::tr( "  This algorithm creates a new vector layer that only contains matching features from an input layer. "
                      "The criteria for adding features to the resulting layer is defined based on the values "
                      "of an attribute from the input layer." );
}

QVariantMap QgsExtractByAttributeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) const
{
  std::unique_ptr< QgsFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    return QVariantMap();

  QString fieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  Operation op = static_cast< Operation >( parameterAsEnum( parameters, QStringLiteral( "OPERATOR" ), context ) );
  QString value = parameterAsString( parameters, QStringLiteral( "VALUE" ), context );

  QString matchingSinkId;
  std::unique_ptr< QgsFeatureSink > matchingSink( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, source->fields(),
      source->wkbType(), source->sourceCrs(), matchingSinkId ) );
  if ( !matchingSink )
    return QVariantMap();

  QString nonMatchingSinkId;
  std::unique_ptr< QgsFeatureSink > nonMatchingSink( parameterAsSink( parameters, QStringLiteral( "FAIL_OUTPUT" ), context, source->fields(),
      source->wkbType(), source->sourceCrs(), nonMatchingSinkId ) );


  int idx = source->fields().lookupField( fieldName );
  QVariant::Type fieldType = source->fields().at( idx ).type();

  if ( fieldType != QVariant::String && ( op == BeginsWith || op == Contains || op == DoesNotContain ) )
  {
#if 0
    op = ''.join( ['"%s", ' % o for o in self.STRING_OPERATORS] )
         raise GeoAlgorithmExecutionException(
           self.tr( 'Operators {0} can be used only with string fields.' ).format( op ) )
#endif
         return QVariantMap();
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
    // raise GeoAlgorithmExecutionException(expression.parserErrorString())
    return QVariantMap();
  }

  QgsExpressionContext expressionContext = createExpressionContext( parameters, context );

  long count = source->featureCount();
  if ( count <= 0 )
    return QVariantMap();

  double step = 100.0 / count;
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

      matchingSink->addFeature( f );

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
        matchingSink->addFeature( f );
      }
      else
      {
        nonMatchingSink->addFeature( f );
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

///@endcond
