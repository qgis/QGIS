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

class QgsAbstractProfileGenerator;

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

    /**
     * Copies properties from specified \a generator to the results object.
     *
     * For instance, this method can be used to copy any properties relating to rendering
     * the gathered results to reflect the \a generator's current properties.
     *
     * The base class method does nothing.
     */
    virtual void copyPropertiesFromGenerator( const QgsAbstractProfileGenerator *generator );
};

/**
 * \brief Encapsulates the context in which an elevation profile is to be generated.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProfileGenerationContext
{
  public:

    /**
     * Returns the maximum allowed error in the generated result, in profile curve map units.
     *
     * By default this is NaN, which indicates that the profile should be generated in the highest precision possible.
     * Larger values will result in a faster profile to generate.
     *
     * \see setMaximumErrorMapUnits()
     */
    double maximumErrorMapUnits() const { return mMaxErrorMapUnits; }

    /**
     * Sets the maximum allowed \a error in the generated result, in profile curve map units.
     *
     * By default this is NaN, which indicates that the profile should be generated in the highest precision possible.
     * Larger values will result in a faster profile to generate.
     *
     * \see maximumErrorMapUnits()
     */
    void setMaximumErrorMapUnits( double error ) { mMaxErrorMapUnits = error; }

    /**
     * Returns the number of map units per pixel in the distance dimension.
     *
     * \see setMapUnitsPerDistancePixel()
     */
    double mapUnitsPerDistancePixel() const { return mMapUnitsPerDistancePixel; }

    /**
     * Sets the number of map \a units per pixel in the distance dimension.
     *
     * \see mapUnitsPerDistancePixel()
     */
    void setMapUnitsPerDistancePixel( double units ) { mMapUnitsPerDistancePixel = units; }

    /**
     * Returns the range of distances to include in the generation.
     *
     * Distances outside this range may be excluded from the generation (if it results in faster profile generation).
     *
     * \see setDistanceRange()
     */
    QgsDoubleRange distanceRange() const { return mDistanceRange; }

    /**
     * Sets the \a range of distances to include in the generation.
     *
     * Distances outside this range may be excluded from the generation (if it results in faster profile generation).
     *
     * \see distanceRange()
     */
    void setDistanceRange( const QgsDoubleRange &range ) { mDistanceRange = range; }

    /**
     * Returns the range of elevations to include in the generation.
     *
     * Elevations outside this range may be excluded from the generation (if it results in faster profile generation).
     *
     * \see setElevationRange()
     */
    QgsDoubleRange elevationRange() const { return mElevationRange; }

    /**
     * Sets the \a range of elevations to include in the generation.
     *
     * Elevations outside this range may be excluded from the generation (if it results in faster profile generation).
     *
     * \see elevationRange()
     */
    void setElevationRange( const QgsDoubleRange &range ) { mElevationRange = range; }

    /**
     * Sets the \a dpi (dots per inch) for the profie, to be used in size conversions.
     *
     * \see dpi()
     */
    void setDpi( double dpi ) { mDpi = dpi; }

    /**
     * Returns the DPI (dots per inch) for the profie, to be used in size conversions.
     *
     * \see setDpi()
     */
    double dpi() const { return mDpi; }

    /**
     * Converts a distance size from the specified units to pixels.
     */
    double convertDistanceToPixels( double size, QgsUnitTypes::RenderUnit unit ) const;

    bool operator==( const QgsProfileGenerationContext &other ) const;
    bool operator!=( const QgsProfileGenerationContext &other ) const;

  private:

    double mMaxErrorMapUnits = std::numeric_limits< double >::quiet_NaN();
    double mMapUnitsPerDistancePixel = 1;
    QgsDoubleRange mDistanceRange;
    QgsDoubleRange mElevationRange;
    double mDpi = 96;
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
     *
     * For generators associated with a map layer the source ID will match the layer's QgsMapLayer::id(). Other (non-map-layer) sources
     * will have a different unique ID with its own custom interpretation.gen
     */
    virtual QString sourceId() const = 0;

    /**
     * Returns flags which reflect how the profile generator operates.
     */
    virtual Qgis::ProfileGeneratorFlags flags() const;

    /**
     * Generate the profile (based on data stored in the class).
     *
     * Returns TRUE if the profile was generated successfully (i.e. the generation
     * was not canceled early).
     */
    virtual bool generateProfile( const QgsProfileGenerationContext &context = QgsProfileGenerationContext() ) = 0;

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
