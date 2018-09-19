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
struct QgsGeometryCheckerContext;


/**
 * \ingroup analysis
 * This class manages all known geometry check factories.
 *
 * QgsGeometryCheckRegistry is not usually directly created, but rather accessed through
 * QgsAnalysis::geometryCheckRegistry().
 */
class ANALYSIS_EXPORT QgsGeometryCheckRegistry
{
  public:

    /**
     * Constructor for QgsGeometryCheckRegistry. QgsGeometryCheckRegistry is not usually directly created, but rather accessed through
     * QgsAnalysis::geometryCheckRegistry().
     */
    QgsGeometryCheckRegistry();

    void initialize();

    /**
     * Destructor
     *
     * Deletes all the registered checks
     */
    ~QgsGeometryCheckRegistry();

    QgsGeometryCheck *geometryCheck( const QString &checkId, QgsGeometryCheckerContext *context, const QVariantMap &geometryCheckConfig ) SIP_TRANSFER;

    /**
     * Get all geometry check factories that are compatible with \a layer and have all of the \a flags set.
     *
     * \since QGIS 3.4
     */
    QList<QgsGeometryCheckFactory *> geometryCheckFactories( QgsVectorLayer *layer, QgsGeometryCheck::Flags flags = nullptr ) const;

    void registerGeometryCheck( const QString &checkId, QgsGeometryCheckFactory *checkFactory SIP_TRANSFER );

  private:
    QMap<QString, QgsGeometryCheckFactory *> mGeometryCheckFactories;
};

#endif // QGSGEOMETRYCHECKREGISTRY_H
