/***************************************************************************
                         qgsalgorithmangletonearest.cpp
                         ---------------------
    begin                : July 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#include "qgsalgorithmangletonearest.h"

#include "qgslinestring.h"
#include "qgsmarkersymbol.h"
#include "qgsrenderer.h"
#include "qgsspatialindex.h"
#include "qgsstyleentityvisitor.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

class SetMarkerRotationVisitor : public QgsStyleEntityVisitorInterface
{
  public:
    SetMarkerRotationVisitor( const QString &rotationField )
      : mRotationField( rotationField )
    {}

    bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &entity ) override
    {
      if ( const QgsStyleSymbolEntity *symbolEntity = dynamic_cast<const QgsStyleSymbolEntity *>( entity.entity ) )
      {
        if ( QgsMarkerSymbol *marker = dynamic_cast<QgsMarkerSymbol *>( symbolEntity->symbol() ) )
        {
          marker->setDataDefinedAngle( QgsProperty::fromField( mRotationField ) );
        }
      }
      return true;
    }

  private:
    QString mRotationField;
};

class SetMarkerRotationPostProcessor : public QgsProcessingLayerPostProcessorInterface
{
  public:
    SetMarkerRotationPostProcessor( std::unique_ptr<QgsFeatureRenderer> renderer, const QString &rotationField )
      : mRenderer( std::move( renderer ) )
      , mRotationField( rotationField )
    {}

    void postProcessLayer( QgsMapLayer *layer, QgsProcessingContext &, QgsProcessingFeedback * ) override
    {
      if ( QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( layer ) )
      {
        SetMarkerRotationVisitor visitor( mRotationField );
        mRenderer->accept( &visitor );
        vl->setRenderer( mRenderer.release() );
        vl->triggerRepaint();
      }
    }

  private:
    std::unique_ptr<QgsFeatureRenderer> mRenderer;
    QString mRotationField;
};

QString QgsAngleToNearestAlgorithm::name() const
{
  return u"angletonearest"_s;
}

QString QgsAngleToNearestAlgorithm::displayName() const
{
  return QObject::tr( "Align points to features" );
}

QStringList QgsAngleToNearestAlgorithm::tags() const
{
  return QObject::tr( "align,marker,stroke,fill,orient,points,lines,angles,rotation,rotate" ).split( ',' );
}

QString QgsAngleToNearestAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsAngleToNearestAlgorithm::groupId() const
{
  return u"cartography"_s;
}

QgsAngleToNearestAlgorithm::~QgsAngleToNearestAlgorithm() = default;

void QgsAngleToNearestAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  mIsInPlace = configuration.value( u"IN_PLACE"_s ).toBool();

  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT"_s, QObject::tr( "Input layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"REFERENCE_LAYER"_s, QObject::tr( "Reference layer" ) ) );

  addParameter( new QgsProcessingParameterDistance( u"MAX_DISTANCE"_s, QObject::tr( "Maximum distance to consider" ), QVariant(), u"INPUT"_s, true, 0 ) );

  if ( !mIsInPlace )
    addParameter( new QgsProcessingParameterString( u"FIELD_NAME"_s, QObject::tr( "Angle field name" ), u"rotation"_s ) );
  else
    addParameter( new QgsProcessingParameterField( u"FIELD_NAME"_s, QObject::tr( "Angle field name" ), u"rotation"_s, u"INPUT"_s ) );

  addParameter( new QgsProcessingParameterBoolean( u"APPLY_SYMBOLOGY"_s, QObject::tr( "Automatically apply symbology" ), true ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Aligned layer" ), Qgis::ProcessingSourceType::VectorPoint ) );
}

Qgis::ProcessingAlgorithmFlags QgsAngleToNearestAlgorithm::flags() const
{
  Qgis::ProcessingAlgorithmFlags f = QgsProcessingAlgorithm::flags();
  f |= Qgis::ProcessingAlgorithmFlag::SupportsInPlaceEdits;
  return f;
}

QString QgsAngleToNearestAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the rotation required to align point features with their nearest "
                      "feature from another reference layer. A new field is added to the output layer which is filled with the angle "
                      "(in degrees, clockwise) to the nearest reference feature.\n\n"
                      "Optionally, the output layer's symbology can be set to automatically use the calculated rotation "
                      "field to rotate marker symbols.\n\n"
                      "If desired, a maximum distance to use when aligning points can be set, to avoid aligning isolated points "
                      "to distant features." );
}

QString QgsAngleToNearestAlgorithm::shortDescription() const
{
  return QObject::tr( "Rotates point features to align them to nearby features." );
}

QgsAngleToNearestAlgorithm *QgsAngleToNearestAlgorithm::createInstance() const
{
  return new QgsAngleToNearestAlgorithm();
}

bool QgsAngleToNearestAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  if ( const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( layer ) )
  {
    return vl->geometryType() == Qgis::GeometryType::Point;
  }
  return false;
}

bool QgsAngleToNearestAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( !mIsInPlace )
  {
    if ( QgsVectorLayer *sourceLayer = parameterAsVectorLayer( parameters, u"INPUT"_s, context ) )
    {
      mSourceRenderer.reset( sourceLayer->renderer()->clone() );
    }
  }

  return true;
}

QVariantMap QgsAngleToNearestAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const double maxDistance = parameters.value( u"MAX_DISTANCE"_s ).isValid() ? parameterAsDouble( parameters, u"MAX_DISTANCE"_s, context ) : std::numeric_limits<double>::quiet_NaN();
  std::unique_ptr<QgsProcessingFeatureSource> input( parameterAsSource( parameters, u"INPUT"_s, context ) );
  if ( !input )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT"_s ) );

  std::unique_ptr<QgsProcessingFeatureSource> referenceSource( parameterAsSource( parameters, u"REFERENCE_LAYER"_s, context ) );
  if ( !referenceSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"REFERENCE_LAYER"_s ) );

  const QString fieldName = parameterAsString( parameters, u"FIELD_NAME"_s, context );

  QgsFields outFields = input->fields();
  int fieldIndex = -1;
  if ( mIsInPlace )
  {
    fieldIndex = outFields.lookupField( fieldName );
  }
  else
  {
    if ( outFields.lookupField( fieldName ) >= 0 )
    {
      throw QgsProcessingException( QObject::tr( "A field with the same name (%1) already exists" ).arg( fieldName ) );
    }
    outFields.append( QgsField( fieldName, QMetaType::Type::Double ) );
  }

  QString dest;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, outFields, input->wkbType(), input->sourceCrs() ) );
  if ( parameters.value( u"OUTPUT"_s ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  // make spatial index
  const QgsFeatureIterator f2 = referenceSource->getFeatures( QgsFeatureRequest().setDestinationCrs( input->sourceCrs(), context.transformContext() ).setNoAttributes() );
  double step = referenceSource->featureCount() > 0 ? 50.0 / referenceSource->featureCount() : 1;
  int i = 0;
  const QgsSpatialIndex index( f2, [&]( const QgsFeature & ) -> bool {
    i++;
    if ( feedback->isCanceled() )
      return false;

    feedback->setProgress( i * step );

    return true; }, QgsSpatialIndex::FlagStoreFeatureGeometries );

  QgsFeature f;

  // Create output vector layer with additional attributes
  step = input->featureCount() > 0 ? 50.0 / input->featureCount() : 1;
  QgsFeatureIterator features = input->getFeatures();
  i = 0;
  while ( features.nextFeature( f ) )
  {
    i++;
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( 50 + i * step );

    QgsAttributes attributes = f.attributes();

    if ( !f.hasGeometry() )
    {
      if ( !mIsInPlace )
        attributes.append( QVariant() );
      else
        attributes[fieldIndex] = QVariant();
      f.setAttributes( attributes );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
    else
    {
      const QList<QgsFeatureId> nearest = index.nearestNeighbor( f.geometry(), 1, std::isnan( maxDistance ) ? 0 : maxDistance );
      if ( nearest.empty() )
      {
        feedback->pushInfo( QObject::tr( "No matching features found within search distance" ) );
        if ( !mIsInPlace )
          attributes.append( QVariant() );
        else
          attributes[fieldIndex] = QVariant();
        f.setAttributes( attributes );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
      else
      {
        if ( nearest.count() > 1 )
        {
          feedback->pushInfo( QObject::tr( "Multiple matching features found at same distance from search feature, found %n feature(s)", nullptr, nearest.count() ) );
        }

        const QgsGeometry joinLine = f.geometry().shortestLine( index.geometry( nearest.at( 0 ) ) );
        if ( const QgsLineString *line = qgsgeometry_cast<const QgsLineString *>( joinLine.constGet() ) )
        {
          if ( !mIsInPlace )
            attributes.append( line->startPoint().azimuth( line->endPoint() ) );
          else
            attributes[fieldIndex] = line->startPoint().azimuth( line->endPoint() );
        }
        else
        {
          if ( !mIsInPlace )
            attributes.append( QVariant() );
          else
            attributes[fieldIndex] = QVariant();
        }
        f.setAttributes( attributes );
        if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
      }
    }
  }

  if ( sink )
    sink->finalize();

  const bool applySymbology = parameterAsBool( parameters, u"APPLY_SYMBOLOGY"_s, context );
  if ( applySymbology )
  {
    if ( mIsInPlace )
    {
      // get in place vector layer
      // (possibly TODO - make this a reusable method!)
      QVariantMap inPlaceParams = parameters;
      inPlaceParams.insert( u"INPUT"_s, parameters.value( u"INPUT"_s ).value<QgsProcessingFeatureSourceDefinition>().source );
      if ( QgsVectorLayer *sourceLayer = parameterAsVectorLayer( inPlaceParams, u"INPUT"_s, context ) )
      {
        std::unique_ptr<QgsFeatureRenderer> sourceRenderer( sourceLayer->renderer()->clone() );
        SetMarkerRotationPostProcessor processor( std::move( sourceRenderer ), fieldName );
        processor.postProcessLayer( sourceLayer, context, feedback );
      }
    }
    else if ( mSourceRenderer && context.willLoadLayerOnCompletion( dest ) )
    {
      context.layerToLoadOnCompletionDetails( dest ).setPostProcessor( new SetMarkerRotationPostProcessor( std::move( mSourceRenderer ), fieldName ) );
    }
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, dest );
  return outputs;
}


///@endcond
