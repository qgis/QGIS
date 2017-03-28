/***************************************************************************
                         qgsprocessingalgorithm.h
                         ------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSPROCESSINGALGORITHM_H
#define QGSPROCESSINGALGORITHM_H

#include "qgis_core.h"
#include <QString>
#include <QVariant>
#include <QIcon>

/**
 * \class QgsProcessingAlgorithm
 * \ingroup core
 * Abstract base class for processing algorithms.
  * \note added in QGIS 3.0
 */
class CORE_EXPORT QgsProcessingAlgorithm
{
  public:

    /**
     * Constructor for QgsProcessingAlgorithm.
     */
    QgsProcessingAlgorithm() = default;

    virtual ~QgsProcessingAlgorithm() = default;

    /**
     * Returns a list of tags which relate to the algorithm, and are used to assist users in searching
     * for suitable algorithms. These tags should be localised.
    */
    virtual QStringList tags() const { return QStringList(); }

};

#endif // QGSPROCESSINGALGORITHM_H


