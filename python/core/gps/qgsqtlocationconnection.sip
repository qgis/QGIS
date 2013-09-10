%Feature MOBILITY_LOCATION

%If (MOBILITY_LOCATION)
class QgsQtLocationConnection: QgsGPSConnection
{
%TypeHeaderCode
#include <qgsqtlocationconnection.h>
%End
  public:
    QgsQtLocationConnection();
    ~QgsQtLocationConnection();

  protected slots:
    /**Needed to make QtLocation detected*/
    void broadcastConnectionAvailable( );

    /**Parse available data source content*/
    void parseData();

    /**Called when the position updated.*/
    // void positionUpdated( const QGeoPositionInfo &info );

%If (!ANDROID)
    /**Called when the number of satellites in view is updated.*/
    void satellitesInViewUpdated( const QList<QGeoSatelliteInfo>& satellites );

    /**Called when the number of satellites in use is updated.*/
    void satellitesInUseUpdated( const QList<QGeoSatelliteInfo>& satellites );
%End
};
%End
