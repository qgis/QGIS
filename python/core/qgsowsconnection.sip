/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsowsconnection.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsOwsConnection : QObject
{
%Docstring
 Connections management
%End

%TypeHeaderCode
#include "qgsowsconnection.h"
%End
  public:

    QgsOwsConnection( const QString &service, const QString &connName );
%Docstring
 Constructor
 \param service service name: WMS,WFS,WCS
 \param connName connection name
%End

    QString connectionName() const;
%Docstring
 Returns the connection name.
.. versionadded:: 3.0
 :rtype: str
%End

    QString connectionInfo() const;
%Docstring
 Returns connection info string.
.. versionadded:: 3.0
 :rtype: str
%End

    QString service() const;
%Docstring
 Returns a string representing the service type, e.g. "WMS".
.. versionadded:: 3.0
 :rtype: str
%End

    QgsDataSourceUri uri() const;
%Docstring
 Returns the connection uri.
 :rtype: QgsDataSourceUri
%End

    static QgsDataSourceUri &addWmsWcsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey );
%Docstring
 Adds uri parameters relating to the settings for a WMS or WCS connection to a QgsDataSourceUri ``uri``.
 Connection settings are taken from the specified QSettings ``settingsKey``.
.. versionadded:: 3.0
 :rtype: QgsDataSourceUri
%End

    static QgsDataSourceUri &addWfsConnectionSettings( QgsDataSourceUri &uri, const QString &settingsKey );
%Docstring
 Adds uri parameters relating to the settings for a WFS connection to a QgsDataSourceUri ``uri``.
 Connection settings are taken from the specified QSettings ``settingsKey``.
.. versionadded:: 3.0
 :rtype: QgsDataSourceUri
%End

    static QStringList connectionList( const QString &service );
%Docstring
Returns the list of connections for the specified service
 :rtype: list of str
%End

    static void deleteConnection( const QString &service, const QString &name );
%Docstring
Deletes the connection for the specified service with the specified name
%End

    static QString selectedConnection( const QString &service );
%Docstring
Retrieves the selected connection for the specified service
 :rtype: str
%End
    static void setSelectedConnection( const QString &service, const QString &name );
%Docstring
Marks the specified connection for the specified service as selected
%End

  protected:

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsowsconnection.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
