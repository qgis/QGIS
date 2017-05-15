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
      UseSelectionIfPresent = 1 << 0,  //!< Filter to selected features when running algorithms (if a selection exists)
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingContext.
     */
    QgsProcessingContext() = default;

    //! QgsProcessingContext cannot be copied
    QgsProcessingContext( const QgsProcessingContext &other ) = delete;
    //! QgsProcessingContext cannot be copied
    QgsProcessingContext &operator=( const QgsProcessingContext &other ) = delete;

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

    /**
     * Returns a list of layers (by ID or datasource) to load into the canvas upon completion of the algorithm or model.
     * \see setLayersToLoadOnCompletion()
     * \see addLayerToLoadOnCompletion()
     */
    QStringList layersToLoadOnCompletion() const
    {
      return mLayersToLoadOnCompletion;
    }

    /**
     * Sets the list of \a layers (by ID or datasource) to load into the canvas upon completion of the algorithm or model.
     * \see addLayerToLoadOnCompletion()
     * \see layersToLoadOnCompletion()
     */
    void setLayersToLoadOnCompletion( const QStringList &layers )
    {
      mLayersToLoadOnCompletion = layers;
    }

    /**
     * Adds a \a layer to load (by ID or datasource) into the canvas upon completion of the algorithm or model.
     * \see setLayersToLoadOnCompletion()
     * \see layersToLoadOnCompletion()
     */
    void addLayerToLoadOnCompletion( const QString &layer )
    {
      mLayersToLoadOnCompletion << layer;
    }

    /**
     * Returns a map of output values stored in the context. These are grouped with the map keys
     * matching the algorithm name for multi-algorithm models.
     * \note not available in Python bindings
     */
    SIP_SKIP QMap<QString, QVariantMap> &outputMap() { return mOutputMap; }

    /**
     * Returns the behavior used for checking invalid geometries in input layers.
     * \see setInvalidGeometryCheck()
     */
    QgsFeatureRequest::InvalidGeometryCheck invalidGeometryCheck() const { return mInvalidGeometryCheck; }

    /**
     * Sets the behavior used for checking invalid geometries in input layers.
     * \see invalidGeometryCheck()
     */
    void setInvalidGeometryCheck( const QgsFeatureRequest::InvalidGeometryCheck &check ) { mInvalidGeometryCheck = check; }


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
     * Returns the default encoding to use for newly created files.
     * \see setDefaultEncoding()
     */
    QString defaultEncoding() const { return mDefaultEncoding; }

    /**
     * Sets the default \a encoding to use for newly created files.
     * \see defaultEncoding()
     */
    void setDefaultEncoding( const QString &encoding ) { mDefaultEncoding = encoding; }

  private:

    QgsProcessingContext::Flags mFlags = 0;
    QPointer< QgsProject > mProject;
    //! Temporary project owned by the context, used for storing temporarily loaded map layers
    QgsMapLayerStore tempLayerStore;
    QMap< QString, QVariantMap > mOutputMap;
    QgsExpressionContext mExpressionContext;
    QgsFeatureRequest::InvalidGeometryCheck mInvalidGeometryCheck = QgsFeatureRequest::GeometryNoCheck;
    std::function< void( const QgsFeature & ) > mInvalidGeometryCallback;
    QString mDefaultEncoding;
    QStringList mLayersToLoadOnCompletion;

#ifdef SIP_RUN
    QgsProcessingContext( const QgsProcessingContext &other );
#endif
};





Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingContext::Flags )

#endif // QGSPROCESSINGPARAMETERS_H




