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
#include "qgsgeos.h"
#include "qgslogger.h"
#include "qgsrasterfilewriter.h"
#include "qgis.h"

///@cond PRIVATE

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

QString QgsReclassifyByLayerAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsReclassifyByLayerAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

void QgsReclassifyByLayerAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT_RASTER" ),
                QObject::tr( "Raster layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "RASTER_BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT_RASTER" ) ) );

  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT_TABLE" ),
                QObject::tr( "Layer containing class breaks" ), QList< int >() << QgsProcessing::TypeVector ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "MIN_FIELD" ),
                QObject::tr( "Minimum class value field" ), QVariant(), QStringLiteral( "INPUT_TABLE" ), QgsProcessingParameterField::Numeric ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "MAX_FIELD" ),
                QObject::tr( "Maximum class value field" ), QVariant(), QStringLiteral( "INPUT_TABLE" ), QgsProcessingParameterField::Numeric ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "VALUE_FIELD" ),
                QObject::tr( "Output value field" ), QVariant(), QStringLiteral( "INPUT_TABLE" ), QgsProcessingParameterField::Numeric ) );

  std::unique_ptr< QgsProcessingParameterNumber > noDataValueParam = qgis::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "NO_DATA" ),
      QObject::tr( "Output no data value" ), QgsProcessingParameterNumber::Double, -9999 );
  noDataValueParam->setFlags( QgsProcessingParameterDefinition::FlagAdvanced );
  addParameter( noDataValueParam.release() );
  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Reclassified raster" ) ) );
}

QString QgsReclassifyByLayerAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm reclassifies a raster band by assigning new class values based on the ranges specified in a vector table." );
}

QgsReclassifyByLayerAlgorithm *QgsReclassifyByLayerAlgorithm::createInstance() const
{
  return new QgsReclassifyByLayerAlgorithm();
}

bool QgsReclassifyByLayerAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT_RASTER" ), context );
  mBand = parameterAsInt( parameters, QStringLiteral( "RASTER_BAND" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT_RASTER" ) ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = std::abs( layer->rasterUnitsPerPixelX() );
  mRasterUnitsPerPixelY = std::abs( layer->rasterUnitsPerPixelX() );
  mNbCellsXProvider = mInterface->xSize();
  mNbCellsYProvider = mInterface->ySize();

  mNoDataValue = parameterAsDouble( parameters, QStringLiteral( "NO_DATA" ), context );

  std::unique_ptr< QgsFeatureSource >tableSource( parameterAsSource( parameters, QStringLiteral( "INPUT_TABLE" ), context ) );
  if ( !tableSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT_TABLE" ) ) );

  QString fieldMin = parameterAsString( parameters, QStringLiteral( "MIN_FIELD" ), context );
  mMinFieldIdx = tableSource->fields().lookupField( fieldMin );
  if ( mMinFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for MIN_FIELD: %1" ).arg( fieldMin ) );
  QString fieldMax = parameterAsString( parameters, QStringLiteral( "MAX_FIELD" ), context );
  mMaxFieldIdx = tableSource->fields().lookupField( fieldMax );
  if ( mMaxFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for MAX_FIELD: %1" ).arg( fieldMax ) );
  QString fieldValue = parameterAsString( parameters, QStringLiteral( "VALUE_FIELD" ), context );
  mValueFieldIdx = tableSource->fields().lookupField( fieldValue );
  if ( mValueFieldIdx < 0 )
    throw QgsProcessingException( QObject::tr( "Invalid field specified for VALUE_FIELD: %1" ).arg( fieldValue ) );

  QgsFeatureRequest request;
  request.setFlags( QgsFeatureRequest::NoGeometry );
  request.setSubsetOfAttributes( QgsAttributeList() << mMinFieldIdx << mMaxFieldIdx << mValueFieldIdx );
  mTableIterator = tableSource->getFeatures( request );

  return true;
}

QVariantMap QgsReclassifyByLayerAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  // step 1 - build up the table of reclassification values
  QVector< Class > classes;
  QgsFeature f;
  while ( mTableIterator.nextFeature( f ) )
  {
    bool ok = false;
    double minValue = f.attribute( mMinFieldIdx ).toDouble( &ok );
    if ( !ok )
      throw QgsProcessingException( QObject::tr( "Invalid value for minimum: %1" ).arg( f.attribute( mMinFieldIdx ).toString() ) );
    double maxValue = f.attribute( mMaxFieldIdx ).toDouble( &ok );
    if ( !ok )
      throw QgsProcessingException( QObject::tr( "Invalid value for maximum: %1" ).arg( f.attribute( mMaxFieldIdx ).toString() ) );
    double value = f.attribute( mValueFieldIdx ).toDouble( &ok );
    if ( !ok )
      throw QgsProcessingException( QObject::tr( "Invalid output value: %1" ).arg( f.attribute( mValueFieldIdx ).toString() ) );

    classes << Class( minValue, maxValue, value );
  }

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  std::unique_ptr< QgsRasterFileWriter > writer = qgis::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( Qgis::Float32, mNbCellsXProvider, mNbCellsYProvider, mExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );

  provider->setNoDataValue( 1, mNoDataValue );

  reclassify( classes, provider.get(), feedback );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

void QgsReclassifyByLayerAlgorithm::reclassify( const QVector<QgsReclassifyByLayerAlgorithm::Class> &classes, QgsRasterDataProvider *destinationRaster, QgsProcessingFeedback *feedback )
{
  int maxWidth = 4000;
  int maxHeight = 4000;

  QgsRasterIterator iter( mInterface.get() );
  iter.setMaximumTileWidth( maxWidth );
  iter.setMaximumTileHeight( maxHeight );
  iter.startRasterRead( mBand, mNbCellsXProvider, mNbCellsYProvider, mExtent );

  int nbBlocksWidth = std::ceil( 1.0 * mNbCellsXProvider / maxWidth );
  int nbBlocksHeight = std::ceil( 1.0 * mNbCellsYProvider / maxHeight );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  destinationRaster->setEditable( true );
  QgsRasterBlock *rasterBlock = nullptr;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, &rasterBlock, iterLeft, iterTop ) )
  {
    feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    std::unique_ptr< QgsRasterBlock > reclassifiedBlock = qgis::make_unique< QgsRasterBlock >( Qgis::Float32, iterCols, iterRows );

    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( rasterBlock->isNoData( row, column ) )
          reclassifiedBlock->setValue( row, column, mNoDataValue );
        else
        {
          double value = rasterBlock->value( row, column );
          double newValue = reclassifyValue( classes, value );
          reclassifiedBlock->setValue( row, column, newValue );
        }
      }
    }
    destinationRaster->writeBlock( reclassifiedBlock.get(), 1, iterLeft, iterTop );

    delete rasterBlock;
  }
  destinationRaster->setEditable( false );
}

double QgsReclassifyByLayerAlgorithm::reclassifyValue( const QVector<QgsReclassifyByLayerAlgorithm::Class> &classes, double input ) const
{
  for ( const QgsReclassifyByLayerAlgorithm::Class &c : classes )
  {
    if ( input >= c.range.min() && input < c.range.max() )
      return c.value;
  }
  return mNoDataValue;

}

///@endcond



