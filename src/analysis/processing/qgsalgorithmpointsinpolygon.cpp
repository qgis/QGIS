/***************************************************************************
                         qgsalgorithmpointsinpolygon.cpp
                         ---------------------
    begin                : November 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsalgorithmpointsinpolygon.h"
#include "qgsprocessing.h"
#include "qgsgeometryengine.h"
#include "qgsvectorlayer.h"
#include "qgsapplication.h"

///@cond PRIVATE

void QgsPointsInPolygonAlgorithm::initParameters( const QVariantMap &configuration )
{
  mIsInPlace = configuration.value( QStringLiteral( "IN_PLACE" ) ).toBool();

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "POINTS" ),
                QObject::tr( "Points" ), QList< int > () << QgsProcessing::TypeVectorPoint ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "WEIGHT" ),
                QObject::tr( "Weight field" ), QVariant(), QStringLiteral( "POINTS" ), QgsProcessingParameterField::Any, false, true ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "CLASSFIELD" ),
                QObject::tr( "Class field" ), QVariant(), QStringLiteral( "POINTS" ), QgsProcessingParameterField::Any, false, true ) );
  if ( mIsInPlace )
  {
    addParameter( new QgsProcessingParameterField( QStringLiteral( "FIELD" ),
                  QObject::tr( "Count field" ), QStringLiteral( "NUMPOINTS" ), inputParameterName() ) );
  }
  else
  {
    addParameter( new QgsProcessingParameterString( QStringLiteral( "FIELD" ),
                  QObject::tr( "Count field name" ), QStringLiteral( "NUMPOINTS" ) ) );
  }
}

QString QgsPointsInPolygonAlgorithm::name() const
{
  return QStringLiteral( "countpointsinpolygon" );
}

QString QgsPointsInPolygonAlgorithm::displayName() const
{
  return QObject::tr( "Count points in polygon" );
}

QStringList QgsPointsInPolygonAlgorithm::tags() const
{
  return QObject::tr( "extract,filter,intersects,intersecting,disjoint,touching,within,contains,overlaps,relation" ).split( ',' );
}

QString QgsPointsInPolygonAlgorithm::svgIconPath() const
{
  return QgsApplication::iconPath( QStringLiteral( "/algorithms/mAlgorithmSumPoints.svg" ) );
}

QIcon QgsPointsInPolygonAlgorithm::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/algorithms/mAlgorithmSumPoints.svg" ) );
}

QString QgsPointsInPolygonAlgorithm::group() const
{
  return QObject::tr( "Vector analysis" );
}

QString QgsPointsInPolygonAlgorithm::groupId() const
{
  return QStringLiteral( "vectoranalysis" );
}

QString QgsPointsInPolygonAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm takes a points layer and a polygon layer and counts the number of points from "
                      "the first one in each polygons of the second one.\n\n"
                      "A new polygons layer is generated, with the exact same content as the input polygons layer, but "
                      "containing an additional field with the points count corresponding to each polygon.\n\n"
                      "An optional weight field can be used to assign weights to each point. If set, the count generated "
                      "will be the sum of the weight field for each point contained by the polygon.\n\n"
                      "Alternatively, a unique class field can be specified. If set, points are classified based on "
                      "the selected attribute, and if several points with the same attribute value are within the polygon, "
                      "only one of them is counted. The final count of the point in a polygon is, therefore, the count of "
                      "different classes that are found in it.\n\n"
                      "Both the weight field and unique class field cannot be specified. If they are, the weight field will "
                      "take precedence and the unique class field will be ignored." );
}

QString QgsPointsInPolygonAlgorithm::shortDescription() const
{
  return QObject::tr( "Counts point features located within polygon features." );
}

QgsPointsInPolygonAlgorithm *QgsPointsInPolygonAlgorithm::createInstance() const
{
  return new QgsPointsInPolygonAlgorithm();
}

QList<int> QgsPointsInPolygonAlgorithm::inputLayerTypes() const
{
  return QList< int >() << QgsProcessing::TypeVectorPolygon;
}

QgsProcessing::SourceType QgsPointsInPolygonAlgorithm::outputLayerType() const
{
  return QgsProcessing::TypeVectorPolygon;
}

QgsCoordinateReferenceSystem QgsPointsInPolygonAlgorithm::outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const
{
  mCrs = inputCrs;
  return mCrs;
}

QString QgsPointsInPolygonAlgorithm::inputParameterName() const
{
  return QStringLiteral( "POLYGONS" );
}

QString QgsPointsInPolygonAlgorithm::inputParameterDescription() const
{
  return QObject::tr( "Polygons" );
}

QString QgsPointsInPolygonAlgorithm::outputName() const
{
  return QObject::tr( "Count" );
}

bool QgsPointsInPolygonAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mFieldName = parameterAsString( parameters, QStringLiteral( "FIELD" ), context );
  mWeightFieldName = parameterAsString( parameters, QStringLiteral( "WEIGHT" ), context );
  mClassFieldName = parameterAsString( parameters, QStringLiteral( "CLASSFIELD" ), context );
  mPointSource.reset( parameterAsSource( parameters, QStringLiteral( "POINTS" ), context ) );
  if ( !mPointSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "POINTS" ) ) );

  if ( !mWeightFieldName.isEmpty() )
  {
    mWeightFieldIndex = mPointSource->fields().lookupField( mWeightFieldName );
    if ( mWeightFieldIndex == -1 )
      throw QgsProcessingException( QObject::tr( "Could not find field %1" ).arg( mWeightFieldName ) );
    mPointAttributes.append( mWeightFieldIndex );
  }

  if ( !mClassFieldName.isEmpty() )
  {
    mClassFieldIndex = mPointSource->fields().lookupField( mClassFieldName );
    if ( mClassFieldIndex == -1 )
      throw QgsProcessingException( QObject::tr( "Could not find field %1" ).arg( mClassFieldIndex ) );
    mPointAttributes.append( mClassFieldIndex );
  }

  if ( mPointSource->hasSpatialIndex() == QgsFeatureSource::SpatialIndexNotPresent )
    feedback->pushWarning( QObject::tr( "No spatial index exists for points layer, performance will be severely degraded" ) );

  return true;
}

QgsFeatureList QgsPointsInPolygonAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsFeature outputFeature = feature;
  if ( !feature.hasGeometry() )
  {
    QgsAttributes attrs = feature.attributes();
    if ( mDestFieldIndex < 0 )
      attrs.append( 0 );
    else
      attrs[mDestFieldIndex] = 0;
    outputFeature.setAttributes( attrs );
    return QList< QgsFeature > () << outputFeature;
  }
  else
  {
    const QgsGeometry polyGeom = feature.geometry();
    std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( polyGeom.constGet() ) );
    engine->prepareGeometry();

    double count = 0;
    QSet< QVariant> classes;

    QgsFeatureRequest req = QgsFeatureRequest().setFilterRect( polyGeom.boundingBox() ).setDestinationCrs( mCrs, context.transformContext() );
    req.setSubsetOfAttributes( mPointAttributes );
    QgsFeatureIterator it = mPointSource->getFeatures( req );

    bool ok = false;
    QgsFeature pointFeature;
    while ( it.nextFeature( pointFeature ) )
    {
      if ( feedback->isCanceled() )
        break;

      if ( engine->contains( pointFeature.geometry().constGet() ) )
      {
        if ( mWeightFieldIndex >= 0 )
        {
          const QVariant weight = pointFeature.attribute( mWeightFieldIndex );
          const double pointWeight = weight.toDouble( &ok );
          // Ignore fields with non-numeric values
          if ( ok )
            count += pointWeight;
          else
            feedback->reportError( QObject::tr( "Weight field value “%1” is not a numeric value" ).arg( weight.toString() ) );
        }
        else if ( mClassFieldIndex >= 0 )
        {
          const QVariant pointClass = pointFeature.attribute( mClassFieldIndex );
          classes.insert( pointClass );
        }
        else
        {
          count++;
        }
      }
    }

    QgsAttributes attrs = feature.attributes();
    double score = 0;

    if ( mClassFieldIndex >= 0 )
      score = classes.size();
    else
      score = count;

    if ( mDestFieldIndex < 0 )
      attrs.append( score );
    else
      attrs[mDestFieldIndex] = score;

    outputFeature.setAttributes( attrs );
    return QList< QgsFeature >() << outputFeature;
  }
}

QgsFields QgsPointsInPolygonAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  if ( mIsInPlace )
  {
    mDestFieldIndex = inputFields.lookupField( mFieldName );
    return inputFields;
  }
  else
  {
    QgsFields outFields = inputFields;
    mDestFieldIndex = inputFields.lookupField( mFieldName );
    if ( mDestFieldIndex < 0 )
      outFields.append( QgsField( mFieldName, QVariant::Double ) );

    mFields = outFields;
    return outFields;
  }
}

bool QgsPointsInPolygonAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  if ( const QgsVectorLayer *vl = qobject_cast< const QgsVectorLayer * >( layer ) )
  {
    return vl->geometryType() == QgsWkbTypes::PolygonGeometry;
  }
  return false;
}


///@endcond



