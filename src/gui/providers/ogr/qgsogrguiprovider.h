/***************************************************************************
      qgsogrguiprovider.h  - GUI for QGIS Data provider for GDAL rasters
                             -------------------
    begin                : June, 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRGUIPROVIDER_H
#define QGSOGRGUIPROVIDER_H

#include "qgsproviderguimetadata.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsOgrGuiProviderMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsOgrGuiProviderMetadata();
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
    QList<QgsProviderSourceWidgetProvider *> sourceWidgetProviders() override;
    QList<QgsDataItemGuiProvider *> dataItemGuiProviders() override;
    QList<QgsProjectStorageGuiProvider *> projectStorageGuiProviders() override;
};

///@endcond
#endif
