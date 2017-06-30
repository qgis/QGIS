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
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Centroids" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Centroids" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
}

QString QgsCentroidAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a new point layer, with points representing the centroid of the geometries in an input layer.\n\n"
                      "The attributes associated to each point in the output layer are the same ones associated to the original features." );
}

QgsCentroidAlgorithm *QgsCentroidAlgorithm::create() const
{
  return new QgsCentroidAlgorithm();
}

bool QgsCentroidAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return false;

  mSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mSinkId, mSource->fields(), QgsWkbTypes::Point, mSource->sourceCrs() ) );
  if ( !mSink )
    return false;

  return true;
}

bool QgsCentroidAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  long count = mSource->featureCount();
  if ( count == 0 )
    return true;

  QgsFeature f;
  QgsFeatureIterator it = mSource->getFeatures();

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
    mSink->addFeature( out, QgsFeatureSink::FastInsert );

    feedback->setProgress( current * step );
    current++;
  }
  mSink->flushBuffer();
  return true;
}

QVariantMap QgsCentroidAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mSinkId );
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
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Buffered" ), QgsProcessingParameterDefinition::TypeVectorPolygon ) );
  addOutput( new QgsProcessingOutputVectorLayer( QStringLiteral( "OUTPUT" ), QObject::tr( "Buffered" ), QgsProcessingParameterDefinition::TypeVectorPoint ) );
}

QString QgsBufferAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes a buffer area for all the features in an input layer, using a fixed or dynamic distance.\n\n"
                      "The segments parameter controls the number of line segments to use to approximate a quarter circle when creating rounded offsets.\n\n"
                      "The end cap style parameter controls how line endings are handled in the buffer.\n\n"
                      "The join style parameter specifies whether round, mitre or beveled joins should be used when offsetting corners in a line.\n\n"
                      "The mitre limit parameter is only applicable for mitre join styles, and controls the maximum distance from the offset curve to use when creating a mitred join." );
}

QgsBufferAlgorithm *QgsBufferAlgorithm::create() const
{
  return new QgsBufferAlgorithm();
}

bool QgsBufferAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return false;

  mSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mSinkId, mSource->fields(), QgsWkbTypes::Polygon, mSource->sourceCrs() ) );
  if ( !mSink )
    return false;

  // fixed parameters
  mDissolve = parameterAsBool( parameters, QStringLiteral( "DISSOLVE" ), context );
  mSegments = parameterAsInt( parameters, QStringLiteral( "SEGMENTS" ), context );
  mEndCapStyle = static_cast< QgsGeometry::EndCapStyle >( 1 + parameterAsInt( parameters, QStringLiteral( "END_CAP_STYLE" ), context ) );
  mJoinStyle = static_cast< QgsGeometry::JoinStyle>( 1 + parameterAsInt( parameters, QStringLiteral( "JOIN_STYLE" ), context ) );
  mMiterLimit = parameterAsDouble( parameters, QStringLiteral( "MITRE_LIMIT" ), context );
  mBufferDistance = parameterAsDouble( parameters, QStringLiteral( "DISTANCE" ), context );
  mDynamicBuffer = QgsProcessingParameters::isDynamic( parameters, QStringLiteral( "DISTANCE" ) );
  if ( mDynamicBuffer )
  {
    mDynamicBufferProperty = parameters.value( QStringLiteral( "DISTANCE" ) ).value< QgsProperty >();
    mExpContext = context.expressionContext();
    mDefaultBuffer = parameterDefinition( QStringLiteral( "DISTANCE" ) )->defaultValue().toDouble();
  }
  return true;
}

bool QgsBufferAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  long count = mSource->featureCount();
  if ( count == 0 )
    return false;

  QgsFeature f;
  QgsFeatureIterator it = mSource->getFeatures();

  double step = 100.0 / count;
  int current = 0;

  double bufferDistance = mBufferDistance;

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
      if ( mDynamicBuffer )
      {
        mExpContext.setFeature( f );
        bufferDistance = mDynamicBufferProperty.valueAsDouble( mExpContext, mDefaultBuffer );
      }

      QgsGeometry outputGeometry = f.geometry().buffer( bufferDistance, mSegments, mEndCapStyle, mJoinStyle, mMiterLimit );
      if ( !outputGeometry )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating buffer for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), QgsMessageLog::WARNING );
      }
      if ( mDissolve )
        bufferedGeometriesForDissolve << outputGeometry;
      else
        out.setGeometry( outputGeometry );
    }

    if ( !mDissolve )
      mSink->addFeature( out, QgsFeatureSink::FastInsert );

    feedback->setProgress( current * step );
    current++;
  }

  if ( mDissolve )
  {
    QgsGeometry finalGeometry = QgsGeometry::unaryUnion( bufferedGeometriesForDissolve );
    QgsFeature f;
    f.setGeometry( finalGeometry );
    f.setAttributes( dissolveAttrs );
    mSink->addFeature( f, QgsFeatureSink::FastInsert );
  }

  mSink->flushBuffer();
  return true;
}

