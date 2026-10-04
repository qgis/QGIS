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

class QgsMapLayer;
class QgsProcessingAlgorithm;

/**
 * \class QgsProcessingLayerPostProcessor
 * \ingroup core
 * \brief A post-processor for result layers created by Processing algorithms.
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


#endif // QGSPROCESSINGPOSTPROCESSOR_H
