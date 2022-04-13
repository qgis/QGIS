/***************************************************************************
                         qgsprofilerenderer.h
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
#ifndef QGSPROFILERENDERER_H
#define QGSPROFILERENDERER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsprofilerequest.h"
#include "qgsrange.h"

#include <QObject>
#include <QFutureWatcher>

class QgsAbstractProfileSource;
class QgsAbstractProfileGenerator;
class QgsAbstractProfileResults;
class QgsFeedback;
class QgsRenderContext;

/**
 * \brief Generates and renders elevation profile plots.
 *
 * This class has two roles:
 *
 * 1. Extraction and generation of the raw elevation profiles from a list of QgsAbstractProfileSource objects.
 * 2. Rendering the results
 *
 * Step 1, involving the generation of the elevation profiles only needs to occur once. This is done via
 * a call to startGeneration(), which commences generation of the profiles from each source in a separate
 * background thread. When the generation is completed for all sources the generationFinished() signal is
 * emitted.
 *
 * After the profile is generated, it can be rendered. The rendering step may be undertaken multiple times
 * (e.g. to render to different image sizes or data ranges) without having to re-generate the raw profile
 * data.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProfilePlotRenderer : public QObject
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsProfilePlotRenderer, using the provided list of profile \a sources to generate the
     * results.
     */
    QgsProfilePlotRenderer( const QList< QgsAbstractProfileSource * > &sources,
                            const QgsProfileRequest &request );

    ~QgsProfilePlotRenderer() override;

    /**
     * Start the generation job and immediately return.
     * Does nothing if the generation is already in progress.
     */
    void startGeneration();

    /**
     * Stop the generation job - does not return until the job has terminated.
     * Does nothing if the generation is not active.
     */
    void cancelGeneration();

    /**
     * Triggers cancellation of the generation job without blocking. The generation job will continue
     * to operate until it is able to cancel, at which stage the generationFinished() signal will be emitted.
     * Does nothing if the generation is not active.
     */
    void cancelGenerationWithoutBlocking();

    //! Block until the current job has finished.
    void waitForFinished();

    //! Returns TRUE if the generation job is currently running in background.
    bool isActive() const;

    /**
     * Returns the limits of the retrieved elevation values.
     */
    QgsDoubleRange zRange() const;

    /**
     * Renders a portion of the profile to an image with the given \a width and \a height.
     */
    QImage renderToImage( int width, int height, double distanceMin, double distanceMax, double zMin, double zMax );

    /**
     * Renders a portion of the profile using the specified render \a context.
     */
    void render( QgsRenderContext &context, double width, double height, double distanceMin, double distanceMax, double zMin, double zMax );

    /**
     * \brief Encapsulates results of snapping a point to the profile.
     * \ingroup core
     */
    class SnapResult
    {
      public:
        //! Snapped point distance along curve
        double snappedDistanceAlongCurve = std::numeric_limits< double >::quiet_NaN();

        //! Snapped point height
        double snappedHeight = std::numeric_limits< double >::quiet_NaN();

        //! Distance snapped
        double snapDistance = std::numeric_limits< double >::max();

        /**
         * Returns TRUE if the result is a valid point.
         */
        bool isValid() const { return !std::isnan( snappedDistanceAlongCurve ); }
    };

    /**
     * Snap a point to the results.
     *
     * The \a maximumCurveDelta and \a maximumHeightDelta arguments specify the maximum allowed difference in the distance along
     * curve and height values from the snapped result.
     */
    SnapResult snapPoint( double distanceAlongCurve, double height, double maximumCurveDelta, double maximumHeightDelta );

  signals:

    //! Emitted when the profile generation is finished (or canceled).
    void generationFinished();

  private slots:

    void onGeneratingFinished();

  private:

    struct ProfileJob
    {
      QgsAbstractProfileGenerator *generator = nullptr;
      std::unique_ptr< QgsAbstractProfileResults > results;
      bool complete = false;
    };

    static void generateProfileStatic( ProfileJob &job );

    std::vector< std::unique_ptr< QgsAbstractProfileGenerator > > mGenerators;
    QgsProfileRequest mRequest;

    std::vector< ProfileJob > mJobs;

    QFuture<void> mFuture;
    QFutureWatcher<void> mFutureWatcher;

    enum { Idle, Generating } mStatus = Idle;


};

#endif // QGSPROFILERENDERER_H
