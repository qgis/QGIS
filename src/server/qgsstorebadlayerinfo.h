/***************************************************************************
                              qgsstorebadlayerinfo.h
                              ----------------------
  begin                : Jan 2019
  copyright            : (C) 2019 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTOREBADLAYERINFO_H
#define QGSSTOREBADLAYERINFO_H

#include "qgsprojectbadlayerhandler.h"
#include "qgis_server.h"
#include <QStringList>

/**
 * \ingroup server
 * Stores layer ids of bad layers
 * \since QGIS 3.6
 */
class SERVER_EXPORT QgsStoreBadLayerInfo: public QgsProjectBadLayerHandler
{
  public:

    /**
     * Default constructor
     */
    QgsStoreBadLayerInfo() = default;

    /**
     * \brief handleBadLayers
     * \param layers layer nodes
     */
    void handleBadLayers( const QList<QDomNode> &layers );

    /**
     * \brief badLayers
     * \returns ids of bad layers
     */
    QStringList badLayers() const { return mBadLayerIds; }

  private:
    QStringList mBadLayerIds;
};

#endif // QGSSTOREBADLAYERINFO_H
