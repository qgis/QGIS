/***************************************************************************
    qgsrasterdataprovider.cpp - DataProvider Interface for raster layers
     --------------------------------------
    Date                 : Mar 11, 2005
    Copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasteridentifyresult.h"
#include "qgslogger.h"
#include "qgspoint.h"
#include "qgsthreadingutils.h"

#include <QTime>
#include <QMap>
#include <QByteArray>
#include <QVariant>

#include <QUrl>
#include <QUrlQuery>
#include <QSet>

#define ERR(message) QgsError(message, "Raster provider")

void QgsRasterDataProvider::setUseSourceNoDataValue( int bandNo, bool use )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mUseSrcNoDataValue.size() < bandNo )
  {
    for ( int i = mUseSrcNoDataValue.size(); i < bandNo; i++ )
    {
      mUseSrcNoDataValue.append( false );
    }
  }
  mUseSrcNoDataValue[bandNo - 1] = use;
}

QgsRasterBlock *QgsRasterDataProvider::block( int bandNo, QgsRectangle  const &boundingBox, int width, int height, QgsRasterBlockFeedback *feedback )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( QStringLiteral( "bandNo = %1 width = %2 height = %3" ).arg( bandNo ).arg( width ).arg( height ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "boundingBox = %1" ).arg( boundingBox.toString() ), 4 );

  std::unique_ptr< QgsRasterBlock > block = std::make_unique< QgsRasterBlock >( dataType( bandNo ), width, height );
  if ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) )
  {
    block->setNoDataValue( sourceNoDataValue( bandNo ) );
  }

  if ( block->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't create raster block" ) );
    block->setError( { tr( "Couldn't create raster block." ), QStringLiteral( "Raster" ) } );
    block->setValid( false );
    return block.release();
  }

  // Read necessary extent only
  QgsRectangle tmpExtent = boundingBox;

  if ( tmpExtent.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Extent outside provider extent" ) );
    block->setError( { tr( "Extent outside provider extent." ), QStringLiteral( "Raster" ) } );
    block->setValid( false );
    block->setIsNoData();
    return block.release();
  }

  const double xRes = boundingBox.width() / width;
  const double yRes = boundingBox.height() / height;
  double tmpXRes, tmpYRes;
  double providerXRes = 0;
  double providerYRes = 0;
  if ( capabilities() & Size )
  {
    providerXRes = extent().width() / xSize();
    providerYRes = extent().height() / ySize();
    tmpXRes = std::max( providerXRes, xRes );
    tmpYRes = std::max( providerYRes, yRes );
    if ( qgsDoubleNear( tmpXRes, xRes ) ) tmpXRes = xRes;
    if ( qgsDoubleNear( tmpYRes, yRes ) ) tmpYRes = yRes;
  }
  else
  {
    tmpXRes = xRes;
    tmpYRes = yRes;
  }

  if ( tmpExtent != boundingBox ||
       tmpXRes > xRes || tmpYRes > yRes )
  {
    // Read smaller extent or lower resolution

    if ( !extent().contains( boundingBox ) )
    {
      const QRect subRect = QgsRasterBlock::subRect( boundingBox, width, height, extent() );
      block->setIsNoDataExcept( subRect );
    }

    // Calculate row/col limits (before tmpExtent is aligned)
    const int fromRow = std::round( ( boundingBox.yMaximum() - tmpExtent.yMaximum() ) / yRes );
    const int toRow = std::round( ( boundingBox.yMaximum() - tmpExtent.yMinimum() ) / yRes ) - 1;
    const int fromCol = std::round( ( tmpExtent.xMinimum() - boundingBox.xMinimum() ) / xRes );
    const int toCol = std::round( ( tmpExtent.xMaximum() - boundingBox.xMinimum() ) / xRes ) - 1;

    QgsDebugMsgLevel( QStringLiteral( "fromRow = %1 toRow = %2 fromCol = %3 toCol = %4" ).arg( fromRow ).arg( toRow ).arg( fromCol ).arg( toCol ), 4 );

    if ( fromRow < 0 || fromRow >= height || toRow < 0 || toRow >= height ||
         fromCol < 0 || fromCol >= width || toCol < 0 || toCol >= width )
    {
      // Should not happen
      QgsDebugMsg( QStringLiteral( "Row or column limits out of range" ) );
      block->setError( { tr( "Row or column limits out of range" ), QStringLiteral( "Raster" ) } );
      block->setValid( false );
      return block.release();
    }

    // If lower source resolution is used, the extent must be aligned to original
    // resolution to avoid possible shift due to resampling
    if ( tmpXRes > xRes )
    {
      int col = std::floor( ( tmpExtent.xMinimum() - extent().xMinimum() ) / providerXRes );
      tmpExtent.setXMinimum( extent().xMinimum() + col * providerXRes );
      col = std::ceil( ( tmpExtent.xMaximum() - extent().xMinimum() ) / providerXRes );
      tmpExtent.setXMaximum( extent().xMinimum() + col * providerXRes );
    }
    if ( tmpYRes > yRes )
    {
      int row = std::floor( ( extent().yMaximum() - tmpExtent.yMaximum() ) / providerYRes );
      tmpExtent.setYMaximum( extent().yMaximum() - row * providerYRes );
      row = std::ceil( ( extent().yMaximum() - tmpExtent.yMinimum() ) / providerYRes );
      tmpExtent.setYMinimum( extent().yMaximum() - row * providerYRes );
    }
    const int tmpWidth = std::round( tmpExtent.width() / tmpXRes );
    const int tmpHeight = std::round( tmpExtent.height() / tmpYRes );
    tmpXRes = tmpExtent.width() / tmpWidth;
    tmpYRes = tmpExtent.height() / tmpHeight;

    QgsDebugMsgLevel( QStringLiteral( "Reading smaller block tmpWidth = %1 height = %2" ).arg( tmpWidth ).arg( tmpHeight ), 4 );
    QgsDebugMsgLevel( QStringLiteral( "tmpExtent = %1" ).arg( tmpExtent.toString() ), 4 );

    std::unique_ptr< QgsRasterBlock > tmpBlock = std::make_unique< QgsRasterBlock >( dataType( bandNo ), tmpWidth, tmpHeight );
    if ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) )
    {
      tmpBlock->setNoDataValue( sourceNoDataValue( bandNo ) );
    }

    if ( !readBlock( bandNo, tmpExtent, tmpWidth, tmpHeight, tmpBlock->bits(), feedback ) )
    {
      QgsDebugMsg( QStringLiteral( "Error occurred while reading block" ) );
      block->setError( { tr( "Error occurred while reading block." ), QStringLiteral( "Raster" ) } );
      block->setValid( false );
      block->setIsNoData();
      return block.release();
    }

    const int pixelSize = dataTypeSize( bandNo );

    const double xMin = boundingBox.xMinimum();
    const double yMax = boundingBox.yMaximum();
    const double tmpXMin = tmpExtent.xMinimum();
    const double tmpYMax = tmpExtent.yMaximum();

    for ( int row = fromRow; row <= toRow; row++ )
    {
      const double y = yMax - ( row + 0.5 ) * yRes;
      const int tmpRow = std::floor( ( tmpYMax - y ) / tmpYRes );

      for ( int col = fromCol; col <= toCol; col++ )
      {
        const double x = xMin + ( col + 0.5 ) * xRes;
        const int tmpCol = std::floor( ( x - tmpXMin ) / tmpXRes );

        if ( tmpRow < 0 || tmpRow >= tmpHeight || tmpCol < 0 || tmpCol >= tmpWidth )
        {
          QgsDebugMsg( QStringLiteral( "Source row or column limits out of range" ) );
          block->setIsNoData(); // so that the problem becomes obvious and fixed
          block->setError( { tr( "Source row or column limits out of range." ), QStringLiteral( "Raster" ) } );
          block->setValid( false );
          return block.release();
        }

        const qgssize tmpIndex = static_cast< qgssize >( tmpRow ) * static_cast< qgssize >( tmpWidth ) + tmpCol;
        const qgssize index = row * static_cast< qgssize >( width ) + col;

        char *tmpBits = tmpBlock->bits( tmpIndex );
        char *bits = block->bits( index );
        if ( !tmpBits )
        {
          QgsDebugMsg( QStringLiteral( "Cannot get input block data tmpRow = %1 tmpCol = %2 tmpIndex = %3." ).arg( tmpRow ).arg( tmpCol ).arg( tmpIndex ) );
          continue;
        }
        if ( !bits )
        {
          QgsDebugMsg( QStringLiteral( "Cannot set output block data." ) );
          continue;
        }
        memcpy( bits, tmpBits, pixelSize );
      }
    }
  }
  else
  {
    if ( !readBlock( bandNo, boundingBox, width, height, block->bits(), feedback ) )
    {
      QgsDebugMsg( QStringLiteral( "Error occurred while reading block" ) );
      block->setIsNoData();
      block->setError( { tr( "Error occurred while reading block." ), QStringLiteral( "Raster" ) } );
      block->setValid( false );
      return block.release();
    }
  }

  // apply scale and offset
  block->applyScaleOffset( bandScale( bandNo ), bandOffset( bandNo ) );
  // apply user no data values
  block->applyNoDataValues( userNoDataValues( bandNo ) );
  return block.release();
}

QgsRasterDataProvider::QgsRasterDataProvider()
  : QgsDataProvider( QString(), QgsDataProvider::ProviderOptions(), QgsDataProvider::ReadFlags() )
  , QgsRasterInterface( nullptr )
  , mTemporalCapabilities( std::make_unique< QgsRasterDataProviderTemporalCapabilities >() )
{

}

QgsRasterDataProvider::QgsRasterDataProvider( const QString &uri, const ProviderOptions &options,
    QgsDataProvider::ReadFlags flags )
  : QgsDataProvider( uri, options, flags )
  , QgsRasterInterface( nullptr )
  , mTemporalCapabilities( std::make_unique< QgsRasterDataProviderTemporalCapabilities >() )
{
}

QgsRasterDataProvider::ProviderCapabilities QgsRasterDataProvider::providerCapabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsRasterDataProvider::NoProviderCapabilities;
}

Qgis::RasterColorInterpretation QgsRasterDataProvider::colorInterpretation( int bandNo ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( bandNo )
  return Qgis::RasterColorInterpretation::Undefined;
}

// TODO
// (WMS) IdentifyFormatFeature is not consistent with QgsRaster::IdentifyFormatValue.
// IdentifyFormatHtml: better error reporting
QgsRasterIdentifyResult QgsRasterDataProvider::identify( const QgsPointXY &point, Qgis::RasterIdentifyFormat format, const QgsRectangle &boundingBox, int width, int height, int /*dpi*/ )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  QMap<int, QVariant> results;

  if ( format != Qgis::RasterIdentifyFormat::Value || !( capabilities() & IdentifyValue ) )
  {
    QgsDebugMsg( QStringLiteral( "Format not supported" ) );
    return QgsRasterIdentifyResult( ERR( tr( "Format not supported" ) ) );
  }

  if ( !extent().contains( point ) )
  {
    // Outside the raster
    for ( int bandNo = 1; bandNo <= bandCount(); bandNo++ )
    {
      results.insert( bandNo, QVariant() );
    }
    return QgsRasterIdentifyResult( Qgis::RasterIdentifyFormat::Value, results );
  }

  QgsRectangle finalExtent = boundingBox;
  if ( finalExtent.isEmpty() )
    finalExtent = extent();

  if ( width == 0 )
  {
    width = capabilities() & Size ? xSize() : 1000;
  }
  if ( height == 0 )
  {
    height = capabilities() & Size ? ySize() : 1000;
  }

  // Calculate the row / column where the point falls
  const double xres = ( finalExtent.width() ) / width;
  const double yres = ( finalExtent.height() ) / height;

  const int col = static_cast< int >( std::floor( ( point.x() - finalExtent.xMinimum() ) / xres ) );
  const int row = static_cast< int >( std::floor( ( finalExtent.yMaximum() - point.y() ) / yres ) );

  const double xMin = finalExtent.xMinimum() + col * xres;
  const double xMax = xMin + xres;
  const double yMax = finalExtent.yMaximum() - row * yres;
  const double yMin = yMax - yres;
  const QgsRectangle pixelExtent( xMin, yMin, xMax, yMax );

  for ( int bandNumber = 1; bandNumber <= bandCount(); bandNumber++ )
  {
    std::unique_ptr< QgsRasterBlock > bandBlock( block( bandNumber, pixelExtent, 1, 1 ) );

    if ( bandBlock )
    {
      const double value = bandBlock->value( 0 );
      results.insert( bandNumber, value );
    }
    else
    {
      results.insert( bandNumber, QVariant() );
    }
  }
  return QgsRasterIdentifyResult( Qgis::RasterIdentifyFormat::Value, results );
}

