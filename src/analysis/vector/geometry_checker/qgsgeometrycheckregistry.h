/***************************************************************************
    qgsgeometrycheckregistry.h
     --------------------------------------
    Date                 : September 2018
    Copyright            : (C) 2018 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYCHECKREGISTRY_H
#define QGSGEOMETRYCHECKREGISTRY_H

#include <QString>
#include <QMap>
#include <QVariant>

#include "qgis_sip.h"
#include "qgis_analysis.h"
#include "qgsgeometrycheck.h"

class QgsGeometryCheckFactory;
struct QgsGeometryCheckContext;


/**
 * \ingroup analysis
 * This class manages all known geometry check factories.
 *
 * QgsGeometryCheckRegistry is not usually directly created, but rather accessed through
 * QgsAnalysis::geometryCheckRegistry().
 *
 * \note This class is a technology preview and unstable API.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryCheckRegistry
{
  public:

    /**
     * Constructor for QgsGeometryCheckRegistry. QgsGeometryCheckRegistry is not usually directly created, but rather accessed through
     * QgsAnalysis::geometryCheckRegistry().
     */
    QgsGeometryCheckRegistry() = default;

    /**
     * Destructor
     *
     * Deletes all the registered checks
     */
    ~QgsGeometryCheckRegistry();

    /**
     * Create a new geometryCheck of type \a checkId
     * Pass the \a context and \a geometryCheckConfiguration to the newly created check.
     * Ownership is transferred to the caller.
     *
     * \since QGIS 3.4
     */
    QgsGeometryCheck *geometryCheck( const QString &checkId, QgsGeometryCheckContext *context, const QVariantMap &geometryCheckConfig ) SIP_FACTORY;

    /**
     * Returns all geometry check factories that are compatible with \a layer and have all of the \a flags set.
     *
     * \since QGIS 3.4
     */
    QList<QgsGeometryCheckFactory *> geometryCheckFactories( QgsVectorLayer *layer,  QgsGeometryCheck::CheckType type, QgsGeometryCheck::Flags flags = nullptr ) const;

    /**
     * Registers a new geometry check factory.
     *
     * \since QGIS 3.4
     */
    void registerGeometryCheck( QgsGeometryCheckFactory *checkFactory SIP_TRANSFER );

  private:
    QMap<QString, QgsGeometryCheckFactory *> mGeometryCheckFactories;
};

#endif // QGSGEOMETRYCHECKREGISTRY_H
