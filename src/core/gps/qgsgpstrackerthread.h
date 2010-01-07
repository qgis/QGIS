#ifndef QGSGPSTRACKERTHREAD_H
#define QGSGPSTRACKERTHREAD_H

#include <QThread>

class QgsGPSConnection;

/**Queries GPS informations in a thread*/
class CORE_EXPORT QgsGPSTrackerThread: public QThread
{
  public:
    QgsGPSTrackerThread( QgsGPSConnection* conn );
    ~QgsGPSTrackerThread();

    void setConnection( QgsGPSConnection* c );
    const QgsGPSConnection* connection() { return mConnection; }

  protected:
    void run();

  private:
    QgsGPSTrackerThread();
    QgsGPSConnection* mConnection;

    void cleanupConnection();
};

#endif // QGSGPSTRACKERTHREAD_H
