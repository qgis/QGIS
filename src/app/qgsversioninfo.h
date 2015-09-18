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

#include <QObject>
#include <QNetworkReply>

class QgsVersionInfo : public QObject
{
    Q_OBJECT
  public:
    explicit QgsVersionInfo( QObject *parent = 0 );

  public slots:
    /**
     * Connects to qgis.org and checks for new versions.
     */
    void checkVersion();

    QString html() const { return mAdditionalHtml; }

    QString downloadInfo() const { return mDownloadInfo; }

    int latestVersionCode() const { return mLatestVersion; }

    bool newVersionAvailable() const;

    bool isDevelopmentVersion() const;

    QNetworkReply::NetworkError error() const { return mError; }

    QString errorString() const { return mErrorString; }

  private slots:
    void versionReplyFinished();

  signals:
    void versionInfoAvailable();

  private:
    int mLatestVersion;
    QString mDownloadInfo;
    QString mAdditionalHtml;
    QNetworkReply::NetworkError mError;
    QString mErrorString;
};

#endif // QGSVERSIONINFO_H
