/***************************************************************************
                         qgsabstractprofilegenerator.h
                         ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
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
#ifndef QGSABSTRACTPROFILEGENERATOR_H
#define QGSABSTRACTPROFILEGENERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QList>

#include "qgspoint.h"
#include "qgsrendercontext.h"

#include <QTransform>

class QgsProfileSnapResult;
class QgsProfileSnapContext;
class QgsProfilePoint;
class QgsGeometry;

/**
 * \brief Abstract base class for storage of elevation profiles.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProfileRenderContext
{
  public:

    /**
     * Constructor for QgsProfileRenderContext, with the specified embedded render \a context.
     */
    QgsProfileRenderContext( QgsRenderContext &context );

    /**
     * Returns a reference to the component QgsRenderContext.
     */
    QgsRenderContext &renderContext() { return mRenderContext; }

    /**
     * Returns the transform from world coordinates to painter coordinates.
     *
     * This transform maps points in (distance, elevation) to (x, y) in painter coordinates.
     *
     * \see setWorldTransform()
     */
    const QTransform &worldTransform() const;

    /**
     * Sets the \a transform from world coordinates to painter coordinates.
     *
     * This transform maps points in (distance, elevation) to (x, y) in painter coordinates.
     *
     * \see worldTransform()
     */
    void setWorldTransform( const QTransform &transform );

    /**
     * Returns the range of distances to include in the render.
     *
     * Distances outside this range should be excluded from the render.
     *
     * \see setDistanceRange()
     */
    QgsDoubleRange distanceRange() const;

    /**
     * Sets the \a range of distances to include in the render.
     *
     * Distances outside this range will be excluded from the render.
     *
     * \see distanceRange()
     */
    void setDistanceRange( const QgsDoubleRange &range );

    /**
     * Returns the range of elevations to include in the render.
     *
     * Elevations outside this range should be excluded from the render.
     *
     * \see setElevationRange()
     */
    QgsDoubleRange elevationRange() const;

    /**
     * Sets the \a range of elevations to include in the render.
     *
     * Elevations outside this range will be excluded from the render.
     *
     * \see elevationRange()
     */
    void setElevationRange( const QgsDoubleRange &range );

  private:

    QgsRenderContext mRenderContext;

    QTransform mWorldTransform;

    QgsDoubleRange mDistanceRange;
    QgsDoubleRange mElevationRange;

};


/**
 * \brief Abstract base class for storage of elevation profiles.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAbstractProfileResults
{
  public:

    virtual ~QgsAbstractProfileResults();

    /**
     * Returns the unique string identifier for the results type.
     */
    virtual QString type() const = 0;

    /**
     * Returns the map of distance (chainage) to height.
     */
    virtual QMap< double, double > distanceToHeightMap() const = 0;

    /**
     * Returns a list of sampled points, with their calculated elevation
     * as the point z value.
     */
    virtual QgsPointSequence sampledPoints() const = 0;

    /**
     * Returns a list of geometries representing the calculated elevation results.
     */
    virtual QVector< QgsGeometry > asGeometries() const = 0;

    /**
     * Renders the results to the specified \a context.
     */
    virtual void renderResults( QgsProfileRenderContext &context ) = 0;

    /**
     * Returns the range of the retrieved elevation values
     */
    virtual QgsDoubleRange zRange() const = 0;

    /**
     * Snaps a \a point to the generated elevation profile.
     */
    virtual QgsProfileSnapResult snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context );
};

/**
 * \brief Abstract base class for objects which generate elevation profiles.
 *
 * The generation is typically done in a background
 * thread, so it is necessary to keep all structures required for generating the
 * profile away from the original profile source because it may change at any time.
 *
 * Because the data needs to be copied (to avoid the need for locking),
 * it is highly desirable to use copy-on-write where possible. This way,
 * the overhead of copying (both memory and CPU) will be kept low.
 * Qt containers and various Qt classes use implicit sharing.
 *
 * The scenario will be:
 *
 * # elevation profile job (doing preparation in the GUI thread) calls
 *   QgsAbstractProfileSource::createProfileGenerator() and gets an instance of this class.
 *   The instance is initialized at that point and should not need
 *   additional calls to the source.
 * # profile job (still in GUI thread) stores the generator for later use.
 * # profile job (in worker thread) calls QgsAbstractProfileGenerator::generateProfile()
 * # profile job (again in GUI thread) will check errors() and report them
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsAbstractProfileGenerator
{

  public:

    virtual ~QgsAbstractProfileGenerator();

    /**
     * Returns a unique identifier representing the source of the profile.
     */
    virtual QString sourceId() const = 0;

    /**
     * Generate the profile (based on data stored in the class).
     *
     * Returns TRUE if the profile was generated successfully (i.e. the generation
     * was not canceled early).
     */
    virtual bool generateProfile() = 0;

    /**
     * Access to feedback object of the generator (may be NULLPTR)
     */
    virtual QgsFeedback *feedback() const = 0;

    /**
     * Takes results from the generator.
     *
     * Ownership is transferred to the caller.
     */
    virtual QgsAbstractProfileResults *takeResults() = 0 SIP_TRANSFERBACK;

};

#endif // QGSABSTRACTPROFILEGENERATOR_H
