/***************************************************************************
    qgsauthbasicmethod.h
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAUTHBASICMETHOD_H
#define QGSAUTHBASICMETHOD_H

#include <QObject>

#include "qgsauthconfig.h"
#include "qgsauthmethod.h"


class QgsAuthBasicMethod : public QgsAuthMethod
{
    Q_OBJECT

  public:
    explicit QgsAuthBasicMethod();
    ~QgsAuthBasicMethod();

    // QgsAuthMethod interface
    QString key() const override;

    QString description() const override;

    QString displayDescription() const override;

    bool updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
                               const QString &dataprovider = QString() ) override;

    bool updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
                                   const QString &dataprovider = QString() ) override;

    void clearCachedConfig( const QString &authcfg ) override;

    void updateMethodConfig( QgsAuthMethodConfig &mconfig ) override;

  private:
    QgsAuthMethodConfig getMethodConfig( const QString &authcfg, bool fullconfig = true );

    void putMethodConfig( const QString &authcfg, const QgsAuthMethodConfig& mconfig );

    void removeMethodConfig( const QString &authcfg );

    QString escapeUserPass( const QString &theVal, QChar delim = '\'' ) const;

    static QMap<QString, QgsAuthMethodConfig> mAuthConfigCache;
};

#endif // QGSAUTHBASICMETHOD_H
