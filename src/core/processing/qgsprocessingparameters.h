/***************************************************************************
                         qgsprocessingparameters.h
                         -------------------------
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

#ifndef QGSPROCESSINGPARAMETERS_H
#define QGSPROCESSINGPARAMETERS_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsproperty.h"
#include <QMap>

class QgsProcessingContext;

/**
 * \class QgsProcessingParameters
 * \ingroup core
 *
 * A collection of utilities for working with parameters when running a processing algorithm.
 *
 * Parameters are stored in a QVariantMap and referenced by a unique string key.
 * The QVariants in parameters are not usually accessed
 * directly, and instead the high level API provided through QgsProcessingParameters
 * parameterAsString(), parameterAsDouble() are used instead.
 *
 * Parameters are evaluated using a provided QgsProcessingContext, allowing
 * the evaluation to understand available map layers and expression contexts
 * (for expression based parameters).
 *
 * \since QGIS 3.0
 */

class CORE_EXPORT QgsProcessingParameters
{

  public:

    /**
     * Returns true if the parameter with matching \a name is a dynamic parameter, and must
     * be evaluated once for every input feature processed.
     */
    static bool isDynamic( const QVariantMap &parameters, const QString &name );

    /**
     * Evaluates the parameter with matching \a name to a static string value.
     */
    static QString parameterAsString( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a name to a static double value.
     */
    static double parameterAsDouble( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a name to a static integer value.
     */
    static int parameterAsInt( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a name to a static boolean value.
     */
    static bool parameterAsBool( const QVariantMap &parameters, const QString &name, const QgsProcessingContext &context );

    /**
     * Evaluates the parameter with matching \a name to a map layer.
     *
     * Layers will either be taken from \a context's active project, or loaded from external
     * sources and stored temporarily in the \a context. In either case, callers do not
     * need to handle deletion of the returned layer.
     */
    static QgsMapLayer *parameterAsLayer( const QVariantMap &parameters, const QString &name, QgsProcessingContext &context );

};


#endif // QGSPROCESSINGPARAMETERS_H


