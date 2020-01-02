/***************************************************************************
                         qgsalgorithmlinedensity.cpp
                         ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Clemens Raffler
    email                : clemens dot raffler at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmlinedensity.h"
#include "qgscircle.h"
#include "qgsgeometryengine.h"
#include "qgsrasterfilewriter.h"

///@cond PRIVATE

QString QgsLineDensityAlgorithm::name() const
{
  return QStringLiteral( "linedensity" );
}

QString QgsLineDensityAlgorithm::displayName() const
{
  return QObject::tr( "Line density" );
}

QStringList QgsLineDensityAlgorithm::tags() const
{
  return QObject::tr( "density,kernel,line,line density,interpolation,weight" ).split( ',' );
}

QString QgsLineDensityAlgorithm::group() const
{
  return QObject::tr( "Interpolation" );
}

QString QgsLineDensityAlgorithm::groupId() const
{
  return QStringLiteral( "interpolation" );
}

void QgsLineDensityAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input line layer" ), QList<int>() << QgsProcessing::TypeVectorLine ) );
  addParameter( new QgsProcessingParameterField( QStringLiteral( "WEIGHT" ), QObject::tr( "Weight field " ), QVariant(), QStringLiteral( "INPUT" ), QgsProcessingParameterField::Numeric, false, true ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "RADIUS" ), QObject::tr( "Search radius" ), 10, QStringLiteral( "INPUT" ), false, 0 ) );
  addParameter( new QgsProcessingParameterDistance( QStringLiteral( "PIXEL_SIZE" ), QObject::tr( "Pixel size" ), 10, QStringLiteral( "INPUT" ), false ) );

  addParameter( new QgsProcessingParameterRasterDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Line density raster" ) ) );
}

QString QgsLineDensityAlgorithm::shortHelpString() const
{
  return QObject::tr( "The line density interpolation algorithm calculates a density measure of linear features "
                      "which is obtained in a circular neighborhood within each raster cell. "
                      "First, the length of the segment of each line that is intersected by the circular neighborhood "
                      "is multiplied with the lines weight factor. In a second step, all length values are summed and "
                      "divided by the area of the circular neighborhood. This process is repeated for all raster cells."
                    );
}

QgsLineDensityAlgorithm *QgsLineDensityAlgorithm::createInstance() const
{
  return new QgsLineDensityAlgorithm();
}

bool QgsLineDensityAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );
  mSource.reset( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !mSource )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  mWeightField = parameterAsString( parameters, QStringLiteral( "WEIGHT" ), context );
  mSearchRadius = parameterAsDouble( parameters, QStringLiteral( "RADIUS" ), context );
  mPixelSize = parameterAsDouble( parameters, QStringLiteral( "PIXEL_SIZE" ), context );

  mExtent = mSource->sourceExtent();
  mCrs = mSource->sourceCrs();

  if ( context.project() )
  {
    mDa.setEllipsoid( context.project()->ellipsoid() );
  }
  mDa.setSourceCrs( mCrs, context.transformContext() );


  //get cell midpoint from top left cell
  QgsPoint firstCellMidpoint = QgsPoint( mExtent.xMinimum() + ( mPixelSize / 2 ), mExtent.yMaximum() - ( mPixelSize / 2 ) );
  QgsCircle searchCircle = QgsCircle( firstCellMidpoint, mSearchRadius );
  mSearchGeometry = QgsGeometry( searchCircle.toPolygon() );
  mSearchGeometryArea = mDa.measureArea( mSearchGeometry );

  mIndex = QgsSpatialIndex( *mSource, nullptr, QgsSpatialIndex::FlagStoreFeatureGeometries );


  return true;
}

QVariantMap QgsLineDensityAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  QFileInfo fi( outputFile );
  const QString outputFormat = QgsRasterFileWriter::driverForExtension( fi.suffix() );

  int rows = std::max( std::ceil( mExtent.height() / mPixelSize ), 1.0 );
  int cols = std::max( std::ceil( mExtent.width() / mPixelSize ), 1.0 );

  //build new raster extent based on number of columns and cellsize
  //this prevents output cellsize being calculated too small
  QgsRectangle rasterExtent = QgsRectangle( mExtent.xMinimum(), mExtent.yMaximum() - ( rows * mPixelSize ), mExtent.xMinimum() + ( cols * mPixelSize ), mExtent.yMaximum() );

  std::unique_ptr< QgsRasterFileWriter > writer = qgis::make_unique< QgsRasterFileWriter >( outputFile );
  writer->setOutputProviderKey( QStringLiteral( "gdal" ) );
  writer->setOutputFormat( outputFormat );
  std::unique_ptr<QgsRasterDataProvider > provider( writer->createOneBandRaster( Qgis::Float32, cols, rows, rasterExtent, mCrs ) );
  if ( !provider )
    throw QgsProcessingException( QObject::tr( "Could not create raster output: %1" ).arg( outputFile ) );
  if ( !provider->isValid() )
    throw QgsProcessingException( QObject::tr( "Could not create raster output %1: %2" ).arg( outputFile, provider->error().message( QgsErrorMessage::Text ) ) );

  provider->setNoDataValue( 1, -9999 );

  int totalCellcnt = rows * cols;
  int cellcnt = 0;

  for ( int row = 0; row < rows; row++ )
  {
    std::unique_ptr< QgsRasterBlock > rasterDataLine = qgis::make_unique< QgsRasterBlock >( Qgis::Float32, cols, 1 );
    for ( int col = 0; col < cols; col++ )
    {
      if ( feedback->isCanceled() )
      {
        break;
      }

      if ( col > 0 )
        mSearchGeometry.translate( mPixelSize, 0 );

      QList<QgsFeatureId> fids = mIndex.intersects( mSearchGeometry.boundingBox() );

      std::unique_ptr< QgsGeometryEngine > engine( QgsGeometry::createGeometryEngine( mSearchGeometry.constGet() ) );
      engine->prepareGeometry();

      //remove non-intersecting fids
      int i = 0;
      while ( i < fids.size() )
      {
        QgsFeatureId fid = fids.at( i );
        QgsGeometry lineGeom = mIndex.geometry( fid );

        if ( engine->intersects( lineGeom.constGet() ) == false )
        {
          fids.removeAt( i );
        }
        i++;
      }

      QgsFeatureIds isectingFids = QSet< QgsFeatureId >::fromList( fids );
      QgsFeatureIterator fit = mSource->getFeatures( QgsFeatureRequest().setFilterFids( isectingFids ) );
      QgsFeature f;
      double abs_density = 0;
      while ( fit.nextFeature( f ) )
      {
        double analysisLineLength =  mDa.measureLength( QgsGeometry( engine->intersection( f.geometry().constGet() ) ) );

        //default weight of lines is set to 1 if no field provided
        double analysisWeight = 1;
        if ( !mWeightField.isEmpty() )
        {
          analysisWeight = f.attribute( mWeightField ).toDouble();
        }
        abs_density += ( analysisLineLength * analysisWeight );
      }

      double lineDensity = abs_density / mSearchGeometryArea;
      rasterDataLine->setValue( 0, col, lineDensity );

      feedback->setProgress( static_cast<double>( cellcnt ) / static_cast<double>( totalCellcnt ) * 100 );
      cellcnt++;
    }
    provider->writeBlock( rasterDataLine.get(), 1, 0, row );

    //'carriage return and newline' for search geometry
    mSearchGeometry.translate( ( cols - 1 ) * -mPixelSize, -mPixelSize );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}


///@endcond
