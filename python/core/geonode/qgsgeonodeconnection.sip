/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geonode/qgsgeonodeconnection.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsGeoNodeConnection : QObject
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

    QString connName() const;
%Docstring
 :rtype: str
%End
    void setConnName( const QString &connName );

    QgsDataSourceUri uri() const;
%Docstring
 :rtype: QgsDataSourceUri
%End
    void setUri( const QgsDataSourceUri &uri );

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

    static QString pathGeoNodeConnection();
%Docstring
 :rtype: str
%End

    static QString pathGeoNodeConnectionDetails();
%Docstring
 :rtype: str
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geonode/qgsgeonodeconnection.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
