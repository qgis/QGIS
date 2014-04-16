/***************************************************************************
    qgswfscapabilities.h
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSCAPABILITIES_H
#define QGSWFSCAPABILITIES_H

#include <QObject>
#include <QNetworkRequest>

#include "qgsrectangle.h"
#include "qgsdatasourceuri.h"

class QNetworkReply;

class QgsWFSCapabilities : public QObject
{
    Q_OBJECT
  public:
    //explicit QgsWFSCapabilities( QString connName, QObject *parent = 0 );
    QgsWFSCapabilities( QString theUri );

    //! Append ? or & if necessary
    QString prepareUri( QString uri );

    //! base service URI
    QString uri() const { return mBaseUrl; }
    //! URI to get capabilities
    QString uriGetCapabilities() const;
    //! URI to get schema of wfs layer
    QString uriDescribeFeatureType( const QString& typeName ) const;
    //! URI to get features
    //! @param filter can be an OGC filter xml or a QGIS expression (containing =,!=, <,>,<=, >=, AND, OR, NOT )
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

    enum ErrorCode { NoError, NetworkError, XmlError, ServerExceptionError, WFSVersionNotSupported };
    ErrorCode errorCode() { return mErrorCode; }
    QString errorMessage() { return mErrorMessage; }

    //! return parsed capabilities - requestCapabilities() must be called before
    GetCapabilities capabilities() { return mCaps; }

    //! set authorization header
    void setAuthorization( QNetworkRequest &request ) const;

  signals:
    void gotCapabilities();

  public slots:
    void capabilitiesReplyFinished();

  protected:
    //QString mConnName;
    //QString mUri;

    QgsDataSourceURI mUri;

    QString mBaseUrl;

    QNetworkReply *mCapabilitiesReply;
    GetCapabilities mCaps;
    ErrorCode mErrorCode;
    QString mErrorMessage;

    //! Username for basic http authentication
    QString mUserName;

    //! Password for basic http authentication
    QString mPassword;
};

#endif // QGSWFSCAPABILITIES_H
