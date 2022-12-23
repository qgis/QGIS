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
#include "qgsrasterfilewriter.h"
#include "qgsreclassifyutils.h"
#include "qgsrasteranalysisutils.h"
#include "qgis.h"
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
  return QStringLiteral( "rasteranalysis" );
}

void QgsReclassifyAlgorithmBase::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ),
                QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT_RASTER" ) ) );

  addAlgorithmParams();

  std::unique_ptr< QgsProcessingParameterNumber > noDataValueParam = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "NO_DATA" ),
      QObject::tr( "Output no data value" ), QgsProcessingParameterNumber::Double, -9999 );
  noDataValueParam->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( noDataValueParam.release() );

  std::unique_ptr< QgsProcessingParameterEnum > boundsHandling = std::make_unique< QgsProcessingParameterEnum >( QStringLiteral( "RANGE_BOUNDARIES" ),
      QObject::tr( "Range boundaries" ), QStringList() << QObject::tr( "min < value <= max" )
      << QObject::tr( "min <= value < max" )
      << QObject::tr( "min <= value <= max" )
      << QObject::tr( "min < value < max" ),
      false, 0 );
  boundsHandling->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( boundsHandling.release() );

  std::unique_ptr< QgsProcessingParameterBoolean > missingValuesParam = std::make_unique< QgsProcessingParameterBoolean >( QStringLiteral( "NODATA_FOR_MISSING" ),
      QObject::tr( "Use no data when no range matches value" ), false, false );
  missingValuesParam->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( missingValuesParam.release() );

  std::unique_ptr< QgsProcessingParameterDefinition > typeChoice = QgsRasterAnalysisUtils::createRasterTypeParameter( QStringLiteral( "DATA_TYPE" ), QObject::tr( "Output data type" ), Qgis::DataType::Float32 );
  typeChoice->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( typeChoice.release() );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Reclassified raster" ) ) );
}

bool QgsReclassifyAlgorithmBase::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  mDataType = QgsRasterAnalysisUtils::rasterTypeChoiceToDataType( parameterAsEnum( parameters, QStringLiteral( "DATA_TYPE" ), context ) );
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_RASTER" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for RASTER_BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand )
                                  .arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = std::abs( layer->rasterUnitsPerPixelX() );
  mRasterUnitsPerPixelY = std::abs( layer->rasterUnitsPerPixelY() );
  mNbCellsXProvider = mInterface->xSize();
  mNbCellsYProvider = mInterface->ySize();

  mNoDataValue = parameterAsDouble( parameters, QStringLiteral( "NO_DATA" ), context );
  mUseNoDataForMissingValues = parameterAsBoolean( parameters, QStringLiteral( "NODATA_FOR_MISSING" ), context );

  const int boundsType = parameterAsEnum( parameters, QStringLiteral( "RANGE_BOUNDARIES" ), context );
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
  const QVector< QgsReclassifyUtils::RasterClass > classes = createClasses( mBoundsType, parameters, context, feedback );

  QgsReclassifyUtils::reportClasses( classes, feedback );
  QgsReclassifyUtils::checkForOverlaps( classes, feedback );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  const QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  std::unique_ptr< QgsRasterFileWriter > writer = std::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( mDataType, mNbCellsXProvider, mNbCellsYProvider, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, mNoDataValue );

  QgsReclassifyUtils::reclassify( classes, mInterface.get(), mBand, mExtent, mNbCellsXProvider, mNbCellsYProvider, provider.get(), mNoDataValue, mUseNoDataForMissingValues,
                                  feedback );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}


//
// QgsReclassifyByLayerAlgorithm
//

