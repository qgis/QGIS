
#include "qgisinterface.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsdistancearea.h"

#include "gpscore.h"
#include "gpspage.h"
#include "positionmarker.h"

#include <QCheckBox>
#include <QDateTime>
#include <QMessageBox>
#include <QToolBox>

#include <cmath>
#include <iostream>

// MSVC compiler doesn't have defined M_PI in math.h
#ifndef M_PI
#define M_PI          3.14159265358979323846
#endif

GpsCore::GpsCore(QgisInterface* qgis)
  : mQgis(qgis)
{
  // init gps widget
  mGpsPage = new GpsPage;

  connect(mGpsPage->chkShowPosition, SIGNAL(toggled(bool)), this, SLOT(toggledShowPosition(bool)));
  
  mTcpSocket = new QTcpSocket(this);

  // reading data from socket
  connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(readGpsData()));

  // socket error handling
  connect(mTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
          this, SLOT(socketError(QAbstractSocket::SocketError)));
  
  connect(mTcpSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
  connect(mTcpSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
  
  // create position marker
  mMarker = new PositionMarker(mQgis->getMapCanvas());
  
  mTrack = new QgsRubberBand(mQgis->getMapCanvas(), false);
  mTrack->setWidth(2);
}

GpsCore::~GpsCore()
{
  delete mGpsPage;
  delete mTcpSocket;
  delete mMarker;
  delete mTrack;
}

QTcpSocket* GpsCore::tcpSocket()
{
  return mTcpSocket;
}

void GpsCore::start()
{
  mInputData.resize(0);

  mTcpSocket->connectToHost("localhost", 2947);
  
  // gps page is added to toolbox when we're connected
}

void GpsCore::stop()
{
  QToolBox* tb = mQgis->getToolBox();
  int index = tb->indexOf(mGpsPage);
  if (index != -1) // does it exist?
  {
    tb->removeItem(index);
  }

  mTcpSocket->disconnectFromHost();
}



void GpsCore::readGpsData()
{
  //std::cout << "readGpsData! " << mTcpSocket->bytesAvailable() << " bytes" << std::endl;
  
  // read data
  QByteArray data = mTcpSocket->readAll();
  //std::cout << (const char*)data << std::endl;
  
  // to protect buffer from getting insanely big when getting bad data
  // erase the buffer when it's bigger than 1 kb
  // (as there are no such long packets)
  if (mInputData.count() > 1024)
  {
    mInputData.resize(0);
  }
  
  // append newly received data
  mInputData += data;
  
  // search for the end of the packet
  QString pck;
  int end;
  while ((end = mInputData.indexOf('\n')) != -1)
  {
    // get the packet
    QByteArray packet = mInputData.left(end);
    
    // erase the packet from input buffer
    mInputData = mInputData.mid(end+1);
  
    // parse the packet
    if (!packet.startsWith("GPSD,"))
    {
      std::cout << "bad data coming!" << std::endl;
      continue;
    }
  
    // GPSD,?=...........\n
    char packetType = packet.at(5);
    packet = packet.mid(7);
    pck = packet; // convert to QString
    
    //std::cout << "packet::" << (const char*) packet << std::endl;
    switch (packetType)
    {
      case 'O':
        parsePacketO(pck);
        break;
      default:
        std::cout << "got packet " << packetType << std::endl;
    }
  }

// an example of GPSD output:
//GPSD,O=GSA 1114599742.00 0.005 48.898057 18.073253  221.50  5.40  2.00 182.2000    4.373  0.000  62.3221  5.19        ?
}

void GpsCore::socketError(QAbstractSocket::SocketError err)
{
  std::cout << "socketError! " << err << std::endl;
  
  QString message;
  if (err == QAbstractSocket::ConnectionRefusedError)
  {
    message = "Connection was refused.\nIs gpsd running?";
  }
  else if (err == QAbstractSocket::RemoteHostClosedError)
  {
    message = "Connection with gpsd has been closed!";
    stop(); // clean up
  }
  else
    message = QString("Error on socket with gpsd: %1").arg(err);
  QMessageBox::critical(NULL, "GPS error", message);
}

void GpsCore::socketConnected()
{
  // create gps page and set it as current
  QToolBox* tb = mQgis->getToolBox();
  tb->addItem(mGpsPage, "GPS info");
  tb->setCurrentWidget(mGpsPage);
  
  std::cout << "GPS STARTED =======================================" << std::endl;
  mTcpSocket->write("w+\n", 2); // switch to watcher mode
}

void GpsCore::socketDisconnected()
{
  std::cout << "disconnected!" << std::endl;
}

void GpsCore::parsePacketO(QString packet)
{
  //std::cout << "got position data" << std::endl;
  //std::cout << packet.toLocal8Bit().data() << std::endl;
  
  if (packet[0] == '?') // no fix?
  {
    std::cout << "no fix!" << std::endl;
    mGpsPage->lblFix->setText("No");
    mMarker->setIcon(false);
    // TODO: what to do with other fields?
    return;
  }

  QStringList strings = packet.split(' ');
  
  // XXX more information can be acquired from M or S request
  mGpsPage->lblFix->setText("Yes");
  mMarker->setIcon(true);
  
  // time
  QString timeStr = strings[1];
  if (timeStr != "?")
  {
    QDateTime time;
    time.setTime_t((uint) timeStr.toDouble()); // time is formatted as %f
    mGpsPage->txtTime->setText(time.toString("hh:mm:ss"));
  }
  
  // strings[2]: time error - not interesting
  
  // display lat & lon
  mGpsPage->txtLat->setText(strings[3]);
  mGpsPage->txtLon->setText(strings[4]);
  
  // TODO: convert from wgs84 to project coordinates
  double lat = strings[3].toDouble();
  double lon = strings[4].toDouble();
  
  // only if the position has changed
  if (lat != mGpsPos.y() && lon != mGpsPos.x())
  {
  
    mGpsPosLast = mGpsPos;
    mGpsPos = QgsPoint(lon,lat);
    
    double angle = mDA.getBearing(mGpsPosLast, mGpsPos);
    angle *= (180/M_PI);

    // set marker
    mMarker->setAngle(angle);
    mMarker->setPosition(mGpsPos);
    
    // hide it if told so (setPosition always shows it)
    bool visible = mGpsPage->chkShowPosition->isChecked();
    if (!visible)
      mMarker->hide();
  
    // add to history
    mTrack->addPoint(QgsPoint(lon,lat));
    // TODO: erase after some time?
  
    // hide it if told so (setPosition always shows it)
    visible = mGpsPage->chkShowHistory->isChecked();
    if (!visible)
      mTrack->hide();
  }
  
  // TODO: more stuff...?
}

void GpsCore::toggledShowPosition(bool toggled)
{
  std::cout << "TOGGLED: " << toggled << std::endl;
}
