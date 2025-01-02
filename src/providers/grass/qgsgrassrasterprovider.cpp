/***************************************************************************
  qgsgrassrasterprovider.cpp  -  QGIS Data provider for
                           GRASS rasters
                             -------------------
    begin                : 16 Jan, 2010
    copyright            : (C) 2010 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <limits>

#include "qgslogger.h"
#include "qgsgrass.h"
#include "qgsrasteridentifyresult.h"
#include "qgsgrassrasterprovider.h"
#include "moc_qgsgrassrasterprovider.cpp"
#include "qgsconfig.h"

#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgshtmlutils.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrasterbandstats.h"

#include <QImage>
#include <QSettings>
#include <QColor>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHash>

#define ERR( message ) QGS_ERROR_MESSAGE( message, "GRASS provider" )
#define QGS_ERROR( message ) QgsError( message, "GRASS provider" )

// Do not use warning dialogs, providers are also created on threads (rendering) where dialogs cannot be used (constructing QPixmap icon)

QgsGrassRasterProvider::QgsGrassRasterProvider( QString const &uri )
  : QgsRasterDataProvider( uri )
  , mNoDataValue( std::numeric_limits<double>::quiet_NaN() )
{
  QgsDebugMsgLevel( "QgsGrassRasterProvider: constructing with uri '" + uri + "'.", 2 );

  if ( !QgsGrass::init() )
  {
    return;
  }

  // Parse URI, it is the same like using GDAL, i.e. path to raster cellhd, i.e.
  // /path/to/gisdbase/location/mapset/cellhd/map
  QFileInfo fileInfo( uri );
  if ( !fileInfo.exists() ) // then we keep it valid forever
  {
    appendError( ERR( tr( "cellhd file %1 does not exist" ).arg( uri ) ) );
    return;
  }

  mMapName = fileInfo.fileName();
  QDir dir = fileInfo.dir();
  QString element = dir.dirName();
  if ( element != QLatin1String( "cellhd" ) )
  {
    appendError( ERR( tr( "Groups not yet supported" ) ) );
    return;
  }
  dir.cdUp(); // skip cellhd
  mMapset = dir.dirName();
  dir.cdUp();
  mLocation = dir.dirName();
  dir.cdUp();
  mGisdbase = dir.path();

  QgsDebugMsgLevel( QString( "gisdbase: %1" ).arg( mGisdbase ), 2 );
  QgsDebugMsgLevel( QString( "location: %1" ).arg( mLocation ), 2 );
  QgsDebugMsgLevel( QString( "mapset: %1" ).arg( mMapset ), 2 );
  QgsDebugMsgLevel( QString( "mapName: %1" ).arg( mMapName ), 2 );

  mTimestamp = dataTimestamp();

  mRasterValue.set( mGisdbase, mLocation, mMapset, mMapName );
  //mValidNoDataValue = true;

  QString error;
  mCrs = QgsGrass::crs( mGisdbase, mLocation, error );
  appendIfError( error );
  QgsDebugMsgLevel( "mCrs: " + mCrs.toWkt(), 2 );

  // the block size can change of course when the raster is overridden
  // ibut it is only called once when statistics are calculated
  error.clear();
  QgsGrass::size( mGisdbase, mLocation, mMapset, mMapName, &mCols, &mRows, error );
  appendIfError( error );

  error.clear();
  mInfo = QgsGrass::info( mGisdbase, mLocation, mMapset, mMapName, QgsGrassObject::Raster, QStringLiteral( "info" ), QgsRectangle(), 0, 0, 3000, error );
  appendIfError( error );

  mGrassDataType = mInfo[QStringLiteral( "TYPE" )].toInt();
  QgsDebugMsgLevel( "mGrassDataType = " + QString::number( mGrassDataType ), 2 );

  // TODO: avoid showing these strange numbers in GUI
  // TODO: don't save no data values in project file, add a flag if value was defined by user

  double myInternalNoDataValue;
  if ( mGrassDataType == CELL_TYPE )
  {
    myInternalNoDataValue = std::numeric_limits<int>::min();
  }
  else if ( mGrassDataType == DCELL_TYPE )
  {
    // Don't use numeric limits, raster layer is using
    //    std::fabs( myValue - mNoDataValue ) <= TINY_VALUE
    // if the mNoDataValue would be a limit, the subtraction could overflow.
    // No data value is shown in GUI, use some nice number.
    // Choose values with small representation error.
    // limit: 1.7976931348623157e+308
    //myInternalNoDataValue = -1e+300;
    myInternalNoDataValue = std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    if ( mGrassDataType != FCELL_TYPE )
    {
      QgsDebugError( "unexpected data type" );
    }

    // limit: 3.40282347e+38
    //myInternalNoDataValue = -1e+30;
    myInternalNoDataValue = std::numeric_limits<float>::quiet_NaN();
  }
  mNoDataValue = myInternalNoDataValue;
  mSrcHasNoDataValue.append( true );
  mSrcNoDataValue.append( mNoDataValue );
  mUseSrcNoDataValue.append( true );
  QgsDebugMsgLevel( QString( "myInternalNoDataValue = %1" ).arg( myInternalNoDataValue ), 2 );

  // TODO: refresh mRows and mCols if raster was rewritten
  // We have to decide some reasonable block size, not to big to occupate too much
  // memory, not too small to result in too many calls to readBlock -> qgis.d.rast
  // for statistics
  int typeSize = QgsRasterBlock::typeSize( dataType( 1 ) );
  if ( mCols > 0 && typeSize > 0 )
  {
    const int cache_size = 10000000; // ~ 10 MB
    mYBlockSize = cache_size / typeSize / mCols;
    if ( mYBlockSize > mRows )
    {
      mYBlockSize = mRows;
    }
    QgsDebugMsgLevel( "mYBlockSize = " + QString::number( mYBlockSize ), 2 );
    mValid = true;
  }
}

QgsGrassRasterProvider::~QgsGrassRasterProvider()
{
  QgsDebugMsgLevel( "QgsGrassRasterProvider: deconstructing.", 2 );
}

QgsGrassRasterProvider *QgsGrassRasterProvider::clone() const
{
  QgsGrassRasterProvider *provider = new QgsGrassRasterProvider( dataSourceUri() );
  provider->copyBaseSettings( *this );
  return provider;
}

bool QgsGrassRasterProvider::readBlock( int bandNo, int xBlock, int yBlock, void *block )
{
  Q_UNUSED( xBlock )
  clearLastError();
  // TODO: optimize, see extent()

  QgsDebugMsgLevel( "yBlock = " + QString::number( yBlock ), 2 );

  QStringList arguments;
  arguments.append( "map=" + mMapName + "@" + mMapset );

  QgsRectangle ext = extent();


  // TODO: cut the last block
  double cellHeight = ext.height() / mRows;
  double yMaximum = ext.yMaximum() - cellHeight * yBlock * mYBlockSize;
  double yMinimum = yMaximum - cellHeight * mYBlockSize;

  QgsDebugMsgLevel( "mYBlockSize = " + QString::number( mYBlockSize ), 2 );
  arguments.append( ( QStringLiteral( "window=%1,%2,%3,%4,%5,%6" )
                        .arg( QgsRasterBlock::printValue( ext.xMinimum() ), QgsRasterBlock::printValue( yMinimum ), QgsRasterBlock::printValue( ext.xMaximum() ), QgsRasterBlock::printValue( yMaximum ) )
                        .arg( mCols )
                        .arg( mYBlockSize ) ) );

  arguments.append( QStringLiteral( "format=value" ) );
  QString cmd = QgsApplication::libexecPath() + "grass/modules/qgis.d.rast";
  QByteArray data;
  try
  {
    data = QgsGrass::runModule( mGisdbase, mLocation, mMapset, cmd, arguments );
  }
  catch ( QgsGrass::Exception &e )
  {
    QString error = tr( "Cannot read raster" ) + " : " + e.what();
    QgsDebugError( error );
    appendError( error );
    // We don't set mValid to false, because the raster can be recreated and work next time
    return false;
  }
  QgsDebugMsgLevel( QString( "%1 bytes read from modules stdout" ).arg( data.size() ), 2 );
  // byteCount() in Qt >= 4.6
  //int size = image->byteCount() < data.size() ? image->byteCount() : data.size();
  int size = mCols * mYBlockSize * dataTypeSize( bandNo );
  QgsDebugMsgLevel( QString( "mCols = %1 mYBlockSize = %2 dataTypeSize = %3" ).arg( mCols ).arg( mYBlockSize ).arg( dataTypeSize( bandNo ) ), 2 );
  if ( size != data.size() )
  {
    QString error = tr( "%1 bytes expected but %2 byte were read from qgis.d.rast" ).arg( size ).arg( data.size() );
    QgsDebugError( error );
    appendError( error );
    size = size < data.size() ? size : data.size();
  }
  memcpy( block, data.data(), size );

  return true;
}

bool QgsGrassRasterProvider::readBlock( int bandNo, QgsRectangle const &viewExtent, int pixelWidth, int pixelHeight, void *block, QgsRasterBlockFeedback *feedback )
{
  Q_UNUSED( feedback )
  QgsDebugMsgLevel( "pixelWidth = " + QString::number( pixelWidth ), 2 );
  QgsDebugMsgLevel( "pixelHeight = " + QString::number( pixelHeight ), 2 );
  QgsDebugMsgLevel( "viewExtent: " + viewExtent.toString(), 2 );
  clearLastError();

  if ( pixelWidth <= 0 || pixelHeight <= 0 )
    return false;

  QStringList arguments;
  arguments.append( "map=" + mMapName + "@" + mMapset );

  arguments.append( ( QStringLiteral( "window=%1,%2,%3,%4,%5,%6" )
                        .arg( QgsRasterBlock::printValue( viewExtent.xMinimum() ), QgsRasterBlock::printValue( viewExtent.yMinimum() ), QgsRasterBlock::printValue( viewExtent.xMaximum() ), QgsRasterBlock::printValue( viewExtent.yMaximum() ) )
                        .arg( pixelWidth )
                        .arg( pixelHeight ) ) );
  arguments.append( QStringLiteral( "format=value" ) );
  QString cmd = QgsApplication::libexecPath() + "grass/modules/qgis.d.rast";
  QByteArray data;
  try
  {
    data = QgsGrass::runModule( mGisdbase, mLocation, mMapset, cmd, arguments );
  }
  catch ( QgsGrass::Exception &e )
  {
    QString error = tr( "Cannot read raster" ) + " : " + e.what();
    QgsDebugError( error );
    appendError( error );

    // We don't set mValid to false, because the raster can be recreated and work next time
    return false;
  }
  QgsDebugMsgLevel( QString( "%1 bytes read from modules stdout" ).arg( data.size() ), 2 );
  // byteCount() in Qt >= 4.6
  //int size = image->byteCount() < data.size() ? image->byteCount() : data.size();
  int size = pixelWidth * pixelHeight * dataTypeSize( bandNo );
  if ( size != data.size() )
  {
    QString error = tr( "%1 bytes expected but %2 byte were read from qgis.d.rast" ).arg( size ).arg( data.size() );
    QgsDebugError( error );
    appendError( error );
    size = size < data.size() ? size : data.size();
  }
  memcpy( block, data.data(), size );

  return true;
}

QgsRasterBandStats QgsGrassRasterProvider::bandStatistics( int bandNo, Qgis::RasterBandStatistics stats, const QgsRectangle &boundingBox, int sampleSize, QgsRasterBlockFeedback * )
{
  QgsDebugMsgLevel( QString( "theBandNo = %1 sampleSize = %2" ).arg( bandNo ).arg( sampleSize ), 2 );
  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, bandNo, stats, boundingBox, sampleSize );

  const auto constMStatistics = mStatistics;
  for ( const QgsRasterBandStats &stats : constMStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsgLevel( "Using cached statistics.", 2 );
      return stats;
    }
  }

  QgsRectangle extent = myRasterBandStats.extent;

  int sampleRows = myRasterBandStats.height;
  int sampleCols = myRasterBandStats.width;

  // With stats we have to be careful about timeout, empirical value,
  // 0.001 / cell should be sufficient using 0.005 to be sure + constant (ms)
  int timeout = 30000 + 0.005 * xSize() * ySize();

  QString error;
  QHash<QString, QString> info = QgsGrass::info( mGisdbase, mLocation, mMapset, mMapName, QgsGrassObject::Raster, QStringLiteral( "stats" ), extent, sampleRows, sampleCols, timeout, error );

  if ( info.isEmpty() || !error.isEmpty() )
  {
    return myRasterBandStats;
  }

  myRasterBandStats.sum = info[QStringLiteral( "SUM" )].toDouble();
  myRasterBandStats.elementCount = info[QStringLiteral( "COUNT" )].toInt();
  myRasterBandStats.minimumValue = info[QStringLiteral( "MIN" )].toDouble();
  myRasterBandStats.maximumValue = info[QStringLiteral( "MAX" )].toDouble();
  myRasterBandStats.range = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
  myRasterBandStats.sumOfSquares = info[QStringLiteral( "SQSUM" )].toDouble();
  myRasterBandStats.mean = info[QStringLiteral( "MEAN" )].toDouble();
  myRasterBandStats.stdDev = info[QStringLiteral( "STDEV" )].toDouble();

  QgsDebugMsgLevel( QString( "min = %1" ).arg( myRasterBandStats.minimumValue ), 2 );
  QgsDebugMsgLevel( QString( "max = %1" ).arg( myRasterBandStats.maximumValue ), 2 );
  QgsDebugMsgLevel( QString( "count = %1" ).arg( myRasterBandStats.elementCount ), 2 );
  QgsDebugMsgLevel( QString( "stdev = %1" ).arg( myRasterBandStats.stdDev ), 2 );

  myRasterBandStats.statsGathered = Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max | Qgis::RasterBandStatistic::Range | Qgis::RasterBandStatistic::Mean | Qgis::RasterBandStatistic::Sum | Qgis::RasterBandStatistic::SumOfSquares | Qgis::RasterBandStatistic::StdDev;

  mStatistics.append( myRasterBandStats );
  return myRasterBandStats;
}

QList<QgsColorRampShader::ColorRampItem> QgsGrassRasterProvider::colorTable( int bandNo ) const
{
  Q_UNUSED( bandNo )
  QList<QgsColorRampShader::ColorRampItem> ct;

  // TODO: check if color can be really discontinuous in GRASS,
  // for now we just believe that they are continuous, i.e. end and beginning
  // of the ramp with the same value has the same color
  // we are also expecting ordered CT records in the list
  QString error;
  QList<QgsGrass::Color> colors = QgsGrass::colors( mGisdbase, mLocation, mMapset, mMapName, error );
  if ( !error.isEmpty() )
  {
    return ct;
  }
  QList<QgsGrass::Color>::iterator i;

  double v = 0.0, r = 0.0, g = 0.0, b = 0.0;
  for ( i = colors.begin(); i != colors.end(); ++i )
  {
    if ( ct.count() == 0 || i->value1 != v || i->red1 != r || i->green1 != g || i->blue1 != b )
    {
      // not added in previous rule
      QgsColorRampShader::ColorRampItem ctItem1;
      ctItem1.value = i->value1;
      ctItem1.color = QColor::fromRgb( i->red1, i->green1, i->blue1 );
      ct.append( ctItem1 );
      QgsDebugMsgLevel( QString( "color %1 %2 %3 %4" ).arg( i->value1 ).arg( i->red1 ).arg( i->green1 ).arg( i->blue1 ), 2 );
    }
    QgsColorRampShader::ColorRampItem ctItem2;
    ctItem2.value = i->value2;
    ctItem2.color = QColor::fromRgb( i->red2, i->green2, i->blue2 );
    ct.append( ctItem2 );
    QgsDebugMsgLevel( QString( "color %1 %2 %3 %4" ).arg( i->value2 ).arg( i->red2 ).arg( i->green2 ).arg( i->blue2 ), 2 );

    v = i->value2;
    r = i->red2;
    g = i->green2;
    b = i->blue2;
  }
  return ct;
}

QgsCoordinateReferenceSystem QgsGrassRasterProvider::crs() const
{
  return mCrs;
}

QgsRectangle QgsGrassRasterProvider::extent() const
{
  // The extend can change of course so we get always fresh, to avoid running always the module
  // we should save mExtent and mLastModified and check if the map was modified

  QString error;
  mExtent = QgsGrass::extent( mGisdbase, mLocation, mMapset, mMapName, QgsGrassObject::Raster, error );

  QgsDebugMsgLevel( "Extent got", 2 );
  return mExtent;
}

// this is only called once when statistics are calculated
int QgsGrassRasterProvider::xBlockSize() const { return mCols; }
int QgsGrassRasterProvider::yBlockSize() const
{
  return mYBlockSize;
}

// TODO this should be always refreshed if raster has changed ?
// maybe also only for stats
int QgsGrassRasterProvider::xSize() const { return mCols; }
int QgsGrassRasterProvider::ySize() const { return mRows; }

QgsRasterIdentifyResult QgsGrassRasterProvider::identify( const QgsPointXY &point, Qgis::RasterIdentifyFormat format, const QgsRectangle &boundingBox, int width, int height, int /*dpi*/ )
{
  Q_UNUSED( boundingBox )
  Q_UNUSED( width )
  Q_UNUSED( height )
  QMap<int, QVariant> results;
  QMap<int, QVariant> noDataResults;
  noDataResults.insert( 1, QVariant() );
  QgsRasterIdentifyResult noDataResult( Qgis::RasterIdentifyFormat::Value, results );

  if ( format != Qgis::RasterIdentifyFormat::Value )
  {
    return QgsRasterIdentifyResult( QGS_ERROR( tr( "Format not supported" ) ) );
  }

  if ( !extent().contains( point ) )
  {
    return noDataResult;
  }

  // TODO: use doubles instead of strings

  // attention, value tool does his own tricks with grass identify() so it stops to refresh values outside extent or null values e.g.

  bool ok;
  double value = mRasterValue.value( point.x(), point.y(), &ok );

  if ( !ok )
  {
    return QgsRasterIdentifyResult( QGS_ERROR( tr( "Cannot read data" ) ) );
  }

  // no data?
  if ( std::isnan( value ) || qgsDoubleNear( value, mNoDataValue ) )
  {
    return noDataResult;
  }

  // Apply user no data
  QgsRasterRangeList myNoDataRangeList = userNoDataValues( 1 );
  if ( QgsRasterRange::contains( value, myNoDataRangeList ) )
  {
    return noDataResult;
  }

  results.insert( 1, value );

  return QgsRasterIdentifyResult( Qgis::RasterIdentifyFormat::Value, results );
}

