/***************************************************************************
  qgslayermetadatalocatorfilter.h - QgsLayerMetadataLocatorFilter

 ---------------------
 begin                : 5.9.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERMETADATALOCATORFILTER_H
#define QGSLAYERMETADATALOCATORFILTER_H

#include "qgis_app.h"
#include "qgslocatorfilter.h"

class APP_EXPORT QgsLayerMetadataLocatorFilter : public QgsLocatorFilter
{
    Q_OBJECT
  public:

    explicit QgsLayerMetadataLocatorFilter( QObject *parent = nullptr );

    // QgsLocatorFilter interface
  public:
    QgsLocatorFilter *clone() const override;
    QString name() const override { return QStringLiteral( "layermetadata" ); };
    QString displayName() const override { return tr( "Search Layer Metadata" ); };
    void fetchResults( const QString &string, const QgsLocatorContext &context, QgsFeedback *feedback ) override;
    void triggerResult( const QgsLocatorResult &result ) override;
    Priority priority() const override { return Medium; }
    QString prefix() const override { return QStringLiteral( "lmd" ); }
};

#endif // QGSLAYERMETADATALOCATORFILTER_H
