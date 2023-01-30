/***************************************************************************
                         qgsmemoryproviderutils.h
                         ------------------------
    begin                : May 2017
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

#ifndef QGSMEMORYPROVIDERUTILS_H
#define QGSMEMORYPROVIDERUTILS_H

#include "qgis_core.h"
#include "qgswkbtypes.h"
#include "qgscoordinatereferencesystem.h"
#include <QString>
#include <QVariant>

class QgsVectorLayer;
class QgsFields;

/**
 * \class QgsMemoryProviderUtils
 * \ingroup core
 * \brief Utility functions for use with the memory vector data provider.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsMemoryProviderUtils
{

  public:

    /**
     * Creates a new memory layer using the specified parameters. The caller takes responsibility
     * for deleting the newly created layer.
     * \param name layer name
     * \param fields fields for layer
     * \param geometryType optional layer geometry type
     * \param crs optional layer CRS for layers with geometry
     * \param loadDefaultStyle optional load default style toggle
     */
    static QgsVectorLayer *createMemoryLayer( const QString &name,
        const QgsFields &fields,
        QgsWkbTypes::Type geometryType = QgsWkbTypes::NoGeometry,
        const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(),
        bool loadDefaultStyle = true ) SIP_FACTORY;
};

#endif // QGSMEMORYPROVIDERUTILS_H