Qgis::RasterInterfaceCapabilities QgsGrassRasterProvider::capabilities() const
{
  Qgis::RasterInterfaceCapabilities capability = Qgis::RasterInterfaceCapability::Identify
                                                 | Qgis::RasterInterfaceCapability::IdentifyValue
                                                 | Qgis::RasterInterfaceCapability::Size;
  return capability;
}

Qgis::DataType QgsGrassRasterProvider::dataType( int bandNo ) const
{
  return sourceDataType( bandNo );
}

Qgis::DataType QgsGrassRasterProvider::sourceDataType( int bandNo ) const
{
  Q_UNUSED( bandNo )
  switch ( mGrassDataType )
  {
    case CELL_TYPE:
      return Qgis::DataType::Int32;
    case FCELL_TYPE:
      return Qgis::DataType::Float32;
    case DCELL_TYPE:
      return Qgis::DataType::Float64;
  }
  return Qgis::DataType::UnknownDataType;
}

int QgsGrassRasterProvider::bandCount() const
{
  // TODO
  return 1;
}

Qgis::RasterColorInterpretation QgsGrassRasterProvider::colorInterpretation( int bandNo ) const
{
  // TODO: avoid loading color table here or cache it
  QList<QgsColorRampShader::ColorRampItem> ct = colorTable( bandNo );
  if ( ct.size() > 0 )
  {
    return Qgis::RasterColorInterpretation::ContinuousPalette;
  }
  return Qgis::RasterColorInterpretation::GrayIndex;
}

