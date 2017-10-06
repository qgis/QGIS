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

    /**
     * Copies all settings which are safe for use across different threads from
     * \a other to this context.
     */
    void copyThreadSafeSettings( const QgsProcessingContext &other )
    {
      mFlags = other.mFlags;
      mProject = other.mProject;
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
    void setFlags( const QgsProcessingContext::Flags &flags ) { mFlags = flags; }

    /**
     * Returns the project in which the algorithm is being executed.
     * \see setProject()
     */
    QgsProject *project() const { return mProject; }

    /**
     * Sets the \a project in which the algorithm will be executed.
     * \see project()
     */
    void setProject( QgsProject *project ) { mProject = project; }

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
     * Returns a reference to the layer store used for storing temporary layers during
     * algorithm execution.
     */
    QgsMapLayerStore *temporaryLayerStore() { return &tempLayerStore; }

    //! Details for layers to load into projects.
    struct LayerDetails
    {

      /**
       * Constructor for LayerDetails.
       */
      LayerDetails( const QString &name, QgsProject *project, const QString &outputName = QString() )
        : name( name )
        , outputName( outputName )
        , project( project )
      {}

      //! Friendly name for layer, to use when loading layer into project.
      QString name;

      //! Associated output name from algorithm which generated the layer.
      QString outputName;

      //! Destination project
      QgsProject *project = nullptr;

    };

    /**
     * Returns a map of layers (by ID or datasource) to LayerDetails, to load into the canvas upon completion of the algorithm or model.
     * \see setLayersToLoadOnCompletion()
     * \see addLayerToLoadOnCompletion()
     */
    QMap< QString, QgsProcessingContext::LayerDetails > layersToLoadOnCompletion() const
    {
      return mLayersToLoadOnCompletion;
    }

    /**
     * Sets the map of \a layers (by ID or datasource) to LayerDetails, to load into the canvas upon completion of the algorithm or model.
     * \see addLayerToLoadOnCompletion()
     * \see layersToLoadOnCompletion()
     */
    void setLayersToLoadOnCompletion( const QMap< QString, QgsProcessingContext::LayerDetails > &layers )
    {
      mLayersToLoadOnCompletion = layers;
    }

    /**
     * Adds a \a layer to load (by ID or datasource) into the canvas upon completion of the algorithm or model.
     * The \a details parameter dictates the LayerDetails.
     * \see setLayersToLoadOnCompletion()
     * \see layersToLoadOnCompletion()
     */
    void addLayerToLoadOnCompletion( const QString &layer, const QgsProcessingContext::LayerDetails &details )
    {
      mLayersToLoadOnCompletion.insert( layer, details );
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
    void setInvalidGeometryCheck( const QgsFeatureRequest::InvalidGeometryCheck &check )
    {
      mInvalidGeometryCheck = check;

      switch ( mInvalidGeometryCheck )
      {
        case  QgsFeatureRequest::GeometryAbortOnInvalid:
        {
          auto callback = []( const QgsFeature & feature )
          {
            throw QgsProcessingException( QObject::tr( "Feature (%1) has invalid geometry. Please fix the geometry or change the Processing setting to the \"Ignore invalid input features\" option." ).arg( feature.id() ) );
          };
          mInvalidGeometryCallback = callback;
          break;
        }

        case QgsFeatureRequest::GeometrySkipInvalid:
        {
          auto callback = [ = ]( const QgsFeature & feature )
          {
            if ( mFeedback )
              mFeedback->reportError( QObject::tr( "Feature (%1) has invalid geometry and has been skipped. Please fix the geometry or change the Processing setting to the \"Ignore invalid input features\" option." ).arg( feature.id() ) );
          };
          mInvalidGeometryCallback = callback;
          break;
        }

        default:
          break;
      }
    }

    /**
     * Sets a callback function to use when encountering an invalid geometry and
     * invalidGeometryCheck() is set to GeometryAbortOnInvalid. This function will be
     * called using the feature with invalid geometry as a parameter.
     * \since QGIS 3.0
     * \see invalidGeometryCallback()
     */
#ifndef SIP_RUN
    void setInvalidGeometryCallback( std::function< void( const QgsFeature & ) > callback ) { mInvalidGeometryCallback = callback; }
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
     * \since QGIS 3.0
     * \note not available in Python bindings
     * \see setInvalidGeometryCallback()
     */
    SIP_SKIP std::function< void( const QgsFeature & ) > invalidGeometryCallback() const { return mInvalidGeometryCallback; }

    /**
     * Sets a callback function to use when encountering a transform error when iterating
     * features. This function will be
     * called using the feature which encountered the transform error as a parameter.
     * \since QGIS 3.0
     * \see transformErrorCallback()
     */
#ifndef SIP_RUN
    void setTransformErrorCallback( std::function< void( const QgsFeature & ) > callback ) { mTransformErrorCallback = callback; }
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
     * \since QGIS 3.0
     * \note not available in Python bindings
     * \see setTransformErrorCallback()
     * \see destinationCrs()
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
    void takeResultsFrom( QgsProcessingContext &context )
    {
      QMap< QString, LayerDetails > loadOnCompletion = context.layersToLoadOnCompletion();
      QMap< QString, LayerDetails >::const_iterator llIt = loadOnCompletion.constBegin();
      for ( ; llIt != loadOnCompletion.constEnd(); ++llIt )
      {
        mLayersToLoadOnCompletion.insert( llIt.key(), llIt.value() );
      }
      context.setLayersToLoadOnCompletion( QMap< QString, LayerDetails >() );
      tempLayerStore.transferLayersFromStore( context.temporaryLayerStore() );
    }

    /**
     * Takes the result map layer with matching \a id from the context and
     * transfers ownership of it back to the caller. This method can be used
     * to remove temporary layers which are not required for further processing
     * from a context.
     */
    QgsMapLayer *takeResultLayer( const QString &id ) SIP_TRANSFERBACK
    {
      return tempLayerStore.takeMapLayer( tempLayerStore.mapLayer( id ) );
    }

  private:

    QgsProcessingContext::Flags mFlags = 0;
    QPointer< QgsProject > mProject;
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

#endif // QGSPROCESSINGPARAMETERS_H




