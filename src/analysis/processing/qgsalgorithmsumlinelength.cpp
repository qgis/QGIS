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

#include "qgsapplication.h"
#include "qgsgeometryengine.h"
#include "qgsprocessing.h"
#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsSumLineLengthAlgorithm::name() const
{
  return u"sumlinelengths"_s;
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
  return QgsApplication::iconPath( u"/algorithms/mAlgorithmSumLengthLines.svg"_s );
}

QIcon QgsSumLineLengthAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( u"/algorithms/mAlgorithmSumLengthLines.svg"_s );
}

QString QgsSumLineLengthAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsSumLineLengthAlgorithm::groupId() const
{
  return u"vectoranalysis"_s;
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

QString QgsSumLineLengthAlgorithm::shortDescription() const
{
  return QObject::tr( "Takes a polygon layer and a line layer and "
                      "measures the total length of lines and the total number of "
                      "them that cross each polygon." );
}

Qgis::ProcessingAlgorithmDocumentationFlags QgsSumLineLengthAlgorithm::documentationFlags() const
{
  return Qgis::ProcessingAlgorithmDocumentationFlag::RespectsEllipsoid;
}

QgsSumLineLengthAlgorithm *QgsSumLineLengthAlgorithm::createInstance() const
{
  return new QgsSumLineLengthAlgorithm();
}

QList<int> QgsSumLineLengthAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon );
}

Qgis::ProcessingSourceType QgsSumLineLengthAlgorithm::outputLayerType() const
{
  return Qgis::ProcessingSourceType::VectorPolygon;
}

QgsCoordinateReferenceSystem QgsSumLineLengthAlgorithm::outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const
{
  mCrs = inputCrs;
  mDa.setSourceCrs( mCrs, mTransformContext );
  return mCrs;
}

QString QgsSumLineLengthAlgorithm::inputParameterName() const
{
  return u"POLYGONS"_s;
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
  mIsInPlace = configuration.value( u"IN_PLACE"_s ).toBool();

  addParameter( new QgsProcessingParameterFeatureSource( u"LINES"_s, QObject::tr( "Lines" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) ) );
  if ( mIsInPlace )
  {
    addParameter( new QgsProcessingParameterField( u"LEN_FIELD"_s, QObject::tr( "Lines length field name" ), u"LENGTH"_s, inputParameterName(), Qgis::ProcessingFieldParameterDataType::Any, false, true ) );
    addParameter( new QgsProcessingParameterField( u"COUNT_FIELD"_s, QObject::tr( "Lines count field name" ), u"COUNT"_s, inputParameterName(), Qgis::ProcessingFieldParameterDataType::Any, false, true ) );
  }
  else
  {
    addParameter( new QgsProcessingParameterString( u"LEN_FIELD"_s, QObject::tr( "Lines length field name" ), u"LENGTH"_s ) );
    addParameter( new QgsProcessingParameterString( u"COUNT_FIELD"_s, QObject::tr( "Lines count field name" ), u"COUNT"_s ) );
  }
}

bool QgsSumLineLengthAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mLengthFieldName = parameterAsString( parameters, u"LEN_FIELD"_s, context );
  mCountFieldName = parameterAsString( parameters, u"COUNT_FIELD"_s, context );

  mLinesSource.reset( parameterAsSource( parameters, u"LINES"_s, context ) );
  if ( !mLinesSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"LINES"_s ) );

  if ( mLinesSource->hasSpatialIndex() == Qgis::SpatialIndexPresence::NotPresent )
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
    mCountFieldIndex = mCountFieldName.isEmpty() ? -1 : inputFields.lookupField( mCountFieldName );
    return inputFields;
  }
  else
  {
    QgsFields outFields = inputFields;
    mLengthFieldIndex = inputFields.lookupField( mLengthFieldName );
    if ( mLengthFieldIndex < 0 )
      outFields.append( QgsField( mLengthFieldName, QMetaType::Type::Double ) );

    mCountFieldIndex = inputFields.lookupField( mCountFieldName );
    if ( mCountFieldIndex < 0 )
      outFields.append( QgsField( mCountFieldName, QMetaType::Type::Double ) );

    mFields = outFields;
    return outFields;
  }
}

bool QgsSumLineLengthAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  if ( const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( layer ) )
  {
    return vl->geometryType() == Qgis::GeometryType::Polygon;
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
    return QList<QgsFeature>() << outputFeature;
  }
  else
  {
    const QgsGeometry polyGeom = feature.geometry();
    std::unique_ptr<QgsGeometryEngine> engine( QgsGeometry::createGeometryEngine( polyGeom.constGet() ) );
    engine->prepareGeometry();

    QgsFeatureRequest req = QgsFeatureRequest().setFilterRect( polyGeom.boundingBox() ).setDestinationCrs( mCrs, context.transformContext() );
    req.setSubsetOfAttributes( QList<int>() );
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
        try
        {
          length += mDa.measureLength( outGeom );
        }
        catch ( QgsCsException & )
        {
          throw QgsProcessingException( QObject::tr( "An error occurred while calculating feature length" ) );
        }
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
    return QList<QgsFeature>() << outputFeature;
  }
}

///@endcond