QString QgsGrassRasterProvider::htmlMetadata() const
{
  QString myMetadata;
  QStringList myList;
  myList.append( "GISDBASE: " + mGisdbase );
  myList.append( "LOCATION: " + mLocation );
  myList.append( "MAPSET: " + mMapset );
  myList.append( "MAP: " + mMapName );

  for ( auto it = mInfo.constBegin(); it != mInfo.constEnd(); ++it )
  {
    myList.append( it.key() + " : " + it.value() );
  }
  myMetadata += QgsHtmlUtils::buildBulletList( myList );
  return myMetadata;
}

bool QgsGrassRasterProvider::isValid() const
{
  return mValid;
}

void QgsGrassRasterProvider::setLastError( const QString &error )
{
  mLastErrorTitle = tr( "GRASS raster provider" );
  mLastError = error;
}

void QgsGrassRasterProvider::clearLastError()
{
  mLastErrorTitle.clear();
  mLastError.clear();
}

void QgsGrassRasterProvider::appendIfError( const QString &error )
{
  if ( !error.isEmpty() )
  {
    appendError( ERR( error ) );
  }
}

QString QgsGrassRasterProvider::lastErrorTitle()
{
  return mLastErrorTitle;
}

QString QgsGrassRasterProvider::lastError()
{
  return mLastError;
}