QString QgsReclassifyByLayerAlgorithm::name() const
{
  return QStringLiteral( "reclassifybylayer" );
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

QgsReclassifyByLayerAlgorithm *QgsReclassifyByLayerAlgorithm::createInstance() const
{
  return new QgsReclassifyByLayerAlgorithm();
}

void QgsReclassifyByLayerAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_TABLE" ),
                QObject::tr( "Layer containing class breaks" ), QList< int >() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "MIN_FIELD" ),
                QObject::tr( "Minimum class value field" ), QVariant(), QStringLiteral( "INPUT_TABLE" ), QgsProcessingParameterField::Numeric ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "MAX_FIELD" ),
                QObject::tr( "Maximum class value field" ), QVariant(), QStringLiteral( "INPUT_TABLE" ), QgsProcessingParameterField::Numeric ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "VALUE_FIELD" ),
                QObject::tr( "Output value field" ), QVariant(), QStringLiteral( "INPUT_TABLE" ), QgsProcessingParameterField::Numeric ) );
}

bool QgsReclassifyByLayerAlgorithm::_prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  std::unique_ptr< QgsFeatureSource >tableSource( parameterAsSource( parameters, QStringLiteral( "INPUT_TABLE" ), context ) );
  if ( !tableSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_TABLE" ) ) );

  const QString fieldMin = parameterAsString( parameters, QStringLiteral( "MIN_FIELD" ), context );
  mMinFieldIdx = tableSource->fields().lookupField( fieldMin );
  if ( mMinFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for MIN_FIELD: %1" ).arg( fieldMin ) );
  const QString fieldMax = parameterAsString( parameters, QStringLiteral( "MAX_FIELD" ), context );
  mMaxFieldIdx = tableSource->fields().lookupField( fieldMax );
  if ( mMaxFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for MAX_FIELD: %1" ).arg( fieldMax ) );
  const QString fieldValue = parameterAsString( parameters, QStringLiteral( "VALUE_FIELD" ), context );
  mValueFieldIdx = tableSource->fields().lookupField( fieldValue );
  if ( mValueFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for VALUE_FIELD: %1" ).arg( fieldValue ) );

  QgsFeatureRequest request;
  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setSubsetOfAttributes( QgsAttributeList() << mMinFieldIdx << mMaxFieldIdx << mValueFieldIdx );
  mTableIterator = tableSource->getFeatures( request );

  return true;
}

QVector<QgsReclassifyUtils::RasterClass> QgsReclassifyByLayerAlgorithm::createClasses( QgsRasterRange::BoundsType boundsType, const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  QVector< QgsReclassifyUtils::RasterClass > classes;
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
  return QStringLiteral( "reclassifybytable" );
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

QgsReclassifyByTableAlgorithm *QgsReclassifyByTableAlgorithm::createInstance() const
{
  return new QgsReclassifyByTableAlgorithm();
}

void QgsReclassifyByTableAlgorithm::addAlgorithmParams()
{
  addParameter( new QgsProcessingParameterMatrix( QStringLiteral( "TABLE" ),
                QObject::tr( "Reclassification table" ),
                1, false, QStringList() << QObject::tr( "Minimum" )
                << QObject::tr( "Maximum" )
                << QObject::tr( "Value" ) ) );
}

bool QgsReclassifyByTableAlgorithm::_prepareAlgorithm( const QVariantMap &, QgsProcessingContext &, QgsProcessingFeedback * )
{
  return true;
}

QVector<QgsReclassifyUtils::RasterClass> QgsReclassifyByTableAlgorithm::createClasses( QgsReclassifyUtils::RasterClass::BoundsType boundsType, const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QVariantList table = parameterAsMatrix( parameters, QStringLiteral( "TABLE" ), context );
  if ( table.count() % 3 != 0 )
    throw QgsProcessingException( QObject::tr( "Invalid value for TABLE: list must contain a multiple of 3 elements (found %1)" ).arg( table.count() ) );

  const int rows = table.count() / 3;
  QVector< QgsReclassifyUtils::RasterClass > classes;
  classes.reserve( rows );
  for ( int row = 0; row < rows; ++row )
  {
    bool ok = false;

    // null values map to nan, which corresponds to a range extended to +/- infinity....
    const QVariant minVariant = table.at( row * 3 );
    double minValue;
    if ( QgsVariantUtils::isNull( minVariant )  || minVariant.toString().isEmpty() )
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