QVariantMap QgsBufferAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mSinkId );
  return outputs;
}

QgsDissolveAlgorithm::QgsDissolveAlgorithm()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ), QObject::tr( "Unique ID fields" ), QVariant(),
                QStringLiteral( "INPUT" ), QgsProcessingParameterField::Any, true, true ) );

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

QgsDissolveAlgorithm *QgsDissolveAlgorithm::create() const
{
  return new QgsDissolveAlgorithm();
}

bool QgsDissolveAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return false;

  mSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mSinkId, mSource->fields(), QgsWkbTypes::multiType( mSource->wkbType() ), mSource->sourceCrs() ) );

  if ( !mSink )
    return false;

  mFields = parameterAsFields( parameters, QStringLiteral( "FIELD" ), context );
  return true;
}

bool QgsDissolveAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  long count = mSource->featureCount();
  if ( count == 0 )
    return true;

  QgsFeature f;
  QgsFeatureIterator it = mSource->getFeatures();

  double step = 100.0 / count;
  int current = 0;

  if ( mFields.isEmpty() )
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
    mSink->addFeature( outputFeature, QgsFeatureSink::FastInsert );
  }
  else
  {
    QList< int > fieldIndexes;
    Q_FOREACH ( const QString &field, mFields )
    {
      int index = mSource->fields().lookupField( field );
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
      mSink->addFeature( outputFeature, QgsFeatureSink::FastInsert );

      feedback->setProgress( current * 100.0 / numberFeatures );
      current++;
    }
  }

  mSink->flushBuffer();
  return true;
}

QVariantMap QgsDissolveAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mSinkId );
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

QgsClipAlgorithm *QgsClipAlgorithm::create() const
{
  return new QgsClipAlgorithm();
}

bool QgsClipAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mFeatureSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mFeatureSource )
    return false;

  mMaskSource.reset( parameterAsSource( parameters, QStringLiteral( "OVERLAY" ), context ) );
  if ( !mMaskSource )
    return false;

  mSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mSinkId, mFeatureSource->fields(), QgsWkbTypes::multiType( mFeatureSource->wkbType() ), mFeatureSource->sourceCrs() ) );

  if ( !mSink )
    return false;

  return true;
}

bool QgsClipAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  // first build up a list of clip geometries
  QList< QgsGeometry > clipGeoms;
  QgsFeatureIterator it = mMaskSource->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QList< int >() ).setDestinationCrs( mFeatureSource->sourceCrs() ) );
  QgsFeature f;
  while ( it.nextFeature( f ) )
  {
    if ( f.hasGeometry() )
      clipGeoms << f.geometry();
  }

  if ( clipGeoms.isEmpty() )
    return true;

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

    QgsFeatureIterator inputIt = mFeatureSource->getFeatures( QgsFeatureRequest().setFilterRect( clipGeom.boundingBox() ) );
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
      mSink->addFeature( outputFeature, QgsFeatureSink::FastInsert );

      if ( singleClipFeature )
        feedback->setProgress( current * step );
    }

    if ( !singleClipFeature )
    {
      // coarse progress report for multiple clip geometries
      feedback->setProgress( 100.0 * static_cast< double >( i ) / clipGeoms.length() );
    }
  }
  mSink->flushBuffer();
  return true;
}

QVariantMap QgsClipAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mSinkId );
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

QgsTransformAlgorithm *QgsTransformAlgorithm::create() const
{
  return new QgsTransformAlgorithm();
}

bool QgsTransformAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return false;

  mCrs = parameterAsCrs( parameters, QStringLiteral( "TARGET_CRS" ), context );

  mSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mSinkId, mSource->fields(), mSource->wkbType(), mCrs ) );
  if ( !mSink )
    return false;

  return true;
}

QVariantMap QgsTransformAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mSinkId );
  return outputs;
}

bool QgsTransformAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  long count = mSource->featureCount();
  if ( count == 0 )
    return true;

  QgsFeature f;
  QgsFeatureRequest req;
  // perform reprojection in the iterators...
  req.setDestinationCrs( mCrs );

  QgsFeatureIterator it = mSource->getFeatures( req );

  double step = 100.0 / count;
  int current = 0;
  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    mSink->addFeature( f, QgsFeatureSink::FastInsert );
    feedback->setProgress( current * step );
    current++;
  }

  return true;
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

