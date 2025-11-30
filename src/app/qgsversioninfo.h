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

#include <QNetworkReply>
#include <QObject>

class QgsVersionInfo : public QObject
{
    Q_OBJECT
  public:
    explicit QgsVersionInfo( QObject *parent = nullptr );

    [[nodiscard]] QString html() const { return mAdditionalHtml; }

    [[nodiscard]] QString downloadInfo() const { return mDownloadInfo; }

    [[nodiscard]] int latestVersionCode() const { return mLatestVersion; }

    [[nodiscard]] bool newVersionAvailable() const;

    [[nodiscard]] bool isDevelopmentVersion() const;

    [[nodiscard]] QNetworkReply::NetworkError error() const { return mError; }

    [[nodiscard]] QString errorString() const { return mErrorString; }

  public slots:

    /**
     * Connects to qgis.org and checks for new versions.
     */
    void checkVersion();

  private slots:
    void versionReplyFinished();

  signals:
    void versionInfoAvailable();

  private:
    int mLatestVersion = 0;
    QString mDownloadInfo;
    QString mAdditionalHtml;
    QNetworkReply::NetworkError mError = QNetworkReply::NoError;
    QString mErrorString;
};

#endif // QGSVERSIONINFO_H
