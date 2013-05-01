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
#include "qgsconfig.h"

#include "qgsapplication.h"
#include "qgscoordinatetransform.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsrasterbandstats.h"

#include <QImage>
#include <QSettings>
#include <QColor>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QHash>

#define ERR(message) QGS_ERROR_MESSAGE(message,"GRASS provider")
#define ERROR(message) QgsError(message,"GRASS provider")

static QString PROVIDER_KEY = "grassraster";
static QString PROVIDER_DESCRIPTION = "GRASS raster provider";

QgsGrassRasterProvider::QgsGrassRasterProvider( QString const & uri )
    : QgsRasterDataProvider( uri ), mValid( false )
{
  QgsDebugMsg( "QgsGrassRasterProvider: constructing with uri '" + uri + "'." );

  mValid = false;
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
  if ( element != "cellhd" )
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

  QgsDebugMsg( QString( "gisdbase: %1" ).arg( mGisdbase ) );
  QgsDebugMsg( QString( "location: %1" ).arg( mLocation ) );
  QgsDebugMsg( QString( "mapset: %1" ).arg( mMapset ) );
  QgsDebugMsg( QString( "mapName: %1" ).arg( mMapName ) );

  mTimestamp = dataTimestamp();

  mRasterValue.start( mGisdbase, mLocation, mMapset, mMapName );
  //mValidNoDataValue = true;

  mCrs = QgsGrass::crs( mGisdbase, mLocation );
  QgsDebugMsg( "mCrs: " + mCrs.toWkt() );

  // the block size can change of course when the raster is overridden
  // ibut it is only called once when statistics are calculated
  QgsGrass::size( mGisdbase, mLocation, mMapset, mMapName, &mCols, &mRows );

  mInfo = QgsGrass::info( mGisdbase, mLocation, mMapset, mMapName, QgsGrass::Raster );

  mGrassDataType = mInfo["TYPE"].toInt();
  QgsDebugMsg( "mGrassDataType = " + QString::number( mGrassDataType ) );

  // TODO: avoid showing these strange numbers in GUI
  // TODO: don't save no data values in project file, add a flag if value was defined by user

  double myInternalNoDataValue;
  if ( mGrassDataType == CELL_TYPE )
  {
    myInternalNoDataValue = -2147483648;
  }
  else if ( mGrassDataType == DCELL_TYPE )
  {
    // Don't use numeric limits, raster layer is using
    //    qAbs( myValue - mNoDataValue ) <= TINY_VALUE
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
      QgsDebugMsg( "unexpected data type" );
    }

    // limit: 3.40282347e+38
    //myInternalNoDataValue = -1e+30;
    myInternalNoDataValue = std::numeric_limits<float>::quiet_NaN();
  }
  mNoDataValue = myInternalNoDataValue;
  QgsDebugMsg( QString( "myInternalNoDataValue = %1" ).arg( myInternalNoDataValue ) );

  // TODO: refresh mRows and mCols if raster was rewritten
  // We have to decide some reasonable block size, not to big to occupate too much
  // memory, not too small to result in too many calls to readBlock -> qgis.d.rast
  // for statistics
  int cache_size = 10000000; // ~ 10 MB
  mYBlockSize = cache_size / ( dataTypeSize( dataType( 1 ) ) ) / mCols;
  if ( mYBlockSize  > mRows )
  {
    mYBlockSize = mRows;
  }
  mValid = true;
  QgsDebugMsg( "mYBlockSize = " + QString::number( mYBlockSize ) );
}

QgsGrassRasterProvider::~QgsGrassRasterProvider()
{
  QgsDebugMsg( "QgsGrassRasterProvider: deconstructing." );
}

QgsRasterInterface * QgsGrassRasterProvider::clone() const
{
  QgsGrassRasterProvider * provider = new QgsGrassRasterProvider( dataSourceUri() );
  return provider;
}