QgsSubdivideAlgorithm *QgsSubdivideAlgorithm::create() const
{
  return new QgsSubdivideAlgorithm();
}

bool QgsSubdivideAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return false;

  mMaxNodes = parameterAsInt( parameters, QStringLiteral( "MAX_NODES" ), context );
  mSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mSinkId, mSource->fields(),
                                QgsWkbTypes::multiType( mSource->wkbType() ), mSource->sourceCrs() ) );
  if ( !mSink )
    return false;

  return true;
}

bool QgsSubdivideAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  long count = mSource->featureCount();
  if ( count == 0 )
    return true;

  QgsFeature f;
  QgsFeatureIterator it = mSource->getFeatures();

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
      out.setGeometry( f.geometry().subdivide( mMaxNodes ) );
      if ( !out.geometry() )
      {
        QgsMessageLog::logMessage( QObject::tr( "Error calculating subdivision for feature %1" ).arg( f.id() ), QObject::tr( "Processing" ), QgsMessageLog::WARNING );
      }
    }
    mSink->addFeature( out, QgsFeatureSink::FastInsert );

    feedback->setProgress( current * step );
    current++;
  }
  mSink->flushBuffer();
  return true;
}

QVariantMap QgsSubdivideAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mSinkId );
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

QgsMultipartToSinglepartAlgorithm *QgsMultipartToSinglepartAlgorithm::create() const
{
  return new QgsMultipartToSinglepartAlgorithm();
}

bool QgsMultipartToSinglepartAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return false;

  QgsWkbTypes::Type sinkType = QgsWkbTypes::singleType( mSource->wkbType() );

  mSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mSinkId, mSource->fields(),
                                sinkType, mSource->sourceCrs() ) );
  if ( !mSink )
    return false;

  return true;
}

bool QgsMultipartToSinglepartAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  long count = mSource->featureCount();
  if ( count == 0 )
    return true;

  QgsFeature f;
  QgsFeatureIterator it = mSource->getFeatures();

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
          mSink->addFeature( out, QgsFeatureSink::FastInsert );
        }
      }
      else
      {
        mSink->addFeature( out, QgsFeatureSink::FastInsert );
      }
    }
    else
    {
      // feature with null geometry
      mSink->addFeature( out, QgsFeatureSink::FastInsert );
    }

    feedback->setProgress( current * step );
    current++;
  }
  mSink->flushBuffer();
  return true;
}

QVariantMap QgsMultipartToSinglepartAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mSinkId );
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

QgsExtractByExpressionAlgorithm *QgsExtractByExpressionAlgorithm::create() const
{
  return new QgsExtractByExpressionAlgorithm();
}

bool QgsExtractByExpressionAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return false;

  mExpressionString = parameterAsExpression( parameters, QStringLiteral( "EXPRESSION" ), context );

  mMatchingSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mMatchingSinkId, mSource->fields(),
                                        mSource->wkbType(), mSource->sourceCrs() ) );
  if ( !mMatchingSink )
    return false;

  mNonMatchingSink.reset( parameterAsSink( parameters, QStringLiteral( "FAIL_OUTPUT" ), context, mNonMatchingSinkId, mSource->fields(),
                          mSource->wkbType(), mSource->sourceCrs() ) );

  mExpressionContext = createExpressionContext( parameters, context );
  return true;
}

bool QgsExtractByExpressionAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  QgsExpression expression( mExpressionString );
  if ( expression.hasParserError() )
  {
    throw QgsProcessingException( expression.parserErrorString() );
  }


  long count = mSource->featureCount();
  if ( count == 0 )
    return true;

  double step = 100.0 / count;
  int current = 0;

  if ( !mNonMatchingSink )
  {
    // not saving failing features - so only fetch good features
    QgsFeatureRequest req;
    req.setFilterExpression( mExpressionString );
    req.setExpressionContext( mExpressionContext );

    QgsFeatureIterator it = mSource->getFeatures( req );
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      mMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );

      feedback->setProgress( current * step );
      current++;
    }
  }
  else
  {
    // saving non-matching features, so we need EVERYTHING
    mExpressionContext.setFields( mSource->fields() );
    expression.prepare( &mExpressionContext );

    QgsFeatureIterator it = mSource->getFeatures();
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      mExpressionContext.setFeature( f );
      if ( expression.evaluate( &mExpressionContext ).toBool() )
      {
        mMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }
      else
      {
        mNonMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }

      feedback->setProgress( current * step );
      current++;
    }
  }
  if ( mMatchingSink )
    mMatchingSink->flushBuffer();
  if ( mNonMatchingSink )
    mNonMatchingSink->flushBuffer();
  return true;
}


