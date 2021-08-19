/***************************************************************************
                         qgsalgorithmrasterlayeruniquevalues.cpp
                         ---------------------
    begin                : January 2019
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

#include "qgsalgorithmrastersurfacevolume.h"
#include "qgsstringutils.h"
#include <QTextStream>

///@cond PRIVATE

QString QgsRasterSurfaceVolumeAlgorithm::name() const
{
  return QStringLiteral( "rastersurfacevolume" );
}

QString QgsRasterSurfaceVolumeAlgorithm::displayName() const
{
  return QObject::tr( "Raster surface volume" );
}

QStringList QgsRasterSurfaceVolumeAlgorithm::tags() const
{
  return QObject::tr( "sum,volume,area,height,terrain,dem,elevation" ).split( ',' );
}

QString QgsRasterSurfaceVolumeAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterSurfaceVolumeAlgorithm::groupId() const
{
  return QStringLiteral( "rasteranalysis" );
}

void QgsRasterSurfaceVolumeAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( QStringLiteral( "INPUT" ),
                QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( QStringLiteral( "BAND" ),
                QObject::tr( "Band number" ), 1, QStringLiteral( "INPUT" ) ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "LEVEL" ),
                QObject::tr( "Base level" ), QgsProcessingParameterNumber::Double, 0 ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "METHOD" ),
                QObject::tr( "Method" ), QStringList()
                << QObject::tr( "Count Only Above Base Level" )
                << QObject::tr( "Count Only Below Base Level" )
                << QObject::tr( "Subtract Volumes Below Base Level" )
                << QObject::tr( "Add Volumes Below Base Level" ) ) );

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT_HTML_FILE" ),
                QObject::tr( "Surface volume report" ), QObject::tr( "HTML files (*.html)" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT_TABLE" ),
                QObject::tr( "Surface volume table" ), QgsProcessing::TypeVector, QVariant(), true, false ) );

  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "VOLUME" ), QObject::tr( "Volume" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "PIXEL_COUNT" ), QObject::tr( "Pixel count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "AREA" ), QObject::tr( "Area" ) ) );
}

QString QgsRasterSurfaceVolumeAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm calculates the volume under a raster grid's surface.\n\n"
                      "Several methods of volume calculation are available, which control whether "
                      "only values above or below the specified base level are considered, or "
                      "whether volumes below the base level should be added or subtracted from the total volume.\n\n"
                      "The algorithm outputs the calculated volume, the total area, and the total number of pixels analysed. "
                      "If the 'Count Only Above Base Level' or 'Count Only Below Base Level' methods are used, "
                      "then the calculated area and pixel count only includes pixels which are above or below the "
                      "specified base level respectively.\n\n"
                      "Units of the calculated volume are dependent on the coordinate reference system of "
                      "the input raster file. For a CRS in meters, with a DEM height in meters, the calculated "
                      "value will be in meters³. If instead the input raster is in a geographic coordinate system "
                      "(e.g. latitude/longitude values), then the result will be in degrees² × meters, and an "
                      "appropriate scaling factor will need to be applied in order to convert to meters³." );
}

QString QgsRasterSurfaceVolumeAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the volume under a raster grid's surface." );
}

QgsRasterSurfaceVolumeAlgorithm *QgsRasterSurfaceVolumeAlgorithm::createInstance() const
{
  return new QgsRasterSurfaceVolumeAlgorithm();
}

bool QgsRasterSurfaceVolumeAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
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

  mLevel = parameterAsDouble( parameters, QStringLiteral( "LEVEL" ), context );
  mMethod = static_cast< Method >( parameterAsEnum( parameters, QStringLiteral( "METHOD" ), context ) );
  return true;
}

QVariantMap QgsRasterSurfaceVolumeAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT_HTML_FILE" ), context );
  QString areaUnit = QgsUnitTypes::toAbbreviatedString( QgsUnitTypes::distanceToAreaUnit( mCrs.mapUnits() ) );

  QString tableDest;
  std::unique_ptr< QgsFeatureSink > sink;
  if ( parameters.contains( QStringLiteral( "OUTPUT_TABLE" ) ) && parameters.value( QStringLiteral( "OUTPUT_TABLE" ) ).isValid() )
  {
    QgsFields outFields;
    outFields.append( QgsField( QStringLiteral( "volume" ), QVariant::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( areaUnit.replace( QStringLiteral( "²" ), QStringLiteral( "2" ) ), QVariant::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( QStringLiteral( "pixel_count" ), QVariant::LongLong ) );
    sink.reset( parameterAsSink( parameters, QStringLiteral( "OUTPUT_TABLE" ), context, tableDest, outFields, QgsWkbTypes::NoGeometry, QgsCoordinateReferenceSystem() ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT_TABLE" ) ) );
  }

  double volume = 0;
  long long count = 0;

  const int maxWidth = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_WIDTH;
  const int maxHeight = QgsRasterIterator::DEFAULT_MAXIMUM_TILE_HEIGHT;
  const int nbBlocksWidth = static_cast< int >( std::ceil( 1.0 * mLayerWidth / maxWidth ) );
  const int nbBlocksHeight = static_cast< int >( std::ceil( 1.0 * mLayerHeight / maxHeight ) );
  const int nbBlocks = nbBlocksWidth * nbBlocksHeight;

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
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
        if ( mHasNoDataValue && rasterBlock->isNoData( row, column ) )
        {
          continue;
        }

        const double z = rasterBlock->value( row, column ) - mLevel;

        switch ( mMethod )
        {
          case CountOnlyAboveBaseLevel:
            if ( z > 0.0 )
            {
              volume += z;
              count++;
            }
            continue;

          case CountOnlyBelowBaseLevel:
            if ( z < 0.0 )
            {
              volume += z;
              count++;
            }
            continue;

          case SubtractVolumesBelowBaseLevel:
            volume += z;
            count++;
            continue;

          case AddVolumesBelowBaseLevel:
            volume += std::fabs( z );
            count++;
            continue;
        }
      }
    }
  }

  QVariantMap outputs;
  const double pixelArea = mRasterUnitsPerPixelX * mRasterUnitsPerPixelY;
  const double area = count * pixelArea;
  volume *= pixelArea;
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
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Volume" ), QString::number( volume, 'g', 16 ) );
      out << QObject::tr( "<p>%1: %2</p>\n" ).arg( QObject::tr( "Pixel count" ) ).arg( count );
      out << QObject::tr( "<p>%1: %2 %3</p>\n" ).arg( QObject::tr( "Area" ), QString::number( area, 'g', 16 ), encodedAreaUnit );
      out << QStringLiteral( "</body></html>" );
      outputs.insert( QStringLiteral( "OUTPUT_HTML_FILE" ), outputFile );
    }
  }

  if ( sink )
  {
    QgsFeature f;
    f.setAttributes( QgsAttributes() << volume << area << count );
    if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink.get(), parameters, QStringLiteral( "OUTPUT_TABLE" ) ) );
    outputs.insert( QStringLiteral( "OUTPUT_TABLE" ), tableDest );
  }
  outputs.insert( QStringLiteral( "VOLUME" ), volume );
  outputs.insert( QStringLiteral( "AREA" ), area );
  outputs.insert( QStringLiteral( "PIXEL_COUNT" ), count );
  return outputs;
}


///@endcond



