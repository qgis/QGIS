/***************************************************************************
    qgsauthmaptilerhmacsha256method.h
    ---------------------
    begin                : January 2022
    copyright            : (C) 2022 by Vincent Cloarec
    author               : Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHHMACSHA256METHOD_H
#define QGSAUTHHMACSHA256METHOD_H

#include <QObject>
#include <QMutex>

#include "qgsauthconfig.h"
#include "qgsauthmethod.h"
#include "qgsauthmethodmetadata.h"


class QgsAuthMapTilerHmacSha256Method : public QgsAuthMethod
{
    Q_OBJECT

  public:

    static const QString AUTH_METHOD_KEY;
    static const QString AUTH_METHOD_DESCRIPTION;
    static const QString AUTH_METHOD_DISPLAY_DESCRIPTION;

    explicit QgsAuthMapTilerHmacSha256Method();

    // QgsAuthMethod interface
    QString key() const override;

    QString description() const override;

    QString displayDescription() const override;

    bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
                               const QString &dataprovider = QString() ) override;

    void clearCachedConfig( const QString &authcfg ) override;
    void updateMethodConfig( QgsAuthMethodConfig &mconfig ) override;

#ifdef HAVE_GUI
    QWidget *editWidget( QWidget *parent )const override;
#endif

  private:
    QgsAuthMethodConfig getMethodConfig( const QString &authcfg, bool fullconfig = true );

    void putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig );

    void removeMethodConfig( const QString &authcfg );

    QByteArray calculateSignature( const QString &token, const QString &keyedUrl );

    static QMap<QString, QgsAuthMethodConfig> sAuthConfigCache;

};


class QgsAuthMapTilerHmacSha256MethodMetadata : public QgsAuthMethodMetadata
{
  public:
    QgsAuthMapTilerHmacSha256MethodMetadata()
      : QgsAuthMethodMetadata( QgsAuthMapTilerHmacSha256Method::AUTH_METHOD_KEY, QgsAuthMapTilerHmacSha256Method::AUTH_METHOD_DESCRIPTION )
    {}
    QgsAuthMapTilerHmacSha256Method *createAuthMethod() const override {return new QgsAuthMapTilerHmacSha256Method;}
};

#endif // QGSAUTHHMACSHA256METHOD_H
