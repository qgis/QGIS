/***************************************************************************
    qgsauthesritokenmethod.h
    ---------------------
    begin                : October 2018
    copyright            : (C) 2018 by Nyall Dawson
    author               : Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHESRITOKENMETHOD_H
#define QGSAUTHESRITOKENMETHOD_H

#include <QObject>
#include <QMutex>

#include "qgsauthconfig.h"
#include "qgsauthmethod.h"
#include "qgsauthmethodmetadata.h"


class QgsAuthEsriTokenMethod : public QgsAuthMethod
{
    Q_OBJECT

  public:
    static const QString AUTH_METHOD_KEY;
    static const QString AUTH_METHOD_DESCRIPTION;
    static const QString AUTH_METHOD_DISPLAY_DESCRIPTION;

    explicit QgsAuthEsriTokenMethod();

    // QgsAuthMethod interface
    QString key() const override;

    QString description() const override;

    QString displayDescription() const override;

    bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg, const QString &dataprovider = QString() ) override;

    void clearCachedConfig( const QString &authcfg ) override;
    void updateMethodConfig( QgsAuthMethodConfig &mconfig ) override;

#ifdef HAVE_GUI
    QWidget *editWidget( QWidget *parent ) const override;
#endif

  private:
    QgsAuthMethodConfig getMethodConfig( const QString &authcfg, bool fullconfig = true );

    void putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig &mconfig );

    void removeMethodConfig( const QString &authcfg );

    static QMap<QString, QgsAuthMethodConfig> sAuthConfigCache;
};


class QgsAuthEsriTokenMethodMetadata : public QgsAuthMethodMetadata
{
  public:
    QgsAuthEsriTokenMethodMetadata()
      : QgsAuthMethodMetadata( QgsAuthEsriTokenMethod::AUTH_METHOD_KEY, QgsAuthEsriTokenMethod::AUTH_METHOD_DESCRIPTION )
    {}
    QgsAuthEsriTokenMethod *createAuthMethod() const override { return new QgsAuthEsriTokenMethod; }
    //QStringList supportedDataProviders() const override;
};

#endif // QGSAUTHESRITOKENMETHOD_H
