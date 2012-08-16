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


#include "qgslogger.h"
#include "qgsgrass.h"
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

static QString PROVIDER_KEY = "grassraster";
static QString PROVIDER_DESCRIPTION = "GRASS raster provider";

QgsGrassRasterProvider::QgsGrassRasterProvider( QString const & uri )
    : QgsRasterDataProvider( uri ), mValid( true )
{
  QgsDebugMsg( "QgsGrassRasterProvider: constructing with uri '" + uri + "'." );

  // Parse URI, it is the same like using GDAL, i.e. path to raster cellhd, i.e.
  // /path/to/gisdbase/location/mapset/cellhd/map
  QFileInfo fileInfo( uri );
  mValid = fileInfo.exists(); // then we keep it valid forever
  mMapName = fileInfo.fileName();
  QDir dir = fileInfo.dir();
  QString element = dir.dirName();
  if ( element != "cellhd" )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QObject::tr( "Groups not yet supported" ) + " (GRASS " + uri + ")" );

    mValid = false;
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
  mValidNoDataValue = true;

  mCrs = QgsGrass::crs( mGisdbase, mLocation );
  QgsDebugMsg( "mCrs: " + mCrs.toWkt() );

  // the block size can change of course when the raster is overridden
  // ibut it is only called once when statistics are calculated
  QgsGrass::size( mGisdbase, mLocation, mMapset, mMapName, &mCols, &mRows );

  mInfo = QgsGrass::info( mGisdbase, mLocation, mMapset, mMapName, QgsGrass::Raster );

  mGrassDataType = mInfo["TYPE"].toInt();
  QgsDebugMsg( "mGrassDataType = " + QString::number( mGrassDataType ) );

  // TODO: refresh mRows and mCols if raster was rewritten
  // We have to decide some reasonable block size, not to big to occupate too much
  // memory, not too small to result in too many calls to readBlock -> qgis.d.rast
  // for statistics
  int cache_size = 10000000; // ~ 10 MB
  mYBlockSize = cache_size / ( dataTypeSize( dataType( 1 ) ) / 8 ) / mCols;
  if ( mYBlockSize  > mRows )
  {
    mYBlockSize = mRows;
  }
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
                     .arg( viewExtent.xMinimum() ).arg( viewExtent.yMinimum() )
                     .arg( viewExtent.xMaximum() ).arg( viewExtent.yMaximum() )
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
                     .arg( ext.xMinimum() ).arg( yMinimum )
                     .arg( ext.xMaximum() ).arg( yMaximum )
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
  int size = mCols * mYBlockSize * dataTypeSize( bandNo ) / 8;
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
                     .arg( viewExtent.xMinimum() ).arg( viewExtent.yMinimum() )
                     .arg( viewExtent.xMaximum() ).arg( viewExtent.yMaximum() )
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
  int size = pixelWidth * pixelHeight * dataTypeSize( bandNo ) / 8;
  if ( size != data.size() )
  {
    QMessageBox::warning( 0, QObject::tr( "Warning" ),
                          QString( "%1 bytes expected but %2 byte were read from qgis.d.rast" ).arg( size ).arg( data.size() ) );
    size = size < data.size() ? size : data.size();
  }
  memcpy( block, data.data(), size );
}

double  QgsGrassRasterProvider::noDataValue() const
{
  double nul;
  // TODO: avoid showing these strange numbers in GUI
  // TODO: don't save no data values in project file, add a flag if value was defined by user
  if ( mGrassDataType == CELL_TYPE )
  {
    //limit: -2147483647;
    nul = -2000000000;
  }
  else if ( mGrassDataType == DCELL_TYPE )
  {
    // Don't use numeric limits, raster layer is using
    //    qAbs( myValue - mNoDataValue ) <= TINY_VALUE
    // if the mNoDataValue would be a limit, the subtraction could overflow.
    // No data value is shown in GUI, use some nice number.
    // Choose values with small representation error.
    // limit: 1.7976931348623157e+308
    nul = -1e+300;
  }
  else
  {
    if ( mGrassDataType != FCELL_TYPE )
    {
      QgsDebugMsg( "unexpected data type" );
    }

    // limit: 3.40282347e+38
    nul = -1e+30;
  }
  QgsDebugMsg( QString( "noDataValue = %1" ).arg( nul ) );
  return nul;
}

