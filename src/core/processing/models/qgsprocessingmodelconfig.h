/***************************************************************************
                         qgsprocessingmodelconfig.h
                         ----------------------
    begin                : April 2024
    copyright            : (C) 2024 by Nyall Dawson
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

#ifndef QGSPROCESSINGMODELCONFIG_H
#define QGSPROCESSINGMODELCONFIG_H

#include "qgis_core.h"
#include "qgis.h"
#include <QSet>

#define SIP_NO_FILE

class QgsMapLayerStore;

/**
 * \ingroup core
 * \brief Configuration settings which control how a Processing model is executed.
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.38
*/
class CORE_EXPORT QgsProcessingModelInitialRunConfig
{
  public:

    QgsProcessingModelInitialRunConfig();
    ~QgsProcessingModelInitialRunConfig();

    /**
     * Returns the subset of child algorithms to run (by child ID).
     *
     * An empty set indicates the entire model should be run.
     *
     * \see setChildAlgorithmSubset()
     */
    QSet<QString> childAlgorithmSubset() const { return mChildAlgorithmSubset; }

    /**
     * Sets the \a subset of child algorithms to run (by child ID).
     *
     * An empty set indicates the entire model should be run.
     *
     * \see childAlgorithmSubset()
     */
    void setChildAlgorithmSubset( const QSet<QString> &subset ) { mChildAlgorithmSubset = subset; }

    /**
     * Returns the map of child algorithm inputs to use as the initial state when running the model.
     *
     * Map keys refer to the child algorithm IDs.
     *
     * \see setInitialChildInputs()
     */
    QVariantMap initialChildInputs() const { return mInitialChildInputs; }

    /**
     * Sets the map of child algorithm \a inputs to use as the initial state when running the model.
     *
     * Map keys refer to the child algorithm IDs.
     *
     * \see initialChildInputs()
     */
    void setInitialChildInputs( const QVariantMap &inputs ) { mInitialChildInputs = inputs; }

    /**
     * Returns the map of child algorithm outputs to use as the initial state when running the model.
     *
     * Map keys refer to the child algorithm IDs.
     *
     * \see setInitialChildOutputs()
     */
    QVariantMap initialChildOutputs() const { return mInitialChildOutputs; }

    /**
     * Sets the map of child algorithm \a outputs to use as the initial state when running the model.
     *
     * Map keys refer to the child algorithm IDs.
     *
     * \see initialChildOutputs()
     */
    void setInitialChildOutputs( const QVariantMap &outputs ) { mInitialChildOutputs = outputs; }

    /**
     * Returns the set of previously executed child algorithm IDs to use as the initial state
     * when running the model.
     *
     * \see setPreviouslyExecutedChildAlgorithms()
     */
    QSet< QString > previouslyExecutedChildAlgorithms() const { return mPreviouslyExecutedChildren; }

    /**
     * Sets the previously executed child algorithm IDs to use as the initial state
     * when running the model.
     *
     * \see previouslyExecutedChildAlgorithms()
     */
    void setPreviouslyExecutedChildAlgorithms( const QSet< QString > &children ) { mPreviouslyExecutedChildren = children; }

    /**
     * Returns a reference to a map store containing copies of temporary layers generated
     * during previous model executions.
     *
     * This may be NULLPTR.
     *
     * \see setPreviousLayerStore()
     * \see takePreviousLayerStore()
     */
    QgsMapLayerStore *previousLayerStore();

    /**
     * Takes the map store containing copies of temporary layers generated
     * during previous model executions.
     *
     * May return NULLPTR if this is not available.
     *
     * \see previousLayerStore()
     * \see setPreviousLayerStore()
     */
    std::unique_ptr< QgsMapLayerStore > takePreviousLayerStore();

    /**
     * Sets the map store containing copies of temporary layers generated
     * during previous model executions.
     *
     * \warning \a store must have previous been moved to a NULLPTR thread via a call
     * to QObject::moveToThread. An assert will be triggered if this condition is not met.
     *
     * \see previousLayerStore()
     * \see takePreviousLayerStore()
     */
    void setPreviousLayerStore( std::unique_ptr< QgsMapLayerStore > store );

  private:

    QSet<QString> mChildAlgorithmSubset;
    QVariantMap mInitialChildInputs;
    QVariantMap mInitialChildOutputs;
    QSet< QString > mPreviouslyExecutedChildren;

    std::unique_ptr< QgsMapLayerStore > mModelInitialLayerStore;


};

#endif // QGSPROCESSINGMODELCONFIG_H
