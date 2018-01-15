/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsgpsdetector.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGPSDetector : QObject
{
%Docstring
Class to detect the GPS port
%End

%TypeHeaderCode
#include "qgsgpsdetector.h"
%End
  public:
    QgsGPSDetector( const QString &portName );
    ~QgsGPSDetector();

    static QList< QPair<QString, QString> > availablePorts();

  public slots:
    void advance();
    void detected( const QgsGPSInformation & );
    void connDestroyed( QObject * );

  signals:
    void detected( QgsGPSConnection * );
    void detectionFailed();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsgpsdetector.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
