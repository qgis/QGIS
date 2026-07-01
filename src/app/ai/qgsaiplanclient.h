/***************************************************************************
    qgsaiplanclient.h
    ---------------------
    begin                : July 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAIPLANCLIENT_H
#define QGSAIPLANCLIENT_H

#include "qgis_app.h"

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

/**
 * Small Strata Cloud Plan API client used by the desktop settings and model picker.
 *
 * The chat endpoint stays `/ai/messages`; this client derives the API origin from
 * that configured endpoint and calls auth, account, and model catalog routes.
 */
class APP_EXPORT QgsAiPlanClient : public QObject
{
    Q_OBJECT

  public:
    struct AccountInfo
    {
        QString id;
        QString email;
        QString tier;
    };

    struct ModelInfo
    {
        QString id;
        QString label;
        QString provider;
        int contextWindow = 0;
        int inputCredits = 0;
        int outputCredits = 0;
        QStringList capabilities;
        QStringList tierAvailability;

        QString displayLabel() const;
        QString tooltip() const;
    };

    explicit QgsAiPlanClient( QObject *parent = nullptr );

    static QString apiBaseForChatEndpoint( const QString &chatEndpoint );
    static QList<ModelInfo> parseModelsJson( const QByteArray &body );
    static AccountInfo parseMeJson( const QByteArray &body );
    static QString cacheFilePath();
    static QList<ModelInfo> cachedModels();
    static void writeCachedModels( const QList<ModelInfo> &models );

    void login( const QString &chatEndpoint, const QString &email, const QString &password );
    void registerAccount( const QString &chatEndpoint, const QString &email, const QString &password );
    void fetchMe( const QString &chatEndpoint, const QString &sessionToken );
    void refreshModels( const QString &chatEndpoint );

  signals:
    void desktopTokenReady( const QString &token );
    void accountReady( const QgsAiPlanClient::AccountInfo &account );
    void modelsReady( const QList<QgsAiPlanClient::ModelInfo> &models, bool fromCache );
    void requestFailed( const QString &message );

  private:
    void authenticate( const QString &chatEndpoint, const QString &email, const QString &password, bool createAccount );
    void requestDesktopToken( const QString &apiBase, const QString &accessToken );
};

Q_DECLARE_METATYPE( QgsAiPlanClient::AccountInfo )
Q_DECLARE_METATYPE( QgsAiPlanClient::ModelInfo )
Q_DECLARE_METATYPE( QList<QgsAiPlanClient::ModelInfo> )

#endif // QGSAIPLANCLIENT_H
