/***************************************************************************
    qgsmapcanvasutils.h
    -------------------
    begin                : June 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPCANVASUTILS_H
#define QGSMAPCANVASUTILS_H

#include "qgis_gui.h"

class QgsMapCanvas;
class QgsVectorLayer;
class QString;

/**
 * \ingroup gui
 * \class QgsMapCanvasUtils
 * \brief Utility functions for working with QgsMapCanvas widgets.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsMapCanvasUtils
{

  public:

    /**
     * Zooms a map \a canvas to features from the specified \a layer which match the given \a filter expression string.
     *
     * The total count of matching features will be returned.
     */
    static long zoomToMatchingFeatures( QgsMapCanvas *canvas, QgsVectorLayer *layer, const QString &filter );

    /**
     * Flashes features from the specified \a layer which match the given \a filter expression string with a map \a canvas.
     *
     * The total count of matching features will be returned.
     */
    static long flashMatchingFeatures( QgsMapCanvas *canvas, QgsVectorLayer *layer, const QString &filter );

};

#endif //QGSMAPCANVASUTILS_H
