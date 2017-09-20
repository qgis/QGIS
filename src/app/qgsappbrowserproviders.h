/***************************************************************************
    qgsappbrowserproviders.h
    -------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPBROWSERPROVIDERS_H
#define QGSAPPBROWSERPROVIDERS_H

#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgscustomdrophandler.h"

class QgsQlrDataItem : public QgsLayerItem
{
    Q_OBJECT

  public:

    QgsQlrDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;

};

class QgsQlrDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    int capabilities() override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

class QgsQlrDropHandler : public QgsCustomDropHandler
{
  public:

    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
};

#endif // QGSAPPBROWSERPROVIDERS_H
