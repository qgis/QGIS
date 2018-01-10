/***************************************************************************
                         qgsalgorithmrasterlayeruniquevalues.cpp
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

#include "qgsalgorithmrasterlayeruniquevalues.h"

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

QgsProcessingAlgorithm::Flags QgsRasterLayerUniqueValuesReportAlgorithm::flags() const
{
  return QgsProcessingAlgorithm::flags() | QgsProcessingAlgorithm::FlagCanRunInBackground;
}

void QgsRasterLayerUniqueValuesReportAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_HTML_FILE" ),
                QObject::tr( "Unique values report" ), QObject::tr( "HTML files (*.html)" ), QVariant(), true ) );

  addOutput( new QgsProcessingOutputHtml( QStringLiteral( "OUTPUT_HTML_FILE" ), QObject::tr( "Unique values report" ) ) );

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
  int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );

  if ( !layer )
    return false;

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
  int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT_HTML_FILE" ), context );

  QHash< double, qgssize > uniqueValues;
  qgssize noDataCount = 0;

  qgssize layerSize = static_cast< qgssize >( mLayerWidth ) * static_cast< qgssize >( mLayerHeight );
  int maxWidth = 4000;
  int maxHeight = 4000;
  int nbBlocksWidth = std::ceil( 1.0 * mLayerWidth / maxWidth );
  int nbBlocksHeight = std::ceil( 1.0 * mLayerHeight / maxHeight );
  int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  QgsRasterIterator iter( mInterface.get() );
  iter.setMaximumTileWidth( maxWidth );
  iter.setMaximumTileHeight( maxHeight );
  iter.startRasterRead( band, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  QgsRasterBlock *rasterBlock = nullptr;
  while ( iter.readNextRasterPart( band, iterCols, iterRows, &rasterBlock, iterLeft, iterTop ) )
  {
    feedback->setProgress( 100 * ( ( iterTop / maxHeight * nbBlocksWidth ) + iterLeft / maxWidth ) / nbBlocks );
    for ( int row = 0; row < iterRows; row++ )
    {
      if ( feedback->isCanceled() )
        break;
      for ( int column = 0; column < iterCols; column++ )
      {
        if ( mHasNoDataValue && rasterBlock->isNoData( row, column ) )
        {
          noDataCount += 1;
        }
        else
        {
          double value = rasterBlock->value( row, column );
          uniqueValues[ value ]++;
        }
      }
    }
    delete rasterBlock;
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

  if ( !outputFile.isEmpty() )
  {
    QFile file( outputFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Text ) )
    {
      QString areaUnit = QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::distanceToAreaUnit( mCrs.mapUnits() ) );
      double pixelArea = mRasterUnitsPerPixelX * mRasterUnitsPerPixelY;

      QTextStream out( &file );
      out << QString( "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n" );
      out << QString( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Analyzed file" ) ).arg( mSource ).arg( QObject::tr( "band" ) ).arg( band );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Extent" ) ).arg( mExtent.toString() );
      out << QObject::tr( "<p>%1: %2 (%3)</p>\n" ).arg( QObject::tr( "Projection" ) ).arg( mCrs.description() ).arg( mCrs.authid() );
      out << QObject::tr( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Width in pixels" ) ).arg( mLayerWidth ).arg( QObject::tr( "units per pixel" ) ).arg( mRasterUnitsPerPixelX );
      out << QObject::tr( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Height in pixels" ) ).arg( mLayerHeight ).arg( QObject::tr( "units per pixel" ) ).arg( mRasterUnitsPerPixelY );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Total pixel count" ) ).arg( layerSize );
      if ( mHasNoDataValue )
        out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "NODATA pixel count" ) ).arg( noDataCount );
      out << QString( "<table><tr><td>%1</td><td>%2</td><td>%3 (%4)</td></tr>\n" ).arg( QObject::tr( "Value" ) ).arg( QObject::tr( "Pixel count" ) ).arg( QObject::tr( "Area" ) ).arg( areaUnit );

      for ( double key : sortedUniqueValues.keys() )
      {
        double area = sortedUniqueValues[key] * pixelArea;
        out << QString( "<tr><td>%1</td><td>%2</td><td>%3</td></tr>\n" ).arg( key ).arg( sortedUniqueValues[key] ).arg( QString::number( area, 'g', 16 ) );
      }
      out << QString( "</table>\n</body></html>" );
      outputs.insert( QStringLiteral( "OUTPUT_HTML_FILE" ), outputFile );
    }
  }

  return outputs;
}


///@endcond



