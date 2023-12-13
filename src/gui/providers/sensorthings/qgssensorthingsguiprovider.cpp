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
#include "qgssensorthingssourcewidget.h"
#include "qgssensorthingsprovider.h"


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

QList<QgsProviderSourceWidgetProvider *> QgsSensorThingsProviderGuiMetadata::sourceWidgetProviders()
{
  QList<QgsProviderSourceWidgetProvider *> providers;
  providers << new QgsSensorThingsSourceWidgetProvider();
  return providers;
}

///@endcond
