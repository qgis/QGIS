/***************************************************************************
                         qgsmemoryproviderutils.h
                         ------------------------
    begin                : May 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
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
 * Utility functions for use with the memory vector data provider.
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
     */
    static QgsVectorLayer *createMemoryLayer( const QString &name,
        const QgsFields &fields,
        QgsWkbTypes::Type geometryType = QgsWkbTypes::NoGeometry,
        const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() ) SIP_FACTORY;
};

#endif // QGSMEMORYPROVIDERUTILS_H


