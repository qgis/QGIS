/***************************************************************************
                         qgsgotolocatorfilters.h
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

#ifndef QGSGOTOLOCATORFILTERS_H
#define QGSGOTOLOCATORFILTERS_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"

class APP_EXPORT QgsGotoLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:
    QgsGotoLocatorFilter( QObject *parent = nullptr );
    [[nodiscard]] QgsGotoLocatorFilter *clone() const override;
    [[nodiscard]] QString name() const override { return QStringLiteral( "goto" ); }
    [[nodiscard]] QString displayName() const override { return tr( "Go to Coordinate" ); }
    [[nodiscard]] Priority priority() const override { return Medium; }
    [[nodiscard]] QString prefix() const override { return QStringLiteral( "go" ); }
    [[nodiscard]] QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
};

#endif // QGSGOTOLOCATORFILTERS_H
