/***************************************************************************
                         qgsalgorithmreclassifybylayer.cpp
                         ---------------------
    begin                : June, 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#include "qgsalgorithmreclassifybylayer.h"

#include <gdal.h>

#include "qgis.h"
#include "qgsrasteranalysisutils.h"
#include "qgsrasterfilewriter.h"
#include "qgsreclassifyutils.h"
#include "qgsvariantutils.h"

///@cond PRIVATE

//
// QgsReclassifyAlgorithmBase
//


QString QgsReclassifyAlgorithmBase::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsReclassifyAlgorithmBase::groupId() const
{
  return u"rasteranalysis"_s;
}

void QgsReclassifyAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_RASTER"_s, QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"RASTER_BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT_RASTER"_s ) );

  addAlgorithmParams();

  auto noDataValueParam = std::make_unique<QgsProcessingParameterNumber>( u"NO_DATA"_s, QObject::tr( "Output NoData value" ), Qgis::ProcessingNumberParameterType::Double, -9999 );
  noDataValueParam->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( noDataValueParam.release() );

  auto boundsHandling = std::make_unique<QgsProcessingParameterEnum>( u"RANGE_BOUNDARIES"_s, QObject::tr( "Range boundaries" ), QStringList() << QObject::tr( "min < value <= max" ) << QObject::tr( "min <= value < max" ) << QObject::tr( "min <= value <= max" ) << QObject::tr( "min < value < max" ), false, 0 );
  boundsHandling->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( boundsHandling.release() );

  auto missingValuesParam = std::make_unique<QgsProcessingParameterBoolean>( u"NODATA_FOR_MISSING"_s, QObject::tr( "Use NoData when no range matches value" ), false, false );
  missingValuesParam->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( missingValuesParam.release() );

  std::unique_ptr<QgsProcessingParameterDefinition> typeChoice = QgsRasterAnalysisUtils::createRasterTypeParameter( u"DATA_TYPE"_s, QObject::tr( "Output data type" ), Qgis::DataType::Float32 );
  typeChoice->setFlags( Qgis::ProcessingParameterFlag::Advanced );
  addParameter( typeChoice.release() );

  // backwards compatibility parameter
  // TODO QGIS 5: remove parameter and related logic
  auto createOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATE_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  createOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  createOptsParam->setFlags( createOptsParam->flags() | Qgis::ProcessingParameterFlag::Hidden );
  addParameter( createOptsParam.release() );

  auto creationOptsParam = std::make_unique<QgsProcessingParameterString>( u"CREATION_OPTIONS"_s, QObject::tr( "Creation options" ), QVariant(), false, true );
  creationOptsParam->setMetadata( QVariantMap( { { u"widget_wrapper"_s, QVariantMap( { { u"widget_type"_s, u"rasteroptions"_s } } ) } } ) );
  creationOptsParam->setFlags( creationOptsParam->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( creationOptsParam.release() );

  addParameter( new QgsProcessingParameterRasterDestination( u"OUTPUT"_s, QObject::tr( "Reclassified raster" ) ) );
}

bool QgsReclassifyAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mDataType = QgsRasterAnalysisUtils::rasterTypeChoiceToDataType( parameterAsEnum( parameters, u"DATA_TYPE"_s, context ) );
  if ( mDataType == Qgis::DataType::Int8 && atoi( GDALVersionInfo( "VERSION_NUM" ) ) < GDAL_COMPUTE_VERSION( 3, 7, 0 ) )
    throw QgsProcessingException( QObject::tr( "Int8 data type requires GDAL version 3.7 or later" ) );

  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT_RASTER"_s, context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_RASTER"_s ) );

  mBand = parameterAsInt( parameters, u"RASTER_BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for RASTER_BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = std::abs( layer->rasterUnitsPerPixelX() );
  mRasterUnitsPerPixelY = std::abs( layer->rasterUnitsPerPixelY() );
  mNbCellsXProvider = mInterface->xSize();
  mNbCellsYProvider = mInterface->ySize();

  mNoDataValue = parameterAsDouble( parameters, u"NO_DATA"_s, context );
  mUseNoDataForMissingValues = parameterAsBoolean( parameters, u"NODATA_FOR_MISSING"_s, context );

  const int boundsType = parameterAsEnum( parameters, u"RANGE_BOUNDARIES"_s, context );
  switch ( boundsType )
  {
    case 0:
      mBoundsType = QgsReclassifyUtils::RasterClass::IncludeMax;
      break;

    case 1:
      mBoundsType = QgsReclassifyUtils::RasterClass::IncludeMin;
      break;

    case 2:
      mBoundsType = QgsReclassifyUtils::RasterClass::IncludeMinAndMax;
      break;

    case 3:
      mBoundsType = QgsReclassifyUtils::RasterClass::Exclusive;
      break;
  }

  return _prepareAlgorithm( parameters, context, feedback );
}

QVariantMap QgsReclassifyAlgorithmBase::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QVector<QgsReclassifyUtils::RasterClass> classes = createClasses( mBoundsType, parameters, context, feedback );

  QgsReclassifyUtils::reportClasses( classes, feedback );
  QgsReclassifyUtils::checkForOverlaps( classes, feedback );

  QString creationOptions = parameterAsString( parameters, u"CREATION_OPTIONS"_s, context ).trimmed();
  // handle backwards compatibility parameter CREATE_OPTIONS
  const QString optionsString = parameterAsString( parameters, u"CREATE_OPTIONS"_s, context );
  if ( !optionsString.isEmpty() )
    creationOptions = optionsString;

  const QString outputFile = parameterAsOutputLayer( parameters, u"OUTPUT"_s, context );
  const QString outputFormat = parameterAsOutputRasterFormat( parameters, u"OUTPUT"_s, context );

  auto writer = std::make_unique<QgsRasterFileWriter>( outputFile );
  writer->setOutputProviderKey( u"gdal"_s );
  if ( !creationOptions.isEmpty() )
  {
    writer->setCreationOptions( creationOptions.split( '|' ) );
  }

  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider> provider( writer->createOneBandRaster( mDataType, mNbCellsXProvider, mNbCellsYProvider, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );

  QgsReclassifyUtils::reclassify( classes, mInterface.get(), mBand, mExtent, mNbCellsXProvider, mNbCellsYProvider, std::move( provider ), mNoDataValue, mUseNoDataForMissingValues, feedback );

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}


//
// QgsReclassifyByLayerAlgorithm
//

QString QgsReclassifyByLayerAlgorithm::name() const
{
  return u"reclassifybylayer"_s;
}

QString QgsReclassifyByLayerAlgorithm::displayName() const
{
  return QObject::tr( "Reclassify by layer" );
}

QStringList QgsReclassifyByLayerAlgorithm::tags() const
{
  return QObject::tr( "raster,reclassify,classes,calculator" ).split( ',' );
}

QString QgsReclassifyByLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reclassifies a raster band by assigning new class values based on the ranges specified in a vector table." );
}

QString QgsReclassifyByLayerAlgorithm::shortDescription() const
{
  return QObject::tr( "Reclassifies a raster band by assigning new class values based on the ranges specified in a vector table." );
}

QgsReclassifyByLayerAlgorithm *QgsReclassifyByLayerAlgorithm::createInstance() const
{
  return new QgsReclassifyByLayerAlgorithm();
}

void QgsReclassifyByLayerAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterFeatureSource( u"INPUT_TABLE"_s, QObject::tr( "Layer containing class breaks" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::Vector ) ) );
  addParameter( new QgsProcessingParameterField( u"MIN_FIELD"_s, QObject::tr( "Minimum class value field" ), QVariant(), u"INPUT_TABLE"_s, Qgis::ProcessingFieldParameterDataType::Numeric ) );
  addParameter( new QgsProcessingParameterField( u"MAX_FIELD"_s, QObject::tr( "Maximum class value field" ), QVariant(), u"INPUT_TABLE"_s, Qgis::ProcessingFieldParameterDataType::Numeric ) );
  addParameter( new QgsProcessingParameterField( u"VALUE_FIELD"_s, QObject::tr( "Output value field" ), QVariant(), u"INPUT_TABLE"_s, Qgis::ProcessingFieldParameterDataType::Numeric ) );
}

bool QgsReclassifyByLayerAlgorithm::_prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  std::unique_ptr<QgsFeatureSource> tableSource( parameterAsSource( parameters, u"INPUT_TABLE"_s, context ) );
  if ( !tableSource )
    throw QgsProcessingException( invalidSourceError( parameters, u"INPUT_TABLE"_s ) );

  const QString fieldMin = parameterAsString( parameters, u"MIN_FIELD"_s, context );
  mMinFieldIdx = tableSource->fields().lookupField( fieldMin );
  if ( mMinFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for MIN_FIELD: %1" ).arg( fieldMin ) );
  const QString fieldMax = parameterAsString( parameters, u"MAX_FIELD"_s, context );
  mMaxFieldIdx = tableSource->fields().lookupField( fieldMax );
  if ( mMaxFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for MAX_FIELD: %1" ).arg( fieldMax ) );
  const QString fieldValue = parameterAsString( parameters, u"VALUE_FIELD"_s, context );
  mValueFieldIdx = tableSource->fields().lookupField( fieldValue );
  if ( mValueFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for VALUE_FIELD: %1" ).arg( fieldValue ) );

  QgsFeatureRequest request;
  request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
  request.setSubsetOfAttributes( QgsAttributeList() << mMinFieldIdx << mMaxFieldIdx << mValueFieldIdx );
  mTableIterator = tableSource->getFeatures( request );

  return true;
}

QVector<QgsReclassifyUtils::RasterClass> QgsReclassifyByLayerAlgorithm::createClasses( QgsRasterRange::BoundsType boundsType, const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVector<QgsReclassifyUtils::RasterClass> classes;
  QgsFeature f;
  while ( mTableIterator.nextFeature( f ) )
  {
    bool ok = false;

    // null values map to nan, which corresponds to a range extended to +/- infinity....
    const QVariant minVariant = f.attribute( mMinFieldIdx );
    double minValue;
    if ( QgsVariantUtils::isNull( minVariant ) || minVariant.toString().isEmpty() )
    {
      minValue = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      minValue = minVariant.toDouble( &ok );
      if ( !ok )
        throw QgsProcessingException( QObject::tr( "Invalid value for minimum: %1" ).arg( minVariant.toString() ) );
    }
    const QVariant maxVariant = f.attribute( mMaxFieldIdx );
    double maxValue;
    if ( QgsVariantUtils::isNull( maxVariant ) || maxVariant.toString().isEmpty() )
    {
      maxValue = std::numeric_limits<double>::quiet_NaN();
      ok = true;
    }
    else
    {
      maxValue = maxVariant.toDouble( &ok );
      if ( !ok )
        throw QgsProcessingException( QObject::tr( "Invalid value for maximum: %1" ).arg( maxVariant.toString() ) );
    }

    const double value = f.attribute( mValueFieldIdx ).toDouble( &ok );
    if ( !ok )
      throw QgsProcessingException( QObject::tr( "Invalid output value: %1" ).arg( f.attribute( mValueFieldIdx ).toString() ) );

    classes << QgsReclassifyUtils::RasterClass( minValue, maxValue, boundsType, value );
  }
  return classes;
}


//
// QgsReclassifyByTableAlgorithm
//

QString QgsReclassifyByTableAlgorithm::name() const
{
  return u"reclassifybytable"_s;
}

QString QgsReclassifyByTableAlgorithm::displayName() const
{
  return QObject::tr( "Reclassify by table" );
}

QStringList QgsReclassifyByTableAlgorithm::tags() const
{
  return QObject::tr( "raster,reclassify,classes,calculator" ).split( ',' );
}

QString QgsReclassifyByTableAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reclassifies a raster band by assigning new class values based on the ranges specified in a fixed table." );
}

QString QgsReclassifyByTableAlgorithm::shortDescription() const
{
  return QObject::tr( "Reclassifies a raster band by assigning new class values based on the ranges specified in a fixed table." );
}

QgsReclassifyByTableAlgorithm *QgsReclassifyByTableAlgorithm::createInstance() const
{
  return new QgsReclassifyByTableAlgorithm();
}

void QgsReclassifyByTableAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterMatrix( u"TABLE"_s, QObject::tr( "Reclassification table" ), 1, false, QStringList() << QObject::tr( "Minimum" ) << QObject::tr( "Maximum" ) << QObject::tr( "Value" ) ) );
}

bool QgsReclassifyByTableAlgorithm::_prepareAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  return true;
}

QVector<QgsReclassifyUtils::RasterClass> QgsReclassifyByTableAlgorithm::createClasses( QgsReclassifyUtils::RasterClass::BoundsType boundsType, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QVariantList table = parameterAsMatrix( parameters, u"TABLE"_s, context );
  if ( table.count() % 3 != 0 )
    throw QgsProcessingException( QObject::tr( "Invalid value for TABLE: list must contain a multiple of 3 elements (found %1)" ).arg( table.count() ) );

  const int rows = table.count() / 3;
  QVector<QgsReclassifyUtils::RasterClass> classes;
  classes.reserve( rows );
  for ( int row = 0; row < rows; ++row )
  {
    bool ok = false;

    // null values map to nan, which corresponds to a range extended to +/- infinity....
    const QVariant minVariant = table.at( row * 3 );
    double minValue;
    if ( QgsVariantUtils::isNull( minVariant ) || minVariant.toString().isEmpty() )
    {
      minValue = std::numeric_limits<double>::quiet_NaN();
    }
    else
    {
      minValue = minVariant.toDouble( &ok );
      if ( !ok )
        throw QgsProcessingException( QObject::tr( "Invalid value for minimum: %1" ).arg( table.at( row * 3 ).toString() ) );
    }
    const QVariant maxVariant = table.at( row * 3 + 1 );
    double maxValue;
    if ( QgsVariantUtils::isNull( maxVariant ) || maxVariant.toString().isEmpty() )
    {
      maxValue = std::numeric_limits<double>::quiet_NaN();
      ok = true;
    }
    else
    {
      maxValue = maxVariant.toDouble( &ok );
      if ( !ok )
        throw QgsProcessingException( QObject::tr( "Invalid value for maximum: %1" ).arg( table.at( row * 3 + 1 ).toString() ) );
    }

    const double value = table.at( row * 3 + 2 ).toDouble( &ok );
    if ( !ok )
      throw QgsProcessingException( QObject::tr( "Invalid output value: %1" ).arg( table.at( row * 3 + 2 ).toString() ) );

    classes << QgsReclassifyUtils::RasterClass( minValue, maxValue, boundsType, value );
  }
  return classes;
}

///@endcond
