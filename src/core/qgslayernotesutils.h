/***************************************************************************
  qgslayernotesutils.h
  --------------------------------------
  Date                 : April 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERNOTESUTILS_H
#define QGSLAYERNOTESUTILS_H

#include "qgis_core.h"
#include <QString>

class QgsMapLayer;

/**
 * \ingroup core
 *
 * \brief Contains utility functions for working with layer notes.
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsLayerNotesUtils
{
  public:

    /**
     * Returns the notes for the specified \a layer.
     *
     * The returned string is a HTML formatted set of user notations for the layer.
     */
    static QString layerNotes( const QgsMapLayer *layer );

    /**
     * Sets the \a notes for the specified \a layer, where \a notes is a HTML formatted string.
     */
    static void setLayerNotes( QgsMapLayer *layer, const QString &notes );

    /**
     * Returns TRUE if the specified \a layer has notes available.
     */
    static bool layerHasNotes( const QgsMapLayer *layer );

    /**
     * Removes any notes for the specified \a layer.
     */
    static void removeNotes( QgsMapLayer *layer );

};


#endif // QGSLAYERNOTESUTILS_H
