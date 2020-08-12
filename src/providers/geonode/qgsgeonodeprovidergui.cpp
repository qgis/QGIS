/***************************************************************************
  qgsgeonodeprovidergui.cpp
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsproviderguimetadata.h"

#include "qgsgeonodedataitemguiprovider.h"
#include "qgsgeonodesourceselect.h"

static const QString PROVIDER_KEY = QStringLiteral( "geonode" );


class QgsGeonodeProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsGeonodeProviderGuiMetadata()
      : QgsProviderGuiMetadata( PROVIDER_KEY )
    {
    }

    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override
    {
      QList<QgsDataItemGuiProvider *> providers;
      providers << new QgsGeoNodeDataItemGuiProvider;
      return providers;
    }

    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsGeoNodeSourceSelectProvider;
      return providers;
    }
};


QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsGeonodeProviderGuiMetadata();
}
