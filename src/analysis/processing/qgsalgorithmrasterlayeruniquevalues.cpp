/***************************************************************************
                         qgsalgorithmrasterlayeruniquevalues.cpp
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmrasterlayeruniquevalues.h"
#include "qgsstringutils.h"
#include <QTextStream>

///@cond PRIVATE

QString QgsRasterLayerUniqueValuesReportAlgorithm::name() const
{
  return QStringLiteral( "rasterlayeruniquevaluesreport" );
}

QString QgsRasterLayerUniqueValuesReportAlgorithm::displayName() const
{
  return QObject::tr( "Raster layer unique values report" );
}

QStringList QgsRasterLayerUniqueValuesReportAlgorithm::tags() const
{
  return QObject::tr( "count,area,statistics" ).split( ',' );
}

QString QgsRasterLayerUniqueValuesReportAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterLayerUniqueValuesReportAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

void QgsRasterLayerUniqueValuesReportAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_HTML_FILE" ),
                QObject::tr( "Unique values report" ), QObject::tr( "HTML files (*.html)" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_TABLE" ),
                QObject::tr( "Unique values table" ), QgsProcessing::TypeVector, QVariant(), true, false ) );

  addOutput( new QgsProcessingOutputString( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( QStringLiteral( "CRS_AUTHID" ), QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "WIDTH_IN_PIXELS" ), QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "HEIGHT_IN_PIXELS" ), QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TOTAL_PIXEL_COUNT" ), QObject::tr( "Total pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "NODATA_PIXEL_COUNT" ), QObject::tr( "NODATA pixel count" ) ) );
}

QString QgsRasterLayerUniqueValuesReportAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm returns the count and area of each unique value in a given raster layer." );
}

QgsRasterLayerUniqueValuesReportAlgorithm *QgsRasterLayerUniqueValuesReportAlgorithm::createInstance() const
{
  return new QgsRasterLayerUniqueValuesReportAlgorithm();
}

bool QgsRasterLayerUniqueValuesReportAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );
  const int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  mBand = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand )
                                  .arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( band );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = layer->rasterUnitsPerPixelX();
  mRasterUnitsPerPixelY = layer->rasterUnitsPerPixelY();
  mSource = layer->source();

  return true;
}

QVariantMap QgsRasterLayerUniqueValuesReportAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT_HTML_FILE" ), context );

  QString areaUnit = QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::distanceToAreaUnit( mCrs.mapUnits() ) );

  QString tableDest;
  std::unique_ptr< QgsFeatureSink > sink;
  if ( parameters.contains( QStringLiteral( "OUTPUT_TABLE" ) ) && parameters.value( QStringLiteral( "OUTPUT_TABLE" ) ).isValid() )
  {
    QgsFields outFields;
    outFields.append( QgsField( QStringLiteral( "value" ), QVariant::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( QStringLiteral( "count" ), QVariant::LongLong, QString(), 20 ) );
    outFields.append( QgsField( areaUnit.replace( QStringLiteral( "²" ), QStringLiteral( "2" ) ), QVariant::Double, QString(), 20, 8 ) );
    sink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT_TABLE" ), context, tableDest, outFields, QgsWkbTypes::NoGeometry, QgsCoordinateReferenceSystem() ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT_TABLE" ) ) );
  }

  QHash< double, qgssize > uniqueValues;
  qgssize noDataCount = 0;

  const qgssize layerSize = static_cast< qgssize >( mLayerWidth ) * static_cast< qgssize >( mLayerHeight );
  const int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  const int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  const int nbBlocksWidth = std::ceil( 1.0 * mLayerWidth / maxWidth );
  const int nbBlocksHeight = std::ceil( 1.0 * mLayerHeight / maxHeight );
  const int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  bool isNoData = false;
  std::unique_ptr< QgsRasterBlock > rasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        const double value = rasterBlock->valueAndNoData( row, column, isNoData );
        if ( mHasNoDataValue && isNoData )
        {
          noDataCount++;
        }
        else
        {
          uniqueValues[ value ]++;
        }
      }
    }
    if ( feedback->isCanceled() )
      break;
  }

  QMap< double, qgssize > sortedUniqueValues;
  for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
  {
    sortedUniqueValues.insert( it.key(), it.value() );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "EXTENT" ), mExtent.toString() );
  outputs.insert( QStringLiteral( "CRS_AUTHID" ), mCrs.authid() );
  outputs.insert( QStringLiteral( "WIDTH_IN_PIXELS" ), mLayerWidth );
  outputs.insert( QStringLiteral( "HEIGHT_IN_PIXELS" ), mLayerHeight );
  outputs.insert( QStringLiteral( "TOTAL_PIXEL_COUNT" ), layerSize );
  outputs.insert( QStringLiteral( "NODATA_PIXEL_COUNT" ), noDataCount );

  const double pixelArea = mRasterUnitsPerPixelX * mRasterUnitsPerPixelY;

  if ( !outputFile.isEmpty() )
  {
    QFile file( outputFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      const QString encodedAreaUnit = QgsStringUtils::ampersandEncode( areaUnit );

      QTextStream out( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      out.setCodec( "UTF-8" );
#endif
      out << QStringLiteral( "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n" );
      out << QStringLiteral( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Analyzed file" ), mSource, QObject::tr( "band" ) ).arg( mBand );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Extent" ), mExtent.toString() );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Projection" ), mCrs.userFriendlyIdentifier() );
      out << QObject::tr( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Width in pixels" ) ).arg( mLayerWidth ).arg( QObject::tr( "units per pixel" ) ).arg( mRasterUnitsPerPixelX );
      out << QObject::tr( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Height in pixels" ) ).arg( mLayerHeight ).arg( QObject::tr( "units per pixel" ) ).arg( mRasterUnitsPerPixelY );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Total pixel count" ) ).arg( layerSize );
      if ( mHasNoDataValue )
        out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "NODATA pixel count" ) ).arg( noDataCount );
      out << QStringLiteral( "<table><tr><td>%1</td><td>%2</td><td>%3 (%4)</td></tr>\n" ).arg( QObject::tr( "Value" ), QObject::tr( "Pixel count" ), QObject::tr( "Area" ), encodedAreaUnit );

      for ( auto it = sortedUniqueValues.constBegin(); it != sortedUniqueValues.constEnd(); ++it )
      {
        const double area = it.value() * pixelArea;
        out << QStringLiteral( "<tr><td>%1</td><td>%2</td><td>%3</td></tr>\n" ).arg( it.key() ).arg( it.value() ).arg( QString::number( area, 'g', 16 ) );
      }
      out << QStringLiteral( "</table>\n</body></html>" );
      outputs.insert( QStringLiteral( "OUTPUT_HTML_FILE" ), outputFile );
    }
  }

  if ( sink )
  {
    for ( auto it = sortedUniqueValues.constBegin(); it != sortedUniqueValues.constEnd(); ++it )
    {
      QgsFeature f;
      const double area = it.value() * pixelArea;
      f.setAttributes( QgsAttributes() << it.key() << it.value() << area );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT_TABLE" ) ) );
    }
    outputs.insert( QStringLiteral( "OUTPUT_TABLE" ), tableDest );
  }

  return outputs;
}


///@endcond



