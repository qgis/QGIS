/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsgpsdetector.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsGpsDetector : QObject
{
%Docstring
Class to detect the GPS port
%End

%TypeHeaderCode
#include "qgsgpsdetector.h"
%End
  public:
    QgsGpsDetector( const QString &portName );
    ~QgsGpsDetector();

    static QList< QPair<QString, QString> > availablePorts();

  public slots:
    void advance();
    void detected( const QgsGpsInformation & );
    void connDestroyed( QObject * );

  signals:
    void detected( QgsGpsConnection * );
    void detectionFailed();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsgpsdetector.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
