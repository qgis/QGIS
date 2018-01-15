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

.. seealso:: :py:func:`setConnectionName`
%End

    void setConnectionName( const QString &connectionName );
%Docstring
Sets the ``name`` of the connection.

.. seealso:: :py:func:`connectionName`
%End

    QgsDataSourceUri uri() const;
%Docstring
Returns the URI for the GeoNode connection.

.. seealso:: :py:func:`setUri`
%End

    void setUri( const QgsDataSourceUri &uri );
%Docstring
Sets the ``uri`` for the GeoNode connection.

.. seealso:: :py:func:`uri`
%End

    QgsDataSourceUri &addWmsConnectionSettings( QgsDataSourceUri &uri ) const;
%Docstring
Adds uri parameters relating to the settings for a WMS layer on the connection to a :py:class:`QgsDataSourceUri` ``uri``.

.. seealso:: :py:func:`addWmsWcsConnectionSettings`
%End

    QgsDataSourceUri &addWfsConnectionSettings( QgsDataSourceUri &uri ) const;
%Docstring
Adds uri parameters relating to the settings for a WFS layer on the connection to a :py:class:`QgsDataSourceUri` ``uri``.

.. seealso:: :py:func:`addWmsWcsConnectionSettings`
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
%End

    static void deleteConnection( const QString &name );
%Docstring
Deletes the GeoNode connection with matching ``name``.
%End

    static QString pathGeoNodeConnection();
%Docstring
Returns the base path for settings related to GeoNode connections.
%End

    static QString pathGeoNodeConnectionDetails();
%Docstring
Returns the base path for settings related to GeoNode connection details.
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/geonode/qgsgeonodeconnection.h                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