QVariantMap QgsExtractByExpressionAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mMatchingSinkId );
  if ( !mNonMatchingSinkId.isEmpty() )
    outputs.insert( QStringLiteral( "FAIL_OUTPUT" ), mNonMatchingSinkId );
  return outputs;
}

QgsExtractByAttributeAlgorithm::QgsExtractByAttributeAlgorithm()
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

QgsExtractByAttributeAlgorithm *QgsExtractByAttributeAlgorithm::create() const
{
  return new QgsExtractByAttributeAlgorithm();
}

bool QgsExtractByAttributeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    return false;

  mFieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  mOp = static_cast< Operation >( parameterAsEnum( parameters, QStringLiteral( "OPERATOR" ), context ) );
  mValue = parameterAsString( parameters, QStringLiteral( "VALUE" ), context );

  mMatchingSink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, mMatchingSinkId, mSource->fields(),
                                        mSource->wkbType(), mSource->sourceCrs() ) );
  if ( !mMatchingSink )
    return false;

  mNonMatchingSink.reset( parameterAsSink( parameters, QStringLiteral( "FAIL_OUTPUT" ), context, mNonMatchingSinkId, mSource->fields(),
                          mSource->wkbType(), mSource->sourceCrs() ) );


  mExpressionContext = createExpressionContext( parameters, context );
  return true;
}

bool QgsExtractByAttributeAlgorithm::processAlgorithm( QgsProcessingContext &, QgsProcessingFeedback *feedback )
{
  int idx = mSource->fields().lookupField( mFieldName );
  QVariant::Type fieldType = mSource->fields().at( idx ).type();

  if ( fieldType != QVariant::String && ( mOp == BeginsWith || mOp == Contains || mOp == DoesNotContain ) )
  {
    QString method;
    switch ( mOp )
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

  QString fieldRef = QgsExpression::quotedColumnRef( mFieldName );
  QString quotedVal = QgsExpression::quotedValue( mValue );
  QString expr;
  switch ( mOp )
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
      expr = QStringLiteral( "%1 LIKE '%2%'" ).arg( fieldRef, mValue );
      break;
    case Contains:
      expr = QStringLiteral( "%1 LIKE '%%2%'" ).arg( fieldRef, mValue );
      break;
    case IsNull:
      expr = QStringLiteral( "%1 IS NULL" ).arg( fieldRef );
      break;
    case IsNotNull:
      expr = QStringLiteral( "%1 IS NOT NULL" ).arg( fieldRef );
      break;
    case DoesNotContain:
      expr = QStringLiteral( "%1 NOT LIKE '%%2%'" ).arg( fieldRef, mValue );
      break;
  }

  QgsExpression expression( expr );
  if ( expression.hasParserError() )
  {
    throw QgsProcessingException( expression.parserErrorString() );
  }

  long count = mSource->featureCount();
  if ( count == 0 )
    return true;

  double step = 100.0 / count;
  int current = 0;

  if ( !mNonMatchingSink )
  {
    // not saving failing features - so only fetch good features
    QgsFeatureRequest req;
    req.setFilterExpression( expr );
    req.setExpressionContext( mExpressionContext );

    QgsFeatureIterator it = mSource->getFeatures( req );
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      mMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );

      feedback->setProgress( current * step );
      current++;
    }
  }
  else
  {
    // saving non-matching features, so we need EVERYTHING
    mExpressionContext.setFields( mSource->fields() );
    expression.prepare( &mExpressionContext );

    QgsFeatureIterator it = mSource->getFeatures();
    QgsFeature f;
    while ( it.nextFeature( f ) )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      mExpressionContext.setFeature( f );
      if ( expression.evaluate( &mExpressionContext ).toBool() )
      {
        mMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }
      else
      {
        mNonMatchingSink->addFeature( f, QgsFeatureSink::FastInsert );
      }

      feedback->setProgress( current * step );
      current++;
    }
  }

  if ( mMatchingSink )
    mMatchingSink->flushBuffer();
  if ( mNonMatchingSink )
    mNonMatchingSink->flushBuffer();
  return true;
}

QVariantMap QgsExtractByAttributeAlgorithm::postProcessAlgorithm( QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), mMatchingSinkId );
  if ( !mNonMatchingSinkId.isEmpty() )
    outputs.insert( QStringLiteral( "FAIL_OUTPUT" ), mNonMatchingSinkId );
  return outputs;
}


///@endcond
