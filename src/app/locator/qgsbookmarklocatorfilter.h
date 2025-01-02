/***************************************************************************
                         qgsbookmarklocatorfilters.h
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

#ifndef QGSBOOKMARKLOCATORFILTERS_H
#define QGSBOOKMARKLOCATORFILTERS_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"


class APP_EXPORT QgsBookmarkLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:
    QgsBookmarkLocatorFilter( QObject *parent = nullptr );
    QgsBookmarkLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "bookmarks" ); }
    QString displayName() const override { return tr( "Spatial Bookmarks" ); }
    Priority priority() const override { return Highest; }
    QString prefix() const override { return QStringLiteral( "b" ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
};

#endif // QGSBOOKMARKLOCATORFILTERS_H
