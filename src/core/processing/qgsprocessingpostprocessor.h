/***************************************************************************
                         qgsprocessingpostprocessor.h
                         ------------------------
    begin                : October 2026
    copyright            : (C) 2026 by Nyall Dawson
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

#ifndef QGSPROCESSINGPOSTPROCESSOR_H
#define QGSPROCESSINGPOSTPROCESSOR_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgsprocessingcontext.h"

class QgsMapLayer;
class QgsProcessingAlgorithm;
class QgsLayerTreeGroup;

/**
 * \class QgsProcessingLayerPostProcessor
 * \ingroup core
 * \brief A post-processor for result layers created by Processing algorithms.
 *
 * \warning This is not considered stable API, and is exposed to Python for internal use only.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingLayerPostProcessor
{
  public:
    QgsProcessingLayerPostProcessor() = default;

    virtual ~QgsProcessingLayerPostProcessor();

    /**
     * Post-processes a layer, e.g. applying a default style to the layer.
     *
     * This method will be called for every new layer created as a result of running
     * the specified Processing \a algorithm.
     */
    virtual void postProcessLayer( QgsMapLayer *layer, const QString &outputName, const QgsProcessingAlgorithm *algorithm );
};


/**
 * \class QgsProcessingResultsHandler
 * \ingroup core
 * \brief Handles results output by Processing algorithms.
 *
 * \warning This is not considered stable API, and is exposed to Python for internal use only.
 *
 * \since QGIS 4.4
 */
class CORE_EXPORT QgsProcessingResultsHandler
{
  public:
    QgsProcessingResultsHandler() = default;

    virtual ~QgsProcessingResultsHandler();

    /**
     * Contains details of a layer result from running an algorithm.
     * \ingroup core
     * \since QGIS 4.4
     */
    class CORE_EXPORT ResultLayerDetails
    {
      public:
        /**
       * Constructor for ResultLayerDetails.
       *
       * Takes ownership of \a layer.
       */
        ResultLayerDetails( QgsMapLayer *layer SIP_TRANSFER )
          : layer( layer )
        {}

        /**
       * Associated map layer.
       */
        QgsMapLayer *layer = nullptr;

        /**
       * Optional target layer tree group, where the layer should be placed.
       */
        QgsLayerTreeGroup *targetLayerTreeGroup = nullptr;

        /**
       * Sort order key for ordering output layers in the layer tree.
       */
        int sortKey = 0;

        /**
       * Destination QGIS project.
       */
        QgsProject *destinationProject = nullptr;
    };

    /**
     * Determines the desired layer name for a map layer output.
     */
    static QString determineOutputName(
      const QString &destinationId, const QgsProcessingContext::LayerDetails &details, const QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, const QVariantMap &parameters
    );

    /**
   * Returns the destination layer tree group to store results in, or NULLPTR if there
   * is no specific destination tree group associated with the layer.
   */
    static QgsLayerTreeGroup *layerTreeResultsGroup( const QgsProcessingContext::LayerDetails &layerDetails, const QgsProcessingContext &context );

    /**
     * Applies post-processing steps to the QgsLayerTreeLayer created for an algorithm's output.
     */
    static void configureResultLayerTreeLayer( QgsLayerTreeLayer *layerTreeLayer );

    /**
     * Responsible for adding layers created by an algorithm to a project and the project's layer tree in the correct location.
     *
     * Returns the map layer which should be set as the project's active layer after adding all the results.
     */
    static QgsMapLayer *addResultLayers( const QVector< QgsProcessingResultsHandler::ResultLayerDetails > &layers, const QgsProcessingContext &context, QgsLayerTreeNode *currentSelectedNode );

    /**
     * Handles the results from running an algorithm.
     *
     * Returns TRUE if all results could be successfully loaded.
     */
    bool handleAlgorithmResults( const QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, const QVariantMap &parameters, QgsProcessingFeedback *feedback = nullptr );

  protected:
#ifndef SIP_RUN
    /**
     * Stores results of running the handleAlgorithmResultsProtected method.
     */
    struct ResultDetails
    {
        //! TRUE if all layers could be successfully loaded
        bool succeeded = false;
        //! Details of newly added layers
        QList<QgsProcessingResultsHandler::ResultLayerDetails> addedLayers;
    };

    /**
     * Handles the results from running an algorithm.
     */
    virtual QgsProcessingResultsHandler::ResultDetails handleAlgorithmResultsProtected(
      const QgsProcessingAlgorithm *algorithm, QgsProcessingContext &context, const QVariantMap &parameters, QgsProcessingFeedback *feedback
    );

    /**
     * Calls addResultLayers(), allowing for extra logic to be injected in subclasses.
     */
    virtual QgsMapLayer *handleAddResultLayers( const QgsProcessingResultsHandler::ResultDetails &details, const QgsProcessingContext &context );

    /**
     * Handles setting the new project active layer after processing the results. The base class version does nothing.
     */
    virtual void setNewActiveLayer( QgsMapLayer *layer );
#endif
};


#endif // QGSPROCESSINGPOSTPROCESSOR_H
