/***************************************************************************
                         qgspointcloudproviderguimetadata.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
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

#ifndef QGPOINTCLOUDPROVIDERGUIMETADATA_H
#define QGPOINTCLOUDPROVIDERGUIMETADATA_H

///@cond PRIVATE
#define SIP_NO_FILE

#include <QList>
#include <QMainWindow>

#include "qgsproviderguimetadata.h"

/**
 * \ingroup gui
 * \brief Provides a generic source select widget for point cloud providers.
 *
 * The generated widget is provider agnostic, and is not attached to any single individual
 * provider but rather adapts to all available point cloud providers.
 *
 * \since QGIS 3.18
 */
class QgsPointCloudProviderGuiMetadata : public QgsProviderGuiMetadata
{
  public:
    QgsPointCloudProviderGuiMetadata();
    QList<QgsSourceSelectProvider *> sourceSelectProviders() override;
};

///@endcond

#endif // QGPOINTCLOUDPROVIDERGUIMETADATA_H
