/***************************************************************************
                         qgsprocessingcontext.h
                         ----------------------
    begin                : April 2017
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

#ifndef QGSPROCESSINGCONTEXT_H
#define QGSPROCESSINGCONTEXT_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsproject.h"
#include "qgsexpressioncontext.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingutils.h"
#include "qgsprocessingmodelresult.h"
#include "qgsprocessingmodelconfig.h"

#include <QThread>
#include <QPointer>

class QgsProcessingLayerPostProcessorInterface;

/**
 * \class QgsProcessingContext
 * \ingroup core
 * \brief Contains information about the context in which a processing algorithm is executed.
 *
 * Contextual information includes settings such as the associated project, and
 * expression context.
*/

class CORE_EXPORT QgsProcessingContext
{
  public:

    //! Flags that affect how processing algorithms are run
    enum Flag SIP_ENUM_BASETYPE( IntFlag )
    {
      // For future API flexibility only and to avoid sip issues, remove when real entries are added to flags.
      Unused = 1 << 0, //!< Temporary unused entry
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    QgsProcessingContext();

    QgsProcessingContext( const QgsProcessingContext &other ) = delete;
    QgsProcessingContext &operator=( const QgsProcessingContext &other ) = delete;

    ~QgsProcessingContext();

    /**
     * Copies all settings which are safe for use across different threads from
     * \a other to this context.
     */
    void copyThreadSafeSettings( const QgsProcessingContext &other )
    {
      mFlags = other.mFlags;
      mProject = other.mProject;
      mTransformContext = other.mTransformContext;
      mExpressionContext = other.mExpressionContext;
      mExpressionContext.setLoadedLayerStore( &tempLayerStore );
      mInvalidGeometryCallback = other.mInvalidGeometryCallback;
      mUseDefaultInvalidGeometryCallback = other.mUseDefaultInvalidGeometryCallback;
      mInvalidGeometryCheck = other.mInvalidGeometryCheck;
      mTransformErrorCallback = other.mTransformErrorCallback;
      mDefaultEncoding = other.mDefaultEncoding;
      mFeedback = other.mFeedback;
      mPreferredVectorFormat = other.mPreferredVectorFormat;
      mPreferredRasterFormat = other.mPreferredRasterFormat;
      mEllipsoid = other.mEllipsoid;
      mDistanceUnit = other.mDistanceUnit;
      mAreaUnit = other.mAreaUnit;
      mLogLevel = other.mLogLevel;
      mTemporaryFolderOverride = other.mTemporaryFolderOverride;
      mMaximumThreads = other.mMaximumThreads;
      mModelResult = other.mModelResult;
    }

    /**
     * Returns any flags set in the context.
     * \see setFlags()
     */
    QgsProcessingContext::Flags flags() const SIP_HOLDGIL { return mFlags; }

    /**
     * Sets \a flags for the context.
     * \see flags()
     */
    void setFlags( QgsProcessingContext::Flags flags ) SIP_HOLDGIL { mFlags = flags; }

    /**
     * Returns the project in which the algorithm is being executed.
     * \see setProject()
     */
    QgsProject *project() const SIP_HOLDGIL { return mProject; }

    /**
     * Sets the \a project in which the algorithm will be executed.
     *
     * This also automatically sets the transformContext(), ellipsoid(), distanceUnit() and
     * areaUnit() to match the project's settings.
     *
     * \see project()
     */
    void setProject( QgsProject *project ) SIP_HOLDGIL
    {
      mProject = project;
      if ( mProject )
      {
        mTransformContext = mProject->transformContext();
        if ( mEllipsoid.isEmpty() )
          mEllipsoid = mProject->ellipsoid();
        if ( mDistanceUnit == Qgis::DistanceUnit::Unknown )
          mDistanceUnit = mProject->distanceUnits();
        if ( mAreaUnit == Qgis::AreaUnit::Unknown )
          mAreaUnit = mProject->areaUnits();
      }
    }

    /**
     * Returns the expression context.
     */
    QgsExpressionContext &expressionContext() SIP_HOLDGIL { return mExpressionContext; }

    /**
     * Returns the expression context.
     */
    SIP_SKIP const QgsExpressionContext &expressionContext() const { return mExpressionContext; }

    /**
     * Sets the expression \a context.
     */
    void setExpressionContext( const QgsExpressionContext &context );

