/***************************************************************************
  qgsdamengprovidergui.cpp
  --------------------------------------
  Date                 : June 2019
  Copyright            : ( C ) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengprovidergui.h"
#include "qgsapplication.h"
#include "qgsproviderguimetadata.h"
#include "qgsdamengsourceselect.h"
#include "qgssourceselectprovider.h"
#include "qgsdamengprovider.h"
#include "qgsdamengdataitems.h"
#include "qgsprojectstorageguiprovider.h"
#include "qgsdamengprojectstoragedialog.h"
#include "qgsdamengdataitemguiprovider.h"
#include "qgsmaplayerconfigwidgetfactory.h"

//! Provider for dameng source select
class QgsDamengSourceSelectProvider : public QgsSourceSelectProvider  //#spellok
{
  public:

    QString providerKey() const override { return QStringLiteral( "dameng" ); }
    QString text() const override { return QObject::tr( "Dameng" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderDatabaseProvider + 20; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddDamengLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsDamengSourceSelect( parent, fl, widgetMode );
    }
};


class QgsDamengProjectStorageGuiProvider : public QgsProjectStorageGuiProvider
{
  public:
    QString type() override { return QStringLiteral( "dameng" ); }
    QString visibleName() override
    {
      return QObject::tr( "Dameng" );
    }

    QString showLoadGui() override
    {
      QgsDamengProjectStorageDialog dlg( false );
      if ( !dlg.exec() )
        return QString();

      return dlg.currentProjectUri();
    }

    QString showSaveGui() override
    {
      QgsDamengProjectStorageDialog dlg( true );
      if ( !dlg.exec() )
        return QString();

      return dlg.currentProjectUri();
    }

};


QgsDamengProviderGuiMetadata::QgsDamengProviderGuiMetadata():
  QgsProviderGuiMetadata( QgsDamengProvider::DAMENG_KEY )
{
}

QList<QgsSourceSelectProvider *> QgsDamengProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsDamengSourceSelectProvider;  //#spellok
  return providers;
}

QList<QgsDataItemGuiProvider *> QgsDamengProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>()
         << new QgsDamengDataItemGuiProvider;
}

QList<QgsProjectStorageGuiProvider *> QgsDamengProviderGuiMetadata::projectStorageGuiProviders()
{
  QList<QgsProjectStorageGuiProvider *> providers;
  providers << new QgsDamengProjectStorageGuiProvider;
  return providers;
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsDamengProviderGuiMetadata();
}
#endif
