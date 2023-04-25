/***************************************************************************
   qgsredshiftprovidergui.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftprovidergui.h"

#include "qgsapplication.h"
#include "qgsprojectstorageguiprovider.h"
#include "qgsproviderguimetadata.h"
#include "qgsredshiftdataitemguiprovider.h"
#include "qgsredshiftdataitems.h"
#include "qgsredshiftprojectstoragedialog.h"
#include "qgsredshiftprovider.h"
#include "qgsredshiftsourceselect.h"
#include "qgssourceselectprovider.h"

//! Provider for redshift source select
class QgsRedshiftSourceSelectProvider : public QgsSourceSelectProvider
{
  public:
    QString providerKey() const override
    {
      return QStringLiteral( "redshift" );
    }
    QString text() const override
    {
      return QObject::tr( "Amazon Redshift" );
    }
    int ordering() const override
    {
      return QgsSourceSelectProvider::OrderDatabaseProvider + 80;
    }
    QIcon icon() const override
    {
      return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddRedshiftLayer.svg" ) );
    }
    QgsAbstractDataSourceWidget *createDataSourceWidget(
      QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget,
      QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsRedshiftSourceSelect( parent, fl, widgetMode );
    }
};

class QgsRedshiftProjectStorageGuiProvider : public QgsProjectStorageGuiProvider
{
  public:
    QString type() override
    {
      return QStringLiteral( "redshift" );
    }
    QString visibleName() override
    {
      return QObject::tr( "Amazon Redshift" );
    }

    QString showLoadGui() override
    {
      QgsRedshiftProjectStorageDialog dlg( false );
      if ( !dlg.exec() )
        return QString();

      return dlg.currentProjectUri();
    }

    QString showSaveGui() override
    {
      QgsRedshiftProjectStorageDialog dlg( true );
      if ( !dlg.exec() )
        return QString();

      return dlg.currentProjectUri();
    }
};

QgsRedshiftProviderGuiMetadata::QgsRedshiftProviderGuiMetadata()
  : QgsProviderGuiMetadata( QgsRedshiftProvider::REDSHIFT_KEY )
{
}

QList<QgsSourceSelectProvider *> QgsRedshiftProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsRedshiftSourceSelectProvider;
  return providers;
}

QList<QgsDataItemGuiProvider *> QgsRedshiftProviderGuiMetadata::dataItemGuiProviders()
{
  return QList<QgsDataItemGuiProvider *>() << new QgsRedshiftDataItemGuiProvider;
}

QList<QgsProjectStorageGuiProvider *> QgsRedshiftProviderGuiMetadata::projectStorageGuiProviders()
{
  QList<QgsProjectStorageGuiProvider *> providers;
  providers << new QgsRedshiftProjectStorageGuiProvider;
  return providers;
}

void QgsRedshiftProviderGuiMetadata::registerGui( QMainWindow *mainWindow )
{
  QgsRSRootItem::sMainWindow = mainWindow;
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsRedshiftProviderGuiMetadata();
}
#endif
