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
  QgsDebugMsgLevel( QString( "bandNo = %1 width = %2 height = %3" ).arg( bandNo ).arg( width ).arg( height ), 4 );
  QgsDebugMsgLevel( QString( "boundingBox = %1" ).arg( boundingBox.toString() ), 4 );

  QgsRasterBlock *block = new QgsRasterBlock( dataType( bandNo ), width, height );
  if ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) )
  {
    block->setNoDataValue( sourceNoDataValue( bandNo ) );
  }

  if ( block->isEmpty() )
  {
    QgsDebugMsg( "Couldn't create raster block" );
    return block;
  }

  // Read necessary extent only
  QgsRectangle tmpExtent = extent().intersect( &boundingBox );

  if ( tmpExtent.isEmpty() )
  {
    QgsDebugMsg( "Extent outside provider extent" );
    block->setIsNoData();
    return block;
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

    QgsDebugMsgLevel( QString( "fromRow = %1 toRow = %2 fromCol = %3 toCol = %4" ).arg( fromRow ).arg( toRow ).arg( fromCol ).arg( toCol ), 4 );

    if ( fromRow < 0 || fromRow >= height || toRow < 0 || toRow >= height ||
         fromCol < 0 || fromCol >= width || toCol < 0 || toCol >= width )
    {
      // Should not happen
      QgsDebugMsg( "Row or column limits out of range" );
      return block;
    }

    // If lower source resolution is used, the extent must beS aligned to original
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

    QgsDebugMsgLevel( QString( "Reading smaller block tmpWidth = %1 height = %2" ).arg( tmpWidth ).arg( tmpHeight ), 4 );
    QgsDebugMsgLevel( QString( "tmpExtent = %1" ).arg( tmpExtent.toString() ), 4 );

    QgsRasterBlock *tmpBlock = new QgsRasterBlock( dataType( bandNo ), tmpWidth, tmpHeight );
    if ( sourceHasNoDataValue( bandNo ) && useSourceNoDataValue( bandNo ) )
    {
      tmpBlock->setNoDataValue( sourceNoDataValue( bandNo ) );
    }

    readBlock( bandNo, tmpExtent, tmpWidth, tmpHeight, tmpBlock->bits(), feedback );

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
          QgsDebugMsg( "Source row or column limits out of range" );
          block->setIsNoData(); // so that the problem becomes obvious and fixed
          delete tmpBlock;
          return block;
        }

        qgssize tmpIndex = static_cast< qgssize >( tmpRow ) * static_cast< qgssize >( tmpWidth ) + tmpCol;
        qgssize index = row * static_cast< qgssize >( width ) + col;

        char *tmpBits = tmpBlock->bits( tmpIndex );
        char *bits = block->bits( index );
        if ( !tmpBits )
        {
          QgsDebugMsg( QString( "Cannot get input block data tmpRow = %1 tmpCol = %2 tmpIndex = %3." ).arg( tmpRow ).arg( tmpCol ).arg( tmpIndex ) );
          continue;
        }
        if ( !bits )
        {
          QgsDebugMsg( "Cannot set output block data." );
          continue;
        }
        memcpy( bits, tmpBits, pixelSize );
      }
    }

    delete tmpBlock;
  }
  else
  {
    readBlock( bandNo, boundingBox, width, height, block->bits(), feedback );
  }

  // apply scale and offset
  block->applyScaleOffset( bandScale( bandNo ), bandOffset( bandNo ) );
  // apply user no data values
  block->applyNoDataValues( userNoDataValues( bandNo ) );
  return block;
}

QgsRasterDataProvider::QgsRasterDataProvider()
  : QgsRasterInterface( nullptr )
{
}

QgsRasterDataProvider::QgsRasterDataProvider( QString const &uri )
  : QgsDataProvider( uri )
  , QgsRasterInterface( nullptr )
{
}

//
//Random Static convenience function
//
/////////////////////////////////////////////////////////
// convenience function for building metadata() HTML table cells
// convenience function for creating a string list from a C style string list
QStringList QgsRasterDataProvider::cStringList2Q_( char **stringList )
{
  QStringList strings;

  // presume null terminated string list
  for ( qgssize i = 0; stringList[i]; ++i )
  {
    strings.append( QString::fromUtf8( stringList[i] ) );
  }

  return strings;

} // cStringList2Q_

