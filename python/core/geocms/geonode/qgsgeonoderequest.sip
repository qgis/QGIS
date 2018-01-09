/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/geonode/qgsgeonoderequest.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/


struct QgsGeoNodeStyle
{
%TypeHeaderCode
#include <qgsgeonoderequest.h>
%End

  QString id;

  QString name;

  QString title;

  QDomDocument body;

  QString styleUrl;
};

class QgsGeoNodeRequest : QObject
{
%Docstring
 Request handler for GeoNode servers.

QgsGeoNodeRequest handles requesting and parsing service details from a GeoNode
server instance, for instance requesting all available layers or layer styles.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsgeonoderequest.h"
%End
  public:

    struct ServiceLayerDetail
    {
      QUuid uuid;
      QString name;
      QString typeName;
      QString title;
      QString wmsURL;
      QString wfsURL;
      QString xyzURL;
    };

    QgsGeoNodeRequest( const QString &baseUrl, bool forceRefresh, QObject *parent = 0 );
%Docstring
Constructor for QgsGeoNodeRequest.

If ``forceRefresh`` is false, then cached copies of the request may be reused.
%End

    ~QgsGeoNodeRequest();

    void request( const QString &endPoint );
%Docstring
Triggers a new request to the GeoNode server, with the requested ``endPoint``.
Any existing request will be aborted.

Calling this method does not block while waiting for a result.

.. warning::

   When using the non-blocking methods in this class, sending
overlapping requests results in undefined behavior. Use separate instances
of QgsGeoNodeRequest instead to avoid this.

.. seealso:: :py:func:`requestBlocking`
%End

    bool requestBlocking( const QString &endPoint );
%Docstring
Triggers a new request to the GeoNode server, with the requested ``endPoint``.
Any existing request will be aborted.

Calling this method will block while waiting for a result. It should not be
used from any code which potentially blocks operation in the main GUI thread.

.. seealso:: :py:func:`request`
%End

    void fetchLayers();
%Docstring
Triggers a new request to fetch the list of available layers from the
server. When complete, the layersFetched() signal will be emitted
with the result.

This method is non-blocking and returns immediately.

.. warning::

   When using the non-blocking methods in this class, sending
overlapping requests results in undefined behavior. Use separate instances
of QgsGeoNodeRequest instead to avoid this.

.. seealso:: :py:func:`layersFetched`

.. seealso:: :py:func:`fetchLayersBlocking`
%End

    QList<QgsGeoNodeRequest::ServiceLayerDetail> fetchLayersBlocking();
%Docstring
Requests the list of available layers from the server.

This method is blocking and will wait for results from the server before returning.
Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.

.. seealso:: :py:func:`fetchLayers`
%End

    QList<QgsGeoNodeStyle> fetchStylesBlocking( const QString &layerName );
%Docstring
Requests the list of available styles for the layer
with matching ``layerName`` from the server.

This method is blocking and will wait for results from the server before returning.
Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
%End

    QgsGeoNodeStyle fetchDefaultStyleBlocking( const QString &layerName );
%Docstring
Requests the default style for the layer with matching ``layerName`` from the server.

This method is blocking and will wait for results from the server before returning.
Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
%End

    QgsGeoNodeStyle fetchStyleBlocking( const QString &styleId );
%Docstring
Requests the details for the style with matching ``styleId`` from the server.

This method is blocking and will wait for results from the server before returning.
Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
%End

    QStringList fetchServiceUrlsBlocking( const QString &serviceType );
%Docstring
Requests the list of unique URLs for available services with matching ``serviceType`` from the server.

This method is blocking and will wait for results from the server before returning.
Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
%End

    QgsStringMap fetchServiceUrlDataBlocking( const QString &serviceType );
%Docstring
Obtains a map of layer name to URL for available services with matching ``serviceType`` from the server.

This method is blocking and will wait for results from the server before returning.
Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
%End

    QString lastError() const;
%Docstring
Returns the most recent error string for any encountered errors, or an empty string if
no errors have been encountered.
%End

    QByteArray lastResponse() const;
%Docstring
Returns the most recent response obtained from the server.
%End

    QString protocol() const;
%Docstring
Returns the network protocol (e.g. 'http') used for connecting with the server.

.. seealso:: :py:func:`setProtocol`
%End

    void setProtocol( const QString &protocol );
%Docstring
Sets the network ``protocol`` (e.g. 'http') used for connecting with the server.

.. seealso:: :py:func:`protocol`
%End

  public slots:

    void abort();
%Docstring
Aborts any active network request immediately.
%End

  signals:

    void statusChanged( const QString &statusQString );
%Docstring
Emitted when the status of an ongoing request is changed.
%End

    void requestFinished();
%Docstring
Emitted when the existing request has been completed.
%End

    void layersFetched( const QList<QgsGeoNodeRequest::ServiceLayerDetail> &layers );
%Docstring
Emitted when the result of a fetchLayers call has been received and processed.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/geonode/qgsgeonoderequest.h                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
