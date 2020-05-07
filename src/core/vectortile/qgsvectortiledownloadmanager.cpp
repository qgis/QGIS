
#include "qgsnetworkaccessmanager.h"

class TileReply : public QObject
{
    QOBJECT
  public:
    TileReply( QString url ): mUrl( url ) {}
    ~TileReply()
    {
      // TODO: notify manager
      //if ( active )
      //  DownloadManager::cancelRequest( this );
    }
    QString url() const { return mUrl; }

    //void setParentReply( QNetworkReply *networkReply ) { mNetworkReply = networkReply; }
    //QNetworkReply *parentReply() const { return mNetworkReply; }

    void setData( const QByteArray &data ) { mData = data; }
    QByteArray data() const { return mData; }

  signals:
    void finished();

  private:
    QString mUrl;
    //QNetworkReply *mNetworkReply = nullptr;
    QByteArray mData;
};

class DownloadManager;

class Worker : public QObject
{
    QOBJECT

  public slots:

    void request( QString url )
    {
      // this is only called if such request does not exist already

      QNetworkRequest request( url );
      request.setAttribute( ( QNetworkRequest::Attribute )( QNetworkRequest::User + 1 ), url );
      QNetworkReply *networkReply = QgsNetworkAccessManager::instance()->get( request );
      connect( networkReply, &QNetworkReply::finished, this, &Worker::tileReplyFinished );
    }

    void cancelRequest( QString url )
    {
      // this is only called if such request has been requested here

      QNetworkReply *r = mNetworkReplies.take( url );
      r->abort();
    }

  private slots:
    void tileReplyFinished()
    {
      QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );

      QString url = reply->request().attribute( ( QNetworkRequest::Attribute )( QNetworkRequest::User + 1 ) ).toString();
      QByteArray data = reply->readAll();

      mNetworkReplies.remove( url );
      reply->deleteLater();

      // there may be multiple replies waiting for the same network request
      QMutexLocker locker( &DownloadManager::mutex );
      const QList<TileReply *> tileReplies = DownloadManager::replies[url];
      for ( TileReply *rr : tileReplies )
      {
        rr->setData( data );
        QMetaObject::invokeMethod( rr, "finished" );
      }
    }

  private:
    QMap<QString, QNetworkReply *> mNetworkReplies;

};

//
// TODO:
// - DownloadManager - singleton?
// - auto-kill thread after some time?
// - auto-start thread when needed
// - in-memory caching
//

/*
 * singleton that lives in the main thread.
 */
class DownloadManager
{
  public:

    static void startThread()
    {
      worker = new Worker;
      worker->moveToThread( &workerThread );
      QObject::connect( &workerThread, &QThread::finished, worker, &QObject::deleteLater );
      workerThread.start();
    }

    static void stopThread()
    {
      workerThread.quit();
      workerThread.wait();
    }

    //! thread-safe
    static TileReply *requestTile( const QString &url )
    {
      QMutexLocker locker( &mutex );

      TileReply *reply = new TileReply( url ); // lives in the same thread as the caller

      if ( replies.contains( url ) )
      {
        // no extra work to do - previous reply is not finished yet
        replies[url].append( reply );
      }
      else
      {
        replies[url] = QList<TileReply *>() << reply;
        // asynchronously request network request
        QMetaObject::invokeMethod( worker, "request", Qt::QueuedConnection, Q_ARG( QString, url ) );
      }

      return reply;
    }

    //! thread-safe
    static void cancelTileRequest( TileReply *reply )
    {
      QMutexLocker locker( &mutex );

      QString url = reply->url();

      replies[url].removeOne( reply );

      if ( replies[url].isEmpty() )
      {
        replies.remove( url );
        // also make sure the underlying request is cancelled
        QMetaObject::invokeMethod( worker, "cancelRequest", Qt::QueuedConnection, Q_ARG( QString, url ) );
      }
    }

  private:
    static QThread workerThread;  // will run its event loop
    static Worker *worker;
    static QMutex mutex;  // protecting data structures with replies
    static QMap<QString, QList<TileReply *> > replies;
};