QString QgsGrassRasterProvider::name() const
{
  return QStringLiteral( "grassraster" );
}

QString QgsGrassRasterProvider::description() const
{
  return QStringLiteral( "GRASS %1 raster provider" ).arg( GRASS_VERSION_MAJOR );
}

QDateTime QgsGrassRasterProvider::dataTimestamp() const
{
  QDateTime time;
  QString mapset = mGisdbase + "/" + mLocation + "/" + mMapset;
  QStringList dirs;
  dirs << QStringLiteral( "cell" ) << QStringLiteral( "colr" );
  const auto constDirs = dirs;
  for ( const QString &dir : constDirs )
  {
    QString path = mapset + "/" + dir + "/" + mMapName;
    QFileInfo fi( path );
    if ( fi.exists() && fi.lastModified() > time )
    {
      time = fi.lastModified();
    }
  }
  QgsDebugMsgLevel( "timestamp = " + time.toString(), 2 );

  return time;
}

void QgsGrassRasterProvider::freeze()
{
  mRasterValue.stop();
  mValid = false;
}

void QgsGrassRasterProvider::thaw()
{
  mValid = true;
}

//-------------------------------- QgsGrassRasterValue ----------------------------------------

QgsGrassRasterValue::~QgsGrassRasterValue()
{
  stop();
}

