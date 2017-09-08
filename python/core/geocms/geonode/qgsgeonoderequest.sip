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
%Docstring
Unique style ID
%End

  QString name;
%Docstring
Style name
%End

  QString title;
%Docstring
Style title
%End

  QDomDocument body;
%Docstring
DOM documenting containing style
%End

  QString styleUrl;
%Docstring
Associated URL
%End
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
%Docstring
Unique identifier (generate on the client side, not at the GeoNode server)
%End
      QString name;
%Docstring
Layer name
%End
      QString typeName;
%Docstring
Layer type name
%End
      QString title;
%Docstring
Layer title
%End
      QString wmsURL;
%Docstring
WMS URL for layer
%End
      QString wfsURL;
%Docstring
WFS URL for layer
%End
      QString xyzURL;
%Docstring
XYZ tileserver URL for layer
%End
    };

    QgsGeoNodeRequest( const QString &baseUrl, bool forceRefresh, QObject *parent = 0 );
%Docstring
 Constructor for QgsGeoNodeRequest.

 If ``forceRefresh`` is false, then cached copies of the request may be reused.
%End

    virtual ~QgsGeoNodeRequest();

    void request( const QString &endPoint );
%Docstring
 Triggers a new request to the GeoNode server, with the requested ``endPoint``.
 Any existing request will be aborted.

 Calling this method does not block while waiting for a result.

 \warning When using the non-blocking methods in this class, sending
 overlapping requests results in undefined behavior. Use separate instances
 of QgsGeoNodeRequest instead to avoid this.

.. seealso:: requestBlocking()
%End

    bool requestBlocking( const QString &endPoint );
%Docstring
 Triggers a new request to the GeoNode server, with the requested ``endPoint``.
 Any existing request will be aborted.

 Calling this method will block while waiting for a result. It should not be
 used from any code which potentially blocks operation in the main GUI thread.

.. seealso:: request()
 :rtype: bool
%End

    void fetchLayers();
%Docstring
 Triggers a new request to fetch the list of available layers from the
 server. When complete, the layersFetched() signal will be emitted
 with the result.

 This method is non-blocking and returns immediately.

 \warning When using the non-blocking methods in this class, sending
 overlapping requests results in undefined behavior. Use separate instances
 of QgsGeoNodeRequest instead to avoid this.

.. seealso:: layersFetched()
.. seealso:: fetchLayersBlocking()
%End

    QList<QgsGeoNodeRequest::ServiceLayerDetail> fetchLayersBlocking();
%Docstring
 Requests the list of available layers from the server.

 This method is blocking and will wait for results from the server before returning.
 Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.

.. seealso:: fetchLayers()
 :rtype: list of QgsGeoNodeRequest.ServiceLayerDetail
%End

    QList<QgsGeoNodeStyle> fetchStylesBlocking( const QString &layerName );
%Docstring
 Requests the list of available styles for the layer
 with matching ``layerName`` from the server.

 This method is blocking and will wait for results from the server before returning.
 Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
 :rtype: list of QgsGeoNodeStyle
%End

    QgsGeoNodeStyle fetchDefaultStyleBlocking( const QString &layerName );
%Docstring
 Requests the default style for the layer with matching ``layerName`` from the server.

 This method is blocking and will wait for results from the server before returning.
 Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
 :rtype: QgsGeoNodeStyle
%End

    QgsGeoNodeStyle fetchStyleBlocking( const QString &styleId );
%Docstring
 Requests the details for the style with matching ``styleId`` from the server.

 This method is blocking and will wait for results from the server before returning.
 Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
 :rtype: QgsGeoNodeStyle
%End

    QStringList fetchServiceUrlsBlocking( const QString &serviceType );
%Docstring
 Requests the list of unique URLs for available services with matching ``serviceType`` from the server.

 This method is blocking and will wait for results from the server before returning.
 Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
 :rtype: list of str
%End

    QgsStringMap fetchServiceUrlDataBlocking( const QString &serviceType );
%Docstring
 Obtains a map of layer name to URL for available services with matching ``serviceType`` from the server.

 This method is blocking and will wait for results from the server before returning.
 Accordingly it should not be used from any code which potentially blocks operation in the main GUI thread.
 :rtype: QgsStringMap
%End

    QString lastError() const;
%Docstring
 Returns the most recent error string for any encountered errors, or an empty string if
 no errors have been encountered.
 :rtype: str
%End

    QByteArray lastResponse() const;
%Docstring
 Returns the most recent response obtained from the server.
 :rtype: QByteArray
%End

    QString protocol() const;
%Docstring
 Returns the network protocol (e.g. 'http') used for connecting with the server.
.. seealso:: setProtocol()
 :rtype: str
%End

    void setProtocol( const QString &protocol );
%Docstring
 Sets the network ``protocol`` (e.g. 'http') used for connecting with the server.
.. seealso:: protocol()
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
