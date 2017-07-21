/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/qgsgeonodeconnection.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsGeoNodeConnection : QgsGeoCmsConnection
{
%Docstring
!
   GeoNode Connections management
%End

%TypeHeaderCode
#include "qgsgeonodeconnection.h"
%End
  public:
    explicit QgsGeoNodeConnection( const QString &connName );
%Docstring
Constructor
%End

    ~QgsGeoNodeConnection();
%Docstring
Destructor
%End

    static QStringList connectionList();
%Docstring
Retrieve all geonode connection
 :rtype: list of str
%End

    static void deleteConnection( const QString &name );
%Docstring
Delete connection with name, name
%End

    static QString selectedConnection();
%Docstring
Get selected connection
 :rtype: str
%End

    static void setSelectedConnection( const QString &name );
%Docstring
Set selected connection
%End

    static const QString pathGeoNodeConnection;
    static const QString pathGeoNodeConnectionDetails;
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/qgsgeonodeconnection.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
