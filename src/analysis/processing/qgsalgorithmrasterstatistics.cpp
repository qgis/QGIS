/***************************************************************************
                         qgsalgorithmrasterstatistics.cpp
                         ---------------------
    begin                : December 2019
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

#include "qgsalgorithmrasterstatistics.h"

#include <QTextStream>

///@cond PRIVATE

QString QgsRasterStatisticsAlgorithm::name() const
{
  return u"rasterlayerstatistics"_s;
}

QString QgsRasterStatisticsAlgorithm::displayName() const
{
  return QObject::tr( "Raster layer statistics" );
}

QStringList QgsRasterStatisticsAlgorithm::tags() const
{
  return QObject::tr( "raster,stats,statistics,maximum,minimum,range,sum,mean,standard,deviation,summary" ).split( ',' );
}

QString QgsRasterStatisticsAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterStatisticsAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

QString QgsRasterStatisticsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes basic statistics from the values in a given band of the raster layer." );
}

QString QgsRasterStatisticsAlgorithm::shortDescription() const
{
  return QObject::tr( "Computes basic statistics from the values in a given band of the raster layer." );
}

QgsRasterStatisticsAlgorithm *QgsRasterStatisticsAlgorithm::createInstance() const
{
  return new QgsRasterStatisticsAlgorithm();
}

void QgsRasterStatisticsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );

  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT_HTML_FILE"_s, QObject::tr( "Statistics" ), QObject::tr( "HTML files (*.html *.HTML)" ), QVariant(), true ) );
  addOutput( new QgsProcessingOutputNumber( u"COUNT"_s, QObject::tr( "Count of non-NoData pixels" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MIN"_s, QObject::tr( "Minimum value" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MAX"_s, QObject::tr( "Maximum value" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"RANGE"_s, QObject::tr( "Range" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"SUM"_s, QObject::tr( "Sum" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MEAN"_s, QObject::tr( "Mean value" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"STD_DEV"_s, QObject::tr( "Standard deviation" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"SUM_OF_SQUARES"_s, QObject::tr( "Sum of the squares" ) ) );
}

QVariantMap QgsRasterStatisticsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  const int band = parameterAsInt( parameters, u"BAND"_s, context );
  if ( band < 1 || band > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( band ).arg( layer->bandCount() ) );

  const QString outputFile = parameterAsFileOutput( parameters, u"OUTPUT_HTML_FILE"_s, context );

  const QgsRasterBandStats stat = layer->dataProvider()->bandStatistics( band, Qgis::RasterBandStatistic::All, QgsRectangle(), 0 );

  QVariantMap outputs;
  outputs.insert( u"COUNT"_s, stat.elementCount );
  outputs.insert( u"MIN"_s, stat.minimumValue );
  outputs.insert( u"MAX"_s, stat.maximumValue );
  outputs.insert( u"RANGE"_s, stat.range );
  outputs.insert( u"SUM"_s, stat.sum );
  outputs.insert( u"MEAN"_s, stat.mean );
  outputs.insert( u"STD_DEV"_s, stat.stdDev );
  outputs.insert( u"SUM_OF_SQUARES"_s, stat.sumOfSquares );

  if ( !outputFile.isEmpty() )
  {
    QFile file( outputFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream out( &file );
      out << u"<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n"_s;
      out << QObject::tr( "<p>Analyzed file: %1 (band %2)</p>\n" ).arg( layer->source() ).arg( band );
      out << QObject::tr( "<p>Count of non-NoData pixels: %1</p>\n" ).arg( stat.elementCount );
      out << QObject::tr( "<p>Minimum value: %1</p>\n" ).arg( stat.minimumValue, 0, 'g', 16 );
      out << QObject::tr( "<p>Maximum value: %1</p>\n" ).arg( stat.maximumValue, 0, 'g', 16 );
      out << QObject::tr( "<p>Range: %1</p>\n" ).arg( stat.range, 0, 'g', 16 );
      out << QObject::tr( "<p>Sum: %1</p>\n" ).arg( stat.sum, 0, 'g', 16 );
      out << QObject::tr( "<p>Mean value: %1</p>\n" ).arg( stat.mean, 0, 'g', 16 );
      out << QObject::tr( "<p>Standard deviation: %1</p>\n" ).arg( stat.stdDev, 0, 'g', 16 );
      out << QObject::tr( "<p>Sum of the squares: %1</p>\n" ).arg( stat.sumOfSquares, 0, 'g', 16 );
      out << u"</body></html>"_s;

      outputs.insert( u"OUTPUT_HTML_FILE"_s, outputFile );
    }
  }

  return outputs;
}

///@endcond