double QgsRasterDataProvider::sample( const QgsPointXY &point, int band,
                                      bool *ok, const QgsRectangle &boundingBox, int width, int height, int dpi )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( ok )
    *ok = false;

  const auto res = identify( point, Qgis::RasterIdentifyFormat::Value, boundingBox, width, height, dpi );
  const QVariant value = res.results().value( band );

  if ( !value.isValid() )
    return std::numeric_limits<double>::quiet_NaN();

  if ( ok )
    *ok = true;

  return value.toDouble( ok );
}

QString QgsRasterDataProvider::lastErrorFormat()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringLiteral( "text/plain" );
}

bool QgsRasterDataProvider::writeBlock( QgsRasterBlock *block, int band, int xOffset, int yOffset )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !block )
    return false;
  if ( !isEditable() )
  {
    QgsDebugMsg( QStringLiteral( "writeBlock() called on read-only provider." ) );
    return false;
  }
  return write( block->bits(), band, block->width(), block->height(), xOffset, yOffset );
}

// typedef QList<QPair<QString, QString> > *pyramidResamplingMethods_t();
QList<QPair<QString, QString> > QgsRasterDataProvider::pyramidResamplingMethods( const QString &providerKey )
{
  QList<QPair<QString, QString> > methods = QgsProviderRegistry::instance()->pyramidResamplingMethods( providerKey );
  if ( methods.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "provider pyramidResamplingMethods returned no methods" ) );
  }
  return methods;
}

