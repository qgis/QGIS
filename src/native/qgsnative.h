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

class QString;

/**
 * \class QgsNative
 * \ingroup native
 * Base class for implementing methods for native system calls that
 * are implemented in subclasses to provide platform abstraction.
 * \since QGIS 3.0
 */
class NATIVE_EXPORT QgsNative
{
  public:

    virtual ~QgsNative() = default;

    /**
     * Brings the QGIS app to front. The default implementation does nothing.
     */
    virtual void currentAppActivateIgnoringOtherApps();

    /**
     * Opens the desktop file explorer at the folder containing \a path,
     * and (if possible) scrolls to and pre-selects the file at \a path itself.
     *
     * The default implementation just calls the QDesktopServices method to open the folder,
     * without selecting the specified file.
     *
     * \since QGIS 3.4
     */
    virtual void openFileExplorerAndSelectFile( const QString &path );
};

#endif // QGSNATIVE_H
