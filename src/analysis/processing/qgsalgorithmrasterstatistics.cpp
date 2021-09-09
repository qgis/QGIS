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
  return QStringLiteral( "rasterlayerstatistics" );
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
  return QStringLiteral( "rasteranalysis" );
}

QString QgsRasterStatisticsAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm computes basic statistics from the values in a given band of the raster layer." );
}

QgsRasterStatisticsAlgorithm *QgsRasterStatisticsAlgorithm::createInstance() const
{
  return new QgsRasterStatisticsAlgorithm();
}

void QgsRasterStatisticsAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_HTML_FILE" ), QObject::tr( "Statistics" ),
                QObject::tr( "HTML files (*.html *.HTML)" ), QVariant(), true ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MIN" ), QObject::tr( "Minimum value" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MAX" ), QObject::tr( "Maximum value" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "RANGE" ), QObject::tr( "Range" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "SUM" ), QObject::tr( "Sum" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "MEAN" ), QObject::tr( "Mean value" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "STD_DEV" ), QObject::tr( "Standard deviation" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "SUM_OF_SQUARES" ), QObject::tr( "Sum of the squares" ) ) );
}

QVariantMap QgsRasterStatisticsAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, QStringLiteral( "INPUT" ), context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, QStringLiteral( "INPUT" ) ) );

  const int band = parameterAsInt( parameters, QStringLiteral( "BAND" ), context );
  if ( band < 1 || band > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( band )
                                  .arg( layer->bandCount() ) );

  const QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT_HTML_FILE" ), context );

  const QgsRasterBandStats stat = layer->dataProvider()->bandStatistics( band, QgsRasterBandStats::All, QgsRectangle(), 0 );

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "MIN" ), stat.minimumValue );
  outputs.insert( QStringLiteral( "MAX" ), stat.maximumValue );
  outputs.insert( QStringLiteral( "RANGE" ), stat.range );
  outputs.insert( QStringLiteral( "SUM" ), stat.sum );
  outputs.insert( QStringLiteral( "MEAN" ), stat.mean );
  outputs.insert( QStringLiteral( "STD_DEV" ), stat.stdDev );
  outputs.insert( QStringLiteral( "SUM_OF_SQUARES" ), stat.sumOfSquares );

  if ( !outputFile.isEmpty() )
  {
    QFile file( outputFile );
    if ( file.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
    {
      QTextStream out( &file );
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
      out.setCodec( "UTF-8" );
#endif
      out << QStringLiteral( "<html><head><meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\"/></head><body>\n" );
      out << QObject::tr( "<p>Analyzed file: %1 (band %2)</p>\n" ).arg( layer->source() ).arg( band );
      out << QObject::tr( "<p>Minimum value: %1</p>\n" ).arg( stat.minimumValue, 0, 'g', 16 );
      out << QObject::tr( "<p>Maximum value: %1</p>\n" ).arg( stat.maximumValue, 0, 'g', 16 );
      out << QObject::tr( "<p>Range: %1</p>\n" ).arg( stat.range, 0, 'g', 16 );
      out << QObject::tr( "<p>Sum: %1</p>\n" ).arg( stat.sum, 0, 'g', 16 );
      out << QObject::tr( "<p>Mean value: %1</p>\n" ).arg( stat.mean, 0, 'g', 16 );
      out << QObject::tr( "<p>Standard deviation: %1</p>\n" ).arg( stat.stdDev, 0, 'g', 16 );
      out << QObject::tr( "<p>Sum of the squares: %1</p>\n" ).arg( stat.sumOfSquares, 0, 'g', 16 );
      out << QStringLiteral( "</body></html>" );

      outputs.insert( QStringLiteral( "OUTPUT_HTML_FILE" ), outputFile );
    }
  }

  return outputs;
}

///@endcond