QImage* QgsGrassRasterProvider::draw( QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight )
{
  QgsDebugMsg( "pixelWidth = "  + QString::number( pixelWidth ) );
  QgsDebugMsg( "pixelHeight = "  + QString::number( pixelHeight ) );
  QgsDebugMsg( "viewExtent: " + viewExtent.toString() );

  QImage *image = new QImage( pixelWidth, pixelHeight, QImage::Format_ARGB32 );
  image->fill( QColor( Qt::gray ).rgb() );

  QStringList arguments;
  arguments.append( "map=" +  mMapName + "@" + mMapset );

  arguments.append(( QString( "window=%1,%2,%3,%4,%5,%6" )
                     .arg( QgsRasterBlock::printValue( viewExtent.xMinimum() ) )
                     .arg( QgsRasterBlock::printValue( viewExtent.yMinimum() ) )
                     .arg( QgsRasterBlock::printValue( viewExtent.xMaximum() ) )
                     .arg( QgsRasterBlock::printValue( viewExtent.yMaximum() ) )
                     .arg( pixelWidth ).arg( pixelHeight ) ) );
  QProcess process( this );
  QString cmd = QgsApplication::libexecPath() + "grass/modules/qgis.d.rast";
  QByteArray data;
  try
  {
    data = QgsGrass::runModule( mGisdbase, mLocation, cmd, arguments );
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot draw raster" ) + "\n"
                          + e.what() );

    // We don't set mValid to false, because the raster can be recreated and work next time
    return image;
  }
  QgsDebugMsg( QString( "%1 bytes read from modules stdout" ).arg( data.size() ) );
  uchar * ptr = image->bits( ) ;
  // byteCount() in Qt >= 4.6
  //int size = image->byteCount() < data.size() ? image->byteCount() : data.size();
  int size = pixelWidth * pixelHeight * 4 < data.size() ? pixelWidth * pixelHeight * 4 : data.size();
  memcpy( ptr, data.data(), size );

  return image;
}


void QgsGrassRasterProvider::readBlock( int bandNo, int xBlock, int yBlock, void *block )
{
  Q_UNUSED( xBlock );
  QgsDebugMsg( "Entered" );
  // TODO: optimize, see extent()

  QgsDebugMsg( "yBlock = "  + QString::number( yBlock ) );

  QStringList arguments;
  arguments.append( "map=" +  mMapName + "@" + mMapset );

  QgsRectangle ext = extent();


  // TODO: cut the last block
  double cellHeight = ext.height() / mRows;
  double yMaximum = ext.yMaximum() - cellHeight * yBlock * mYBlockSize;
  double yMinimum = yMaximum - cellHeight * mYBlockSize;

  QgsDebugMsg( "mYBlockSize = " + QString::number( mYBlockSize ) );
  arguments.append(( QString( "window=%1,%2,%3,%4,%5,%6" )
                     .arg( QgsRasterBlock::printValue( ext.xMinimum() ) )
                     .arg( QgsRasterBlock::printValue( yMinimum ) )
                     .arg( QgsRasterBlock::printValue( ext.xMaximum() ) )
                     .arg( QgsRasterBlock::printValue( yMaximum ) )
                     .arg( mCols ).arg( mYBlockSize ) ) );

  arguments.append( "format=value" );
  QProcess process( this );
  QString cmd = QgsApplication::libexecPath() + "grass/modules/qgis.d.rast";
  QByteArray data;
  try
  {
    data = QgsGrass::runModule( mGisdbase, mLocation, cmd, arguments );
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot draw raster" ) + "\n"
                          + e.what() );

    // We don't set mValid to false, because the raster can be recreated and work next time
  }
  QgsDebugMsg( QString( "%1 bytes read from modules stdout" ).arg( data.size() ) );
  // byteCount() in Qt >= 4.6
  //int size = image->byteCount() < data.size() ? image->byteCount() : data.size();
  int size = mCols * mYBlockSize * dataTypeSize( bandNo );
  QgsDebugMsg( QString( "mCols = %1 mYBlockSize = %2 dataTypeSize = %3" ).arg( mCols ).arg( mYBlockSize ).arg( dataTypeSize( bandNo ) ) );
  if ( size != data.size() )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QString( "%1 bytes expected but %2 byte were read from qgis.d.rast" ).arg( size ).arg( data.size() ) );
    size = size < data.size() ? size : data.size();
  }
  memcpy( block, data.data(), size );
}