bool QgsRasterDataProvider::hasPyramids()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QList<QgsRasterPyramid> pyramidList = buildPyramidList();
  return std::any_of( pyramidList.constBegin(), pyramidList.constEnd(), []( QgsRasterPyramid pyramid ) { return pyramid.getExists(); } );
}

void QgsRasterDataProvider::setUserNoDataValue( int bandNo, const QgsRasterRangeList &noData )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( bandNo >= mUserNoDataValue.size() )
  {
    for ( int i = mUserNoDataValue.size(); i < bandNo; i++ )
    {
      mUserNoDataValue.append( QgsRasterRangeList() );
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "set %1 band %1 no data ranges" ).arg( noData.size() ), 4 );

  if ( mUserNoDataValue[bandNo - 1] != noData )
  {
    // Clear statistics
    mStatistics.erase( std::remove_if( mStatistics.begin(), mStatistics.end(), [bandNo]( const QgsRasterBandStats & stats )
    {
      return stats.bandNumber == bandNo;
    } ), mStatistics.end() );
    mHistograms.erase( std::remove_if( mHistograms.begin(), mHistograms.end(), [bandNo]( const QgsRasterHistogram & histogram )
    {
      return histogram.bandNumber == bandNo;
    } ), mHistograms.end() );
    mUserNoDataValue[bandNo - 1] = noData;
  }
}

