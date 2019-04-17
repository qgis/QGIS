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
#include "qgsfeaturerequest.h"
#include "qgsmaplayerlistutils.h"
#include "qgsexception.h"
#include "qgsprocessingfeedback.h"
#include "qgsprocessingutils.h"

class QgsProcessingLayerPostProcessorInterface;

/**
 * \class QgsProcessingContext
 * \ingroup core
 * Contains information about the context in which a processing algorithm is executed.
 *
 * Contextual information includes settings such as the associated project, and
 * expression context.
 * \since QGIS 3.0
*/

class CORE_EXPORT QgsProcessingContext
{
  public:

    //! Flags that affect how processing algorithms are run
    enum Flag
    {
      // UseSelectionIfPresent = 1 << 0,
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingContext.
     */
    QgsProcessingContext()
    {
      auto callback = [ = ]( const QgsFeature & feature )
      {
        if ( mFeedback )
          mFeedback->reportError( QObject::tr( "Encountered a transform error when reprojecting feature with id %1." ).arg( feature.id() ) );
      };
      mTransformErrorCallback = callback;
    }

    //! QgsProcessingContext cannot be copied
    QgsProcessingContext( const QgsProcessingContext &other ) = delete;
    //! QgsProcessingContext cannot be copied
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
      mInvalidGeometryCallback = other.mInvalidGeometryCallback;
      mInvalidGeometryCheck = other.mInvalidGeometryCheck;
      mTransformErrorCallback = other.mTransformErrorCallback;
      mDefaultEncoding = other.mDefaultEncoding;
      mFeedback = other.mFeedback;
    }

    /**
     * Returns any flags set in the context.
     * \see setFlags()
     */
    QgsProcessingContext::Flags flags() const { return mFlags; }

    /**
     * Sets \a flags for the context.
     * \see flags()
     */
    void setFlags( QgsProcessingContext::Flags flags ) { mFlags = flags; }

    /**
     * Returns the project in which the algorithm is being executed.
     * \see setProject()
     */
    QgsProject *project() const { return mProject; }

    /**
     * Sets the \a project in which the algorithm will be executed.
     *
     * This also automatically sets the transformContext() to match
     * the project's transform context.
     *
     * \see project()
     */
    void setProject( QgsProject *project )
    {
      mProject = project;
      if ( mProject )
        mTransformContext = mProject->transformContext();
    }

    /**
     * Returns the expression context.
     */
    QgsExpressionContext &expressionContext() { return mExpressionContext; }

    /**
     * Returns the expression context.
     */
    SIP_SKIP const QgsExpressionContext &expressionContext() const { return mExpressionContext; }

    /**
     * Sets the expression \a context.
     */
    void setExpressionContext( const QgsExpressionContext &context ) { mExpressionContext = context; }

    /**
     * Returns the coordinate transform context.
     * \see setTransformContext()
     */
    QgsCoordinateTransformContext transformContext() const { return mTransformContext; }

    /**
     * Sets the coordinate transform \a context.
     *
     * Note that setting a project for the context will automatically set the coordinate transform
     * context.
     *
     * \see transformContext()
     */
    void setTransformContext( const QgsCoordinateTransformContext &context ) { mTransformContext = context; }

    /**
     * Returns a reference to the layer store used for storing temporary layers during
     * algorithm execution.
     */
    QgsMapLayerStore *temporaryLayerStore() { return &tempLayerStore; }

    /**
     * Details for layers to load into projects.
     * \ingroup core
     * \since QGIS 3.0
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

        //! Default constructor
        LayerDetails() = default;

        //! Friendly name for layer, to use when loading layer into project.
        QString name;

        //! Associated output name from algorithm which generated the layer.
        QString outputName;

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
    QMap< QString, QgsProcessingContext::LayerDetails > layersToLoadOnCompletion() const
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
    bool willLoadLayerOnCompletion( const QString &layer ) const
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
    void setLayersToLoadOnCompletion( const QMap< QString, QgsProcessingContext::LayerDetails > &layers );

    /**
     * Adds a \a layer to load (by ID or datasource) into the canvas upon completion of the algorithm or model.
     * The \a details parameter dictates the LayerDetails.
     * \see setLayersToLoadOnCompletion()
     * \see layersToLoadOnCompletion()
     * \see willLoadLayerOnCompletion()
     * \see layerToLoadOnCompletionDetails()
     */
    void addLayerToLoadOnCompletion( const QString &layer, const QgsProcessingContext::LayerDetails &details );

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
    QgsProcessingContext::LayerDetails &layerToLoadOnCompletionDetails( const QString &layer )
    {
      return mLayersToLoadOnCompletion[ layer ];
    }

