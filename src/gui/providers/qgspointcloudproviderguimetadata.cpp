/***************************************************************************
                         qgspointcloudproviderguimetadata.cpp
                         --------------------
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

#include "qgspointcloudproviderguimetadata.h"

#include "qgsapplication.h"
#include "qgspointcloudsourceselect.h"
#include "qgssourceselectprovider.h"

///@cond PRIVATE

class QgsPointCloudSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    [[nodiscard]] QString providerKey() const override { return QStringLiteral( "pointcloud" ); }
    [[nodiscard]] QString text() const override { return QObject::tr( "Point Cloud" ); }
    [[nodiscard]] int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 25; }
    [[nodiscard]] QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPointCloudLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsPointCloudSourceSelect( parent, fl, widgetMode );
    }
};

QgsPointCloudProviderGuiMetadata::QgsPointCloudProviderGuiMetadata()
  : QgsProviderGuiMetadata( QStringLiteral( "pointcloud" ) )
{
}

QList<QgsSourceSelectProvider *> QgsPointCloudProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsPointCloudSourceSelectProvider;
  return providers;
}

///@endcond
