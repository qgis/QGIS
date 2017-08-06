/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geonode/qgsgeonoderequest.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




struct QgsServiceLayerDetail
{
%TypeHeaderCode
#include <qgsgeonoderequest.h>
%End
  QUuid uuid;
  QString name;
  QString typeName;
  QString title;
  QString wmsURL;
  QString wfsURL;
  QString xyzURL;
};

class QgsGeoNodeRequest : QObject
{

%TypeHeaderCode
#include "qgsgeonoderequest.h"
%End
  public:
    explicit QgsGeoNodeRequest( bool forceRefresh, QObject *parent = 0 );
    QgsGeoNodeRequest( const QString &baseUrl, /*const QgsWmsAuthorization &auth,*/ bool forceRefresh, QObject *parent = 0 );
    virtual ~QgsGeoNodeRequest();

    bool request( QString endPoint );
%Docstring
 :rtype: bool
%End

    bool getLayers();
%Docstring
 :rtype: bool
%End

    QList<QgsServiceLayerDetail> parseLayers( QByteArray layerResponse );
%Docstring
 :rtype: list of QgsServiceLayerDetail
%End

    QStringList serviceUrls( QString serviceType );
%Docstring
 :rtype: list of str
%End

    QgsStringMap serviceUrlData( QString serviceType );
%Docstring
 :rtype: QgsStringMap
%End

    QString lastError() const;
%Docstring
 :rtype: str
%End

    QByteArray response() const;
%Docstring
 :rtype: QByteArray
%End

    QNetworkReply *reply() const;
%Docstring
 :rtype: QNetworkReply
%End

    void abort();
%Docstring
Abort network request immediately
%End

    QString getProtocol() const;
%Docstring
 :rtype: str
%End
    void setProtocol( const QString &protocol );

  signals:
    void statusChanged( const QString &statusQString );
%Docstring
 emit a signal to be caught by qgisapp and display a statusQString on status bar
%End

    void requestFinished();
%Docstring
 emit a signal once the request is finished
%End

  protected slots:
    void replyFinished();
    void replyProgress( qint64, qint64 );

  protected:









};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geonode/qgsgeonoderequest.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
