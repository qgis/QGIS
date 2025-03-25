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
#include "qgslinesymbol.h"
#include "qgsprofilerequest.h"
#include "qgsabstractprofilegenerator.h"
#include "qgsrange.h"

#include <QObject>
#include <QFutureWatcher>

class QgsAbstractProfileSource;
class QgsAbstractProfileGenerator;
class QgsAbstractProfileResults;
class QgsFeedback;
class QgsRenderContext;
class QgsProfileSnapResult;
class QgsProfileSnapContext;
class QgsProfilePoint;

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

    /**
     * Constructor for QgsProfilePlotRenderer, using the provided list of profile \a generators to generate the
     * results.
     *
     * \note Not available in Python bindings
     *
     * \since QGIS 3.32
     */
    QgsProfilePlotRenderer( std::vector<std::unique_ptr<QgsAbstractProfileGenerator> > generators,
                            const QgsProfileRequest &request ) SIP_SKIP;

    ~QgsProfilePlotRenderer() override;

    /**
     * Returns the ordered list of source IDs for the sources used by the renderer.
     */
    QStringList sourceIds() const;

    /**
     * Start the generation job and immediately return.
     * Does nothing if the generation is already in progress.
     */
    void startGeneration();

    /**
     * Generate the profile results synchronously in this thread. The function does not return until the generation
     * is complete.
     *
     * This is an alternative to ordinary API (using startGeneration() + waiting for generationFinished() signal).
     * Users are discouraged to use this method unless they have a strong reason for doing it.
     * The synchronous generation blocks the main thread, making the application unresponsive.
     * Also, it is not possible to cancel generation while it is in progress.
     *
     * \since QGIS 3.30
     */
    void generateSynchronously();

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
     * Sets the \a context in which the profile generation will occur.
     *
     * Depending on the sources present, this may trigger automatically a regeneration of results.
     */
    void setContext( const QgsProfileGenerationContext &context );

    /**
     * Invalidates previous results from all refinable sources.
     */
    void invalidateAllRefinableSources();

    /**
     * Replaces the existing source with matching ID.
     *
     * The matching stored source will be deleted and replaced with \a source.
     */
    void replaceSource( QgsAbstractProfileSource *source );

    /**
     * Invalidates the profile results from the source with matching ID.
     *
     * The matching stored source will be deleted and replaced with \a source.
     *
     * Returns TRUE if results were previously stored for the matching source and have been invalidated.
     *
     * \see regenerateInvalidatedResults()
     */
    bool invalidateResults( QgsAbstractProfileSource *source );

    /**
     * Starts a background regeneration of any invalidated results and immediately returns.
     *
     * Does nothing if the generation is already in progress.
     *
     * \see invalidateResults()
     */
    void regenerateInvalidatedResults();

    /**
     * Returns the limits of the retrieved elevation values.
     */
    QgsDoubleRange zRange() const;

    /**
     * Renders a portion of the profile to an image with the given \a width and \a height.
     *
     * If \a sourceId is empty then all sources will be rendered, otherwise only the matching source will be rendered.
     */
    QImage renderToImage( int width, int height, double distanceMin, double distanceMax, double zMin, double zMax, const QString &sourceId = QString(), double devicePixelRatio = 1.0 );

    /**
     * Renders a portion of the profile using the specified render \a context.
     *
     * If \a sourceId is empty then all sources will be rendered, otherwise only the matching source will be rendered.
     */
    void render( QgsRenderContext &context, double width, double height, double distanceMin, double distanceMax, double zMin, double zMax, const QString &sourceId = QString() );

    /**
     * Returns the default line symbol to use for subsections lines.
     *
     * \see setSubsectionsSymbol()
     * \since QGIS 3.44
     */
    static std::unique_ptr<QgsLineSymbol> defaultSubSectionsSymbol() SIP_FACTORY;

    /**
     * Returns the line symbol used to draw the subsections.
     *
     * \see setSubsectionsSymbol()
     * \since QGIS 3.44
     */
    QgsLineSymbol *subsectionsSymbol();

    /**
     * Sets the \a symbol used to draw the subsections. If \a symbol is NULLPTR, the subsections are not drawn.
     * Ownership of \a symbol is transferred.
     *
     * \see subsectionsSymbol()
     * \since QGIS 3.44
     */
    void setSubsectionsSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Renders the vertices of the profile curve as vertical lines using the specified render \a context.
     * The style of the lines the style corresponds to the symbol defined by setSubsectionsSymbol().
     *
     * \see setSubsectionsSymbol()
     * \since QGIS 3.44
     */
    void renderSubsectionsIndicator( QgsRenderContext &context, const QRectF &plotArea, double distanceMin, double distanceMax, double zMin, double zMax );

    /**
     * Snap a \a point to the results.
     */
    QgsProfileSnapResult snapPoint( const QgsProfilePoint &point, const QgsProfileSnapContext &context );

    /**
     * Identify results visible at the specified profile \a point.
     */
    QVector<QgsProfileIdentifyResults> identify( const QgsProfilePoint &point, const QgsProfileIdentifyContext &context );

    /**
     * Identify results visible within the specified ranges.
     */
    QVector<QgsProfileIdentifyResults> identify( const QgsDoubleRange &distanceRange, const QgsDoubleRange &elevationRange, const QgsProfileIdentifyContext &context );

    /**
     * Exports the profile results as a set of features.
     *
     * \since QGIS 3.32
     */
    QVector< QgsAbstractProfileResults::Feature > asFeatures( Qgis::ProfileExportType type, QgsFeedback *feedback = nullptr );

  signals:

    //! Emitted when the profile generation is finished (or canceled).
    void generationFinished();

  private slots:

    void onGeneratingFinished();

  private:

    static QTransform computeRenderTransform( double width, double height, double distanceMin, double distanceMax, double zMin, double zMax );

    struct ProfileJob
    {
      QgsAbstractProfileGenerator *generator = nullptr;
      QgsProfileGenerationContext context;
      std::unique_ptr< QgsAbstractProfileResults > results;
      std::unique_ptr< QgsAbstractProfileResults > invalidatedResults;
      bool complete = false;
      QMutex mutex;
    };

    static void generateProfileStatic( std::unique_ptr< ProfileJob > &job );
    bool replaceSourceInternal( QgsAbstractProfileSource *source, bool clearPreviousResults );

    std::vector< std::unique_ptr< QgsAbstractProfileGenerator > > mGenerators;
    QgsProfileRequest mRequest;
    QgsProfileGenerationContext mContext;

    std::vector< std::unique_ptr< ProfileJob > > mJobs;

    QFuture<void> mFuture;
    QFutureWatcher<void> mFutureWatcher;

    enum { Idle, Generating } mStatus = Idle;

    std::unique_ptr<QgsLineSymbol> mSubsectionsSymbol;

};

#endif // QGSPROFILERENDERER_H