QString QgsRasterDataProvider::makeTableCell( QString const &value )
{
  return "<p>\n" + value + "</p>\n";
} // makeTableCell_

// convenience function for building metadata() HTML table cells
QString QgsRasterDataProvider::makeTableCells( QStringList const &values )
{
  QString s( QStringLiteral( "<tr>" ) );

  for ( QStringList::const_iterator i = values.begin();
        i != values.end();
        ++i )
  {
    s += QgsRasterDataProvider::makeTableCell( *i );
  }

  s += QLatin1String( "</tr>" );

  return s;
} // makeTableCell_

QString QgsRasterDataProvider::metadata()
{
  QString s;
  return s;
}

// Default implementation for values
QgsRasterIdentifyResult QgsRasterDataProvider::identify( const QgsPointXY &point, QgsRaster::IdentifyFormat format, const QgsRectangle &boundingBox, int width, int height, int /*dpi*/ )
{
  QgsDebugMsgLevel( "Entered", 4 );
  QMap<int, QVariant> results;

  if ( format != QgsRaster::IdentifyFormatValue || !( capabilities() & IdentifyValue ) )
  {
    QgsDebugMsg( "Format not supported" );
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
    QgsRasterBlock *myBlock = block( i, pixelExtent, 1, 1 );

    if ( myBlock )
    {
      double value = myBlock->value( 0 );

      results.insert( i, value );
      delete myBlock;
    }
    else
    {
      results.insert( i, QVariant() );
    }
  }
  return QgsRasterIdentifyResult( QgsRaster::IdentifyFormatValue, results );
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
    QgsDebugMsg( "writeBlock() called on read-only provider." );
    return false;
  }
  return write( block->bits(), band, block->width(), block->height(), xOffset, yOffset );
}

typedef QList<QPair<QString, QString> > *pyramidResamplingMethods_t();
QList<QPair<QString, QString> > QgsRasterDataProvider::pyramidResamplingMethods( const QString &providerKey )
{
  pyramidResamplingMethods_t *pPyramidResamplingMethods = reinterpret_cast< pyramidResamplingMethods_t * >( cast_to_fptr( QgsProviderRegistry::instance()->function( providerKey,  "pyramidResamplingMethods" ) ) );
  if ( pPyramidResamplingMethods )
  {
    QList<QPair<QString, QString> > *methods = pPyramidResamplingMethods();
    if ( !methods )
    {
      QgsDebugMsg( "provider pyramidResamplingMethods returned no methods" );
    }
    else
    {
      return *methods;
    }
  }
  else
  {
    QgsDebugMsg( "Could not resolve pyramidResamplingMethods provider library" );
  }
  return QList<QPair<QString, QString> >();
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
  QgsDebugMsgLevel( QString( "set %1 band %1 no data ranges" ).arg( noData.size() ), 4 );

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

typedef QgsRasterDataProvider *createFunction_t( const QString &,
    const QString &, int,
    Qgis::DataType,
    int, int, double *,
    const QgsCoordinateReferenceSystem &,
    QStringList );

QgsRasterDataProvider *QgsRasterDataProvider::create( const QString &providerKey,
    const QString &uri,
    const QString &format, int nBands,
    Qgis::DataType type,
    int width, int height, double *geoTransform,
    const QgsCoordinateReferenceSystem &crs,
    const QStringList &createOptions )
{
  createFunction_t *createFn = reinterpret_cast< createFunction_t * >( cast_to_fptr( QgsProviderRegistry::instance()->function( providerKey, "create" ) ) );
  if ( !createFn )
  {
    QgsDebugMsg( "Cannot resolve 'create' function in " + providerKey + " provider" );
    // TODO: it would be good to return invalid QgsRasterDataProvider
    // with QgsError set, but QgsRasterDataProvider has pure virtual methods
    return nullptr;
  }
  return createFn( uri, format, nBands, type, width, height, geoTransform, crs, createOptions );
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
}

// ENDS
