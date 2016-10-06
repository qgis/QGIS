// Class to detect the GPS port
class QgsGPSDetector: QObject
{
%TypeHeaderCode
#include "qgsgpsdetector.h"
%End

  public:
    QgsGPSDetector( const QString& portName );
    ~QgsGPSDetector();

    static QList< QPair<QString, QString> > availablePorts();

  public slots:
    void advance();
    void detected( const QgsGPSInformation& );
    void connDestroyed( QObject * );

  signals:
    void detected( QgsGPSConnection * );
    void detectionFailed();
};
