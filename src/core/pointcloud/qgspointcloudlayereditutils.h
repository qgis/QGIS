/***************************************************************************
    qgspointcloudlayereditutils.h
    ---------------------
    begin                : December 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPOINTCLOUDLAYEREDITUTILS_H
#define QGSPOINTCLOUDLAYEREDITUTILS_H

#include "qgis_core.h"
#include "qgspointcloudindex.h"

#include <optional>

#include <QVector>
#include <QByteArray>

#define SIP_NO_FILE

class QgsPointCloudLayer;
class QgsPointCloudNodeId;
class QgsPointCloudAttribute;
class QgsPointCloudAttributeCollection;
class QgsPointCloudRequest;

class QgsCopcPointCloudIndex;

/**
 * \ingroup core
 *
 * \brief Contains utility functions for editing point cloud layers.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.42
 */
class CORE_EXPORT QgsPointCloudLayerEditUtils
{
  public:
    //! Ctor
    QgsPointCloudLayerEditUtils() = delete;

    //! Takes \a data comprising of \a allAttributes and returns a QByteArray with data only for the attributes included in the \a request
    static QByteArray dataForAttributes( const QgsPointCloudAttributeCollection &allAttributes, const QByteArray &data, const QgsPointCloudRequest &request );

    //! Check if \a value is within proper range for the \a attribute
    static bool isAttributeValueValid( const QgsPointCloudAttribute &attribute, double value );

    //! Sets new classification value for the given points in voxel and return updated chunk data
    static QByteArray updateChunkValues( QgsCopcPointCloudIndex *copcIndex, const QByteArray &chunkData, const QgsPointCloudAttribute &attribute, const QgsPointCloudNodeId &n, const QHash<int, double> &pointValues, std::optional<double> newValue = std::nullopt );

};

#endif // QGSPOINTCLOUDLAYEREDITUTILS_H