    /**
     * Returns the coordinate transform context.
     * \see setTransformContext()
     */
    QgsCoordinateTransformContext transformContext() const SIP_HOLDGIL { return mTransformContext; }

    /**
     * Sets the coordinate transform \a context.
     *
     * Note that setting a project for the context will automatically set the coordinate transform
     * context.
     *
     * \see transformContext()
     */
    void setTransformContext( const QgsCoordinateTransformContext &context ) SIP_HOLDGIL { mTransformContext = context; }

    /**
     * Returns the ellipsoid to use for distance and area calculations.
     *
     * \see setEllipsoid()
     * \since QGIS 3.16
     */
    QString ellipsoid() const SIP_HOLDGIL;

    /**
     * Sets a specified \a ellipsoid to use for distance and area calculations.
     *
     * If not explicitly set, the ellipsoid will default to the project()'s ellipsoid setting.
     *
     * \see ellipsoid()
     * \since QGIS 3.16
     */
    void setEllipsoid( const QString &ellipsoid ) SIP_HOLDGIL;

    /**
     * Returns the distance unit to use for distance calculations.
     *
     * \see setDistanceUnit()
     * \see areaUnit()
     * \since QGIS 3.16
     */
    Qgis::DistanceUnit distanceUnit() const SIP_HOLDGIL;

    /**
     * Sets the \a unit to use for distance calculations.
     *
     * If not explicitly set, the unit will default to the project()'s distance unit setting.
     *
     * \see distanceUnit()
     * \see setAreaUnit()
     * \since QGIS 3.16
     */
    void setDistanceUnit( Qgis::DistanceUnit unit ) SIP_HOLDGIL;

    /**
     * Returns the area unit to use for area calculations.
     *
     * \see setAreaUnit()
     * \see distanceUnit()
     * \since QGIS 3.16
     */
    Qgis::AreaUnit areaUnit() const SIP_HOLDGIL;

    /**
     * Sets the \a unit to use for area calculations.
     *
     * If not explicitly set, the unit will default to the project()'s area unit setting.
     *
     * \see areaUnit()
     * \see setDistanceUnit()
     * \since QGIS 3.16
     */
    void setAreaUnit( Qgis::AreaUnit areaUnit ) SIP_HOLDGIL;

    /**
     * Returns the current time range to use for temporal operations.
     *
     * \see setCurrentTimeRange()
     * \since QGIS 3.18
     */
    QgsDateTimeRange currentTimeRange() const SIP_HOLDGIL;

    /**
     * Sets the \a current time range to use for temporal operations.
     *
     * \see currentTimeRange()
     * \since QGIS 3.18
     */
    void setCurrentTimeRange( const QgsDateTimeRange &currentTimeRange ) SIP_HOLDGIL;

    /**
     * Returns a reference to the layer store used for storing temporary layers during
     * algorithm execution.
     */
    QgsMapLayerStore *temporaryLayerStore() SIP_HOLDGIL { return &tempLayerStore; }

    /**
     * \brief Details for layers to load into projects.
     * \ingroup core
     */
    class CORE_EXPORT LayerDetails
    {
      public:

        /**
         * Constructor for LayerDetails.
         */
        LayerDetails( const QString &name, QgsProject *project, const QString &outputName = QString(), QgsProcessingUtils::LayerHint layerTypeHint = QgsProcessingUtils::LayerHint::UnknownType )
          : name( name )
          , outputName( outputName )
          , layerTypeHint( layerTypeHint )
          , project( project )
        {}

        LayerDetails() = default;

        /**
         * Friendly name for layer, possibly for use when loading layer into project.
         *
         * \warning Instead of directly using this value, prefer to call setOutputLayerName() to
         * generate a layer name which respects the user's local Processing settings.
         */
        QString name;

        /**
         * Set to TRUE if LayerDetails::name should always be used as the loaded layer name, regardless
         * of the user's local Processing settings.
         * \since QGIS 3.16
         */
        bool forceName = false;

        /**
         * Associated output name from algorithm which generated the layer.
         */
        QString outputName;

        /**
         * Optional name for a layer tree group under which to place the layer when loading it into a project.
         *
         * \since QGIS 3.32
         */
        QString groupName;

        /**
         * Optional sorting key for sorting output layers when loading them into a project.
         *
         * Layers with a greater sort key will be placed over layers with a lesser sort key.
         *
         * \since QGIS 3.32
         */
        int layerSortKey = 0;

