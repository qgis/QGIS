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

%TypeHeaderCode
#include "qgsgeonoderequest.h"
%End
  public:

    explicit QgsGeoNodeRequest( bool forceRefresh, QObject *parent = 0 );
%Docstring
 Constructor for QgsGeoNodeRequest.

 If ``forceRefresh`` is false, then cached copies of the request may be reused.
%End
    QgsGeoNodeRequest( const QString &baseUrl, bool forceRefresh, QObject *parent = 0 );
    virtual ~QgsGeoNodeRequest();

    bool request( const QString &endPoint );
%Docstring
 :rtype: bool
%End

    QList<QgsServiceLayerDetail> getLayers();
%Docstring
 :rtype: list of QgsServiceLayerDetail
%End

    QList<QgsGeoNodeStyle> getStyles( const QString &layerName );
%Docstring
 :rtype: list of QgsGeoNodeStyle
%End

    QgsGeoNodeStyle getDefaultStyle( const QString &layerName );
%Docstring
 :rtype: QgsGeoNodeStyle
%End

    QgsGeoNodeStyle getStyle( const QString &styleID );
%Docstring
 :rtype: QgsGeoNodeStyle
%End

    QStringList serviceUrls( const QString &serviceType );
%Docstring
Obtain list of unique URLs in the geonode
 :rtype: list of str
%End

    QgsStringMap serviceUrlData( const QString &serviceType );
%Docstring
Obtain map of layer name and url for a service type
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
