/***************************************************************************
  qgsprojectstoragegui.h
  --------------------------------------
  Date                 : June 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTSTORAGEGUI_H
#define QGSPROJECTSTORAGEGUI_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QString>

/**
 * \ingroup core
 * Abstract interface for project storage GUI - to be implemented by various backends
 * and registered in QgsProjectStorageGuiRegistry.
 *
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsProjectStorageGuiProvider
{
  public:
    virtual ~QgsProjectStorageGuiProvider() = default;

    /**
     * Unique identifier of the project storage type. If type() returns "memory", all project file names
     * starting with "memory:" will have read/write redirected through that storage implementation.
     */
    virtual QString type() = 0;

    /**
     * Returns human-readable name of the storage. Used as the menu item text in QGIS. Empty name
     * indicates that the storage does not implement GUI support (showLoadGui() and showSaveGui()).
     * The name may be translatable and ideally unique as well.
     */
    virtual QString visibleName() { return QString(); }

    /**
     * Opens GUI to allow user to select a project to be loaded (GUI specific to this storage type).
     * Returns project URI if user has picked a project or empty string if the GUI was canceled.
     */
    virtual QString showLoadGui() { return QString(); }

    /**
     * Opens GUI to allow user to select where a project should be saved (GUI specific to this storage type).
     * Returns project URI if user has picked a destination or empty string if the GUI was canceled.
     */
    virtual QString showSaveGui() { return QString(); }
};

#endif // QGSPROJECTSTORAGEGUI_H
