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
#include "qgsprovidermetadata.h"
#include "qgsrasterprojector.h"
#include "qgslogger.h"
#include "qgsapplication.h"

#include <QTime>
#include <QMap>
#include <QByteArray>
#include <QVariant>

#define ERR(message) QgsError(message, "Raster provider")

void QgsRasterDataProvider::setUseSourceNoDataValue( int bandNo, bool use )
{
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
  QgsDebugMsgLevel( QStringLiteral( "bandNo = %1 width = %2 height = %3" ).arg( bandNo ).arg( width ).arg( height ), 4 );
  QgsDebugMsgLevel( QStringLiteral( "boundingBox = %1" ).arg( boundingBox.toString() ), 4 );

  std::unique_ptr< QgsRasterBlock > block = qgis::make_unique< QgsRasterBlock >( dataType( bandNo ), width, height );
  if ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) )
  {
    block->setNoDataValue( sourceNoDataValue( bandNo ) );
  }

  if ( block->isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Couldn't create raster block" ) );
    return block.release();
  }

  // Read necessary extent only
  QgsRectangle tmpExtent = boundingBox;

  if ( tmpExtent.isEmpty() )
  {
    QgsDebugMsg( QStringLiteral( "Extent outside provider extent" ) );
    block->setIsNoData();
    return block.release();
  }

  double xRes = boundingBox.width() / width;
  double yRes = boundingBox.height() / height;
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
      QRect subRect = QgsRasterBlock::subRect( boundingBox, width, height, extent() );
      block->setIsNoDataExcept( subRect );
    }

    // Calculate row/col limits (before tmpExtent is aligned)
    int fromRow = std::round( ( boundingBox.yMaximum() - tmpExtent.yMaximum() ) / yRes );
    int toRow = std::round( ( boundingBox.yMaximum() - tmpExtent.yMinimum() ) / yRes ) - 1;
    int fromCol = std::round( ( tmpExtent.xMinimum() - boundingBox.xMinimum() ) / xRes );
    int toCol = std::round( ( tmpExtent.xMaximum() - boundingBox.xMinimum() ) / xRes ) - 1;

    QgsDebugMsgLevel( QStringLiteral( "fromRow = %1 toRow = %2 fromCol = %3 toCol = %4" ).arg( fromRow ).arg( toRow ).arg( fromCol ).arg( toCol ), 4 );

    if ( fromRow < 0 || fromRow >= height || toRow < 0 || toRow >= height ||
         fromCol < 0 || fromCol >= width || toCol < 0 || toCol >= width )
    {
      // Should not happen
      QgsDebugMsg( QStringLiteral( "Row or column limits out of range" ) );
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
    int tmpWidth = std::round( tmpExtent.width() / tmpXRes );
    int tmpHeight = std::round( tmpExtent.height() / tmpYRes );
    tmpXRes = tmpExtent.width() / tmpWidth;
    tmpYRes = tmpExtent.height() / tmpHeight;

    QgsDebugMsgLevel( QStringLiteral( "Reading smaller block tmpWidth = %1 height = %2" ).arg( tmpWidth ).arg( tmpHeight ), 4 );
    QgsDebugMsgLevel( QStringLiteral( "tmpExtent = %1" ).arg( tmpExtent.toString() ), 4 );

    std::unique_ptr< QgsRasterBlock > tmpBlock = qgis::make_unique< QgsRasterBlock >( dataType( bandNo ), tmpWidth, tmpHeight );
    if ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) )
    {
      tmpBlock->setNoDataValue( sourceNoDataValue( bandNo ) );
    }

    if ( !readBlock( bandNo, tmpExtent, tmpWidth, tmpHeight, tmpBlock->bits(), feedback ) )
    {
      QgsDebugMsg( QStringLiteral( "Error occurred while reading block" ) );
      block->setIsNoData();
      return block.release();
    }

    int pixelSize = dataTypeSize( bandNo );

    double xMin = boundingBox.xMinimum();
    double yMax = boundingBox.yMaximum();
    double tmpXMin = tmpExtent.xMinimum();
    double tmpYMax = tmpExtent.yMaximum();

    for ( int row = fromRow; row <= toRow; row++ )
    {
      double y = yMax - ( row + 0.5 ) * yRes;
      int tmpRow = std::floor( ( tmpYMax - y ) / tmpYRes );

      for ( int col = fromCol; col <= toCol; col++ )
      {
        double x = xMin + ( col + 0.5 ) * xRes;
        int tmpCol = std::floor( ( x - tmpXMin ) / tmpXRes );

        if ( tmpRow < 0 || tmpRow >= tmpHeight || tmpCol < 0 || tmpCol >= tmpWidth )
        {
          QgsDebugMsg( QStringLiteral( "Source row or column limits out of range" ) );
          block->setIsNoData(); // so that the problem becomes obvious and fixed
          return block.release();
        }

        qgssize tmpIndex = static_cast< qgssize >( tmpRow ) * static_cast< qgssize >( tmpWidth ) + tmpCol;
        qgssize index = row * static_cast< qgssize >( width ) + col;

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
  : QgsDataProvider( QString(), QgsDataProvider::ProviderOptions() )
  , QgsRasterInterface( nullptr )
  , mTemporalCapabilities( qgis::make_unique< QgsRasterDataProviderTemporalCapabilities >() )
{

}

QgsRasterDataProvider::QgsRasterDataProvider( const QString &uri, const ProviderOptions &options )
  : QgsDataProvider( uri, options )
  , QgsRasterInterface( nullptr )
  , mTemporalCapabilities( qgis::make_unique< QgsRasterDataProviderTemporalCapabilities >() )
{
}

QgsRasterDataProvider::ProviderCapabilities QgsRasterDataProvider::providerCapabilities() const
{
  return QgsRasterDataProvider::NoProviderCapabilities;
}

//
//Random Static convenience function
//
/////////////////////////////////////////////////////////
// convenience function for building metadata() HTML table cells

QString QgsRasterDataProvider::htmlMetadata()
{
  QString s;
  return s;
}

// TODO
// (WMS) IdentifyFormatFeature is not consistent with QgsRaster::IdentifyFormatValue.
// IdentifyFormatHtml: better error reporting
QgsRasterIdentifyResult QgsRasterDataProvider::identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox, int width, int height, int /*dpi*/ )
{
  QgsDebugMsgLevel( QStringLiteral( "Entered" ), 4 );
  QMap<int, QVariant> results;

  if ( format != QgsRaster::IdentifyFormatValue || !( capabilities() & IdentifyValue ) )
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
    return QgsRasterIdentifyResult( QgsRaster::IdentifyFormatValue, results );
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
  double xres = ( finalExtent.width() ) / width;
  double yres = ( finalExtent.height() ) / height;

  int col = static_cast< int >( std::floor( ( point.x() - finalExtent.xMinimum() ) / xres ) );
  int row = static_cast< int >( std::floor( ( finalExtent.yMaximum() - point.y() ) / yres ) );

  double xMin = finalExtent.xMinimum() + col * xres;
  double xMax = xMin + xres;
  double yMax = finalExtent.yMaximum() - row * yres;
  double yMin = yMax - yres;
  QgsRectangle pixelExtent( xMin, yMin, xMax, yMax );

  for ( int i = 1; i <= bandCount(); i++ )
  {
    std::unique_ptr< QgsRasterBlock > bandBlock( block( i, pixelExtent, 1, 1 ) );

    if ( bandBlock )
    {
      double value = bandBlock->value( 0 );

      results.insert( i, value );
    }
    else
    {
      results.insert( i, QVariant() );
    }
  }
  return QgsRasterIdentifyResult( QgsRaster::IdentifyFormatValue, results );
}

double QgsRasterDataProvider::sample( const QgsPointXY &point, int band,
                                      bool *ok, const QgsRectangle &boundingBox, int width, int height, int dpi )
{
  if ( ok )
    *ok = false;

  const auto res = identify( point, QgsRaster::IdentifyFormatValue, boundingBox, width, height, dpi );
  const QVariant value = res.results().value( band );

  if ( !value.isValid() )
    return std::numeric_limits<double>::quiet_NaN();

  if ( ok )
    *ok = true;

  return value.toDouble( ok );
}

QString QgsRasterDataProvider::lastErrorFormat()
{
  return QStringLiteral( "text/plain" );
}

bool QgsRasterDataProvider::writeBlock( QgsRasterBlock *block, int band, int xOffset, int yOffset )
{
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
  QList<QgsRasterPyramid> myPyramidList = buildPyramidList();

  if ( myPyramidList.isEmpty() )
    return false;

  QList<QgsRasterPyramid>::iterator myRasterPyramidIterator;
  for ( myRasterPyramidIterator = myPyramidList.begin();
        myRasterPyramidIterator != myPyramidList.end();
        ++myRasterPyramidIterator )
  {
    if ( myRasterPyramidIterator->exists )
    {
      return true;
    }
  }
  return false;
}

void QgsRasterDataProvider::setUserNoDataValue( int bandNo, const QgsRasterRangeList &noData )
{
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
    int i = 0;
    while ( i < mStatistics.size() )
    {
      if ( mStatistics.value( i ).bandNumber == bandNo )
      {
        mStatistics.removeAt( i );
        mHistograms.removeAt( i );
      }
      else
      {
        i++;
      }
    }
    mUserNoDataValue[bandNo - 1] = noData;
  }
}

