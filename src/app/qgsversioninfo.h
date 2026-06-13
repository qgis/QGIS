/***************************************************************************

               ----------------------------------------------------
              date                 : 18.8.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVERSIONINFO_H
#define QGSVERSIONINFO_H

#include "qgis_app.h"

#include <QByteArray>
#include <QNetworkReply>
#include <QObject>
#include <QString>

class APP_EXPORT QgsVersionInfo : public QObject
{
    Q_OBJECT
  public:
    struct ReleaseDetails
    {
        int versionCode = 0;
        QString version;
        QString url;
        QString body;
    };

    explicit QgsVersionInfo( QObject *parent = nullptr );

    QString html() const { return mAdditionalHtml; }

    QString downloadInfo() const { return mDownloadInfo; }

    int latestVersionCode() const { return mLatestVersion; }

    QString latestVersion() const { return mLatestVersionString; }

    QString releaseUrl() const { return mReleaseUrl; }

    bool newVersionAvailable() const;

    bool isDevelopmentVersion() const;

    QNetworkReply::NetworkError error() const { return mError; }

    QString errorString() const { return mErrorString; }

    static int versionCodeFromString( const QString &versionString, bool *ok = nullptr );

    static int versionCodeFromTag( const QString &tagName, bool *ok = nullptr );

    static bool releaseDetailsFromGitHubReleases( const QByteArray &content, ReleaseDetails &details, QString *errorString = nullptr );

  public slots:

    /**
     * Connects to GitHub and checks for new Strata versions.
     */
    void checkVersion();

  private slots:
    void versionReplyFinished();

  signals:
    void versionInfoAvailable();

  private:
    int mLatestVersion = 0;
    QString mLatestVersionString;
    QString mReleaseUrl;
    QString mDownloadInfo;
    QString mAdditionalHtml;
    QNetworkReply::NetworkError mError = QNetworkReply::NoError;
    QString mErrorString;
};

#endif // QGSVERSIONINFO_H
