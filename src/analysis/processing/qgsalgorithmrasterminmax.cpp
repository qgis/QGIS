/***************************************************************************
                         qgsalgorithmrasterminmax.cpp
                         ---------------------
    begin                : October 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#include "qgsalgorithmrasterminmax.h"

///@cond PRIVATE

QString QgsRasterMinMaxAlgorithm::name() const
{
  return u"rasterminmax"_s;
}

QString QgsRasterMinMaxAlgorithm::displayName() const
{
  return QObject::tr( "Raster minimum/maximum" );
}

QStringList QgsRasterMinMaxAlgorithm::tags() const
{
  return QObject::tr( "dem,statistics,value,extrema,extremes,largest,smallest" ).split( ',' );
}

QString QgsRasterMinMaxAlgorithm::group() const
{
  return QObject::tr( "Raster analysis" );
}

QString QgsRasterMinMaxAlgorithm::groupId() const
{
  return u"rasteranalysis"_s;
}

void QgsRasterMinMaxAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT"_s, QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterBand( u"BAND"_s, QObject::tr( "Band number" ), 1, u"INPUT"_s ) );
  addParameter( new QgsProcessingParameterEnum( u"EXTRACT"_s, QObject::tr( "Extract extrema" ), QStringList() << QObject::tr( "Minimum and Maximum" ) << QObject::tr( "Minimum" ) << QObject::tr( "Maximum" ), false, 0 ) );

  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Output" ), Qgis::ProcessingSourceType::VectorPoint, QVariant(), true, true ) );

  addOutput( new QgsProcessingOutputNumber( u"MINIMUM"_s, QObject::tr( "Minimum" ) ) );
  addOutput( new QgsProcessingOutputNumber( u"MAXIMUM"_s, QObject::tr( "Maximum" ) ) );
}

QString QgsRasterMinMaxAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts extrema (minimum and maximum) values from a given band of the raster layer.\n\n"
                      "The output is a vector layer containing point features for the selected extrema, at the center of the associated pixel.\n\n"
                      "If multiple pixels in the raster share the minimum or maximum value, then only one of these pixels will be included in the output." );
}

QString QgsRasterMinMaxAlgorithm::shortDescription() const
{
  return QObject::tr( "Calculates the minimum and maximum pixel in a raster layer." );
}

QgsRasterMinMaxAlgorithm *QgsRasterMinMaxAlgorithm::createInstance() const
{
  return new QgsRasterMinMaxAlgorithm();
}

bool QgsRasterMinMaxAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT"_s, context );

  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT"_s ) );

  mBand = parameterAsInt( parameters, u"BAND"_s, context );
  if ( mBand < 1 || mBand > layer->bandCount() )
    throw QgsProcessingException( QObject::tr( "Invalid band number for BAND (%1): Valid values for input raster are 1 to %2" ).arg( mBand ).arg( layer->bandCount() ) );

  mInterface.reset( layer->dataProvider()->clone() );
  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( mBand );
  mLayerWidth = layer->width();
  mLayerHeight = layer->height();
  mExtent = layer->extent();
  mCrs = layer->crs();
  mRasterUnitsPerPixelX = std::abs( layer->rasterUnitsPerPixelX() );
  mRasterUnitsPerPixelY = std::abs( layer->rasterUnitsPerPixelY() );
  return true;
}

QVariantMap QgsRasterMinMaxAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QString dest;
  std::unique_ptr<QgsFeatureSink> sink;
  if ( parameters.value( u"OUTPUT"_s ).isValid() )
  {
    QgsFields outFields;
    outFields.append( QgsField( u"value"_s, QMetaType::Type::Double, QString(), 20, 8 ) );
    outFields.append( QgsField( u"extremum_type"_s, QMetaType::Type::QString ) );
    sink.reset( parameterAsSink( parameters, u"OUTPUT"_s, context, dest, outFields, Qgis::WkbType::Point, mCrs ) );
    if ( !sink )
      throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );
  }

  const int extractType = parameterAsInt( parameters, u"EXTRACT"_s, context );

  QgsRasterIterator iter( mInterface.get() );
  iter.startRasterRead( mBand, mLayerWidth, mLayerHeight, mExtent );

  int iterLeft = 0;
  int iterTop = 0;
  int iterCols = 0;
  int iterRows = 0;
  std::unique_ptr<QgsRasterBlock> rasterBlock;

  double rasterMinimum = std::numeric_limits<double>::quiet_NaN();
  double rasterMaximum = std::numeric_limits<double>::quiet_NaN();
  QgsPointXY rasterMinPoint;
  QgsPointXY rasterMaxPoint;

  auto blockRowColToXY = [this]( const QgsRectangle &blockExtent, int row, int col ) -> QgsPointXY {
    return QgsPointXY( blockExtent.xMinimum() + mRasterUnitsPerPixelX * ( col + 0.5 ), blockExtent.yMaximum() - mRasterUnitsPerPixelY * ( row + 0.5 ) );
  };

  QgsRectangle blockExtent;
  while ( iter.readNextRasterPart( mBand, iterCols, iterRows, rasterBlock, iterLeft, iterTop, &blockExtent ) )
  {
    if ( feedback->isCanceled() )
      break;

    double blockMinimum = std::numeric_limits<double>::quiet_NaN();
    double blockMaximum = std::numeric_limits<double>::quiet_NaN();
    int blockMinRow = 0;
    int blockMinCol = 0;
    int blockMaxRow = 0;
    int blockMaxCol = 0;
    switch ( extractType )
    {
      case 0:
      {
        if ( rasterBlock->minimumMaximum( blockMinimum, blockMinRow, blockMinCol, blockMaximum, blockMaxRow, blockMaxCol ) )
        {
          if ( std::isnan( rasterMinimum ) || blockMinimum < rasterMinimum )
          {
            rasterMinimum = blockMinimum;
            rasterMinPoint = blockRowColToXY( blockExtent, blockMinRow, blockMinCol );
          }
          if ( std::isnan( rasterMaximum ) || blockMaximum > rasterMaximum )
          {
            rasterMaximum = blockMaximum;
            rasterMaxPoint = blockRowColToXY( blockExtent, blockMaxRow, blockMaxCol );
          }
        }
        break;
      }

      case 1:
      {
        if ( rasterBlock->minimum( blockMinimum, blockMinRow, blockMinCol ) )
        {
          if ( std::isnan( rasterMinimum ) || blockMinimum < rasterMinimum )
          {
            rasterMinimum = blockMinimum;
            rasterMinPoint = blockRowColToXY( blockExtent, blockMinRow, blockMinCol );
          }
        }
        break;
      }

      case 2:
      {
        if ( rasterBlock->maximum( blockMaximum, blockMaxRow, blockMaxCol ) )
        {
          if ( std::isnan( rasterMaximum ) || blockMaximum > rasterMaximum )
          {
            rasterMaximum = blockMaximum;
            rasterMaxPoint = blockRowColToXY( blockExtent, blockMaxRow, blockMaxCol );
          }
        }
        break;
      }
      default:
        break;
    }
    feedback->setProgress( 100 * iter.progress( mBand ) );
  }

  QVariantMap outputs;
  if ( sink )
  {
    QgsFeature f;
    if ( !std::isnan( rasterMinimum ) )
    {
      f.setAttributes( QgsAttributes() << rasterMinimum << u"minimum"_s );
      f.setGeometry( QgsGeometry::fromPointXY( rasterMinPoint ) );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
    if ( !std::isnan( rasterMaximum ) )
    {
      f.setAttributes( QgsAttributes() << rasterMaximum << u"maximum"_s );
      f.setGeometry( QgsGeometry::fromPointXY( rasterMaxPoint ) );
      if ( !sink->addFeature( f, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
    }
    outputs.insert( u"OUTPUT"_s, dest );
  }
  outputs.insert( u"MINIMUM"_s, !std::isnan( rasterMinimum ) ? QVariant::fromValue( rasterMinimum ) : QVariant() );
  outputs.insert( u"MAXIMUM"_s, !std::isnan( rasterMaximum ) ? QVariant::fromValue( rasterMaximum ) : QVariant() );
  return outputs;
}


///@endcond