void QgsGrassRasterProvider::readBlock( int bandNo, QgsRectangle  const & viewExtent, int pixelWidth, int pixelHeight, void *block )
{
  QgsDebugMsg( "Entered" );
  QgsDebugMsg( "pixelWidth = "  + QString::number( pixelWidth ) );
  QgsDebugMsg( "pixelHeight = "  + QString::number( pixelHeight ) );
  QgsDebugMsg( "viewExtent: " + viewExtent.toString() );

  if ( pixelWidth <= 0 || pixelHeight <= 0 )
    return;

  QStringList arguments;
  arguments.append( "map=" +  mMapName + "@" + mMapset );

  arguments.append(( QString( "window=%1,%2,%3,%4,%5,%6" )
                     .arg( QgsRasterBlock::printValue( viewExtent.xMinimum() ) )
                     .arg( QgsRasterBlock::printValue( viewExtent.yMinimum() ) )
                     .arg( QgsRasterBlock::printValue( viewExtent.xMaximum() ) )
                     .arg( QgsRasterBlock::printValue( viewExtent.yMaximum() ) )
                     .arg( pixelWidth ).arg( pixelHeight ) ) );
  arguments.append( "format=value" );
  QProcess process( this );
  QString cmd = QgsApplication::libexecPath() + "grass/modules/qgis.d.rast";
  QByteArray data;
  try
  {
    data = QgsGrass::runModule( mGisdbase, mLocation, cmd, arguments );
  }
  catch ( QgsGrass::Exception &e )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ), QObject::tr( "Cannot draw raster" ) + "\n"
                          + e.what() );

    // We don't set mValid to false, because the raster can be recreated and work next time
    return;
  }
  QgsDebugMsg( QString( "%1 bytes read from modules stdout" ).arg( data.size() ) );
  // byteCount() in Qt >= 4.6
  //int size = image->byteCount() < data.size() ? image->byteCount() : data.size();
  int size = pixelWidth * pixelHeight * dataTypeSize( bandNo );
  if ( size != data.size() )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QString( "%1 bytes expected but %2 byte were read from qgis.d.rast" ).arg( size ).arg( data.size() ) );
    size = size < data.size() ? size : data.size();
  }
  memcpy( block, data.data(), size );
}

QgsRasterBandStats QgsGrassRasterProvider::bandStatistics( int theBandNo, int theStats, const QgsRectangle & theExtent, int theSampleSize )
{
  QgsDebugMsg( QString( "theBandNo = %1 theSampleSize = %2" ).arg( theBandNo ).arg( theSampleSize ) );
  QgsRasterBandStats myRasterBandStats;
  initStatistics( myRasterBandStats, theBandNo, theStats, theExtent, theSampleSize );

  foreach ( QgsRasterBandStats stats, mStatistics )
  {
    if ( stats.contains( myRasterBandStats ) )
    {
      QgsDebugMsg( "Using cached statistics." );
      return stats;
    }
  }

  QgsRectangle extent = myRasterBandStats.extent;

  int sampleRows = myRasterBandStats.height;
  int sampleCols = myRasterBandStats.width;

  // With stats we have to be careful about timeout, empirical value,
  // 0.001 / cell should be sufficient using 0.005 to be sure + constant (ms)
  int timeout = 30000 + 0.005 * xSize() * ySize();

  QHash<QString, QString> info = QgsGrass::info( mGisdbase, mLocation, mMapset, mMapName, QgsGrass::Raster, "stats", extent, sampleRows, sampleCols, timeout );

  if ( info.isEmpty() )
  {
    return myRasterBandStats;
  }

  myRasterBandStats.sum = info["SUM"].toDouble();
  myRasterBandStats.elementCount = info["COUNT"].toInt();
  myRasterBandStats.minimumValue = info["MIN"].toDouble();
  myRasterBandStats.maximumValue = info["MAX"].toDouble();
  myRasterBandStats.range = myRasterBandStats.maximumValue - myRasterBandStats.minimumValue;
  myRasterBandStats.sumOfSquares = info["SQSUM"].toDouble();
  myRasterBandStats.mean = info["MEAN"].toDouble();
  myRasterBandStats.stdDev = info["STDEV"].toDouble();

  QgsDebugMsg( QString( "min = %1" ).arg( myRasterBandStats.minimumValue ) );
  QgsDebugMsg( QString( "max = %1" ).arg( myRasterBandStats.maximumValue ) );
  QgsDebugMsg( QString( "count = %1" ).arg( myRasterBandStats.elementCount ) );
  QgsDebugMsg( QString( "stdev = %1" ).arg( myRasterBandStats.stdDev ) );

  myRasterBandStats.statsGathered = QgsRasterBandStats::Min | QgsRasterBandStats::Max |
                                    QgsRasterBandStats::Range | QgsRasterBandStats::Mean |
                                    QgsRasterBandStats::Sum | QgsRasterBandStats::SumOfSquares |
                                    QgsRasterBandStats::StdDev;

  mStatistics.append( myRasterBandStats );
  return myRasterBandStats;
}

