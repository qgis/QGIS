/***************************************************************************
                         qgsalgorithmaddixyfields.cpp
                         -----------------------------------
    begin                : March 2019
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

#include "qgsalgorithmaddxyfields.h"

#include "qgsvectorlayer.h"

///@cond PRIVATE

QString QgsAddXYFieldsAlgorithm::name() const
{
  return u"addxyfields"_s;
}

QString QgsAddXYFieldsAlgorithm::displayName() const
{
  return QObject::tr( "Add X/Y fields to layer" );
}

QString QgsAddXYFieldsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm adds X and Y (or latitude/longitude) fields to a point layer. The X/Y fields can be calculated in a different CRS to the layer (e.g. creating latitude/longitude fields for a layer in a project CRS)." );
}

QString QgsAddXYFieldsAlgorithm::shortDescription() const
{
  return QObject::tr( "Adds X and Y (or latitude/longitude) fields to a point layer." );
}

QStringList QgsAddXYFieldsAlgorithm::tags() const
{
  return QObject::tr( "add,create,latitude,longitude,columns,attributes" ).split( ',' );
}

QString QgsAddXYFieldsAlgorithm::group() const
{
  return QObject::tr( "Vector table" );
}

QString QgsAddXYFieldsAlgorithm::groupId() const
{
  return u"vectortable"_s;
}

QString QgsAddXYFieldsAlgorithm::outputName() const
{
  return QObject::tr( "Added fields" );
}

QList<int> QgsAddXYFieldsAlgorithm::inputLayerTypes() const
{
  return QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint );
}

QgsAddXYFieldsAlgorithm *QgsAddXYFieldsAlgorithm::createInstance() const
{
  return new QgsAddXYFieldsAlgorithm();
}

Qgis::ProcessingFeatureSourceFlags QgsAddXYFieldsAlgorithm::sourceFlags() const
{
  return Qgis::ProcessingFeatureSourceFlag::SkipGeometryValidityChecks;
}

void QgsAddXYFieldsAlgorithm::initParameters( const QVariantMap &configuration )
{
  mIsInPlace = configuration.value( u"IN_PLACE"_s ).toBool();

  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "Coordinate system" ), u"EPSG:4326"_s ) );

  if ( !mIsInPlace )
    addParameter( new QgsProcessingParameterString( u"PREFIX"_s, QObject::tr( "Field prefix" ), QVariant(), false, true ) );
  else
  {
    addParameter( new QgsProcessingParameterField( u"FIELD_X"_s, QObject::tr( "X field" ), QVariant(), u"INPUT"_s ) );
    addParameter( new QgsProcessingParameterField( u"FIELD_Y"_s, QObject::tr( "Y field" ), QVariant(), u"INPUT"_s ) );
  }
}

QgsFields QgsAddXYFieldsAlgorithm::outputFields( const QgsFields &inputFields ) const
{
  if ( mIsInPlace )
  {
    mInPlaceXFieldIndex = inputFields.lookupField( mInPlaceXField );
    mInPlaceYFieldIndex = inputFields.lookupField( mInPlaceYField );
    return inputFields;
  }
  else
  {
    const QString xFieldName = mPrefix + 'x';
    const QString yFieldName = mPrefix + 'y';

    QgsFields newFields;
    newFields.append( QgsField( xFieldName, QMetaType::Type::Double, QString(), 20, 10 ) );
    newFields.append( QgsField( yFieldName, QMetaType::Type::Double, QString(), 20, 10 ) );
    return QgsProcessingUtils::combineFields( inputFields, newFields );
  }
}

QgsCoordinateReferenceSystem QgsAddXYFieldsAlgorithm::outputCrs( const QgsCoordinateReferenceSystem &inputCrs ) const
{
  mSourceCrs = inputCrs;
  return inputCrs;
}

bool QgsAddXYFieldsAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( !mIsInPlace )
    mPrefix = parameterAsString( parameters, u"PREFIX"_s, context );
  else
  {
    mInPlaceXField = parameterAsString( parameters, u"FIELD_X"_s, context );
    mInPlaceYField = parameterAsString( parameters, u"FIELD_Y"_s, context );
  }

  mCrs = parameterAsCrs( parameters, u"CRS"_s, context );
  return true;
}

QgsFeatureList QgsAddXYFieldsAlgorithm::processFeature( const QgsFeature &feature, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  if ( mTransformNeedsInitialization )
  {
    mTransform = QgsCoordinateTransform( mSourceCrs, mCrs, context.transformContext() );
    mTransformNeedsInitialization = false;
  }
  if ( mIsInPlace && mInPlaceXFieldIndex == -1 )
  {
    throw QgsProcessingException( QObject::tr( "Destination field not found" ) );
  }

  QVariant x;
  QVariant y;
  if ( feature.hasGeometry() )
  {
    if ( feature.geometry().isMultipart() )
      throw QgsProcessingException( QObject::tr( "Multipoint features are not supported - please convert to single point features first." ) );

    const QgsPointXY point = feature.geometry().asPoint();
    try
    {
      const QgsPointXY transformed = mTransform.transform( point );
      x = transformed.x();
      y = transformed.y();
    }
    catch ( QgsCsException & )
    {
      feedback->reportError( QObject::tr( "Could not transform point to destination CRS" ) );
    }
  }
  QgsFeature f = feature;
  QgsAttributes attributes = f.attributes();
  if ( !mIsInPlace )
  {
    attributes << x << y;
  }
  else
  {
    attributes[mInPlaceXFieldIndex] = std::move( x );
    attributes[mInPlaceYFieldIndex] = std::move( y );
  }
  f.setAttributes( attributes );
  return QgsFeatureList() << f;
}

bool QgsAddXYFieldsAlgorithm::supportInPlaceEdit( const QgsMapLayer *layer ) const
{
  if ( const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( layer ) )
  {
    return vl->geometryType() == Qgis::GeometryType::Point;
  }
  return false;
}

///@endcond
