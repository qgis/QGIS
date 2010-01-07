#include "qgsgpstrackerthread.h"
#include "qgsgpsconnection.h"

QgsGPSTrackerThread::QgsGPSTrackerThread( QgsGPSConnection* conn ): mConnection( conn )
{
}

QgsGPSTrackerThread::QgsGPSTrackerThread(): mConnection( 0 )
{
  cleanupConnection();
}

QgsGPSTrackerThread::~QgsGPSTrackerThread()
{
  delete mConnection;
}

void QgsGPSTrackerThread::run()
{
  if ( !mConnection )
  {
    return;
  }

  if ( !mConnection->connect() )
  {
    return;
  }

  //QTimer needs to be started in the same thread, so we create a new instance here
  QTimer* t = new QTimer();
  t->setInterval( mConnection->timer()->interval() );
  mConnection->setTimer( t );

  mConnection->startPolling();
  exec();
  mConnection->stopPolling();
  mConnection->close();
}

void QgsGPSTrackerThread::cleanupConnection()
{
  delete mConnection;
  mConnection = 0;
}

void QgsGPSTrackerThread::setConnection( QgsGPSConnection* c )
{
  cleanupConnection();
  mConnection = c;
}