QgsRasterDataProviderTemporalCapabilities *QgsRasterDataProvider::temporalCapabilities()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTemporalCapabilities.get();
}

const QgsRasterDataProviderTemporalCapabilities *QgsRasterDataProvider::temporalCapabilities() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mTemporalCapabilities.get();
}

QgsRasterDataProvider *QgsRasterDataProvider::create( const QString &providerKey,
    const QString &uri,
    const QString &format, int nBands,
    Qgis::DataType type,
    int width, int height, double *geoTransform,
    const QgsCoordinateReferenceSystem &crs,
    const QStringList &createOptions )
{
  QgsRasterDataProvider *ret = QgsProviderRegistry::instance()->createRasterDataProvider(
                                 providerKey,
                                 uri, format,
                                 nBands, type, width,
                                 height, geoTransform, crs, createOptions );
  if ( !ret )
  {
    QgsDebugMsg( "Cannot resolve 'createRasterDataProviderFunction' function in " + providerKey + " provider" );
  }

  // TODO: it would be good to return invalid QgsRasterDataProvider
  // with QgsError set, but QgsRasterDataProvider has pure virtual methods

  return ret;
}

QString QgsRasterDataProvider::identifyFormatName( Qgis::RasterIdentifyFormat format )
{
  switch ( format )
  {
    case Qgis::RasterIdentifyFormat::Value:
      return QStringLiteral( "Value" );
    case Qgis::RasterIdentifyFormat::Text:
      return QStringLiteral( "Text" );
    case Qgis::RasterIdentifyFormat::Html:
      return QStringLiteral( "Html" );
    case Qgis::RasterIdentifyFormat::Feature:
      return QStringLiteral( "Feature" );
    case Qgis::RasterIdentifyFormat::Undefined:
      break;
  }
  return QStringLiteral( "Undefined" );
}

