/***************************************************************************
                         qgsalgorithmsumlinelength.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Alexander Bruy
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

#include "qgsalgorithmsumlinelength.h"
#include "qgsprocessing.h"
#include "qgsgeometryengine.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"

///@cond PRIVATE

QString QgsSumLineLengthAlgorithm::name() const
{
  return QStringLiteral( "sumlinelengths" );
}

QString QgsSumLineLengthAlgorithm::displayName() const
{
  return QObject::tr( "Sum line lengths" );
}

QStringList QgsSumLineLengthAlgorithm::tags() const
{
  return QObject::tr( "line,intersects,intersecting,sum,length,count" ).split( ',' );
}

QString QgsSumLineLengthAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmSumLengthLines.svg" ) );
}

QIcon QgsSumLineLengthAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmSumLengthLines.svg" ) );
}

QString QgsSumLineLengthAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsSumLineLengthAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

QString QgsSumLineLengthAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a polygon layer and a line layer and "
                      "measures the total length of lines and the total number of "
                      "them that cross each polygon.\n\n"
                      "The resulting layer has the same features as the input polygon "
                      "layer, but with two additional attributes containing the length "
                      "and count of the lines across each polygon. The names of these "
                      "two fields can be configured in the algorithm parameters." );
}

QgsSumLineLengthAlgorithm *QgsSumLineLengthAlgorithm::createInstance() const
{
  return new QgsSumLineLengthAlgorithm();
}

QList<int> QgsSumLineLengthAlgorithm::inputLayerTypes() const
{
  return QList< int >() << QgsProcessing::TypeVectorPolygon;
}

QgsProcessing::SourceType QgsSumLineLengthAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsCoordinateReferenceSystem QgsSumLineLengthAlgorithm::outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const
{
  mCrs = inputCrs;
  mDa.setSourceCrs( mCrs, mTransformContext );
  return mCrs;
}

QString QgsSumLineLengthAlgorithm::inputParameterName() const
{
  return QStringLiteral( "POLYGONS" );
}

QString QgsSumLineLengthAlgorithm::inputParameterDescription() const
{
  return QObject::tr( "Polygons" );
}

QString QgsSumLineLengthAlgorithm::outputName() const
{
  return QObject::tr( "Line length" );
}

void QgsSumLineLengthAlgorithm::initParameters( const QVariantMap &configuration )
{
  mIsInPlace = configuration.value( QStringLiteral( "IN_PLACE" ) ).toBool();

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "LINES" ),
                QObject::tr( "Lines" ), QList< int > () << QgsProcessing::TypeVectorLine ) );
  if ( mIsInPlace )
  {
    addParameter( new QgsProcessingParameterField( QStringLiteral( "LEN_FIELD" ),
                  QObject::tr( "Lines length field name" ), QStringLiteral( "LENGTH" ), inputParameterName(), QgsProcessingParameterField::Any, false, true ) );
    addParameter( new QgsProcessingParameterField( QStringLiteral( "COUNT_FIELD" ),
                  QObject::tr( "Lines count field name" ), QStringLiteral( "COUNT" ), inputParameterName(), QgsProcessingParameterField::Any, false, true ) );
  }
  else
  {
    addParameter( new QgsProcessingParameterString( QStringLiteral( "LEN_FIELD" ),
                  QObject::tr( "Lines length field name" ), QStringLiteral( "LENGTH" ) ) );
    addParameter( new QgsProcessingParameterString( QStringLiteral( "COUNT_FIELD" ),
                  QObject::tr( "Lines count field name" ), QStringLiteral( "COUNT" ) ) );
  }
}

bool QgsSumLineLengthAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mLengthFieldName = parameterAsString( parameters, QStringLiteral( "LEN_FIELD" ), context );
  mCountFieldName = parameterAsString( parameters, QStringLiteral( "COUNT_FIELD" ), context );

  mLinesSource.reset( parameterAsSource( parameters, QStringLiteral( "LINES" ), context ) );
  if ( !mLinesSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "LINES" ) ) );

  if ( mLinesSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for lines layer, performance will be severely degraded" ) );

  mDa.setEllipsoid( context.ellipsoid() );
  mTransformContext = context.transformContext();

  return true;
}

QgsFields QgsSumLineLengthAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  if ( mIsInPlace )
  {
    mLengthFieldIndex = mLengthFieldName.isEmpty() ? -1 : inputFields.lookupField( mLengthFieldName );
    mCountFieldIndex = mCountFieldName.isEmpty() ? -1 :  inputFields.lookupField( mCountFieldName );
    return inputFields;
  }
  else
  {
    QgsFields outFields = inputFields;
    mLengthFieldIndex = inputFields.lookupField( mLengthFieldName );
    if ( mLengthFieldIndex < 0 )
      outFields.append( QgsField( mLengthFieldName, QVariant::Double ) );

    mCountFieldIndex = inputFields.lookupField( mCountFieldName );
    if ( mCountFieldIndex < 0 )
      outFields.append( QgsField( mCountFieldName, QVariant::Double ) );

    mFields = outFields;
    return outFields;
  }
}

bool QgsSumLineLengthAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  if ( const QgsVectorLayer *vl = qobject_cast< const QgsVectorLayer * >( layer ) )
  {
    return vl->geometryType() == QgsWkbTypes::PolygonGeometry;
  }
  return false;
}

QgsFeatureList QgsSumLineLengthAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeature outputFeature = feature;
  if ( !feature.hasGeometry() )
  {
    QgsAttributes attrs = feature.attributes();
    if ( !mIsInPlace && mLengthFieldIndex < 0 )
      attrs.append( 0 );
    else if ( mLengthFieldIndex >= 0 )
      attrs[mLengthFieldIndex] = 0;

    if ( !mIsInPlace && mCountFieldIndex < 0 )
      attrs.append( 0 );
    else if ( mCountFieldIndex >= 0 )
      attrs[mCountFieldIndex] = 0;

    outputFeature.setAttributes( attrs );
    return QList< QgsFeature > () << outputFeature;
  }
  else
  {
    const QgsGeometry polyGeom = feature.geometry();
    std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( polyGeom.constGet() ) );
    engine->prepareGeometry();

    QgsFeatureRequest req = QgsFeatureRequest().setFilterRect( polyGeom.boundingBox() ).setDestinationCrs( mCrs, context.transformContext() );
    req.setSubsetOfAttributes( QList< int >() );
    QgsFeatureIterator it = mLinesSource->getFeatures( req );

    double count = 0;
    double length = 0;

    QgsFeature lineFeature;
    while ( it.nextFeature( lineFeature ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( engine->intersects( lineFeature.geometry().constGet() ) )
      {
        const QgsGeometry outGeom = polyGeom.intersection( lineFeature.geometry() );
        length += mDa.measureLength( outGeom );
        count++;
      }
    }

    QgsAttributes attrs = feature.attributes();
    if ( !mIsInPlace && mLengthFieldIndex < 0 )
      attrs.append( length );
    else if ( mLengthFieldIndex >= 0 )
      attrs[mLengthFieldIndex] = length;

    if ( !mIsInPlace && mCountFieldIndex < 0 )
      attrs.append( count );
    else if ( mCountFieldIndex >= 0 )
      attrs[mCountFieldIndex] = count;

    outputFeature.setAttributes( attrs );
    return QList< QgsFeature >() << outputFeature;
  }
}

///@endcond
