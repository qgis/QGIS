/***************************************************************************
                         qgslayoutlocatorfilters.h
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

#ifndef QGSLAYOUTLOCATORFILTERS_H
#define QGSLAYOUTLOCATORFILTERS_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"

class APP_EXPORT QgsLayoutLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT

  public:
    QgsLayoutLocatorFilter( QObject *parent = nullptr );
    [[nodiscard]] QgsLayoutLocatorFilter *clone() const override;
    [[nodiscard]] QString name() const override { return QStringLiteral( "layouts" ); }
    [[nodiscard]] QString displayName() const override { return tr( "Project Layouts" ); }
    [[nodiscard]] Priority priority() const override { return Highest; }
    [[nodiscard]] QString prefix() const override { return QStringLiteral( "pl" ); }
    [[nodiscard]] QgsLocatorFilter::Flags flags() const override { return QgsLocatorFilter::FlagFast; }

    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
};

#endif // QGSLAYOUTLOCATORFILTERS_H