QString QgsRasterDataProvider::identifyFormatLabel( Qgis::RasterIdentifyFormat format )
{
  switch ( format )
  {
    case Qgis::RasterIdentifyFormat::Value:
      return tr( "Value" );
    case Qgis::RasterIdentifyFormat::Text:
      return tr( "Text" );
    case Qgis::RasterIdentifyFormat::Html:
      return tr( "Html" );
    case Qgis::RasterIdentifyFormat::Feature:
      return tr( "Feature" );
    case Qgis::RasterIdentifyFormat::Undefined:
      break;
  }
  return QStringLiteral( "Undefined" );
}

Qgis::RasterIdentifyFormat QgsRasterDataProvider::identifyFormatFromName( const QString &formatName )
{
  if ( formatName == QLatin1String( "Value" ) )
    return Qgis::RasterIdentifyFormat::Value;
  if ( formatName == QLatin1String( "Text" ) )
    return Qgis::RasterIdentifyFormat::Text;
  if ( formatName == QLatin1String( "Html" ) )
    return Qgis::RasterIdentifyFormat::Html;
  if ( formatName == QLatin1String( "Feature" ) )
    return Qgis::RasterIdentifyFormat::Feature;
  return Qgis::RasterIdentifyFormat::Undefined;
}

QgsRasterInterface::Capability QgsRasterDataProvider::identifyFormatToCapability( Qgis::RasterIdentifyFormat format )
{
  switch ( format )
  {
    case Qgis::RasterIdentifyFormat::Value:
      return IdentifyValue;
    case Qgis::RasterIdentifyFormat::Text:
      return IdentifyText;
    case Qgis::RasterIdentifyFormat::Html:
      return IdentifyHtml;
    case Qgis::RasterIdentifyFormat::Feature:
      return IdentifyFeature;
    case Qgis::RasterIdentifyFormat::Undefined:
      break;
  }
  return NoCapabilities;
}

QList<double> QgsRasterDataProvider::nativeResolutions() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QList< double >();
}

bool QgsRasterDataProvider::ignoreExtents() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return false;
}

QgsPoint QgsRasterDataProvider::transformCoordinates( const QgsPoint &point, QgsRasterDataProvider::TransformType type )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( point )
  Q_UNUSED( type )
  return QgsPoint();
}

bool QgsRasterDataProvider::userNoDataValuesContains( int bandNo, double value ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsRasterRangeList rangeList = mUserNoDataValue.value( bandNo - 1 );
  return QgsRasterRange::contains( value, rangeList );
}

