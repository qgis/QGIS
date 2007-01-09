
#ifndef GPSCORE_H
#define GPSCORE_H

#include <QObject>
#include <QTcpSocket>

#include "qgspoint.h"
#include "qgsdistancearea.h"

class QPainter;
class GpsPage;
class QgisInterface;
class PositionMarker;
class QgsRubberBand;

class GpsCore : public QObject
{
  Q_OBJECT;
  
  public:
    GpsCore(QgisInterface* qgis);
    ~GpsCore();
    
    void start();
    void stop();
    
    QTcpSocket* tcpSocket();
    
    void parsePacketO(QString packet);
    
  public slots:
    void readGpsData();
    void socketError(QAbstractSocket::SocketError);
    void socketConnected();
    void socketDisconnected();
    
    void toggledShowPosition(bool);

  private:
    
    //! pointer to socket for communication with GPSD
    QTcpSocket* mTcpSocket;
    
    //! input buffer for receiving packets from GPSD
    QByteArray mInputData;
    
    //! pointer to QGIS interface
    QgisInterface* mQgis;

    //! pointer to GPS page
    GpsPage* mGpsPage;
    
    //! position marker
    PositionMarker* mMarker;
    
    //! rubber band for position history
    QgsRubberBand* mTrack;
    
    QgsPoint mGpsPos;
    QgsPoint mGpsPosLast;

    QgsDistanceArea mDA;
};

#endif