QList<QgsColorRampShader::ColorRampItem> QgsGrassRasterProvider::colorTable( int bandNo )const
{
  Q_UNUSED( bandNo );
  QgsDebugMsg( "Entered" );
  QList<QgsColorRampShader::ColorRampItem> ct;

  // TODO: check if color can be realy discontinuous in GRASS,
  // for now we just believe that they are continuous, i.e. end and beginning
  // of the ramp with the same value has the same color
  // we are also expecting ordered CT records in the list
  QList<QgsGrass::Color> colors = QgsGrass::colors( mGisdbase, mLocation, mMapset, mMapName );
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
      QgsDebugMsg( QString( "color %1 %2 %3 %4" ).arg( i->value1 ).arg( i->red1 ).arg( i->green1 ).arg( i->blue1 ) );
    }
    QgsColorRampShader::ColorRampItem ctItem2;
    ctItem2.value = i->value2;
    ctItem2.color = QColor::fromRgb( i->red2, i->green2, i->blue2 );
    ct.append( ctItem2 );
    QgsDebugMsg( QString( "color %1 %2 %3 %4" ).arg( i->value2 ).arg( i->red2 ).arg( i->green2 ).arg( i->blue2 ) );

    v = i->value2; r = i->red2; g = i->green2; b = i->blue2;
  }
  return ct;
}

QgsCoordinateReferenceSystem QgsGrassRasterProvider::crs()
{
  QgsDebugMsg( "Entered" );
  return mCrs;
}

QgsRectangle QgsGrassRasterProvider::extent()
{
  QgsDebugMsg( "Entered" );
  // The extend can change of course so we get always fresh, to avoid running always the module
  // we should save mExtent and mLastModified and check if the map was modified

  mExtent = QgsGrass::extent( mGisdbase, mLocation, mMapset, mMapName, QgsGrass::Raster );

  QgsDebugMsg( "Extent got" );
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

QgsRasterIdentifyResult QgsGrassRasterProvider::identify( const QgsPoint & thePoint, QgsRaster::IdentifyFormat theFormat, const QgsRectangle &theExtent, int theWidth, int theHeight )
{
  Q_UNUSED( theExtent );
  Q_UNUSED( theWidth );
  Q_UNUSED( theHeight );
  QgsDebugMsg( "Entered" );
  QMap<int, QVariant> results;
  QMap<int, QVariant> noDataResults;
  noDataResults.insert( 1, QVariant() );
  QgsRasterIdentifyResult noDataResult( QgsRaster::IdentifyFormatValue, results );

  if ( theFormat != QgsRaster::IdentifyFormatValue )
  {
    return QgsRasterIdentifyResult( ERROR( tr( "Format not supported" ) ) );
  }

  if ( !extent().contains( thePoint ) )
  {
    return noDataResult;
  }

  // TODO: use doubles instead of strings

  // attention, value tool does his own tricks with grass identify() so it stops to refresh values outside extent or null values e.g.

  bool ok;
  double value = mRasterValue.value( thePoint.x(), thePoint.y(), &ok );

  if ( !ok )
  {
    return QgsRasterIdentifyResult( ERROR( tr( "Cannot read data" ) ) );
  }

  // no data?
  if ( qIsNaN( value ) || qgsDoubleNear( value, mNoDataValue ) )
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

  return QgsRasterIdentifyResult( QgsRaster::IdentifyFormatValue, results );
}

int QgsGrassRasterProvider::capabilities() const
{
  int capability = QgsRasterDataProvider::Identify
                   | QgsRasterDataProvider::IdentifyValue
                   | QgsRasterDataProvider::Size;
  return capability;
}

QGis::DataType QgsGrassRasterProvider::dataType( int bandNo ) const
{
  return srcDataType( bandNo );
}

QGis::DataType QgsGrassRasterProvider::srcDataType( int bandNo ) const
{
  Q_UNUSED( bandNo );
  switch ( mGrassDataType )
  {
    case CELL_TYPE:
      return QGis::Int32;
      break;
    case FCELL_TYPE:
      return QGis::Float32;
      break;
    case DCELL_TYPE:
      return QGis::Float64;
      break;
  }
  return QGis::UnknownDataType;
}

int QgsGrassRasterProvider::bandCount() const
{
  // TODO
  return 1;
}

int QgsGrassRasterProvider::colorInterpretation( int bandNo ) const
{
  // TODO: avoid loading color table here or cache it
  QList<QgsColorRampShader::ColorRampItem> ct = colorTable( bandNo );
  if ( ct.size() > 0 )
  {
    return QgsRaster::ContinuousPalette;
  }
  return QgsRaster::GrayIndex;
}

QString QgsGrassRasterProvider::metadata()
{
  QString myMetadata ;
  QStringList myList;
  myList.append( "GISDBASE: " + mGisdbase );
  myList.append( "LOCATION: " + mLocation );
  myList.append( "MAPSET: " + mMapset );
  myList.append( "MAP: " + mMapName );

  QHash<QString, QString>::iterator i;
  for ( i = mInfo.begin(); i != mInfo.end(); ++i )
  {
    myList.append( i.key() + " : " + i.value() );
  }
  myMetadata += QgsRasterDataProvider::makeTableCells( myList );


  return myMetadata;
}

bool QgsGrassRasterProvider::isValid()
{
  return mValid;
}

QString QgsGrassRasterProvider::lastErrorTitle()
{
  return  QString( "Not implemented" );
}

QString QgsGrassRasterProvider::lastError()
{
  return  QString( "Not implemented" );
}

QString  QgsGrassRasterProvider::name() const
{
  return PROVIDER_KEY;
}

QString  QgsGrassRasterProvider::description() const
{
  return PROVIDER_DESCRIPTION;
}

QDateTime QgsGrassRasterProvider::dataTimestamp() const
{
  QDateTime time;
  QString mapset = mGisdbase + "/" + mLocation + "/" + mMapset;
  QStringList dirs;
  dirs << "cell" << "colr";
  foreach ( QString dir, dirs )
  {
    QString path = mapset + "/" + dir + "/" + mMapName;
    QFileInfo fi( path );
    if ( fi.exists() && fi.lastModified() > time )
    {
      time = fi.lastModified();
    }
  }
  QgsDebugMsg( "timestamp = " + time.toString() );
  return time;
}

/**
 * Class factory to return a pointer to a newly created
 * QgsGrassRasterProvider object
 */
QGISEXTERN QgsGrassRasterProvider * classFactory( const QString *uri )
{
  return new QgsGrassRasterProvider( *uri );
}
/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return PROVIDER_KEY;
}
/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return PROVIDER_DESCRIPTION;
}
/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