void QgsRasterDataProvider::copyBaseSettings( const QgsRasterDataProvider &other )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mDpi = other.mDpi;
  mSrcNoDataValue = other.mSrcNoDataValue;
  mSrcHasNoDataValue = other.mSrcHasNoDataValue;
  mUseSrcNoDataValue = other.mUseSrcNoDataValue;
  mUserNoDataValue = other.mUserNoDataValue;
  mExtent = other.mExtent;
  mProviderResamplingEnabled = other.mProviderResamplingEnabled;
  mZoomedInResamplingMethod = other.mZoomedInResamplingMethod;
  mZoomedOutResamplingMethod = other.mZoomedOutResamplingMethod;
  mMaxOversampling = other.mMaxOversampling;

  // copy temporal properties
  if ( mTemporalCapabilities && other.mTemporalCapabilities )
  {
    *mTemporalCapabilities = *other.mTemporalCapabilities;
  }
}

static QgsRasterDataProvider::ResamplingMethod resamplingMethodFromString( const QString &str )
{
  if ( str == QLatin1String( "bilinear" ) )
  {
    return QgsRasterDataProvider::ResamplingMethod::Bilinear;
  }
  else if ( str == QLatin1String( "cubic" ) )
  {
    return QgsRasterDataProvider::ResamplingMethod::Cubic;
  }
  else if ( str == QLatin1String( "cubicSpline" ) )
  {
    return QgsRasterDataProvider::ResamplingMethod::CubicSpline;
  }
  else if ( str == QLatin1String( "lanczos" ) )
  {
    return QgsRasterDataProvider::ResamplingMethod::Lanczos;
  }
  else if ( str == QLatin1String( "average" ) )
  {
    return QgsRasterDataProvider::ResamplingMethod::Average;
  }
  else if ( str == QLatin1String( "mode" ) )
  {
    return QgsRasterDataProvider::ResamplingMethod::Mode;
  }
  else if ( str == QLatin1String( "gauss" ) )
  {
    return QgsRasterDataProvider::ResamplingMethod::Gauss;
  }
  return  QgsRasterDataProvider::ResamplingMethod::Nearest;
}

void QgsRasterDataProvider::readXml( const QDomElement &filterElem )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( filterElem.isNull() )
  {
    return;
  }

  const QDomElement resamplingElement = filterElem.firstChildElement( QStringLiteral( "resampling" ) );
  if ( !resamplingElement.isNull() )
  {
    setMaxOversampling( resamplingElement.attribute( QStringLiteral( "maxOversampling" ), QStringLiteral( "2.0" ) ).toDouble() );
    setZoomedInResamplingMethod( resamplingMethodFromString( resamplingElement.attribute( QStringLiteral( "zoomedInResamplingMethod" ) ) ) );
    setZoomedOutResamplingMethod( resamplingMethodFromString( resamplingElement.attribute( QStringLiteral( "zoomedOutResamplingMethod" ) ) ) );
    enableProviderResampling( resamplingElement.attribute( QStringLiteral( "enabled" ) ) == QLatin1String( "true" ) );
  }
}

static QString resamplingMethodToString( QgsRasterDataProvider::ResamplingMethod method )
{
  switch ( method )
  {
    case QgsRasterDataProvider::ResamplingMethod::Nearest:
      return QStringLiteral( "nearestNeighbour" );
    case QgsRasterDataProvider::ResamplingMethod::Bilinear:
      return QStringLiteral( "bilinear" );
    case QgsRasterDataProvider::ResamplingMethod::Cubic:
      return QStringLiteral( "cubic" );
    case QgsRasterDataProvider::ResamplingMethod::CubicSpline:
      return QStringLiteral( "cubicSpline" );
    case QgsRasterDataProvider::ResamplingMethod::Lanczos:
      return QStringLiteral( "lanczos" );
    case QgsRasterDataProvider::ResamplingMethod::Average:
      return QStringLiteral( "average" );
    case QgsRasterDataProvider::ResamplingMethod::Mode:
      return QStringLiteral( "mode" );
    case QgsRasterDataProvider::ResamplingMethod::Gauss:
      return QStringLiteral( "gauss" );
  }
  // should not happen
  return QStringLiteral( "nearestNeighbour" );
}

