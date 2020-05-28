/***************************************************************************
                      qgssinglegeometrycheck.h
                     --------------------------------------
Date                 : 6.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSINGLEGEOMETRYCHECK_H
#define QGSSINGLEGEOMETRYCHECK_H

#include <QList>
#include <QCoreApplication>

#include "qgsgeometry.h"
#include "qgsgeometrycheck.h"
#include "qgsgeometrycheckerror.h"

#include "qgis_analysis.h"

class QgsFeature;
class QgsSingleGeometryCheck;

/**
 * \ingroup analysis
 *
 * An error from a QgsSingleGeometryCheck.
 *
 * \note This class is a technology preview and unstable API.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsSingleGeometryCheckError
{
  public:

    /**
     * Creates a new single geometry check error.
     */
    QgsSingleGeometryCheckError( const QgsSingleGeometryCheck *check, const QgsGeometry &geometry, const QgsGeometry &errorLocation, const QgsVertexId &vertexId = QgsVertexId() )
      : mCheck( check )
      , mGeometry( geometry )
      , mErrorLocation( errorLocation )
      , mVertexId( vertexId )
    {}

    virtual ~QgsSingleGeometryCheckError() = default;

    /**
     * Update this error with the information from \a other.
     * Will be used to update existing errors whenever they are re-checked.
     */
    virtual void update( const QgsSingleGeometryCheckError *other );

    /**
     * Check if this error is equal to \a other.
     * Is reimplemented by subclasses with additional information, comparison
     * of base information is done in parent class.
     */
    virtual bool isEqual( const QgsSingleGeometryCheckError *other ) const;

    /**
     * Apply a list of \a changes.
     */
    virtual bool handleChanges( const QList<QgsGeometryCheck::Change> &changes ) SIP_SKIP;

    /**
     * A human readable description of this error.
     */
    virtual QString description() const;

    /**
     * The check that created this error.
     *
     * \since QGIS 3.4
     */
    const QgsSingleGeometryCheck *check() const;

    /**
     * The exact location of the error.
     *
     * \since QGIS 3.4
     */
    QgsGeometry errorLocation() const;

    /**
     * The vertex id of the error. May be invalid depending on the check.
     *
     * \since QGIS 3.4
     */
    QgsVertexId vertexId() const;

  protected:
    const QgsSingleGeometryCheck *mCheck = nullptr;
    QgsGeometry mGeometry;
    QgsGeometry mErrorLocation;
    QgsVertexId mVertexId;
};

/**
 * \ingroup analysis
 *
 * Wraps a QgsSingleGeometryError into a standard QgsGeometryCheckError.
 * The single error can be obtained via singleError.
 *
 * \note This class is a technology preview and unstable API.
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsGeometryCheckErrorSingle : public QgsGeometryCheckError
{
  public:

    /**
     * Creates a new error for a QgsSingleGeometryCheck.
     */
    QgsGeometryCheckErrorSingle( QgsSingleGeometryCheckError *singleError, const QgsGeometryCheckerUtils::LayerFeature &layerFeature );

    /**
     * The underlying single error.
     */
    QgsSingleGeometryCheckError *singleError() const;

    bool handleChanges( const QgsGeometryCheck::Changes &changes ) override SIP_SKIP;

  private:
    QgsSingleGeometryCheckError *mError = nullptr;
};

/**
 * \ingroup analysis
 *
 * Base class for geometry checks for a single geometry without any context of the layer or other layers in the project.
 * Classic examples are validity checks like self-intersection.
 *
 * Subclasses need to implement the processGeometry method.
 *
 * \since QGIS 3.4
 */
class ANALYSIS_EXPORT QgsSingleGeometryCheck : public QgsGeometryCheck
{
  public:

    /**
     * Creates a new single geometry check.
     */
    QgsSingleGeometryCheck( const QgsGeometryCheckContext *context,
                            const QVariantMap &configuration )
      : QgsGeometryCheck( context, configuration )
    {}

    void collectErrors( const QMap<QString, QgsFeaturePool *> &featurePools,
                        QList<QgsGeometryCheckError *> &errors,
                        QStringList &messages,
                        QgsFeedback *feedback = nullptr,
                        const QgsGeometryCheck::LayerFeatureIds &ids = QgsGeometryCheck::LayerFeatureIds() ) const FINAL;

    /**
     * Check the \a geometry for errors. It may make use of \a configuration options.
     *
     * Returns a list of QgsSingleGeometryCheckErrors, ownership is transferred to the caller.
     * An empty list is returned for geometries without errors.
     *
     * \since QGIS 3.4
     */
    virtual QList<QgsSingleGeometryCheckError *> processGeometry( const QgsGeometry &geometry ) const = 0;

  private:

    /**
     * Converts a QgsSingleGeometryCheckError to a QgsGeometryCheckErrorSingle.
     *
     * \since QGIS 3.4
     */
    QgsGeometryCheckErrorSingle *convertToGeometryCheckError( QgsSingleGeometryCheckError *singleGeometryCheckError, const QgsGeometryCheckerUtils::LayerFeature &layerFeature ) const;

};

#endif // QGSSINGLEGEOMETRYCHECK_H