        /**
         * Layer type hint.
         *
         * \since QGIS 3.4
         */
        QgsProcessingUtils::LayerHint layerTypeHint = QgsProcessingUtils::LayerHint::UnknownType;

        /**
         * Layer post-processor. May be NULLPTR if no post-processing is required.
         * \see setPostProcessor()
         * \since QGIS 3.2
         */
        QgsProcessingLayerPostProcessorInterface *postProcessor() const;

        /**
         * Sets the layer post-processor. May be NULLPTR if no post-processing is required.
         *
         * Ownership of \a processor is transferred.
         *
         * \see postProcessor()
         * \since QGIS 3.2
         */
        void setPostProcessor( QgsProcessingLayerPostProcessorInterface *processor SIP_TRANSFER );

        /**
         * Sets a \a layer name to match this output, respecting any local user settings which affect this name.
         *
         * \since QGIS 3.10.1
         */
        void setOutputLayerName( QgsMapLayer *layer ) const;

        //! Destination project
        QgsProject *project = nullptr;

      private:

        // Ideally a unique_ptr, but cannot be due to use within QMap. Is cleaned up by QgsProcessingContext.
        QgsProcessingLayerPostProcessorInterface *mPostProcessor = nullptr;

    };

    /**
     * Returns a map of layers (by ID or datasource) to LayerDetails, to load into the canvas upon completion of the algorithm or model.
     * \see setLayersToLoadOnCompletion()
     * \see addLayerToLoadOnCompletion()
     * \see willLoadLayerOnCompletion()
     * \see layerToLoadOnCompletionDetails()
     */
    QMap< QString, QgsProcessingContext::LayerDetails > layersToLoadOnCompletion() const SIP_HOLDGIL
    {
      return mLayersToLoadOnCompletion;
    }

    /**
     * Returns TRUE if the given \a layer (by ID or datasource) will be loaded into the current project
     * upon completion of the algorithm or model.
     * \see layersToLoadOnCompletion()
     * \see setLayersToLoadOnCompletion()
     * \see addLayerToLoadOnCompletion()
     * \see layerToLoadOnCompletionDetails()
     * \since QGIS 3.2
     */
    bool willLoadLayerOnCompletion( const QString &layer ) const SIP_HOLDGIL
    {
      return mLayersToLoadOnCompletion.contains( layer );
    }

    /**
     * Sets the map of \a layers (by ID or datasource) to LayerDetails, to load into the canvas upon completion of the algorithm or model.
     * \see addLayerToLoadOnCompletion()
     * \see layersToLoadOnCompletion()
     * \see willLoadLayerOnCompletion()
     * \see layerToLoadOnCompletionDetails()
     */
    void setLayersToLoadOnCompletion( const QMap< QString, QgsProcessingContext::LayerDetails > &layers ) SIP_HOLDGIL;

    /**
     * Adds a \a layer to load (by ID or datasource) into the canvas upon completion of the algorithm or model.
     * The \a details parameter dictates the LayerDetails.
     * \see setLayersToLoadOnCompletion()
     * \see layersToLoadOnCompletion()
     * \see willLoadLayerOnCompletion()
     * \see layerToLoadOnCompletionDetails()
     */
    void addLayerToLoadOnCompletion( const QString &layer, const QgsProcessingContext::LayerDetails &details ) SIP_HOLDGIL;

    /**
     * Returns a reference to the details for a given \a layer which is loaded on completion of the
     * algorithm or model.
     *
     * \warning First check willLoadLayerOnCompletion(), or calling this method will incorrectly
     * add \a layer as a layer to load on completion.
     *
     * \see willLoadLayerOnCompletion()
     * \see addLayerToLoadOnCompletion()
     * \see setLayersToLoadOnCompletion()
     * \see layersToLoadOnCompletion()
     * \since QGIS 3.2
     */
    QgsProcessingContext::LayerDetails &layerToLoadOnCompletionDetails( const QString &layer ) SIP_HOLDGIL
    {
      return mLayersToLoadOnCompletion[ layer ];
    }

    /**
     * Returns the behavior used for checking invalid geometries in input layers.
     * \see setInvalidGeometryCheck()
     */
    Qgis::InvalidGeometryCheck invalidGeometryCheck() const SIP_HOLDGIL { return mInvalidGeometryCheck; }

