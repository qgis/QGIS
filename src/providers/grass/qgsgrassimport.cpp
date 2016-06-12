/***************************************************************************
    qgsgrassimport.cpp  -  Import to GRASS mapset
                             -------------------
    begin                : May, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QByteArray>
#include <QtConcurrentRun>

#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgsfeature.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgsproviderregistry.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasteriterator.h"

#include "qgsgrassimport.h"

extern "C"
{
#include <grass/version.h>
#include <grass/gis.h>
#include <grass/raster.h>
#include <grass/imagery.h>
}

//------------------------------ QgsGrassImport ------------------------------------
QgsGrassImportIcon *QgsGrassImportIcon::instance()
{
  static QgsGrassImportIcon* sInstance = new QgsGrassImportIcon();
  return sInstance;
}

QgsGrassImportIcon::QgsGrassImportIcon()
    : QgsAnimatedIcon( QgsApplication::iconPath( "/mIconImport.gif" ) )
{
}

//------------------------------ QgsGrassImportProcess ------------------------------------
QgsGrassImportProgress::QgsGrassImportProgress( QProcess *process, QObject *parent )
    : QObject( parent )
    , mProcess( process )
    , mProgressMin( 0 )
    , mProgressMax( 0 )
    , mProgressValue( 0 )
{
  connect( mProcess, SIGNAL( readyReadStandardError() ), SLOT( onReadyReadStandardError() ) );
}

void QgsGrassImportProgress::setProcess( QProcess *process )
{
  mProcess = process;
  connect( mProcess, SIGNAL( readyReadStandardError() ), SLOT( onReadyReadStandardError() ) );
}

void QgsGrassImportProgress::onReadyReadStandardError()
{
  if ( mProcess )
  {
    // TODO: parse better progress output
    QString output = QString::fromLocal8Bit( mProcess->readAllStandardError() );
    Q_FOREACH ( const QString& line, output.split( "\n" ) )
    {
      QgsDebugMsg( "line = '" + line + "'" );
      QString text, html;
      int value;
      QgsGrass::ModuleOutput type =  QgsGrass::parseModuleOutput( line, text, html, value );
      if ( type == QgsGrass::OutputPercent )
      {
        mProgressMin = 0;
        mProgressMax = 100;
        mProgressValue = value;
        emit progressChanged( html, mProgressHtml, mProgressMin, mProgressMax, mProgressValue );
      }
      else if ( type == QgsGrass::OutputProgress )
      {
        html = tr( "Progress: %1" ).arg( value );
        append( html );
      }
      else if ( type == QgsGrass::OutputMessage || type == QgsGrass::OutputWarning || type == QgsGrass::OutputError )
      {
        append( html );
      }
    }
  }
}

void QgsGrassImportProgress::append( const QString & html )
{
  QgsDebugMsg( "html = " + html );
  if ( !mProgressHtml.isEmpty() )
  {
    mProgressHtml += "<br>";
  }
  mProgressHtml += html;
  emit progressChanged( html, mProgressHtml, mProgressMin, mProgressMax, mProgressValue );
}

void QgsGrassImportProgress::setRange( int min, int max )
{
  mProgressMin = min;
  mProgressMax = max;
  mProgressValue = min;
  emit progressChanged( "", mProgressHtml, mProgressMin, mProgressMax, mProgressValue );
}

void QgsGrassImportProgress::setValue( int value )
{
  mProgressValue = value;
  emit progressChanged( "", mProgressHtml, mProgressMin, mProgressMax, mProgressValue );
}

//------------------------------ QgsGrassImport ------------------------------------
QgsGrassImport::QgsGrassImport( QgsGrassObject grassObject )
    : QObject()
    , mGrassObject( grassObject )
    , mCanceled( false )
    , mProcess( 0 )
    , mProgress( 0 )
    , mFutureWatcher( 0 )
{
  // QMovie used by QgsAnimatedIcon is using QTimer which cannot be start from another thread
  // (it works on Linux however) so we cannot start it connecting from QgsGrassImportItem and
  // connect it also here (QgsGrassImport is constructed on the main thread) to a slot doing nothing.
  QgsGrassImportIcon::instance()->connectFrameChanged( this, SLOT( frameChanged() ) );
}

QgsGrassImport::~QgsGrassImport()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    QgsDebugMsg( "mFutureWatcher not finished -> waitForFinished()" );
    mFutureWatcher->waitForFinished();
  }
  QgsGrassImportIcon::instance()->disconnectFrameChanged( this, SLOT( frameChanged() ) );
}

void QgsGrassImport::setError( QString error )
{
  QgsDebugMsg( "error: " + error );
  mError = error;
}

QString QgsGrassImport::error()
{
  return mError;
}

void QgsGrassImport::importInThread()
{
  mFutureWatcher = new QFutureWatcher<bool>( this );
  connect( mFutureWatcher, SIGNAL( finished() ), SLOT( onFinished() ) );
  mFutureWatcher->setFuture( QtConcurrent::run( run, this ) );
}

bool QgsGrassImport::run( QgsGrassImport *imp )
{
  imp->import();
  return true;
}

void QgsGrassImport::onFinished()
{
  emit finished( this );
}

QStringList QgsGrassImport::names() const
{
  QStringList list;
  list << mGrassObject.name();
  return list;
}

bool QgsGrassImport::isCanceled() const
{
  return mCanceled;
}

void QgsGrassImport::cancel()
{
  mCanceled = true;
}

//------------------------------ QgsGrassRasterImport ------------------------------------
QgsGrassRasterImport::QgsGrassRasterImport( QgsRasterPipe* pipe, const QgsGrassObject& grassObject,
    const QgsRectangle &extent, int xSize, int ySize )
    : QgsGrassImport( grassObject )
    , mPipe( pipe )
    , mExtent( extent )
    , mXSize( xSize )
    , mYSize( ySize )
{
}

QgsGrassRasterImport::~QgsGrassRasterImport()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    QgsDebugMsg( "mFutureWatcher not finished -> waitForFinished()" );
    mFutureWatcher->waitForFinished();
  }
  delete mPipe;
}

bool QgsGrassRasterImport::import()
{
  if ( !mPipe )
  {
    setError( "Pipe is null." );
    return false;
  }

  QgsRasterDataProvider * provider = mPipe->provider();
  if ( !provider )
  {
    setError( "Pipe has no provider." );
    return false;
  }

  if ( !provider->isValid() )
  {
    setError( "Provider is not valid." );
    return false;
  }


  struct Cell_head defaultWindow;
  if ( !QgsGrass::defaultRegion( mGrassObject.gisdbase(), mGrassObject.location(), &defaultWindow ) )
  {
    setError( "Cannot get default window" );
    return false;
  }

  int redBand = 0;
  int greenBand = 0;
  int blueBand = 0;
  for ( int band = 1; band <= provider->bandCount(); band++ )
  {
    QgsDebugMsg( QString( "band = %1" ).arg( band ) );
    int colorInterpretation = provider->colorInterpretation( band );
    if ( colorInterpretation ==  QgsRaster::RedBand )
    {
      redBand = band;
    }
    else if ( colorInterpretation ==  QgsRaster::GreenBand )
    {
      greenBand = band;
    }
    else if ( colorInterpretation ==  QgsRaster::BlueBand )
    {
      blueBand = band;
    }

    QGis::DataType qgis_out_type = QGis::UnknownDataType;
#ifdef QGISDEBUG
    RASTER_MAP_TYPE data_type = -1;
#endif
    switch ( provider->dataType( band ) )
    {
      case QGis::Byte:
      case QGis::UInt16:
      case QGis::Int16:
      case QGis::UInt32:
      case QGis::Int32:
        qgis_out_type = QGis::Int32;
        break;
      case QGis::Float32:
        qgis_out_type = QGis::Float32;
        break;
      case QGis::Float64:
        qgis_out_type = QGis::Float64;
        break;
      case QGis::ARGB32:
      case QGis::ARGB32_Premultiplied:
        qgis_out_type = QGis::Int32;  // split to multiple bands?
        break;
      case QGis::CInt16:
      case QGis::CInt32:
      case QGis::CFloat32:
      case QGis::CFloat64:
      case QGis::UnknownDataType:
        setError( tr( "Data type %1 not supported" ).arg( provider->dataType( band ) ) );
        return false;
    }

    QgsDebugMsg( QString( "data_type = %1" ).arg( data_type ) );

    QString module = QgsGrass::qgisGrassModulePath() + "/qgis.r.in";
    QStringList arguments;
    QString name = mGrassObject.name();
    if ( provider->bandCount() > 1 )
    {
      // raster.<band> to keep in sync with r.in.gdal
      name += QString( ".%1" ).arg( band );
    }
    arguments.append( "output=" + name );    // get list of all output names
    QTemporaryFile gisrcFile;
    try
    {
      mProcess = QgsGrass::startModule( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), module, arguments, gisrcFile );
    }
    catch ( QgsGrass::Exception &e )
    {
      setError( e.what() );
      return false;
    }
    if ( !mProgress )
    {
      mProgress = new QgsGrassImportProgress( mProcess, this );
    }
    else
    {
      mProgress->setProcess( mProcess );
    }
    mProgress->append( tr( "Writing band %1/%2" ).arg( band ).arg( provider->bandCount() ) );

    QDataStream outStream( mProcess );

    outStream << ( qint32 ) defaultWindow.proj;
    outStream << ( qint32 ) defaultWindow.zone;
    outStream << mExtent << ( qint32 )mXSize << ( qint32 )mYSize;
    outStream << ( qint32 )qgis_out_type;

    // calculate reasonable block size (5MB)
    int maximumTileHeight = 5000000 / mXSize;
    maximumTileHeight = std::max( 1, maximumTileHeight );
    // smaller if reprojecting so that it can be canceled quickly
    if ( mPipe->projector() )
    {
      maximumTileHeight = std::max( 1, 100000 / mXSize );
    }

    QgsRasterIterator iter( mPipe->last() );
    iter.setMaximumTileWidth( mXSize );
    iter.setMaximumTileHeight( maximumTileHeight );

    iter.startRasterRead( band, mXSize, mYSize, mExtent );

    int iterLeft = 0;
    int iterTop = 0;
    int iterCols = 0;
    int iterRows = 0;
    QgsRasterBlock* block = 0;
    mProcess->setReadChannel( QProcess::StandardOutput );
    mProgress->setRange( 0, mYSize - 1 );
    while ( iter.readNextRasterPart( band, iterCols, iterRows, &block, iterLeft, iterTop ) )
    {
      for ( int row = 0; row < iterRows; row++ )
      {
        mProgress->setValue( iterTop + row );

        if ( !block->convert( qgis_out_type ) )
        {
          setError( tr( "Cannot convert block (%1) to data type %2" ).arg( block->toString() ).arg( qgis_out_type ) );
          delete block;
          return false;
        }
        // prepare null values
        double noDataValue;
        if ( block->hasNoDataValue() )
        {
          noDataValue = block->noDataValue();
        }
        else
        {
          switch ( qgis_out_type )
          {
            case QGis::Int32:
              noDataValue = -2147483648.0;
              break;
            case QGis::Float32:
              noDataValue = std::numeric_limits<float>::max() * -1.0;
              break;
            case QGis::Float64:
              noDataValue = std::numeric_limits<double>::max() * -1.0;
              break;
            default: // should not happen
              noDataValue = std::numeric_limits<double>::max() * -1.0;
          }
          for ( qgssize i = 0; i < ( qgssize )block->width()*block->height(); i++ )
          {
            if ( block->isNoData( i ) )
            {
              block->setValue( i, noDataValue );
            }
          }
        }

        char * data = block->bits( row, 0 );
        int size = iterCols * block->dataTypeSize();
        QByteArray byteArray = QByteArray::fromRawData( data, size ); // does not copy data and does not take ownership
        if ( isCanceled() )
        {
          outStream << true; // cancel module
          break;
        }
        outStream << false; // not canceled
        outStream << noDataValue;

        outStream << byteArray;

        // Without waitForBytesWritten() it does not finish ok on Windows (process timeout)
        mProcess->waitForBytesWritten( -1 );

#ifndef Q_OS_WIN
        // wait until the row is written to allow quick cancel (don't send data to buffer)
        mProcess->waitForReadyRead();
        bool result;
        outStream >> result;
#endif
      }
      delete block;
      if ( isCanceled() )
      {
        outStream << true; // cancel module
        break;
      }
    }

    // TODO: send something back from module and read it here to close map correctly in module

    mProcess->closeWriteChannel();
    // TODO: best timeout?
    mProcess->waitForFinished( 30000 );

    QString stdoutString = mProcess->readAllStandardOutput().data();
    QString stderrString = mProcess->readAllStandardError().data();

    QString processResult = QString( "exitStatus=%1, exitCode=%2, error=%3, errorString=%4 stdout=%5, stderr=%6" )
                            .arg( mProcess->exitStatus() ).arg( mProcess->exitCode() )
                            .arg( mProcess->error() ).arg( mProcess->errorString(),
                                                           stdoutString.replace( "\n", ", " ), stderrString.replace( "\n", ", " ) );
    QgsDebugMsg( "processResult: " + processResult );

    if ( mProcess->exitStatus() != QProcess::NormalExit )
    {
      setError( mProcess->errorString() );
      mProcess = 0;
      return false;
    }

    if ( mProcess->exitCode() != 0 )
    {
      setError( stderrString );
      delete mProcess;
      mProcess = 0;
      return false;
    }

    delete mProcess;
    mProcess = 0;
  }
  QgsDebugMsg( QString( "redBand = %1 greenBand = %2 blueBand = %3" ).arg( redBand ).arg( greenBand ).arg( blueBand ) );
  if ( redBand > 0 && greenBand > 0 && blueBand > 0 )
  {
    // TODO: check if the group exists
    // I_find_group()
    QString name = mGrassObject.name();

    G_TRY
    {
      QgsGrass::setMapset( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset() );
      struct Ref ref;
      I_get_group_ref( name.toUtf8().data(), &ref );
      QString redName = name + QString( ".%1" ).arg( redBand );
      QString greenName = name + QString( ".%1" ).arg( greenBand );
      QString blueName = name + QString( ".%1" ).arg( blueBand );
      I_add_file_to_group_ref( redName.toUtf8().data(), mGrassObject.mapset().toUtf8().data(), &ref );
      I_add_file_to_group_ref( greenName.toUtf8().data(), mGrassObject.mapset().toUtf8().data(), &ref );
      I_add_file_to_group_ref( blueName.toUtf8().data(), mGrassObject.mapset().toUtf8().data(), &ref );
      I_put_group_ref( name.toUtf8().data(), &ref );
    }
    G_CATCH( QgsGrass::Exception &e )
    {
      QgsDebugMsg( QString( "Cannot create group: %1" ).arg( e.what() ) );
    }
  }
  return true;
}

QString QgsGrassRasterImport::srcDescription() const
{
  if ( !mPipe || !mPipe->provider() )
  {
    return "";
  }
  return mPipe->provider()->dataSourceUri();
}

QStringList QgsGrassRasterImport::extensions( QgsRasterDataProvider* provider )
{
  QStringList list;
  if ( provider && provider->bandCount() > 1 )
  {
    int bands = provider->bandCount();
    list.reserve( bands );
    for ( int band = 1; band <= bands; ++band )
    {
      list << QString( ".%1" ).arg( band );
    }
  }
  return list;
}

QStringList QgsGrassRasterImport::names() const
{
  QStringList list;
  if ( mPipe && mPipe->provider() )
  {
    Q_FOREACH ( const QString& ext, extensions( mPipe->provider() ) )
    {
      list << mGrassObject.name() + ext;
    }
  }
  if ( list.isEmpty() )
  {
    list << mGrassObject.name();
  }
  return list;
}

//------------------------------ QgsGrassVectorImport ------------------------------------
QgsGrassVectorImport::QgsGrassVectorImport( QgsVectorDataProvider* provider, const QgsGrassObject& grassObject )
    : QgsGrassImport( grassObject )
    , mProvider( provider )
{
}

QgsGrassVectorImport::~QgsGrassVectorImport()
{
  if ( mFutureWatcher && !mFutureWatcher->isFinished() )
  {
    QgsDebugMsg( "mFutureWatcher not finished -> waitForFinished()" );
    mFutureWatcher->waitForFinished();
  }
  delete mProvider;
}

bool QgsGrassVectorImport::import()
{

  if ( !mProvider )
  {
    setError( "Provider is null." );
    return false;
  }

  if ( !mProvider->isValid() )
  {
    setError( "Provider is not valid." );
    return false;
  }

  QgsCoordinateReferenceSystem providerCrs = mProvider->crs();
  QgsCoordinateReferenceSystem mapsetCrs = QgsGrass::crsDirect( mGrassObject.gisdbase(), mGrassObject.location() );
  QgsDebugMsg( "providerCrs = " + providerCrs.toWkt() );
  QgsDebugMsg( "mapsetCrs = " + mapsetCrs.toWkt() );
  QgsCoordinateTransform coordinateTransform;
  bool doTransform = false;
  if ( providerCrs.isValid() && mapsetCrs.isValid() && providerCrs != mapsetCrs )
  {
    coordinateTransform.setSourceCrs( providerCrs );
    coordinateTransform.setDestCRS( mapsetCrs );
    doTransform = true;
  }

  QString module = QgsGrass::qgisGrassModulePath() + "/qgis.v.in";
  QStringList arguments;
  QString name = mGrassObject.name();
  arguments.append( "output=" + name );
  QTemporaryFile gisrcFile;
  try
  {
    mProcess = QgsGrass::startModule( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), module, arguments, gisrcFile );
  }
  catch ( QgsGrass::Exception &e )
  {
    setError( e.what() );
    return false;
  }
  mProgress = new QgsGrassImportProgress( mProcess, this );

  QDataStream outStream( mProcess );
  mProcess->setReadChannel( QProcess::StandardOutput );

  QGis::WkbType wkbType = mProvider->geometryType();
  bool isPolygon = QGis::singleType( QGis::flatType( wkbType ) ) == QGis::WKBPolygon;
  outStream << ( qint32 )wkbType;

  outStream << mProvider->fields();

  // FID may be 0, but cat should be >= 1 -> check if fid 0 exists
  qint32 fidToCatPlus = 0;
  QgsFeature feature;
  QgsFeatureIterator iterator = mProvider->getFeatures( QgsFeatureRequest().setFilterFid( 0 ) );
  if ( iterator.nextFeature( feature ) )
  {
    fidToCatPlus = 1;
  }
  iterator.close();
  outStream << fidToCatPlus;

  qint32 featureCount = mProvider->featureCount();
  outStream << featureCount;

  mProgress->append( tr( "Writing features" ) );
  for ( int i = 0; i < ( isPolygon ? 2 : 1 ); i++ ) // two cycles with polygons
  {
    iterator = mProvider->getFeatures();
    // rewind does not work
#if 0
    if ( i > 0 ) // second run for polygons
    {
      iterator.rewind();
    }
#endif
    QgsDebugMsg( "send features" );
    // Better to get real progress from module (without buffer)
#if 0
    mProgress->setRange( 1, featureCount );
#endif
    int count = 0;
    while ( iterator.nextFeature( feature ) )
    {
      if ( !feature.isValid() )
      {
        continue;
      }
      if ( doTransform && feature.geometry() )
      {
        feature.geometry()->transform( coordinateTransform );
      }
      if ( isCanceled() )
      {
        outStream << true; // cancel module
        break;
      }
      outStream << false; // not canceled
      outStream << feature;

      // Without waitForBytesWritten() it does not finish ok on Windows (data lost)
      mProcess->waitForBytesWritten( -1 );

#ifndef Q_OS_WIN
      // wait until the feature is written to allow quick cancel (don't send data to buffer)

      // Feedback disabled because it was sometimes hanging on Linux, for example, when importing polygons
      // the features were written ok in the first run, but after cleaning of polygons, which takes some time
      // it was hanging here for few seconds after each feature, but data did not arrive to the modulee anyway,
      // QFSFileEnginePrivate::nativeRead() was hanging on fgetc()

      // TODO: inspect what is happening in QProcess, if there is some buffer and how to disable it
#if 0
      mProcess->waitForReadyRead();
      bool result;
      outStream >> result;
#endif
#endif
      count++;
#if 0
      if ( count % 100 == 0 )
      {
        mProgress->setValue( count );
      }
#endif
      // get some feedback for large datasets
      if ( count % 10000 == 0 )
      {
        QgsDebugMsg( QString( "%1 features written" ).arg( count ) );
      }
    }

    feature = QgsFeature(); // indicate end by invalid feature
    outStream << false; // not canceled
    outStream << feature;

    mProcess->waitForBytesWritten( -1 );
    QgsDebugMsg( "features sent" );
#ifndef Q_OS_WIN
#if 0
    mProcess->waitForReadyRead();
    bool result;
    outStream >> result;
    QgsDebugMsg( "got feedback" );
#endif
#endif
    iterator.close();
  }

  // Close write channel before waiting for response to avoid stdin buffer problem on Windows
  mProcess->closeWriteChannel();

  QgsDebugMsg( "waitForReadyRead" );
  bool result;
  mProcess->waitForReadyRead();
  outStream >> result;
  QgsDebugMsg( QString( "result = %1" ).arg( result ) );

  QgsDebugMsg( "waitForFinished" );
  mProcess->waitForFinished( 30000 );

  QString stdoutString = mProcess->readAllStandardOutput().data();
  QString stderrString = mProcess->readAllStandardError().data();

  QString processResult = QString( "exitStatus=%1, exitCode=%2, error=%3, errorString=%4 stdout=%5, stderr=%6" )
                          .arg( mProcess->exitStatus() ).arg( mProcess->exitCode() )
                          .arg( mProcess->error() ).arg( mProcess->errorString(),
                                                         stdoutString.replace( "\n", ", " ), stderrString.replace( "\n", ", " ) );
  QgsDebugMsg( "processResult: " + processResult );

  if ( mProcess->exitStatus() != QProcess::NormalExit )
  {
    setError( mProcess->errorString() );
    delete mProcess;
    mProcess = 0;
    return false;
  }

  if ( mProcess->exitCode() != 0 )
  {
    setError( stderrString );
    delete mProcess;
    mProcess = 0;
    return false;
  }

  delete mProcess;
  mProcess = 0;
  return true;
}

QString QgsGrassVectorImport::srcDescription() const
{
  if ( !mProvider )
  {
    return "";
  }
  return mProvider->dataSourceUri();
}

//------------------------------ QgsGrassCopy ------------------------------------
QgsGrassCopy::QgsGrassCopy( const QgsGrassObject& srcObject, const QgsGrassObject& destObject )
    : QgsGrassImport( destObject )
    , mSrcObject( srcObject )
{
}

QgsGrassCopy::~QgsGrassCopy()
{
}

bool QgsGrassCopy::import()
{

  try
  {
    QgsGrass::copyObject( mSrcObject, mGrassObject );
  }
  catch ( QgsGrass::Exception &e )
  {
    setError( e.what() );
    return false;
  }

  return true;
}


QString QgsGrassCopy::srcDescription() const
{
  return mSrcObject.toString();
}

//------------------------------ QgsGrassExternal ------------------------------------
QgsGrassExternal::QgsGrassExternal( const QString& gdalSource, const QgsGrassObject& destObject )
    : QgsGrassImport( destObject )
    , mSource( gdalSource )
{
}

QgsGrassExternal::~QgsGrassExternal()
{
}

bool QgsGrassExternal::import()
{

  try
  {
    QString cmd = QgsGrass::gisbase() + "/bin/r.external";
    QStringList arguments;

    if ( QFileInfo( mSource ).exists() )
    {
      arguments << "input=" + mSource;
    }
    else
    {
      arguments << "source=" + mSource;
    }
    arguments << "output=" + mGrassObject.name();

    // TODO: best timeout
    int timeout = -1;
    // throws QgsGrass::Exception
    QgsGrass::runModule( mGrassObject.gisdbase(), mGrassObject.location(), mGrassObject.mapset(), cmd, arguments, timeout, false );
  }
  catch ( QgsGrass::Exception &e )
  {
    setError( e.what() );
    return false;
  }

  return true;
}

QString QgsGrassExternal::srcDescription() const
{
  return mSource;
}
