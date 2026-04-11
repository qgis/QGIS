/***************************************************************************
                         qgsalgorithmhypsometriccurves.cpp
                         ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Alexander Bruy
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

#include "qgsalgorithmhypsometriccurves.h"

#include "qgsrasteranalysisutils.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsHypsometricCurvesAlgorithm::name() const
{
  return u"hypsometriccurves"_s;
}

QString QgsHypsometricCurvesAlgorithm::displayName() const
{
  return QObject::tr( "Hypsometric curves" );
}

QStringList QgsHypsometricCurvesAlgorithm::tags() const
{
  return QObject::tr( "dem,hypsometric,curves,area,elevation,distribution" ).split( ',' );
}

QString QgsHypsometricCurvesAlgorithm::group() const
{
  return QObject::tr( "Raster terrain analysis" );
}

QString QgsHypsometricCurvesAlgorithm::groupId() const
{
  return u"rasterterrainanalysis"_s;
}

QString QgsHypsometricCurvesAlgorithm::shortHelpString() const
{
  return QObject::tr(
    "This algorithm computes hypsometric curves for an input Digital Elevation Model (DEM) "
    "clipped to the boundaries of features from a vector layer.\n\n"
    "A hypsometric curve is a cumulative distribution function of elevations within a "
    "geographic area. It illustrates the relationship between altitude and area, providing "
    "insight into the geomorphological maturity of a catchment or landscape. The curve "
    "plots the total area (either absolute or percentage) found at or below a specific elevation.\n\n"
    "The algorithm generates one CSV file for each feature in the boundary layer. These "
    "files contain two columns: the cumulative area and the corresponding elevation level."
  );
}

QString QgsHypsometricCurvesAlgorithm::shortDescription() const
{
  return QObject::tr( "Computes hypsometric curves for an input Digital Elevation Model (DEM) as table files." );
}

QgsHypsometricCurvesAlgorithm *QgsHypsometricCurvesAlgorithm::createInstance() const
{
  return new QgsHypsometricCurvesAlgorithm();
}

void QgsHypsometricCurvesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterRasterLayer( u"INPUT_DEM"_s, QObject::tr( "DEM to analyze" ) ) );
  addParameter( new QgsProcessingParameterFeatureSource( u"BOUNDARY_LAYER"_s, QObject::tr( "Boundary layer" ), QList<int>() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) ) );
  auto stepParam = std::make_unique<QgsProcessingParameterNumber>( u"STEP"_s, QObject::tr( "Step" ), Qgis::ProcessingNumberParameterType::Double, 100, false, 0 );
  stepParam->setHelp( QObject::tr( "The vertical interval (elevation increment) used to group data. A smaller step results in a smoother, more detailed curve." ) );
  addParameter( stepParam.release() );
  addParameter( new QgsProcessingParameterBoolean( u"USE_PERCENTAGE"_s, QObject::tr( "Use % of area instead of absolute value" ), false ) );
  addParameter( new QgsProcessingParameterFolderDestination( u"OUTPUT_DIRECTORY"_s, QObject::tr( "Hypsometric curves" ), QVariant(), true ) );
  addParameter( new QgsProcessingParameterFeatureSink( u"OUTPUT"_s, QObject::tr( "Hypsometric curves" ), Qgis::ProcessingSourceType::Vector, QVariant(), true, false ) );
}

bool QgsHypsometricCurvesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  QgsRasterLayer *layer = parameterAsRasterLayer( parameters, u"INPUT_DEM"_s, context );
  if ( !layer )
    throw QgsProcessingException( invalidRasterError( parameters, u"INPUT_DEM"_s ) );

  mHasNoDataValue = layer->dataProvider()->sourceHasNoDataValue( 1 );
  mNodataValue = layer->dataProvider()->sourceNoDataValue( 1 );
  mRasterInterface.reset( layer->dataProvider()->clone() );
  mRasterExtent = layer->extent();
  mCrs = layer->crs();
  mCellSizeX = std::abs( layer->rasterUnitsPerPixelX() );
  mCellSizeY = std::abs( layer->rasterUnitsPerPixelY() );
  mNbCellsXProvider = mRasterInterface->xSize();
  mNbCellsYProvider = mRasterInterface->ySize();

  mStep = parameterAsDouble( parameters, u"STEP"_s, context );
  mUsePercentage = parameterAsBool( parameters, u"USE_PERCENTAGE"_s, context );

  return true;
}

QVariantMap QgsHypsometricCurvesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  std::unique_ptr<QgsFeatureSource> source( parameterAsSource( parameters, u"BOUNDARY_LAYER"_s, context ) );
  if ( !source )
  {
    throw QgsProcessingException( invalidSourceError( parameters, u"BOUNDARY_LAYER"_s ) );
  }

  const QString outputPath = parameterAsString( parameters, u"OUTPUT_DIRECTORY"_s, context );
  if ( !outputPath.isEmpty() && !QDir().mkpath( outputPath ) )
  {
    throw QgsProcessingException( QObject::tr( "Could not create output directory '%1'." ).arg( outputPath ) );
  }

  QgsFields sourceFields = source->fields();
  QgsFields newFields = QgsFields();
  newFields.append( QgsField( u"polygon_id"_s, QMetaType::Type::Int, QString(), 20 ) );
  newFields.append( QgsField( u"area"_s, QMetaType::Type::Double, QString(), 20, 6 ) );
  newFields.append( QgsField( u"elevation"_s, QMetaType::Type::Double, QString(), 20, 6 ) );

  const QgsFields outputFields = QgsProcessingUtils::combineFields( sourceFields, newFields );

  QString destId;
  std::unique_ptr<QgsFeatureSink> sink( parameterAsSink( parameters, u"OUTPUT"_s, context, destId, outputFields, Qgis::WkbType::NoGeometry, source->sourceCrs() ) );
  if ( parameters.value( u"OUTPUT"_s ).isValid() && !sink )
    throw QgsProcessingException( invalidSinkError( parameters, u"OUTPUT"_s ) );

  const long long featureCount = source->featureCount();
  const double progressStep = featureCount > 0 ? 100.0 / static_cast<double>( featureCount ) : 0.0;
  long long current = 0;

  QgsFeatureRequest request;
  request.setNoAttributes();
  if ( source->sourceCrs() != mCrs )
  {
    request.setDestinationCrs( mCrs, context.transformContext() );
  }
  QgsFeatureIterator it = source->getFeatures( request );
  QgsFeature f;

  QVector<double> elevations;

  while ( it.nextFeature( f ) )
  {
    if ( feedback->isCanceled() )
    {
      break;
    }

    feedback->setProgress( static_cast<double>( current ) * progressStep );

    if ( !f.hasGeometry() )
    {
      current++;
      continue;
    }

    const QgsGeometry featureGeometry = f.geometry();
    const QgsRectangle featureRect = featureGeometry.boundingBox().intersect( mRasterExtent );
    if ( featureRect.isEmpty() )
    {
      feedback->pushInfo( QObject::tr( "Feature %1 does not intersect the raster extent." ).arg( f.id() ) );
      current++;
      continue;
    }

    int startCol, startRow, endCol, endRow;
    QgsRasterAnalysisUtils::mapToPixel( featureRect.xMinimum(), featureRect.yMaximum(), mRasterExtent, mCellSizeX, mCellSizeY, startCol, startRow );
    QgsRasterAnalysisUtils::mapToPixel( featureRect.xMaximum(), featureRect.yMinimum(), mRasterExtent, mCellSizeX, mCellSizeY, endCol, endRow );

    const int nCellsX = endCol - startCol;
    const int nCellsY = endRow - startRow;

    if ( nCellsX <= 0 || nCellsY <= 0 )
    {
      feedback->pushInfo( QObject::tr( "Feature %1 is smaller than the raster cell size." ).arg( f.id() ) );
      current++;
      continue;
    }

    const QgsRectangle rasterBlockExtent(
      mRasterExtent.xMinimum() + startCol * mCellSizeX,
      mRasterExtent.yMaximum() - ( startRow + nCellsY ) * mCellSizeY,
      mRasterExtent.xMinimum() + ( startCol + nCellsX ) * mCellSizeX,
      mRasterExtent.yMaximum() - startRow * mCellSizeY
    );


    elevations.clear();
    elevations.reserve( static_cast<qsizetype>( nCellsX ) * nCellsY );

    auto collectValue = [&]( double value, const QgsPointXY & ) {
      if ( mHasNoDataValue && qgsDoubleNear( value, mNodataValue ) )
      {
        return;
      }
      elevations.append( value );
    };

    QgsRasterAnalysisUtils::statisticsFromMiddlePointTest( mRasterInterface.get(), 1, featureGeometry, nCellsX, nCellsY, mCellSizeX, mCellSizeY, rasterBlockExtent, collectValue, true );

    if ( elevations.isEmpty() )
    {
      feedback->pushInfo( QObject::tr( "Feature %1 does not intersect the raster or is entirely in a NODATA area." ).arg( f.id() ) );
      current++;
      continue;
    }

    QMap<double, double> hypsometry = calculateHypsometry( elevations, feedback );
    if ( hypsometry.isEmpty() )
    {
      current++;
      continue;
    }

    if ( !outputPath.isEmpty() )
    {
      const QString csvPath = QDir( outputPath ).filePath( u"histogram_%1_%2.csv"_s.arg( source->sourceName() ).arg( f.id() ) );
      QFile outFile( csvPath );
      if ( !outFile.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
        feedback->reportError( QObject::tr( "Could not open output file '%1' for writing." ).arg( csvPath ) );
        current++;
        continue;
      }

      QTextStream stream( &outFile );
      stream.setEncoding( QStringConverter::Utf8 );
      stream.setRealNumberNotation( QTextStream::SmartNotation );
      stream.setRealNumberPrecision( std::numeric_limits<double>::max_digits10 );
      stream << QObject::tr( "Area" ) << ',' << QObject::tr( "Elevation" ) << '\n';

      for ( auto it = hypsometry.cbegin(); it != hypsometry.cend(); ++it )
      {
        stream << it.value() << ',' << it.key() << '\n';
      }
      outFile.close();
    }

    if ( sink )
    {
      QgsAttributes sourceAttributes = f.attributes();
      for ( auto it = hypsometry.cbegin(); it != hypsometry.cend(); ++it )
      {
        QgsFeature feat;
        QgsAttributes attributes = sourceAttributes;
        feat.setFields( outputFields );
        attributes.append( f.id() );
        attributes.append( it.value() ); // area
        attributes.append( it.key() );   // elevation
        feat.setAttributes( attributes );
        if ( !sink->addFeature( feat, QgsFeatureSink::FastInsert ) )
        {
          throw QgsProcessingException( writeFeatureError( sink.get(), parameters, u"OUTPUT"_s ) );
        }
      }
    }

    current++;
  }

  QVariantMap results;
  results.insert( u"OUTPUT_DIRECTORY"_s, outputPath );
  if ( sink )
  {
    results.insert( u"OUTPUT"_s, destId );
  }
  return results;
}

QMap<double, double> QgsHypsometricCurvesAlgorithm::calculateHypsometry( const QVector<double> &elevations, QgsProcessingFeedback *feedback ) const
{
  if ( elevations.isEmpty() )
  {
    return {};
  }

  const auto [minIt, maxIt] = std::minmax_element( elevations.begin(), elevations.end() );
  const double minValue = *minIt;
  const double maxValue = *maxIt;

  const qgssize bins = static_cast<qgssize>( std::ceil( ( maxValue - minValue ) / mStep ) );
  if ( bins > MAX_BINS )
  {
    feedback->reportError( QObject::tr( "The combination of elevation range %1 – %2 and step %3 requires too many histograms bins. Please use a larger step value." ).arg( minValue ).arg( maxValue ).arg( mStep ), true );
    return {};
  }

  std::map<double, qgssize> histogram;
  {
    double startValue = minValue;
    double tmpValue = minValue + mStep;
    while ( startValue < maxValue )
    {
      // prevent endless loop when adding mStep does not advance tmpValue,
      // e.g. when tmpValue is very large
      if ( tmpValue <= startValue )
      {
        break;
      }

      histogram[tmpValue] = 0;
      startValue = tmpValue;
      tmpValue += mStep;
    }
  }

  for ( const double v : elevations )
  {
    auto it = histogram.upper_bound( v );
    if ( it != histogram.end() )
    {
      ++it->second;
    }
  }

  const double totalPixels = static_cast<double>( elevations.size() );
  const double multiplier = mUsePercentage ? 100.0 / totalPixels : mCellSizeX * mCellSizeY;

  // hypsometric data: key = upper bin value, value = area
  QMap<double, double> data;

  for ( const auto &[key, count] : histogram )
  {
    data[key] = static_cast<double>( count ) * multiplier;
  }

  double cumulative = 0.0;
  for ( auto &area : data )
  {
    cumulative += area;
    area = cumulative;
  }

  return data;
}

///@endcond
