/***************************************************************************
  qgsauthawss3method.h
  --------------------------------------
  Date                 : December 2022
  Copyright            : (C) 2022 by Jacky Volpes
  Email                : jacky dot volpes at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHAWSS3METHOD_H
#define QGSAUTHAWSS3METHOD_H

#include <QObject>
#include <QMutex>

#include "qgsauthconfig.h"
#include "qgsauthmethod.h"
#include "qgsauthmethodmetadata.h"


class QgsAuthAwsS3Method : public QgsAuthMethod
{
    Q_OBJECT

  public:

    static const QString AUTH_METHOD_KEY;
    static const QString AUTH_METHOD_DESCRIPTION;
    static const QString AUTH_METHOD_DISPLAY_DESCRIPTION;

    explicit QgsAuthAwsS3Method();

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

    static QMap<QString, QgsAuthMethodConfig> sAuthConfigCache;

};


class QgsAuthAwsS3MethodMetadata : public QgsAuthMethodMetadata
{
  public:
    QgsAuthAwsS3MethodMetadata()
      : QgsAuthMethodMetadata( QgsAuthAwsS3Method::AUTH_METHOD_KEY, QgsAuthAwsS3Method::AUTH_METHOD_DESCRIPTION )
    {}
    QgsAuthAwsS3Method *createAuthMethod() const override {return new QgsAuthAwsS3Method;}
};

#endif // QGSAUTHAWSS3METHOD_H
