/***************************************************************************
                         qgswktlocatorfilter.h
                         --------------------------
    begin                : June 2025
    copyright            : (C) 2025 by Johannes Kröger
    email                : qgis at johanneskroeger dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWKTLOCATORFILTER_H
#define QGSWKTLOCATORFILTER_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"


class APP_EXPORT QgsWktLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:
    QgsWktLocatorFilter( QObject *parent = nullptr );
    QgsWktLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "wkt" ); }
    QString displayName() const override { return tr( "Load WKT" ); }
    Priority priority() const override { return Highest; }
    QString prefix() const override { return QStringLiteral( "wkt" ); }
    QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
};

#endif // QGSWKTLOCATORFILTER_H
