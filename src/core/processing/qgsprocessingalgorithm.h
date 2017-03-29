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
     * Returns the algorithm name, used for identifying the algorithm. This string
     * should be fixed for the algorithm, and must not be localised. The name should
     * be unique within each provider.
     * @see displayName()
     * @see group()
     * @see tags()
    */
    virtual QString name() const = 0;

    /**
     * Returns the translated algorithm name, which should be used for any user-visible display
     * of the algorithm name.
     * @see name()
     */
    virtual QString displayName() const = 0;

    /**
     * Returns a list of tags which relate to the algorithm, and are used to assist users in searching
     * for suitable algorithms. These tags should be localised.
    */
    virtual QStringList tags() const { return QStringList(); }

    /**
     * Returns an icon for the algorithm.
     * @see svgIconPath()
    */
    virtual QIcon icon() const;

    /**
     * Returns a path to an SVG version of the algorithm's icon.
     * @see icon()
     */
    virtual QString svgIconPath() const;

    /**
     * Returns the name of the group this algorithm belongs to. This string
     * should be localised.
     * @see tags()
    */
    virtual QString group() const { return QString(); }

};

#endif // QGSPROCESSINGALGORITHM_H


