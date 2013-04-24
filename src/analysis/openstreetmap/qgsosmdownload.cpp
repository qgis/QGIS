#include "qgsosmdownload.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "qgsnetworkaccessmanager.h"
#include "qgsrectangle.h"


QString QgsOSMDownload::defaultServiceUrl()
{
  return "http://overpass-api.de/api/interpreter";
}


QString QgsOSMDownload::queryFromRect( const QgsRectangle& rect )
{
  return QString( "(node(%1,%2,%3,%4);<;);out;" ).arg( rect.yMinimum() ).arg( rect.xMinimum() )
         .arg( rect.yMaximum() ).arg( rect.xMaximum() );
}


QgsOSMDownload::QgsOSMDownload()
    : mServiceUrl( defaultServiceUrl() ), mReply( 0 )
{
}

QgsOSMDownload::~QgsOSMDownload()
{
  if ( mReply )
  {
    mReply->abort();
    mReply->deleteLater();
    mReply = 0;
  }
}


bool QgsOSMDownload::start()
{
  mError.clear();

  if ( mQuery.isEmpty() )
  {
    mError = tr( "No query has been specified." );
    return false;
  }

  if ( mReply )
  {
    mError = tr( "There is already a pending request for data." );
    return false;
  }

  if ( !mFile.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
  {
    mError = tr( "Cannot open output file: %1" ).arg( mFile.fileName() );
    return false;
  }

  QgsNetworkAccessManager* nwam = QgsNetworkAccessManager::instance();

  QUrl url( mServiceUrl );
  url.addQueryItem( "data", mQuery );

  QNetworkRequest request( url );
  request.setRawHeader( "User-Agent", "QGIS" );

  mReply = nwam->get( request );

  connect( mReply, SIGNAL( readyRead() ), this, SLOT( onReadyRead() ) );
  connect( mReply, SIGNAL( error( QNetworkReply::NetworkError ) ), this, SLOT( onError( QNetworkReply::NetworkError ) ) );
  connect( mReply, SIGNAL( finished() ), this, SLOT( onFinished() ) );
  connect( mReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SIGNAL( downloadProgress( qint64, qint64 ) ) );

  return true;
}


bool QgsOSMDownload::abort()
{
  if ( !mReply )
    return false;

  mReply->abort();
  return true;
}


void QgsOSMDownload::onReadyRead()
{
  Q_ASSERT( mReply );

  QByteArray data = mReply->read( 1024 * 1024 );
  mFile.write( data );
}


void QgsOSMDownload::onFinished()
{
  qDebug( "finished" );
  Q_ASSERT( mReply );

  mReply->deleteLater();
  mReply = 0;

  mFile.close();

  emit finished();
}


void QgsOSMDownload::onError( QNetworkReply::NetworkError err )
{
  qDebug( "error: %d", err );
  Q_ASSERT( mReply );

  mError = mReply->errorString();
}


bool QgsOSMDownload::isFinished() const
{
  if ( !mReply )
    return true;

  return mReply->isFinished();
}
