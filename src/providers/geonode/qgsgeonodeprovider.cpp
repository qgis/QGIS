/***************************************************************************
                              qgsgeonodeprovider.cpp
                              ----------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QList>

#include "qgis.h"
#include "qgsprovidermetadata.h"
#ifdef HAVE_GUI
#include "qgsproviderguimetadata.h"
#include "qgsgeonodedataitems.h"
#include "qgsgeonodesourceselect.h"
#endif

static const QString PROVIDER_KEY = QStringLiteral( "geonode" );
static const QString PROVIDER_DESCRIPTION = QStringLiteral( "GeoNode provider" );

class QgsGeonodeProviderMetadata: public QgsProviderMetadata
{
  public:
    QgsGeonodeProviderMetadata(): QgsProviderMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION ) {}
#ifdef HAVE_GUI
    QList<QgsDataItemProvider *> dataItemProviders() const override
    {
      QList<QgsDataItemProvider *> providers;
      providers << new QgsGeoNodeDataItemProvider();
      return providers;
    }
#endif
};

#ifdef HAVE_GUI
class QgsGeonodeProviderGuiMetadata: public QgsProviderGuiMetadata
{
  public:
    QgsGeonodeProviderGuiMetadata(): QgsProviderGuiMetadata( PROVIDER_KEY, PROVIDER_DESCRIPTION ) {}
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override
    {
      QList<QgsSourceSelectProvider *> providers;
      providers << new QgsGeoNodeSourceSelectProvider;
      return providers;
    }
};
#endif

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsGeonodeProviderMetadata();
}

#ifdef HAVE_GUI
QGISEXTERN QgsProviderGuiMetadata *providerGuiMetadataFactory()
{
  return new QgsGeonodeProviderGuiMetadata();
}
#endif