double  QgsGrassRasterProvider::minimumValue( int bandNo ) const
{
  Q_UNUSED( bandNo );
  return mInfo["MIN_VALUE"].toDouble();
}
double  QgsGrassRasterProvider::maximumValue( int bandNo ) const
{
  Q_UNUSED( bandNo );
  return mInfo["MAX_VALUE"].toDouble();
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
  // The extend can change of course so we get always fresh, to avoid running always the module
  // we should save mExtent and mLastModified and check if the map was modified

  mExtent = QgsGrass::extent( mGisdbase, mLocation, mMapset, mMapName, QgsGrass::Raster );
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

bool QgsGrassRasterProvider::identify( const QgsPoint& thePoint, QMap<QString, QString>& theResults )
{
  QgsDebugMsg( "Entered" );
  //theResults["Error"] = tr( "Out of extent" );
  //theResults = QgsGrass::query( mGisdbase, mLocation, mMapset, mMapName, QgsGrass::Raster, thePoint.x(), thePoint.y() );
  QString value = mRasterValue.value( thePoint.x(), thePoint.y() );
  theResults.clear();
  // attention, value tool does his own tricks with grass identify() so it stops to refresh values outside extent or null values e.g.
  if ( value == "out" )
  {
    value = tr( "Out of extent" );
  }
  if ( value == "null" )
  {
    value = tr( "null (no data)" );
  }
  theResults["value"] = value;
  QgsDebugMsg( "value = " + value );
  return true;
}

int QgsGrassRasterProvider::capabilities() const
{
  int capability = QgsRasterDataProvider::Identify
                   | QgsRasterDataProvider::ExactResolution
                   | QgsRasterDataProvider::ExactMinimumMaximum
                   | QgsRasterDataProvider::Size;
  return capability;
}

QgsRasterInterface::DataType QgsGrassRasterProvider::dataType( int bandNo ) const
{
  return srcDataType( bandNo );
}

QgsRasterInterface::DataType QgsGrassRasterProvider::srcDataType( int bandNo ) const
{
  Q_UNUSED( bandNo );
  switch ( mGrassDataType )
  {
    case CELL_TYPE:
      return QgsRasterDataProvider::Int32;
      break;
    case FCELL_TYPE:
      return QgsRasterDataProvider::Float32;
      break;
    case DCELL_TYPE:
      return QgsRasterDataProvider::Float64;
      break;
  }
  return QgsRasterDataProvider::UnknownDataType;
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
    return QgsRasterDataProvider::PaletteIndex;
  }
  return QgsRasterDataProvider::GrayIndex;
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

QString QgsGrassRasterProvider::identifyAsText( const QgsPoint& point )
{
  Q_UNUSED( point );
  return  QString( "Not implemented" );
}

QString QgsGrassRasterProvider::identifyAsHtml( const QgsPoint& point )
{
  Q_UNUSED( point );
  return  QString( "Not implemented" );
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

QString QgsGrassRasterValue::value( double x, double y )
{
  QString value = "error";
  if ( !mProcess )
    return value; // throw some exception?
  QString coor = QString( "%1 %2\n" ).arg( x ).arg( y );
  QgsDebugMsg( "coor : " + coor );
  mProcess->write( coor.toAscii() ); // how to flush, necessary?
  mProcess->waitForReadyRead();
  QString str = mProcess->readLine().trimmed();
  QgsDebugMsg( "read from stdout : " + str );

  QStringList list = str.trimmed().split( ":" );
  if ( list.size() == 2 )
  {
    value = list[1];
  }
  return value;
}


