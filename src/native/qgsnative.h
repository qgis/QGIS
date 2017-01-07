/***************************************************************************
    qgsnative.h - abstracted interface to native system calls
                             -------------------
    begin                : January 2017
    copyright            : (C) 2017 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNATIVE_H
#define QGSNATIVE_H

#include "qgis_native.h"

/**
 * Base class for implementing methods for native system calls that
 * are implemented in subclasses to provide platform abstraction.
 */
class NATIVE_EXPORT QgsNative
{
  public:
    QgsNative();

    /**
     * Bring QGIS to front. Default implementation does nothing.
     *
     * @note Added in QGIS 3.0
     */
    virtual void currentAppActivateIgnoringOtherApps();
};

#endif // QGSNATIVE_H
