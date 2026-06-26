/***************************************************************************
  qgsimageserverprovidergui.cpp
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsimageserverprovidergui.h"

#include "qgsarcgisimageserversourcewidget.h"
#include "qgsmaplayer.h"
#include "qgsproviderguimetadata.h"
#include "qgsprovidersourcewidgetprovider.h"
#include "qgssourceselectprovider.h"

#include <QString>

using namespace Qt::StringLiterals;


class QgsImageServerSourceWidgetProvider : public QgsProviderSourceWidgetProvider
{
  public:
    QgsImageServerSourceWidgetProvider()
      : QgsProviderSourceWidgetProvider()
    {}
    QString providerKey() const override { return u"arcgisimageserver"_s; }
    bool canHandleLayer( QgsMapLayer *layer ) const override
    {
      if ( layer->providerType() != "arcgisimageserver"_L1 )
        return false;

      return true;
    }
    QgsProviderSourceWidget *createWidget( QgsMapLayer *layer, QWidget *parent = nullptr ) override
    {
      if ( layer->providerType() == "arcgisimageserver"_L1 )
      {
        return new QgsArcGisImageServerSourceWidget( layer, parent );
      }
      return nullptr;
    }
};


QgsImageServerProviderGuiMetadata::QgsImageServerProviderGuiMetadata()
  : QgsProviderGuiMetadata( u"arcgisimageserver"_s )
{}


QList<QgsProviderSourceWidgetProvider *> QgsImageServerProviderGuiMetadata::sourceWidgetProviders()
{
  QList<QgsProviderSourceWidgetProvider *> providers;
  providers << new QgsImageServerSourceWidgetProvider();
  return providers;
}


#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsImageServerProviderGuiMetadata();
}
#endif
