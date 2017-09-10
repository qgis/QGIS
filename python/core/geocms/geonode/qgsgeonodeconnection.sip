/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/geonode/qgsgeonodeconnection.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsGeoNodeConnection
{
%Docstring
 Encapsulates settings related to a single GeoNode connection.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsgeonodeconnection.h"
%End
  public:

    explicit QgsGeoNodeConnection( const QString &name );
%Docstring
 Constructor for a QgsGeoNodeConnection with the specified ``name``.
%End

    QString connectionName() const;
%Docstring
 Returns the name of the connection.
.. seealso:: setConnectionName()
 :rtype: str
%End

    void setConnectionName( const QString &connectionName );
%Docstring
 Sets the ``name`` of the connection.
.. seealso:: connectionName()
%End

    QgsDataSourceUri uri() const;
%Docstring
 Returns the URI for the GeoNode connection.
.. seealso:: setUri()
 :rtype: QgsDataSourceUri
%End

    void setUri( const QgsDataSourceUri &uri );
%Docstring
 Sets the ``uri`` for the GeoNode connection.
.. seealso:: uri()
%End

};

class QgsGeoNodeConnectionUtils
{
%Docstring
 Contains various utilities for managing the known collection of
 GeoNode servers associated with a QGIS install.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsgeonodeconnection.h"
%End
  public:

    static QStringList connectionList();
%Docstring
 Returns a list of all known GeoNode connection names.
 :rtype: list of str
%End

    static void deleteConnection( const QString &name );
%Docstring
 Deletes the GeoNode connection with matching ``name``.
%End

    static QString pathGeoNodeConnection();
%Docstring
 Returns the base path for settings related to GeoNode connections.
 :rtype: str
%End

    static QString pathGeoNodeConnectionDetails();
%Docstring
 Returns the base path for settings related to GeoNode connection details.
 :rtype: str
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/geonode/qgsgeonodeconnection.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
