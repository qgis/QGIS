/***************************************************************************
    qgssensorthingsguiprovider.cpp
     --------------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssensorthingsguiprovider.h"
///@cond PRIVATE

#include <QList>
#include <QAction>
#include <QDesktopServices>
#include <QMenu>
#include <QMessageBox>

#include "qgsmaplayer.h"
#include "qgssensorthingssourceselect.h"
#include "qgssensorthingssourcewidget.h"
#include "qgssensorthingsprovider.h"
#include "qgssensorthingsdataitemguiprovider.h"
#include "qgsapplication.h"
#include "qgssubsetstringeditorprovider.h"
#include "qgssensorthingssubseteditor.h"
#include "qgsvectorlayer.h"

//
// QgsSensorThingsSourceSelectProvider
//

QString QgsSensorThingsSourceSelectProvider::providerKey() const
{
  return QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY;
}

QString QgsSensorThingsSourceSelectProvider::text() const
{
  return QObject::tr( "SensorThings" );
}

QIcon QgsSensorThingsSourceSelectProvider::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddSensorThingsLayer.svg" ) );
}

int QgsSensorThingsSourceSelectProvider::ordering() const
{
  return OrderRemoteProvider + 200;
}

QgsAbstractDataSourceWidget *QgsSensorThingsSourceSelectProvider::createDataSourceWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode ) const
{
  return new QgsSensorThingsSourceSelect( parent, fl, widgetMode );
}


//
// QgsSensorThingsSourceWidgetProvider
//

QgsSensorThingsSourceWidgetProvider::QgsSensorThingsSourceWidgetProvider()
{

}

QString QgsSensorThingsSourceWidgetProvider::providerKey() const
{
  return QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY;
}

bool QgsSensorThingsSourceWidgetProvider::canHandleLayer( QgsMapLayer *layer ) const
{
  return layer->providerType() == QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY;
}

QgsProviderSourceWidget *QgsSensorThingsSourceWidgetProvider::createWidget( QgsMapLayer *layer, QWidget *parent )
{
  if ( layer->providerType() != QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY )
    return nullptr;

  return new QgsSensorThingsSourceWidget( parent );
}


//
// QgsSensorThingsProviderGuiMetadata
//
QgsSensorThingsProviderGuiMetadata::QgsSensorThingsProviderGuiMetadata():
  QgsProviderGuiMetadata( QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY )
{
}

QList<QgsSourceSelectProvider *> QgsSensorThingsProviderGuiMetadata::sourceSelectProviders()
{
  return { new QgsSensorThingsSourceSelectProvider };
}

QList<QgsProviderSourceWidgetProvider *> QgsSensorThingsProviderGuiMetadata::sourceWidgetProviders()
{
  return { new QgsSensorThingsSourceWidgetProvider() };
}

QList<QgsDataItemGuiProvider *> QgsSensorThingsProviderGuiMetadata::dataItemGuiProviders()
{
  return { new QgsSensorThingsDataItemGuiProvider() };
}

class QgsSensorThingsSubsetStringEditorProvider: public QgsSubsetStringEditorProvider
{
  public:

    QString providerKey() const override { return QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY; }

    bool canHandleLayer( QgsVectorLayer *layer ) const override
    {
      QgsDataProvider *provider = layer->dataProvider();
      return static_cast< bool >( qobject_cast<QgsSensorThingsProvider *>( provider ) );
    }

    QgsSubsetStringEditorInterface *createDialog( QgsVectorLayer *layer, QWidget *parent, Qt::WindowFlags fl ) override
    {
      QgsDataProvider *provider = layer->dataProvider();
      QgsSensorThingsProvider *sensorThingsProvider = qobject_cast<QgsSensorThingsProvider *>( provider );
      if ( !sensorThingsProvider )
        return nullptr;

      return new QgsSensorThingsSubsetEditor( layer, QgsFields(), parent, fl );
    }
};


QList<QgsSubsetStringEditorProvider *> QgsSensorThingsProviderGuiMetadata::subsetStringEditorProviders()
{
  return QList<QgsSubsetStringEditorProvider *>()
         << new QgsSensorThingsSubsetStringEditorProvider;
}

///@endcond