void QgsGrassRasterValue::set( const QString &gisdbase, const QString &location, const QString &mapset, const QString &map )
{
  mGisdbase = gisdbase;
  mLocation = location;
  mMapset = mapset;
  mMapName = map;
}

void QgsGrassRasterValue::start()
{
  if ( mProcess )
  {
    QgsDebugMsgLevel( "already running", 2 );
  }
  // TODO: catch exceptions
  QString module = QgsGrass::qgisGrassModulePath() + "/qgis.g.info";
  QStringList arguments;

  arguments.append( QStringLiteral( "info=query" ) );
  arguments.append( "rast=" + mMapName + "@" + mMapset );
  try
  {
    mProcess = QgsGrass::startModule( mGisdbase, mLocation, mMapset, module, arguments, mGisrcFile );
  }
  catch ( QgsGrass::Exception &e )
  {
    QString error = e.what();
    Q_UNUSED( error )
    QgsDebugError( error );
  }
}

void QgsGrassRasterValue::stop()
{
  if ( mProcess )
  {
    QgsDebugMsgLevel( "closing process", 2 );
    mProcess->closeWriteChannel();
    mProcess->waitForFinished();
    QgsDebugMsgLevel( "process finished", 2 );
    delete mProcess;
    mProcess = nullptr;
  }
}

double QgsGrassRasterValue::value( double x, double y, bool *ok )
{
  *ok = false;
  double value = std::numeric_limits<double>::quiet_NaN();

  if ( !mProcess )
  {
    start();
  }

  if ( !mProcess )
  {
    return value;
  }

  QString coor = QStringLiteral( "%1 %2\n" ).arg( QgsRasterBlock::printValue( x ), QgsRasterBlock::printValue( y ) );
  QgsDebugMsgLevel( "coor : " + coor, 2 );
  mProcess->write( coor.toLatin1() ); // how to flush, necessary?
  mProcess->waitForReadyRead();
  QString str = mProcess->readLine().trimmed();
  QgsDebugMsgLevel( "read from stdout : " + str, 2 );

  // TODO: use doubles instead of strings

  QStringList list = str.trimmed().split( ':' );
  if ( list.size() == 2 )
  {
    if ( list[1] == QLatin1String( "error" ) )
      return value;
    value = list[1].toDouble( ok );
  }
  return value;
}