void QgsRasterDataProvider::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QDomElement providerElement = doc.createElement( QStringLiteral( "provider" ) );
  parentElem.appendChild( providerElement );

  QDomElement resamplingElement = doc.createElement( QStringLiteral( "resampling" ) );
  providerElement.appendChild( resamplingElement );

  resamplingElement.setAttribute( QStringLiteral( "enabled" ),
                                  mProviderResamplingEnabled ? QStringLiteral( "true" ) :  QStringLiteral( "false" ) );

  resamplingElement.setAttribute( QStringLiteral( "zoomedInResamplingMethod" ),
                                  resamplingMethodToString( mZoomedInResamplingMethod ) );

  resamplingElement.setAttribute( QStringLiteral( "zoomedOutResamplingMethod" ),
                                  resamplingMethodToString( mZoomedOutResamplingMethod ) );

  resamplingElement.setAttribute( QStringLiteral( "maxOversampling" ),
                                  QString::number( mMaxOversampling ) );
}

QgsRasterAttributeTable *QgsRasterDataProvider::attributeTable( int bandNumber ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  try
  {
    return mAttributeTables.at( bandNumber ).get();
  }
  catch ( std::out_of_range const & )
  {
    return nullptr;
  }
}

void QgsRasterDataProvider::setAttributeTable( int bandNumber, QgsRasterAttributeTable *attributeTable )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( attributeTable )
  {
    mAttributeTables[ bandNumber ] = std::unique_ptr<QgsRasterAttributeTable>( attributeTable );
  }
  else
  {
    removeAttributeTable( bandNumber );
  }
}

void QgsRasterDataProvider::removeAttributeTable( int bandNumber )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mAttributeTables.find( bandNumber ) != std::end( mAttributeTables ) )
  {
    mAttributeTables.erase( bandNumber );
  }
}

bool QgsRasterDataProvider::writeFileBasedAttributeTable( int bandNumber, const QString &path, QString *errorMessage ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsRasterAttributeTable *rat { attributeTable( bandNumber ) };
  if ( ! rat )
  {
    if ( errorMessage )
    {
      *errorMessage = QObject::tr( "Raster has no Raster Attribute Table for band %1" ).arg( bandNumber );
    }
    return false;
  }

  return rat->writeToFile( path, errorMessage );
}

bool QgsRasterDataProvider::readNativeAttributeTable( QString *errorMessage )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( errorMessage )
  {
    *errorMessage = QObject::tr( "Raster data provider has no native Raster Attribute Table support." );
  }
  return false;
}

bool QgsRasterDataProvider::readFileBasedAttributeTable( int bandNumber, const QString &path, QString *errorMessage )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  std::unique_ptr<QgsRasterAttributeTable> rat = std::make_unique<QgsRasterAttributeTable>();
  if ( rat->readFromFile( path, errorMessage ) )
  {
    setAttributeTable( bandNumber, rat.release() );
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsRasterDataProvider::writeNativeAttributeTable( QString *errorMessage )  //#spellok
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( errorMessage );
  return false;
}

QString QgsRasterDataProvider::colorInterpretationName( int bandNo ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return colorName( colorInterpretation( bandNo ) );
}