    /**
     * Sets the behavior used for checking invalid geometries in input layers.
     * Settings this to anything but QgsFeatureRequest::GeometryNoCheck will also
     * reset the invalidGeometryCallback() to a default implementation.
     * \see invalidGeometryCheck()
     */
    void setInvalidGeometryCheck( Qgis::InvalidGeometryCheck check );

    /**
     * Sets a callback function to use when encountering an invalid geometry and
     * invalidGeometryCheck() is set to GeometryAbortOnInvalid. This function will be
     * called using the feature with invalid geometry as a parameter.
     * \see invalidGeometryCallback()
     */
#ifndef SIP_RUN
    void setInvalidGeometryCallback( const std::function< void( const QgsFeature & ) > &callback ) { mInvalidGeometryCallback = callback; mUseDefaultInvalidGeometryCallback = false; }
#else
    void setInvalidGeometryCallback( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setInvalidGeometryCallback( [a0]( const QgsFeature &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QgsFeature, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    Py_END_ALLOW_THREADS
    % End
#endif

    /**
     * Returns the callback function to use when encountering an invalid geometry and
     * invalidGeometryCheck() is set to GeometryAbortOnInvalid.
     * \note not available in Python bindings
     * \see setInvalidGeometryCallback()
     */
    SIP_SKIP std::function< void( const QgsFeature & ) > invalidGeometryCallback( QgsFeatureSource *source = nullptr ) const;

    /**
     * Returns the default callback function to use for a particular invalid geometry \a check
     * \note not available in Python bindings
     * \since QGIS 3.14
     */
    SIP_SKIP std::function< void( const QgsFeature & ) > defaultInvalidGeometryCallbackForCheck( Qgis::InvalidGeometryCheck check, QgsFeatureSource *source = nullptr ) const;

    /**
     * Sets a callback function to use when encountering a transform error when iterating
     * features. This function will be
     * called using the feature which encountered the transform error as a parameter.
     * \see transformErrorCallback()
     */
#ifndef SIP_RUN
    void setTransformErrorCallback( const std::function< void( const QgsFeature & ) > &callback ) { mTransformErrorCallback = callback; }
#else
    void setTransformErrorCallback( SIP_PYCALLABLE / AllowNone / );
    % MethodCode
    Py_BEGIN_ALLOW_THREADS

    sipCpp->setTransformErrorCallback( [a0]( const QgsFeature &arg )
    {
      SIP_BLOCK_THREADS
      Py_XDECREF( sipCallMethod( NULL, a0, "D", &arg, sipType_QgsFeature, NULL ) );
      SIP_UNBLOCK_THREADS
    } );

    Py_END_ALLOW_THREADS
    % End
#endif

    /**
     * Returns the callback function to use when encountering a transform error when iterating
     * features.
     * \note not available in Python bindings
     * \see setTransformErrorCallback()
     */
    std::function< void( const QgsFeature & ) > transformErrorCallback() const { return mTransformErrorCallback; } SIP_SKIP

    /**
     * Returns the default encoding to use for newly created files.
     * \see setDefaultEncoding()
     */
    QString defaultEncoding() const SIP_HOLDGIL { return mDefaultEncoding; }

    /**
     * Sets the default \a encoding to use for newly created files.
     * \see defaultEncoding()
     */
    void setDefaultEncoding( const QString &encoding ) SIP_HOLDGIL { mDefaultEncoding = encoding; }

    /**
     * Returns the associated feedback object.
     * \see setFeedback()
     */
    QgsProcessingFeedback *feedback() SIP_HOLDGIL { return mFeedback; }

    /**
     * Sets an associated \a feedback object. This allows context related functions
     * to report feedback and errors to users and processing logs. While ideally this feedback
     * object should outlive the context, only a weak pointer to \a feedback is stored
     * and no errors will occur if feedback is deleted before the context.
     * Ownership of \a feedback is not transferred.
     * \see setFeedback()
     */
    void setFeedback( QgsProcessingFeedback *feedback ) SIP_HOLDGIL { mFeedback = feedback; }

    /**
     * Returns the thread in which the context lives.
     * \see pushToThread()
     */
    QThread *thread() SIP_HOLDGIL { return tempLayerStore.thread(); }

    /**
     * Pushes the thread affinity for the context (including all layers contained in the temporaryLayerStore()) into
     * another \a thread. This method is only safe to call when the current thread matches the existing thread
     * affinity for the context (see thread()).
     * \see thread()
     */
    void pushToThread( QThread *thread )
    {
      // cppcheck-suppress assertWithSideEffect
      Q_ASSERT_X( QThread::currentThread() == QgsProcessingContext::thread(), "QgsProcessingContext::pushToThread", "Cannot push context to another thread unless the current thread matches the existing context thread affinity" );
      tempLayerStore.moveToThread( thread );
    }

    /**
     * Takes the results from another \a context and merges them with the results currently
     * stored in this context. This includes settings like any layers loaded in the temporaryLayerStore()
     * and layersToLoadOnCompletion().
     * This is only safe to call when both this context and the other \a context share the same
     * thread() affinity, and that thread is the current thread.
     */
    void takeResultsFrom( QgsProcessingContext &context );

    /**
     * Returns a map layer from the context with a matching \a identifier.
     * This method considers layer IDs, names and sources when matching
     * the \a identifier (see QgsProcessingUtils::mapLayerFromString()
     * for details).
     *
     * Ownership is not transferred and remains with the context.
     *
     * \see takeResultLayer()
     */
    QgsMapLayer *getMapLayer( const QString &identifier );

    /**
     * Takes the result map layer with matching \a id from the context and
     * transfers ownership of it back to the caller. This method can be used
     * to remove temporary layers which are not required for further processing
     * from a context.
     *
     * \see getMapLayer()
     */
    QgsMapLayer *takeResultLayer( const QString &id ) SIP_TRANSFERBACK;

    /**
     * Returns the preferred vector format to use for vector outputs.
     *
     * This method returns a file extension to use when creating vector outputs (e.g. "shp"). Generally,
     * it is preferable to use the extension associated with a particular parameter, which can be retrieved through
     * QgsProcessingDestinationParameter::defaultFileExtension(). However, in some cases, a specific parameter
     * may not be available to call this method on (e.g. for an algorithm which has only an output folder parameter
     * and which creates multiple output layers in that folder). In this case, the format returned by this
     * function should be used when creating these outputs.
     *
     * It is the algorithm's responsibility to check whether the returned format is acceptable for the algorithm,
     * and to provide an appropriate fallback when the returned format is not usable.
     *
     * \see setPreferredVectorFormat()
     * \see preferredRasterFormat()
     *
     * \since QGIS 3.10
     */
    QString preferredVectorFormat() const SIP_HOLDGIL { return mPreferredVectorFormat; }

    /**
     * Sets the preferred vector \a format to use for vector outputs.
     *
     * This method sets a file extension to use when creating vector outputs (e.g. "shp"). Generally,
     * it is preferable to use the extension associated with a particular parameter, which can be retrieved through
     * QgsProcessingDestinationParameter::defaultFileExtension(). However, in some cases, a specific parameter
     * may not be available to call this method on (e.g. for an algorithm which has only an output folder parameter
     * and which creates multiple output layers in that folder). In this case, the format set by this
     * function will be used when creating these outputs.
     *
     * \see preferredVectorFormat()
     * \see setPreferredRasterFormat()
     *
     * \since QGIS 3.10
     */
    void setPreferredVectorFormat( const QString &format ) SIP_HOLDGIL { mPreferredVectorFormat = format; }

    /**
     * Returns the preferred raster format to use for vector outputs.
     *
     * This method returns a file extension to use when creating raster outputs (e.g. "tif"). Generally,
     * it is preferable to use the extension associated with a particular parameter, which can be retrieved through
     * QgsProcessingDestinationParameter::defaultFileExtension(). However, in some cases, a specific parameter
     * may not be available to call this method on (e.g. for an algorithm which has only an output folder parameter
     * and which creates multiple output layers in that folder). In this case, the format returned by this
     * function should be used when creating these outputs.
     *
     * It is the algorithm's responsibility to check whether the returned format is acceptable for the algorithm,
     * and to provide an appropriate fallback when the returned format is not usable.
     *
     * \see setPreferredRasterFormat()
     * \see preferredVectorFormat()
     *
     * \since QGIS 3.10
     */
    QString preferredRasterFormat() const SIP_HOLDGIL { return mPreferredRasterFormat; }

    /**
     * Sets the preferred raster \a format to use for vector outputs.
     *
     * This method sets a file extension to use when creating raster outputs (e.g. "tif"). Generally,
     * it is preferable to use the extension associated with a particular parameter, which can be retrieved through
     * QgsProcessingDestinationParameter::defaultFileExtension(). However, in some cases, a specific parameter
     * may not be available to call this method on (e.g. for an algorithm which has only an output folder parameter
     * and which creates multiple output layers in that folder). In this case, the format set by this
     * function will be used when creating these outputs.
     *
     * \see preferredRasterFormat()
     * \see setPreferredVectorFormat()
     *
     * \since QGIS 3.10
     */
    void setPreferredRasterFormat( const QString &format ) SIP_HOLDGIL { mPreferredRasterFormat = format; }

    /**
     * Returns the logging level for algorithms to use when pushing feedback messages to users.
     *
     * \see setLogLevel()
     * \since QGIS 3.20
     */
    Qgis::ProcessingLogLevel logLevel() const SIP_HOLDGIL;

    /**
     * Sets the logging \a level for algorithms to use when pushing feedback messages to users.
     *
     * \see logLevel()
     * \since QGIS 3.20
     */
    void setLogLevel( Qgis::ProcessingLogLevel level ) SIP_HOLDGIL;

    /**
     * Returns the (optional) temporary folder to use when running algorithms.
     *
     * If set, this overrides the standard global Processing temporary folder and should be used
     * for all temporary files created during algorithm execution.
     *
     * \see setTemporaryFolder()
     * \since QGIS 3.32
     */
    QString temporaryFolder() const SIP_HOLDGIL;

    /**
     * Sets the (optional) temporary \a folder to use when running algorithms.
     *
     * If set, this overrides the standard global Processing temporary folder and should be used
     * for all temporary files created during algorithm execution.
     *
     * \see temporaryFolder()
     * \since QGIS 3.32
     */
    void setTemporaryFolder( const QString &folder ) SIP_HOLDGIL;

    /**
     * Returns the (optional) number of threads to use when running algorithms.
     *
     * \warning Not all algorithms which support multithreaded execution will
     * respect this setting, depending on the multi-threading framework in use.
     * Multithreaded algorithms must check this value and adapt their thread
     * handling accordingly -- the setting will not be automatically applied.
     *
     * \see setMaximumThreads()
     * \since QGIS 3.32
     */
    int maximumThreads() const SIP_HOLDGIL;

    /**
     * Sets the (optional) number of \a threads to use when running algorithms.
     *
     * If set, this overrides the standard global Processing number of threads setting.
     * Note that if algorithm implementation does not support multhreaded execution, this
     * setting will be ignored.
     *
     * \warning Not all algorithms which support multithreaded execution will
     * respect this setting, depending on the multi-threading framework in use.
     * Multithreaded algorithms must check this value and adapt their thread
     * handling accordingly -- the setting will not be automatically applied.
     *
     * \see maximumThreads()
     * \since QGIS 3.32
     */
    void setMaximumThreads( int threads ) SIP_HOLDGIL;

    /**
     * Exports the context's settings to a variant map.
     *
     * \since QGIS 3.24
     */
    QVariantMap exportToMap() const SIP_HOLDGIL;

    /**
     * Flags controlling the results given by asQgisProcessArguments().
     *
     * \since QGIS 3.24
     */
    enum class ProcessArgumentFlag : int SIP_ENUM_BASETYPE( IntFlag )
    {
      IncludeProjectPath = 1 << 0, //!< Include the associated project path argument
    };
    Q_DECLARE_FLAGS( ProcessArgumentFlags, ProcessArgumentFlag )

    /**
     * Returns list of the equivalent qgis_process arguments representing the settings from the context.
     *
     * \since QGIS 3.24
     */
    QStringList asQgisProcessArguments( QgsProcessingContext::ProcessArgumentFlags flags = QgsProcessingContext::ProcessArgumentFlags() ) const;

    /**
     * Returns a reference to the model initial run configuration, used
     * to run a model algorithm.
     *
     * This may be NULLPTR, e.g. when the context is not being used to run a model.
     *
     * \note This configuration will only be used when running a "top-level" model algorithm, and
     * will not be passed on to child models used within that initial top-level model.
     *
     * \note Not available in Python bindings
     *
     * \see setModelInitialRunConfig()
     * \see takeModelInitialRunConfig()
     *
     * \since QGIS 3.38
     */
    QgsProcessingModelInitialRunConfig *modelInitialRunConfig() SIP_SKIP;

    /**
     * Takes the model initial run configuration from the context.
     *
     * May return NULLPTR, e.g. when the context is not being used to run a model.
     *
     * \note Not available in Python bindings
     *
     * \see modelInitialRunConfig()
     * \see setModelInitialRunConfig()
     *
     * \since QGIS 3.38
     */
    std::unique_ptr< QgsProcessingModelInitialRunConfig > takeModelInitialRunConfig() SIP_SKIP;

    /**
     * Sets the model initial run configuration, used to run a model algorithm.
     *
     * \note This configuration will only be used when running a "top-level" model algorithm, and
     * will not be passed on to child models used within that initial top-level model.
     *
     * \note Not available in Python bindings
     *
     * \see modelInitialRunConfig()
     * \see takeModelInitialRunConfig()
     *
     * \since QGIS 3.38
     */
    void setModelInitialRunConfig( std::unique_ptr< QgsProcessingModelInitialRunConfig > config ) SIP_SKIP;

    /**
     * Returns the model results, populated when the context is used to run a model algorithm.
     *
     * \since QGIS 3.38
     */
    QgsProcessingModelResult modelResult() const { return mModelResult; }

    /**
     * Returns a reference to the model results, populated when the context is used
     * to run a model algorithm.
     *
     * \note Not available in Python bindings

     * \since QGIS 3.38
     */
    QgsProcessingModelResult &modelResult() SIP_SKIP { return mModelResult; }

  private:

    QgsProcessingContext::Flags mFlags = QgsProcessingContext::Flags();
    QPointer< QgsProject > mProject;
    QgsCoordinateTransformContext mTransformContext;

    QString mEllipsoid;
    Qgis::DistanceUnit mDistanceUnit = Qgis::DistanceUnit::Unknown;
    Qgis::AreaUnit mAreaUnit = Qgis::AreaUnit::Unknown;

    QgsDateTimeRange mCurrentTimeRange;

    //! Temporary project owned by the context, used for storing temporarily loaded map layers
    QgsMapLayerStore tempLayerStore;
    QgsExpressionContext mExpressionContext;

    Qgis::InvalidGeometryCheck mInvalidGeometryCheck = Qgis::InvalidGeometryCheck::NoCheck;
    bool mUseDefaultInvalidGeometryCallback = true;
    std::function< void( const QgsFeature & ) > mInvalidGeometryCallback;

    std::function< void( const QgsFeature & ) > mTransformErrorCallback;
    QString mDefaultEncoding;
    QMap< QString, LayerDetails > mLayersToLoadOnCompletion;

    QPointer< QgsProcessingFeedback > mFeedback;

    QString mPreferredVectorFormat;
    QString mPreferredRasterFormat;

    Qgis::ProcessingLogLevel mLogLevel = Qgis::ProcessingLogLevel::DefaultLevel;

    QString mTemporaryFolderOverride;
    int mMaximumThreads = QThread::idealThreadCount();

    std::unique_ptr< QgsProcessingModelInitialRunConfig > mModelConfig;
    QgsProcessingModelResult mModelResult;

#ifdef SIP_RUN
    QgsProcessingContext( const QgsProcessingContext &other );
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingContext::Flags )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingContext::ProcessArgumentFlags )


/**
 * \brief An interface for layer post-processing handlers for execution following a processing algorithm operation.
 *
 * Note that post-processing of a layer will ONLY occur if that layer is set to be loaded into a QGIS project
 * on algorithm completion. See QgsProcessingContext::layersToLoadOnCompletion().
 *
 * Algorithms that wish to set post-processing steps for generated layers should implement this interface
 * in a separate class (NOT the algorithm class itself!).
 *
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingLayerPostProcessorInterface
{
  public:

    virtual ~QgsProcessingLayerPostProcessorInterface() = default;

    /**
      * Post-processes the specified \a layer, following successful execution of a processing algorithm. This method
      * always runs in the main thread and can be used to setup renderers, editor widgets, metadata, etc for
      * the given layer.
      *
      * Post-processing classes can utilize settings from the algorithm's \a context and report logging messages
      * or errors via the given \a feedback object.
      *
      * In the case of an algorithm run as part of a larger model, the post-processing occurs following the completed
      * execution of the entire model.
      *
      * Note that post-processing of a layer will ONLY occur if that layer is set to be loaded into a QGIS project
      * on algorithm completion. See QgsProcessingContext::layersToLoadOnCompletion().
      */
    virtual void postProcessLayer( QgsMapLayer *layer, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) = 0;

};


#endif // QGSPROCESSINGPARAMETERS_H




