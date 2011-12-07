#ifndef QGSWFSCONNECTION_H
#define QGSWFSCONNECTION_H

#include <QObject>

#include "qgsrectangle.h"

class QNetworkReply;

class QgsWFSConnection : public QObject
{
    Q_OBJECT
  public:
    explicit QgsWFSConnection( QString connName, QObject *parent = 0 );

    static QStringList connectionList();

    static void deleteConnection( QString name );

    static QString selectedConnection();
    static void setSelectedConnection( QString name );

    //! base service URI
    QString uri() const { return mUri; }
    //! URI to get capabilities
    QString uriGetCapabilities() const;
    //! URI to get schema of wfs layer
    QString uriDescribeFeatureType( const QString& typeName ) const;
    //! URI to get features
    QString uriGetFeature( QString typeName,
                           QString crs = QString(),
                           QString filter = QString(),
                           QgsRectangle bBox = QgsRectangle() ) const;

    //! start network connection to get capabilities
    void requestCapabilities();

    //! description of a vector layer
    struct FeatureType
    {
      QString name;
      QString title;
      QString abstract;
      QList<QString> crslist; // first is default
    };

    //! parsed get capabilities document
    struct GetCapabilities
    {
      void clear() { featureTypes.clear(); }

      QList<FeatureType> featureTypes;
    };

    enum ErrorCode { NoError, NetworkError, XmlError, ServerExceptionError };
    ErrorCode errorCode() { return mErrorCode; }
    QString errorMessage() { return mErrorMessage; }

    //! return parsed capabilities - requestCapabilities() must be called before
    GetCapabilities capabilities() { return mCaps; }

  signals:
    void gotCapabilities();

  public slots:
    void capabilitiesReplyFinished();

  protected:
    QString mConnName;
    QString mUri;

    QNetworkReply *mCapabilitiesReply;
    GetCapabilities mCaps;
    ErrorCode mErrorCode;
    QString mErrorMessage;
};

#endif // QGSWFSCONNECTION_H