QgsGrassRasterValue::QgsGrassRasterValue() : mProcess( 0 )
{
}

void QgsGrassRasterValue::start( QString gisdbase, QString location,
                                 QString mapset, QString map )
{
  mGisdbase = gisdbase;
  mLocation = location;
  mMapset = mapset;
  mMapName = map;
  // TODO: catch exceptions
  QString module = QgsApplication::libexecPath() + "grass/modules/qgis.g.info";
  QStringList arguments;

  arguments.append( "info=query" );
  arguments.append( "rast=" +  mMapName + "@" + mMapset );
  mProcess = QgsGrass::startModule( mGisdbase, mLocation, module, arguments, mGisrcFile );
}
QgsGrassRasterValue::~QgsGrassRasterValue()
{
  if ( mProcess )
  {
    QgsDebugMsg( "closing process" );
    mProcess->closeWriteChannel();
    mProcess->waitForFinished();
    QgsDebugMsg( "process finished" );
    delete mProcess;
  }
}

double QgsGrassRasterValue::value( double x, double y, bool *ok )
{
  *ok = false;
  double value = std::numeric_limits<double>::quiet_NaN();

  if ( !mProcess ) return value;

  QString coor = QString( "%1 %2\n" ).arg( QgsRasterBlock::printValue( x ) )
                 .arg( QgsRasterBlock::printValue( y ) );
  QgsDebugMsg( "coor : " + coor );
  mProcess->write( coor.toAscii() ); // how to flush, necessary?
  mProcess->waitForReadyRead();
  QString str = mProcess->readLine().trimmed();
  QgsDebugMsg( "read from stdout : " + str );

  // TODO: use doubles instead of strings

  QStringList list = str.trimmed().split( ":" );
  if ( list.size() == 2 )
  {
    if ( list[1] == "error" ) return value;
    value = list[1].toDouble( ok );
  }
  return value;
}