    /**
     * Returns the behavior used for checking invalid geometries in input layers.
     * \see setInvalidGeometryCheck()
     */
    QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck() const { return mInvalidGeometryCheck; }

    /**
     * Sets the behavior used for checking invalid geometries in input layers.
     * Settings this to anything but QgsFeatureRequest::GeometryNoCheck will also
     * reset the invalidGeometryCallback() to a default implementation.
     * \see invalidGeometryCheck()
     */
    void setInvalidGeometryCheck( QgsFeatureRequest::InvalidGeometryCheck check );

    /**
     * Sets a callback function to use when encountering an invalid geometry and
     * invalidGeometryCheck() is set to GeometryAbortOnInvalid. This function will be
     * called using the feature with invalid geometry as a parameter.
     * \see invalidGeometryCallback()
     * \since QGIS 3.0
     */
#ifndef SIP_RUN
    void setInvalidGeometryCallback( const std::function< void( const QgsFeature & ) > &callback ) { mInvalidGeometryCallback = callback; }
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
     * \since QGIS 3.0
     */
    SIP_SKIP std::function< void( const QgsFeature & ) > invalidGeometryCallback() const { return mInvalidGeometryCallback; }

    /**
     * Sets a callback function to use when encountering a transform error when iterating
     * features. This function will be
     * called using the feature which encountered the transform error as a parameter.
     * \see transformErrorCallback()
     * \since QGIS 3.0
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
     * \since QGIS 3.0
     */
    std::function< void( const QgsFeature & ) > transformErrorCallback() const { return mTransformErrorCallback; } SIP_SKIP

    /**
     * Returns the default encoding to use for newly created files.
     * \see setDefaultEncoding()
     */
    QString defaultEncoding() const { return mDefaultEncoding; }

    /**
     * Sets the default \a encoding to use for newly created files.
     * \see defaultEncoding()
     */
    void setDefaultEncoding( const QString &encoding ) { mDefaultEncoding = encoding; }

    /**
     * Returns the associated feedback object.
     * \see setFeedback()
     */
    QgsProcessingFeedback *feedback() { return mFeedback; }

    /**
     * Sets an associated \a feedback object. This allows context related functions
     * to report feedback and errors to users and processing logs. While ideally this feedback
     * object should outlive the context, only a weak pointer to \a feedback is stored
     * and no errors will occur if feedback is deleted before the context.
     * Ownership of \a feedback is not transferred.
     * \see setFeedback()
     */
    void setFeedback( QgsProcessingFeedback *feedback ) { mFeedback = feedback; }

    /**
     * Returns the thread in which the context lives.
     * \see pushToThread()
     */
    QThread *thread() { return tempLayerStore.thread(); }

    /**
     * Pushes the thread affinity for the context (including all layers contained in the temporaryLayerStore()) into
     * another \a thread. This method is only safe to call when the current thread matches the existing thread
     * affinity for the context (see thread()).
     * \see thread()
     */
    void pushToThread( QThread *thread )
    {
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

  private:

    QgsProcessingContext::Flags mFlags = nullptr;
    QPointer< QgsProject > mProject;
    QgsCoordinateTransformContext mTransformContext;
    //! Temporary project owned by the context, used for storing temporarily loaded map layers
    QgsMapLayerStore tempLayerStore;
    QgsExpressionContext mExpressionContext;
    QgsFeatureRequest::InvalidGeometryCheck mInvalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck;
    std::function< void( const QgsFeature & ) > mInvalidGeometryCallback;
    std::function< void( const QgsFeature & ) > mTransformErrorCallback;
    QString mDefaultEncoding;
    QMap< QString, LayerDetails > mLayersToLoadOnCompletion;

    QPointer< QgsProcessingFeedback > mFeedback;

#ifdef SIP_RUN
    QgsProcessingContext( const QgsProcessingContext &other );
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingContext::Flags )


/**
 * An interface for layer post-processing handlers for execution following a processing algorithm operation.
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




