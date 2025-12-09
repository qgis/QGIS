/***************************************************************************
                             qgslayoutguiutils.h
                             -------------------
    Date                 : October 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTGUIUTILS_H
#define QGSLAYOUTGUIUTILS_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"

class QgsMapCanvas;

/**
 * \ingroup gui
 * \brief Utils for layout handling from app.
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutGuiUtils
{
  public:
    /**
     * Registers the GUI handlers for known layout item types.
     */
    static void registerGuiForKnownItemTypes( QgsMapCanvas *mapCanvas );
};


#endif // QGSLAYOUTGUIUTILS_H
