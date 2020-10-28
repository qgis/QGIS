/***************************************************************************
                         qgseptdataprovider.h
                         ---------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEPTPROVIDER_H
#define QGSEPTPROVIDER_H

#include "qgis_core.h"
#include "qgspointclouddataprovider.h"
#include "qgsprovidermetadata.h"

#include <memory>

#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsEptPointCloudIndex;

class QgsEptProvider: public QgsPointCloudDataProvider
{
    Q_OBJECT
  public:
    QgsEptProvider( const QString &uri,
                    const QgsDataProvider::ProviderOptions &providerOptions,
                    QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() );

    ~QgsEptProvider();
    QgsCoordinateReferenceSystem crs() const override;

    QgsRectangle extent() const override;

    bool isValid() const override;

    QString name() const override;

    QString description() const override;

    QgsPointCloudIndex *index() const override;

  private:
    std::unique_ptr<QgsEptPointCloudIndex> mIndex;
    bool mIsValid = false;
};

class QgsEptProviderMetadata : public QgsProviderMetadata
{
  public:
    QgsEptProviderMetadata();
    QgsEptProvider *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override;
    QList< QgsDataItemProvider * > dataItemProviders() const override;
    QString encodeUri( const QVariantMap &parts ) override;
    QVariantMap decodeUri( const QString &uri ) override;
};

///@endcond
#endif // QGSEPTPROVIDER_H
