/***************************************************************************
  qgstiledmeshproviderguimetadata.cpp
  --------------------------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledmeshproviderguimetadata.h"
#include "qgssourceselectprovider.h"
#include "qgsapplication.h"
#include "qgstiledmeshsourceselect.h"
#include "qgstiledmeshdataitemguiprovider.h"

///@cond PRIVATE

class QgsTiledMeshSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "tiledmesh" ); }
    QString text() const override { return QObject::tr( "Tiled Mesh" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 51; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddTiledMeshLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsTiledMeshSourceSelect( parent, fl, widgetMode );
    }
};

QgsTiledMeshProviderGuiMetadata::QgsTiledMeshProviderGuiMetadata()
  : QgsProviderGuiMetadata( QStringLiteral( "tiledmesh" ) )
{
}

QList<QgsDataItemGuiProvider *> QgsTiledMeshProviderGuiMetadata::dataItemGuiProviders()
{
  return { new QgsTiledMeshDataItemGuiProvider() };
}

QList<QgsSourceSelectProvider *> QgsTiledMeshProviderGuiMetadata::sourceSelectProviders()
{
  return { new QgsTiledMeshSourceSelectProvider };
}

///@endcond
