/***************************************************************************
                             qgsprocessingguiutils.h
                             ------------------------
    Date                 : June 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGGUIUTILS_H
#define QGSPROCESSINGGUIUTILS_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsprocessingcontext.h"

class QgsLayerTreeLayer;
class QgsLayerTreeView;

/**
 * \class QgsProcessingGuiUtils
 * \ingroup gui
 *
 * \brief Contains utility functions relating to Processing GUI components.
 *
 * \warning This is not considered stable API, and is exposed to Python for internal use only.
 *
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsProcessingGuiUtils
{
  public:
    /**
     * Contains details of a layer result from running an algorithm.
     * \ingroup gui
     * \since QGIS 3.44
     */
    class GUI_EXPORT ResultLayerDetails
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
     * Applies post-processing steps to the QgsLayerTreeLayer created for an algorithm's output.
     */
    static void configureResultLayerTreeLayer( QgsLayerTreeLayer *layerTreeLayer );

    /**
     * Returns the destination layer tree group to store results in, or NULLPTR if there
     * is no specific destination tree group associated with the layer.
     */
    static QgsLayerTreeGroup *layerTreeResultsGroup( const QgsProcessingContext::LayerDetails &layerDetails, const QgsProcessingContext &context );

    /**
     * Responsible for adding layers created by an algorithm to a project and the project's layer tree in the correct location.
     */
    static void addResultLayers( const QVector< QgsProcessingGuiUtils::ResultLayerDetails > &layers, const QgsProcessingContext &context, QgsLayerTreeView *view = nullptr );
};


#endif // QGSPROCESSINGGUIUTILS_H
