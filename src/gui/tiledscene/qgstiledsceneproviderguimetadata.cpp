/***************************************************************************
  qgstiledsceneproviderguimetadata.cpp
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

#include "qgstiledsceneproviderguimetadata.h"

#include "qgsapplication.h"
#include "qgssourceselectprovider.h"
#include "qgstiledscenedataitemguiprovider.h"
#include "qgstiledscenesourceselect.h"

///@cond PRIVATE

class QgsTiledSceneSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override { return u"tiledscene"_s; }
    QString text() const override { return QObject::tr( "Scene" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderRemoteProvider + 51; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( u"/mActionAddTiledSceneLayer.svg"_s ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsTiledSceneSourceSelect( parent, fl, widgetMode );
    }
};

QgsTiledSceneProviderGuiMetadata::QgsTiledSceneProviderGuiMetadata()
  : QgsProviderGuiMetadata( u"tiledscene"_s )
{
}

QList<QgsDataItemGuiProvider *> QgsTiledSceneProviderGuiMetadata::dataItemGuiProviders()
{
  return { new QgsTiledSceneDataItemGuiProvider() };
}

QList<QgsSourceSelectProvider *> QgsTiledSceneProviderGuiMetadata::sourceSelectProviders()
{
  return { new QgsTiledSceneSourceSelectProvider };
}

///@endcond
