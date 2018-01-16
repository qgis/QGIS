/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsqtlocationconnection.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






%Feature MOBILITY_LOCATION

%If (MOBILITY_LOCATION)

class QgsQtLocationConnection: QgsGpsConnection
{
%Docstring

.. note::

   may not be available in Python bindings on all platforms
%End

%TypeHeaderCode
#include "qgsqtlocationconnection.h"
%End
  public:
    QgsQtLocationConnection();

  protected slots:
    void broadcastConnectionAvailable();
%Docstring
Needed to make QtLocation detected
%End

    virtual void parseData();

%Docstring
Parse available data source content
%End


%If (!ANDROID)

    void satellitesInViewUpdated( const QList<QGeoSatelliteInfo> &satellites );
%Docstring
Called when the number of satellites in view is updated.

.. note::

   not available in Python bindings on android
%End

    void satellitesInUseUpdated( const QList<QGeoSatelliteInfo> &satellites );
%Docstring
Called when the number of satellites in use is updated.

.. note::

   not available in Python bindings on android
%End

%End

};

%End // MOBILITY_LOCATION

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/gps/qgsqtlocationconnection.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
