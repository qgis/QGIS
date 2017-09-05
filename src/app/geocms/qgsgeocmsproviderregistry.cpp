/***************************************************************************
                            qgsgeocmsproviderregistry.cpp
                            -----------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgeocmsproviderregistry.h"
#include "qgsgui.h"
#include "qgssourceselectproviderregistry.h"
#include "geocms/geonode/qgsgeonodesourceselect.h"

QgsGeoCmsProviderRegistry::QgsGeoCmsProviderRegistry()
{
  init();
}

void QgsGeoCmsProviderRegistry::init()
{
  QgsGui::sourceSelectProviderRegistry()->addProvider( new QgsGeoNodeSourceSelectProvider() );
}
