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
#include "qgsunittypes.h"

#include <QTextStream>

///@cond PRIVATE

QString QgsRasterLayerUniqueValuesReportAlgorithm::name() const
{
  return u"rasterlayeruniquevaluesreport"_s;
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
  return u"rasteranalysis"_s;
}

void QgsRasterLayerUniqueValuesReportAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT_HTML_FILE"_s, QObject::tr( "Unique values report" ), QObject::tr( "HTML files (*.html)" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT_TABLE"_s, QObject::tr( "Unique values table" ), Qgis::ProcessingSourceType::Vector, QVariant(), true, false ) );

  addOutput( new QgsProcessingOutputString( u"EXTENT"_s, QObject::tr( "Extent" ) ) );
  addOutput( new QgsProcessingOutputString( u"CRS_AUTHID"_s, QObject::tr( "CRS authority identifier" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"WIDTH_IN_PIXELS"_s, QObject::tr( "Width in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"HEIGHT_IN_PIXELS"_s, QObject::tr( "Height in pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"TOTAL_PIXEL_COUNT"_s, QObject::tr( "Total pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"NODATA_PIXEL_COUNT"_s, QObject::tr( "NoData pixel count" ) ) );
}

QString QgsRasterLayerUniqueValuesReportAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm returns the count and area of each unique value in a given raster layer. "
                      "The area calculation is done in the area unit of the layer's CRS." );
}

QString QgsRasterLayerUniqueValuesReportAlgorithm::shortDescription() const
{
  return QObject::tr( "Returns the count and area of each unique value in a given raster layer." );
}

QgsRasterLayerUniqueValuesReportAlgorithm *QgsRasterLayerUniqueValuesReportAlgorithm::createInstance() const
{
  return new QgsRasterLayerUniqueValuesReportAlgorithm();
}

bool QgsRasterLayerUniqueValuesReportAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );
  const int band = parameterAsInt( parameters, u"BAND"_s, context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

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
  const QString outputFile = parameterAsFileOutput( parameters, u"OUTPUT_HTML_FILE"_s, context );

  QString areaUnit = QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::distanceToAreaUnit( mCrs.mapUnits() ) );

  QString tableDest;
  std::unique_ptr<QgsFeatureSink> sink;
  if ( parameters.contains( u"OUTPUT_TABLE"_s ) && parameters.value( u"OUTPUT_TABLE"_s ).isValid() )
  {
    QgsFields outFields;
    outFields.append( QgsField( u"value"_s, QMetaType::Type::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( u"count"_s, QMetaType::Type::LongLong, QString(), 20 ) );
    outFields.append( QgsField( areaUnit.replace( u"²"_s, "2"_L1 ), QMetaType::Type::Double, QString(), 20, 8 ) );
    sink.reset( parameterAsSink( parameters, u"OUTPUT_TABLE"_s, context, tableDest, outFields, Qgis::WkbType::NoGeometry, QgsCoordinateReferenceSystem() ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT_TABLE"_s ) );
  }

  QHash<double, qgssize> uniqueValues;
  qgssize noDataCount = 0;

  const qgssize layerSize = static_cast<qgssize>( mLayerWidth ) * static_cast<qgssize>( mLayerHeight );

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  bool isNoData = false;
  std::unique_ptr<QgsRasterBlock> rasterBlock;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop ) )
  {
    feedback->setProgress( 100 * iter.progress( mBand ) );
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
          uniqueValues[value]++;
        }
      }
    }
    if ( feedback->isCanceled() )
      break;
  }

  QMap<double, qgssize> sortedUniqueValues;
  for ( auto it = uniqueValues.constBegin(); it != uniqueValues.constEnd(); ++it )
  {
    sortedUniqueValues.insert( it.key(), it.value() );
  }

  QVariantMap outputs;
  outputs.insert( u"EXTENT"_s, mExtent.toString() );
  outputs.insert( u"CRS_AUTHID"_s, mCrs.authid() );
  outputs.insert( u"WIDTH_IN_PIXELS"_s, mLayerWidth );
  outputs.insert( u"HEIGHT_IN_PIXELS"_s, mLayerHeight );
  outputs.insert( u"TOTAL_PIXEL_COUNT"_s, layerSize );
  outputs.insert( u"NODATA_PIXEL_COUNT"_s, noDataCount );

  const double pixelArea = mRasterUnitsPerPixelX * mRasterUnitsPerPixelY;

  if ( !outputFile.isEmpty() )
  {
    QFile file( outputFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      const QString encodedAreaUnit = QgsStringUtils::ampersandEncode( areaUnit );

      QTextStream out( &file );
      out << u"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n"_s;
      out << u"<p>%1: %2 (%3 %4)</p>\n"_s.arg( QObject::tr( "Analyzed file" ), mSource, QObject::tr( "band" ) ).arg( mBand );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Extent" ), mExtent.toString() );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Projection" ), mCrs.userFriendlyIdentifier() );
      out << QObject::tr( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Width in pixels" ) ).arg( mLayerWidth ).arg( QObject::tr( "units per pixel" ) ).arg( mRasterUnitsPerPixelX );
      out << QObject::tr( "<p>%1: %2 (%3 %4)</p>\n" ).arg( QObject::tr( "Height in pixels" ) ).arg( mLayerHeight ).arg( QObject::tr( "units per pixel" ) ).arg( mRasterUnitsPerPixelY );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Total pixel count" ) ).arg( layerSize );
      if ( mHasNoDataValue )
        out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "NoData pixel count" ) ).arg( noDataCount );
      out << u"<table><tr><td>%1</td><td>%2</td><td>%3 (%4)</td></tr>\n"_s.arg( QObject::tr( "Value" ), QObject::tr( "Pixel count" ), QObject::tr( "Area" ), encodedAreaUnit );

      for ( auto it = sortedUniqueValues.constBegin(); it != sortedUniqueValues.constEnd(); ++it )
      {
        const double area = it.value() * pixelArea;
        out << u"<tr><td>%1</td><td>%2</td><td>%3</td></tr>\n"_s.arg( it.key() ).arg( it.value() ).arg( QString::number( area, 'g', 16 ) );
      }
      out << u"</table>\n</body></html>"_s;
      outputs.insert( u"OUTPUT_HTML_FILE"_s, outputFile );
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
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT_TABLE"_s ) );
    }
    sink->finalize();
    outputs.insert( u"OUTPUT_TABLE"_s, tableDest );
  }

  return outputs;
}


///@endcond
