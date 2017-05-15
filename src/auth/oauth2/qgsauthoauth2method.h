/***************************************************************************
    begin                : July 13, 2016
    copyright            : (C) 2016 by Monsanto Company, USA
    author               : Larry Shaffer, Boundless Spatial
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSAUTHOAUTH2METHOD_H
#define QGSAUTHOAUTH2METHOD_H

#include <QObject>
#include <QDialog>
#include <QEventLoop>
#include <QTimer>

#include "qgsauthmethod.h"


class QgsO2;

class QgsAuthOAuth2Method : public QgsAuthMethod
{
    Q_OBJECT

  public:
    explicit QgsAuthOAuth2Method();
    ~QgsAuthOAuth2Method();

    // QgsAuthMethod interface
    QString key() const override;

    QString description() const override;

    QString displayDescription() const override;

    bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
                               const QString &dataprovider = QString() ) override;

    bool updateNetworkReply( QNetworkReply *reply, const QString &authcfg,
                             const QString &dataprovider ) override;

    bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
                                   const QString &dataprovider = QString() ) override;

    void clearCachedConfig( const QString &authcfg ) override;

    void updateMethodConfig( QgsAuthMethodConfig &mconfig ) override;

  public slots:
    void onLinkedChanged();
    void onLinkingFailed();
    void onLinkingSucceeded();

    void onOpenBrowser( const QUrl &url );
    void onCloseBrowser();
    void onReplyFinished();
    void onNetworkError( QNetworkReply::NetworkError err );
    void onRefreshFinished( QNetworkReply::NetworkError err );

  private:
    QString mTempStorePath;

    QgsO2 *getOAuth2Bundle( const QString &authcfg, bool fullconfig = true );

    void putOAuth2Bundle( const QString &authcfg, QgsO2 *bundle );

    void removeOAuth2Bundle( const QString &authcfg );

    static QMap<QString, QgsO2 *> sOAuth2ConfigCache;

    QgsO2 *authO2( const QString &authcfg );
};

#endif // QGSAUTHOAUTH2METHOD_H
