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

#include <QTime>
#include <QMap>
#include <QByteArray>
#include <QVariant>

#include <qmath.h>

#define ERRMSG(message) QGS_ERROR_MESSAGE(message, "Raster provider")
#define ERR(message) QgsError(message, "Raster provider")

void QgsRasterDataProvider::setUseSrcNoDataValue( int bandNo, bool use )
{
  if ( mUseSrcNoDataValue.size() < bandNo )
  {
    for ( int i = mUseSrcNoDataValue.size(); i < bandNo; i++ )
    {
      mUseSrcNoDataValue.append( false );
    }
  }
  mUseSrcNoDataValue[bandNo-1] = use;
}

double QgsRasterDataProvider::noDataValue( int bandNo ) const
{
  if ( mSrcHasNoDataValue.value( bandNo - 1 ) && mUseSrcNoDataValue.value( bandNo - 1 ) )
  {
    return mSrcNoDataValue.value( bandNo -1 );
  }
  return mInternalNoDataValue.value( bandNo -1 );
}

QgsRasterBlock * QgsRasterDataProvider::block( int theBandNo, QgsRectangle  const & theExtent, int theWidth, int theHeight )
{
  QgsDebugMsg( QString( "theBandNo = %1 theWidth = %2 theHeight = %3" ).arg( theBandNo ).arg( theWidth ).arg( theHeight ) );
  QgsDebugMsg( QString( "theExtent = %1" ).arg( theExtent.toString() ) );

  QgsRasterBlock *block = new QgsRasterBlock( dataType( theBandNo ), theWidth, theHeight, noDataValue( theBandNo ) );

  if ( block->isEmpty() )
  {
    QgsDebugMsg( "Couldn't create raster block" );
    return block;
  }

  // Read necessary extent only
  QgsRectangle tmpExtent = extent().intersect( &theExtent );

  if ( tmpExtent.isEmpty() )
  {
    QgsDebugMsg( "Extent outside provider extent" );
    block->setIsNoData();
    return block;
  }

  double xRes = theExtent.width() / theWidth;
  double yRes = theExtent.height() / theHeight;
  double tmpXRes, tmpYRes;
  double providerXRes = 0;
  double providerYRes = 0;
  if ( capabilities() & ExactResolution )
  {
    providerXRes = extent().width() / xSize();
    providerYRes = extent().height() / ySize();
    tmpXRes = qMax( providerXRes, xRes );
    tmpYRes = qMax( providerYRes, yRes );
    if ( doubleNear( tmpXRes, xRes ) ) tmpXRes = xRes;
    if ( doubleNear( tmpYRes, yRes ) ) tmpYRes = yRes;
  }
  else
  {
    tmpXRes = xRes;
    tmpYRes = yRes;
  }

  if ( tmpExtent != theExtent ||
       tmpXRes > xRes || tmpYRes > yRes )
  {
    // Read smaller extent or lower resolution

    // Calculate row/col limits (before tmpExtent is aligned)
    int fromRow = qRound(( theExtent.yMaximum() - tmpExtent.yMaximum() ) / yRes );
    int toRow = qRound(( theExtent.yMaximum() - tmpExtent.yMinimum() ) / yRes ) - 1;
    int fromCol = qRound(( tmpExtent.xMinimum() - theExtent.xMinimum() ) / xRes ) ;
    int toCol = qRound(( tmpExtent.xMaximum() - theExtent.xMinimum() ) / xRes ) - 1;

    QgsDebugMsg( QString( "fromRow = %1 toRow = %2 fromCol = %3 toCol = %4" ).arg( fromRow ).arg( toRow ).arg( fromCol ).arg( toCol ) );

    if ( fromRow < 0 || fromRow >= theHeight || toRow < 0 || toRow >= theHeight ||
         fromCol < 0 || fromCol >= theWidth || toCol < 0 || toCol >= theWidth )
    {
      // Should not happen
      QgsDebugMsg( "Row or column limits out of range" );
      return block;
    }

    // If lower source resolution is used, the extent must beS aligned to original
    // resolution to avoid possible shift due to resampling
    if ( tmpXRes > xRes )
    {
      int col = floor(( tmpExtent.xMinimum() - extent().xMinimum() ) / providerXRes );
      tmpExtent.setXMinimum( extent().xMinimum() + col * providerXRes );
      col = ceil(( tmpExtent.xMaximum() - extent().xMinimum() ) / providerXRes );
      tmpExtent.setXMaximum( extent().xMinimum() + col * providerXRes );
    }
    if ( tmpYRes > yRes )
    {
      int row = floor(( extent().yMaximum() - tmpExtent.yMaximum() ) / providerYRes );
      tmpExtent.setYMaximum( extent().yMaximum() - row * providerYRes );
      row = ceil(( extent().yMaximum() - tmpExtent.yMinimum() ) / providerYRes );
      tmpExtent.setYMinimum( extent().yMaximum() - row * providerYRes );
    }
    int tmpWidth = qRound( tmpExtent.width() / tmpXRes );
    int tmpHeight = qRound( tmpExtent.height() / tmpYRes );
    tmpXRes = tmpExtent.width() / tmpWidth;
    tmpYRes = tmpExtent.height() / tmpHeight;

    QgsDebugMsg( QString( "Reading smaller block tmpWidth = %1 theHeight = %2" ).arg( tmpWidth ).arg( tmpHeight ) );
    QgsDebugMsg( QString( "tmpExtent = %1" ).arg( tmpExtent.toString() ) );

    block->setIsNoData();

    QgsRasterBlock *tmpBlock = new QgsRasterBlock( dataType( theBandNo ), tmpWidth, tmpHeight, noDataValue( theBandNo ) );

    readBlock( theBandNo, tmpExtent, tmpWidth, tmpHeight, tmpBlock->data() );

    int pixelSize = dataTypeSize( theBandNo );

    double xMin = theExtent.xMinimum();
    double yMax = theExtent.yMaximum();
    double tmpXMin = tmpExtent.xMinimum();
    double tmpYMax = tmpExtent.yMaximum();

    for ( int row = fromRow; row <= toRow; row++ )
    {
      double y = yMax - ( row + 0.5 ) * yRes;
      int tmpRow = floor(( tmpYMax - y ) / tmpYRes );

      for ( int col = fromCol; col <= toCol; col++ )
      {
        double x = xMin + ( col + 0.5 ) * xRes;
        int tmpCol = floor(( x - tmpXMin ) / tmpXRes );

        if ( tmpRow < 0 || tmpRow >= tmpHeight || tmpCol < 0 || tmpCol >= tmpWidth )
        {
          QgsDebugMsg( "Source row or column limits out of range" );
          block->setIsNoData(); // so that the problem becomes obvious and fixed
          delete tmpBlock;
          return block;
        }

        size_t tmpIndex = tmpRow * tmpWidth + tmpCol;
        size_t index = row * theWidth + col;

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
    readBlock( theBandNo, theExtent, theWidth, theHeight, block->data() );
  }

  // apply user no data values
  // TODO: there are other readBlock methods where no data are not applied
  block->applyNodataValues( userNoDataValue( theBandNo ) );
  return block;
}

QgsRasterDataProvider::QgsRasterDataProvider()
    : QgsRasterInterface( 0 )
    , mDpi( -1 )
{
}

QgsRasterDataProvider::QgsRasterDataProvider( QString const & uri )
    : QgsDataProvider( uri )
    , QgsRasterInterface( 0 )
    , mDpi( -1 )
{
}

//
//Random Static convenience function
//
/////////////////////////////////////////////////////////
//TODO: Change these to private function or make seprate class
// convenience function for building metadata() HTML table cells
// convenience function for creating a string list from a C style string list
QStringList QgsRasterDataProvider::cStringList2Q_( char ** stringList )
{
  QStringList strings;

  // presume null terminated string list
  for ( size_t i = 0; stringList[i]; ++i )
  {
    strings.append( stringList[i] );
  }

  return strings;

} // cStringList2Q_


QString QgsRasterDataProvider::makeTableCell( QString const & value )
{
  return "<p>\n" + value + "</p>\n";
} // makeTableCell_



// convenience function for building metadata() HTML table cells
QString QgsRasterDataProvider::makeTableCells( QStringList const & values )
{
  QString s( "<tr>" );

  for ( QStringList::const_iterator i = values.begin();
        i != values.end();
        ++i )
  {
    s += QgsRasterDataProvider::makeTableCell( *i );
  }

  s += "</tr>";

  return s;
} // makeTableCell_

QString QgsRasterDataProvider::metadata()
{
  QString s;
  return s;
}

// Default implementation for values
//QMap<int, QVariant> QgsRasterDataProvider::identify( const QgsPoint & thePoint, IdentifyFormat theFormat, const QgsRectangle &theExtent, int theWidth, int theHeight )
QgsRasterIdentifyResult QgsRasterDataProvider::identify( const QgsPoint & thePoint, IdentifyFormat theFormat, const QgsRectangle &theExtent, int theWidth, int theHeight )
{
  QgsDebugMsg( "Entered" );
  QMap<int, QVariant> results;

  if ( theFormat != IdentifyFormatValue || !( capabilities() & IdentifyValue ) )
  {
    QgsDebugMsg( "Format not supported" );
    return QgsRasterIdentifyResult( ERR( tr( "Format not supported" ) ) );
  }

  if ( !extent().contains( thePoint ) )
  {
    // Outside the raster
    for ( int bandNo = 1; bandNo <= bandCount(); bandNo++ )
    {
      results.insert( bandNo, noDataValue( bandNo ) );
    }
    return QgsRasterIdentifyResult( QgsRasterDataProvider::IdentifyFormatValue, results );
  }

  QgsRectangle myExtent = theExtent;
  if ( myExtent.isEmpty() )  myExtent = extent();

  if ( theWidth == 0 )
  {
    theWidth = capabilities() & Size ? xSize() : 1000;
  }
  if ( theHeight == 0 )
  {
    theHeight = capabilities() & Size ? ySize() : 1000;
  }

  // Calculate the row / column where the point falls
  double xres = ( myExtent.width() ) / theWidth;
  double yres = ( myExtent.height() ) / theHeight;

  int col = ( int ) floor(( thePoint.x() - myExtent.xMinimum() ) / xres );
  int row = ( int ) floor(( myExtent.yMaximum() - thePoint.y() ) / yres );

  double xMin = myExtent.xMinimum() + col * xres;
  double xMax = xMin + xres;
  double yMax = myExtent.yMaximum() - row * yres;
  double yMin = yMax - yres;
  QgsRectangle pixelExtent( xMin, yMin, xMax, yMax );

  for ( int i = 1; i <= bandCount(); i++ )
  {
    QgsRasterBlock * myBlock = block( i, pixelExtent, 1, 1 );

    double value = noDataValue( i );
    if ( myBlock ) value = myBlock->value( 0 );

    results.insert( i, value );
  }
  return QgsRasterIdentifyResult( QgsRasterDataProvider::IdentifyFormatValue, results );
}

QMap<QString, QString> QgsRasterDataProvider::identify( const QgsPoint & thePoint, const QgsRectangle &theExtent, int theWidth, int theHeight )
{
  QMap<QString, QString> results;

  QgsRasterDataProvider::IdentifyFormat identifyFormat;
  if ( capabilities() & QgsRasterDataProvider::IdentifyValue )
  {
    identifyFormat = QgsRasterDataProvider::IdentifyFormatValue;
  }
  else if ( capabilities() & QgsRasterDataProvider::IdentifyHtml )
  {
    identifyFormat = QgsRasterDataProvider::IdentifyFormatHtml;
  }
  else if ( capabilities() & QgsRasterDataProvider::IdentifyText )
  {
    identifyFormat = QgsRasterDataProvider::IdentifyFormatText;
  }
  else
  {
    return results;
  }

  QgsRasterIdentifyResult myResult = identify( thePoint, identifyFormat, theExtent, theWidth, theHeight );
  QMap<int, QVariant> myResults = myResult.results();

  if ( identifyFormat == QgsRasterDataProvider::IdentifyFormatValue )
  {
    foreach ( int bandNo, myResults.keys() )
    {
      double value = myResults.value( bandNo ).toDouble();
      QString valueString;
      if ( isNoDataValue( bandNo, value ) )
      {
        valueString = tr( "no data" );
      }
      else
      {
        valueString = QgsRasterBlock::printValue( value );
      }
      results.insert( generateBandName( bandNo ), valueString );
    }
  }
  else // text or html
  {
    foreach ( int bandNo, myResults.keys() )
    {
      QString value = myResults.value( bandNo ).toString();
      // TODO: better 'attribute' name, in theory it may be something else than WMS
      // feature info
      if ( identifyFormat == QgsRasterDataProvider::IdentifyFormatText )
      {
        value = "<pre>" + value + "</pre>";
      }
      results.insert( tr( "Feature info" ), value );
    }
  }

  return results;
}

QString QgsRasterDataProvider::lastErrorFormat()
{
  return "text/plain";
}

// pyramids resampling

// TODO move this to gdal provider
// but we need some way to get a static instance of the provider
// or use function pointers like in QgsRasterFormatSaveOptionsWidget::helpOptions()

// see http://www.gdal.org/gdaladdo.html
//     http://www.gdal.org/classGDALDataset.html#a2aa6f88b3bbc840a5696236af11dde15
//     http://www.gdal.org/classGDALRasterBand.html#afaea945b13ec9c86c2d783b883c68432

// from http://www.gdal.org/gdaladdo.html
//   average_mp is unsuitable for use thus not included

// from qgsgdalprovider.cpp (removed)
//   NOTE magphase is disabled in the gui since it tends
//   to create corrupted images. The images can be repaired
//   by running one of the other resampling strategies below.
//   see ticket #284
QStringList QgsRasterDataProvider::mPyramidResamplingListGdal = QStringList();
QgsStringMap QgsRasterDataProvider::mPyramidResamplingMapGdal = QgsStringMap();

void QgsRasterDataProvider::initPyramidResamplingDefs()
{
  mPyramidResamplingListGdal.clear();
  mPyramidResamplingListGdal << tr( "Nearest Neighbour" ) << tr( "Average" ) << tr( "Gauss" ) << tr( "Cubic" ) << tr( "Mode" ) << tr( "None" ); // << tr( "Average magphase" )
  mPyramidResamplingMapGdal.clear();
  mPyramidResamplingMapGdal[ tr( "Nearest Neighbour" )] = "NEAREST";
  mPyramidResamplingMapGdal[ tr( "Average" )] = "AVERAGE";
  mPyramidResamplingMapGdal[ tr( "Gauss" )] = "GAUSS";
  mPyramidResamplingMapGdal[ tr( "Cubic" )] = "CUBIC";
  mPyramidResamplingMapGdal[ tr( "Mode" )] = "MODE";
  // mPyramidResamplingMapGdal[ tr( "Average magphase" ) ] = "average_magphase";
  mPyramidResamplingMapGdal[ tr( "None" )] = "NONE" ;
}

QStringList QgsRasterDataProvider::pyramidResamplingMethods( QString providerKey )
{
  if ( mPyramidResamplingListGdal.isEmpty() )
    initPyramidResamplingDefs();

  return providerKey == "gdal" ? mPyramidResamplingListGdal : QStringList();
}

QString QgsRasterDataProvider::pyramidResamplingArg( QString method, QString providerKey )
{
  if ( providerKey != "gdal" )
    return QString();

  if ( mPyramidResamplingListGdal.isEmpty() )
    initPyramidResamplingDefs();

  if ( mPyramidResamplingMapGdal.contains( method ) )
    return mPyramidResamplingMapGdal.value( method );
  else
    return "NEAREST";
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

#if 0
double QgsRasterDataProvider::readValue( void *data, int type, int index )
{
  if ( !data )
    return std::numeric_limits<double>::quiet_NaN();

  switch ( type )
  {
    case QGis::Byte:
      return ( double )(( GByte * )data )[index];
      break;
    case QGis::UInt16:
      return ( double )(( GUInt16 * )data )[index];
      break;
    case QGis::Int16:
      return ( double )(( GInt16 * )data )[index];
      break;
    case QGis::UInt32:
      return ( double )(( GUInt32 * )data )[index];
      break;
    case QGis::Int32:
      return ( double )(( GInt32 * )data )[index];
      break;
    case QGis::Float32:
      return ( double )(( float * )data )[index];
      break;
    case QGis::Float64:
      return ( double )(( double * )data )[index];
      break;
    default:
      QgsLogger::warning( "GDAL data type is not supported" );
  }

  return std::numeric_limits<double>::quiet_NaN();
}
#endif

void QgsRasterDataProvider::setUserNoDataValue( int bandNo, QList<QgsRasterBlock::Range> noData )
{
  //if ( bandNo > bandCount() ) return;
  if ( bandNo >= mUserNoDataValue.size() )
  {
    for ( int i = mUserNoDataValue.size(); i < bandNo; i++ )
    {
      mUserNoDataValue.append( QList<QgsRasterBlock::Range>() );
    }
  }
  QgsDebugMsg( QString( "set %1 band %1 no data ranges" ).arg( noData.size() ) );

  if ( mUserNoDataValue[bandNo-1] != noData )
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
    mUserNoDataValue[bandNo-1] = noData;
  }
}

typedef QgsRasterDataProvider * createFunction_t( const QString&,
    const QString&, int,
    QGis::DataType,
    int, int, double*,
    const QgsCoordinateReferenceSystem&,
    QStringList );

QgsRasterDataProvider* QgsRasterDataProvider::create( const QString &providerKey,
    const QString &uri,
    const QString& format, int nBands,
    QGis::DataType type,
    int width, int height, double* geoTransform,
    const QgsCoordinateReferenceSystem& crs,
    QStringList createOptions )
{
  createFunction_t *createFn = ( createFunction_t* ) cast_to_fptr( QgsProviderRegistry::instance()->function( providerKey, "create" ) );
  if ( !createFn )
  {
    QgsDebugMsg( "Cannot resolve 'create' function in " + providerKey + " provider" );
    // TODO: it would be good to return invalid QgsRasterDataProvider
    // with QgsError set, but QgsRasterDataProvider has pure virtual methods
    return 0;
  }
  return createFn( uri, format, nBands, type, width, height, geoTransform, crs, createOptions );
}

QString QgsRasterDataProvider::identifyFormatName( IdentifyFormat format )
{
  switch ( format )
  {
    case IdentifyFormatValue:
      return "Value";
    case IdentifyFormatText:
      return "Text";
    case IdentifyFormatHtml:
      return "Html";
    case IdentifyFormatFeature:
      return "Feature";
    default:
      return "Undefined";
  }
}

QString QgsRasterDataProvider::identifyFormatLabel( IdentifyFormat format )
{
  switch ( format )
  {
    case IdentifyFormatValue:
      return tr( "Value" );
    case IdentifyFormatText:
      return ( "Text" );
    case IdentifyFormatHtml:
      return tr( "Html" );
    case IdentifyFormatFeature:
      return tr( "Feature" );
    default:
      return "Undefined";
  }
}

QgsRasterDataProvider::IdentifyFormat QgsRasterDataProvider::identifyFormatFromName( QString formatName )
{
  if ( formatName == "Value" ) return IdentifyFormatValue;
  if ( formatName == "Text" ) return IdentifyFormatText;
  if ( formatName == "Html" ) return IdentifyFormatHtml;
  if ( formatName == "Feature" ) return IdentifyFormatFeature;
  return IdentifyFormatUndefined;
}

QgsRasterInterface::Capability QgsRasterDataProvider::identifyFormatToCapability( IdentifyFormat format )
{
  switch ( format )
  {
    case IdentifyFormatValue:
      return IdentifyValue;
    case IdentifyFormatText:
      return IdentifyText;
    case IdentifyFormatHtml:
      return IdentifyHtml;
    case IdentifyFormatFeature:
      return IdentifyFeature;
    default:
      return NoCapabilities;
  }
}

// ENDS
