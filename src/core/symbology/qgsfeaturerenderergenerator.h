/***************************************************************************
     qgsfeaturerenderergenerator.h
     -----------------------
    Date                 : December 2020
    Copyright            : (C) 2020 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFEATURERENDERERGENERATOR_H
#define QGSFEATURERENDERERGENERATOR_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QString>

class QgsFeatureRenderer;

/**
 * \ingroup core
 * \class QgsFeatureRendererGenerator
 * \brief An interface for objects which generate feature renderers for vector layers.
 * \since QGIS 3.18
 */
class CORE_EXPORT QgsFeatureRendererGenerator
{
  public:

    virtual ~QgsFeatureRendererGenerator() = default;

    /**
     * Returns a unique ID string identifying the renderer generator.
     */
    virtual QString id() const = 0;

    /**
     * Returns a double value which dictates the stacking or z order level associated with
     * the returned renderer.
     *
     * A value > 0 will always be rendered AFTER the normal renderer for a vector layer (i.e. the
     * renderer will be drawn on top of the normal feature renderer), while a value < 0 will always
     * be rendered BEFORE the normal renderer (i.e. the rendered features will be drawn below the
     * normal feature renderer).
     *
     * Since a layer may potentially have multiple extra renderers created by QgsFeatureRendererGenerator
     * subclasses, the level will always be used to control the order that these renderers are drawn.
     * A renderer with a lower level() return value will always be drawn before those with a higher level()
     * value.
     *
     * The default implementation returns 1.0, i.e. features will be rendered ABOVE the normal
     * vector layer renderer.
     *
     * \note If two QgsFeatureRendererGenerator implementations return the same level() value, then their
     * ordering will be unpredictable.
     */
    virtual double level() const;

    /**
     * Creates a new feature renderer to use when rendering a vector layer.
     *
     * Caller takes ownership of the returned renderer.
     */
    virtual QgsFeatureRenderer *createRenderer() const = 0 SIP_FACTORY;
};

#endif // QGSFEATURERENDERERGENERATOR_H
