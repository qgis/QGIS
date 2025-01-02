/***************************************************************************
                         qgssettingslocatorfilters.h
                         --------------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSLOCATORFILTERS_H
#define QGSSETTINGSLOCATORFILTERS_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"


class APP_EXPORT QgsSettingsLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:
    QgsSettingsLocatorFilter( QObject *parent = nullptr );
    QgsSettingsLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "optionpages" ); }
    QString displayName() const override { return tr( "Settings" ); }
    Priority priority() const override { return Highest; }
    QString prefix() const override { return QStringLiteral( "set" ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;

  private:
    QMap<QString, QString> settingsPage( const QString &type, const QString &page );
};

#endif // QGSSETTINGSLOCATORFILTERS_H