QgsRasterDataProvider::VirtualRasterParameters QgsRasterDataProvider::decodeVirtualRasterProviderUri( const QString &uri, bool *ok )
{
  QUrl url = QUrl::fromPercentEncoding( uri.toUtf8() );
  const QUrlQuery query( url.query() );
  VirtualRasterParameters components;

  if ( ! query.hasQueryItem( QStringLiteral( "crs" ) ) )
  {
    QgsDebugMsg( "crs is missing" );
    if ( ok ) *ok = false;
    return components;
  }
  if ( ! components.crs.createFromString( query.queryItemValue( QStringLiteral( "crs" ) ) ) )
  {
    QgsDebugMsg( "failed to create crs" );
    if ( ok ) *ok = false;
    return components;
  }


  if ( ! query.hasQueryItem( QStringLiteral( "extent" ) ) )
  {
    QgsDebugMsg( "extent is missing" );
    if ( ok ) *ok = false;
    return components;
  }
  QStringList pointValuesList = query.queryItemValue( QStringLiteral( "extent" ) ).split( ',' );
  if ( pointValuesList.size() != 4 )
  {
    QgsDebugMsg( "the extent is not correct" );
    if ( ok ) *ok = false;
    return components;
  }
  components.extent = QgsRectangle( pointValuesList.at( 0 ).toDouble(), pointValuesList.at( 1 ).toDouble(),
                                    pointValuesList.at( 2 ).toDouble(), pointValuesList.at( 3 ).toDouble() );

  if ( ! query.hasQueryItem( QStringLiteral( "width" ) ) )
  {
    QgsDebugMsg( "width is missing" );
    if ( ok ) *ok = false;
    return components;
  }
  bool flagW;
  components.width = query.queryItemValue( QStringLiteral( "width" ) ).toInt( & flagW );
  if ( !flagW ||  components.width < 0 )
  {
    QgsDebugMsg( "invalid or negative width input" );
    if ( ok ) *ok = false;
    return components;
  }

  if ( ! query.hasQueryItem( QStringLiteral( "height" ) ) )
  {
    QgsDebugMsg( "height is missing" );
    if ( ok ) *ok = false;
    return components;
  }
  bool flagH;
  components.height = query.queryItemValue( QStringLiteral( "height" ) ).toInt( & flagH );
  if ( !flagH ||  components.height < 0 )
  {
    QgsDebugMsg( "invalid or negative width input" );
    if ( ok ) *ok = false;
    return components;
  }

  if ( ! query.hasQueryItem( QStringLiteral( "formula" ) ) )
  {
    QgsDebugMsg( "formula is missing" );
    if ( ok ) *ok = false;
    return components;
  }
  components.formula = query.queryItemValue( QStringLiteral( "formula" ) );

  for ( const auto &item : query.queryItems() )
  {
    if ( !( item.first.mid( item.first.indexOf( ':' ), -1 ) == QLatin1String( ":uri" ) ) )
    {
      continue;
    }

    VirtualRasterInputLayers rLayer;
    rLayer.name = item.first.mid( 0, item.first.indexOf( ':' ) );
    rLayer.uri = query.queryItemValue( item.first );
    rLayer.provider = query.queryItemValue( item.first.mid( 0, item.first.indexOf( ':' ) ) + QStringLiteral( ":provider" ) );

    if ( rLayer.uri.isNull() || rLayer.provider.isNull() )
    {
      QgsDebugMsg( "One or more raster information are missing" );
      if ( ok ) *ok = false;
      return components;
    }

    components.rInputLayers.append( rLayer ) ;

  }

  if ( ok ) *ok = true;
  return components;
}

QString QgsRasterDataProvider::encodeVirtualRasterProviderUri( const VirtualRasterParameters &parts )
{
  QUrl uri;
  QUrlQuery query;

  if ( parts.crs.isValid() )
  {
    query.addQueryItem( QStringLiteral( "crs" ), parts.crs.authid() );
  }

  if ( ! parts.extent.isNull() )
  {
    QString rect = QString( "%1,%2,%3,%4" ).arg( qgsDoubleToString( parts.extent.xMinimum() ), qgsDoubleToString( parts.extent.yMinimum() ),
                   qgsDoubleToString( parts.extent.xMaximum() ), qgsDoubleToString( parts.extent.yMaximum() ) );

    query.addQueryItem( QStringLiteral( "extent" ), rect );
  }

  query.addQueryItem( QStringLiteral( "width" ), QString::number( parts.width ) );

  query.addQueryItem( QStringLiteral( "height" ), QString::number( parts.height ) );

  query.addQueryItem( QStringLiteral( "formula" ), parts.formula );

  if ( ! parts.rInputLayers.isEmpty() )
  {
    for ( const auto &it : parts.rInputLayers )
    {
      query.addQueryItem( it.name + QStringLiteral( ":uri" ), it.uri );
      query.addQueryItem( it.name + QStringLiteral( ":provider" ), it.provider );
    }
  }
  uri.setQuery( query );
  return QString( QUrl::toPercentEncoding( uri.toEncoded() ) );
}