QgsRasterDataProviderTemporalCapabilities *QgsRasterDataProvider::temporalCapabilities()
{
  return mTemporalCapabilities.get();
}

const QgsRasterDataProviderTemporalCapabilities *QgsRasterDataProvider::temporalCapabilities() const
{
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

QString QgsRasterDataProvider::identifyFormatName( QgsRaster::IdentifyFormat format )
{
  switch ( format )
  {
    case QgsRaster::IdentifyFormatValue:
      return QStringLiteral( "Value" );
    case QgsRaster::IdentifyFormatText:
      return QStringLiteral( "Text" );
    case QgsRaster::IdentifyFormatHtml:
      return QStringLiteral( "Html" );
    case QgsRaster::IdentifyFormatFeature:
      return QStringLiteral( "Feature" );
    default:
      return QStringLiteral( "Undefined" );
  }
}

QString QgsRasterDataProvider::identifyFormatLabel( QgsRaster::IdentifyFormat format )
{
  switch ( format )
  {
    case QgsRaster::IdentifyFormatValue:
      return tr( "Value" );
    case QgsRaster::IdentifyFormatText:
      return tr( "Text" );
    case QgsRaster::IdentifyFormatHtml:
      return tr( "Html" );
    case QgsRaster::IdentifyFormatFeature:
      return tr( "Feature" );
    default:
      return QStringLiteral( "Undefined" );
  }
}

QgsRaster::IdentifyFormat QgsRasterDataProvider::identifyFormatFromName( const QString &formatName )
{
  if ( formatName == QLatin1String( "Value" ) ) return QgsRaster::IdentifyFormatValue;
  if ( formatName == QLatin1String( "Text" ) ) return QgsRaster::IdentifyFormatText;
  if ( formatName == QLatin1String( "Html" ) ) return QgsRaster::IdentifyFormatHtml;
  if ( formatName == QLatin1String( "Feature" ) ) return QgsRaster::IdentifyFormatFeature;
  return QgsRaster::IdentifyFormatUndefined;
}

QgsRasterInterface::Capability QgsRasterDataProvider::identifyFormatToCapability( QgsRaster::IdentifyFormat format )
{
  switch ( format )
  {
    case QgsRaster::IdentifyFormatValue:
      return IdentifyValue;
    case QgsRaster::IdentifyFormatText:
      return IdentifyText;
    case QgsRaster::IdentifyFormatHtml:
      return IdentifyHtml;
    case QgsRaster::IdentifyFormatFeature:
      return IdentifyFeature;
    default:
      return NoCapabilities;
  }
}

QList<double> QgsRasterDataProvider::nativeResolutions() const
{
  return QList< double >();
}

bool QgsRasterDataProvider::ignoreExtents() const
{
  return false;
}

QgsPoint QgsRasterDataProvider::transformCoordinates( const QgsPoint &point, QgsRasterDataProvider::TransformType type )
{
  Q_UNUSED( point )
  Q_UNUSED( type )
  return QgsPoint();
}

bool QgsRasterDataProvider::userNoDataValuesContains( int bandNo, double value ) const
{
  QgsRasterRangeList rangeList = mUserNoDataValue.value( bandNo - 1 );
  return QgsRasterRange::contains( value, rangeList );
}

void QgsRasterDataProvider::copyBaseSettings( const QgsRasterDataProvider &other )
{
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
  return  QgsRasterDataProvider::ResamplingMethod::Nearest;
}

void QgsRasterDataProvider::readXml( const QDomElement &filterElem )
{
  if ( filterElem.isNull() )
  {
    return;
  }

  QDomElement resamplingElement = filterElem.firstChildElement( QStringLiteral( "resampling" ) );
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
    case QgsRasterDataProvider::ResamplingMethod::Nearest : return QStringLiteral( "nearestNeighbour" );
    case QgsRasterDataProvider::ResamplingMethod::Bilinear : return QStringLiteral( "bilinear" );
    case QgsRasterDataProvider::ResamplingMethod::Cubic : return QStringLiteral( "cubic" );
  }
  // should not happen
  return QStringLiteral( "nearestNeighbour" );
}

void QgsRasterDataProvider::writeXml( QDomDocument &doc, QDomElement &parentElem ) const
{
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


// ENDS
