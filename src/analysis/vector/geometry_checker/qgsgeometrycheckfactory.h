/***************************************************************************
    qgsgeometrycheckfactory.h
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

#ifndef QGSGEOMETRYCHECKFACTORY_H
#define QGSGEOMETRYCHECKFACTORY_H

#include <QString>
#include <QMap>
#include <QVariantMap>

#include "qgsgeometrycheck.h"
#include "qgis_sip.h"
#include "qgis_analysis.h"

#include "qgsgeometryselfintersectioncheck.h"
#include "qgssinglegeometrycheck.h"

class QgsGeometryCheck;
class QgsSingleGeometryCheck;

struct QgsGeometryCheckContext;

/**
 * \ingroup analysis
 *
 * A factory for geometry checks.
 *
 * \note This class is a technology preview and unstable API.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryCheckFactory SIP_ABSTRACT
{
  public:

    /**
     * Destructor
     *
     * Deletes all the registered checks
     */
    virtual ~QgsGeometryCheckFactory() = default;

    /**
     * Creates a new geometry check with \a context and \a configuration.
     */
    virtual QgsGeometryCheck *createGeometryCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration ) const = 0 SIP_FACTORY;

    /**
     * The unique id for this geometry check.
     */
    virtual QString id() const = 0;

    /**
     * A human readable description for this check.
     */
    virtual QString description() const = 0;

    /**
     * Checks if this check should be made available for \a layer.
     */
    virtual bool isCompatible( QgsVectorLayer *layer ) const = 0;

    /**
     * Flags for this check.
     */
    virtual QgsGeometryCheck::Flags flags() const = 0;

    /**
     * The type of this check.
     */
    virtual QgsGeometryCheck::CheckType checkType() const = 0;
};

/**
 * \ingroup analysis
 * Template to create a factory for a geometry check.
 *
 * \note Not available in Python bindings.
 */
template<class T>
class QgsGeometryCheckFactoryT : public QgsGeometryCheckFactory
{
  public:
    QgsGeometryCheck *createGeometryCheck( const QgsGeometryCheckContext *context, const QVariantMap &configuration ) const override
    {
      return new T( context, configuration );
    }

    QString description() const override
    {
      return T::factoryDescription();
    }

    QString id() const override
    {
      return T::factoryId();
    }

    bool isCompatible( QgsVectorLayer *layer ) const override
    {
      return T::factoryIsCompatible( layer );
    }

    QgsGeometryCheck::Flags flags() const override
    {
      return T::factoryFlags();
    }

    QgsGeometryCheck::CheckType checkType() const override
    {
      return T::factoryCheckType();
    }
};


#endif // QGSGEOMETRYCHECKFACTORY_H
