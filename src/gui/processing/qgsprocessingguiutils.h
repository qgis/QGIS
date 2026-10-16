/***************************************************************************
                             qgsprocessingguiutils.h
                             ------------------------
    Date                 : June 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGGUIUTILS_H
#define QGSPROCESSINGGUIUTILS_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingpostprocessor.h"

class QgsLayerTreeLayer;
class QgsLayerTreeView;

/**
 * \class QgsProcessingGuiUtils
 * \ingroup gui
 *
 * \brief Contains utility functions relating to Processing GUI components.
 *
 * \warning This is not considered stable API, and is exposed to Python for internal use only.
 *
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsProcessingGuiUtils
{
  public:
    /**
     * Returns the map of Processing menus to a list of algorithm IDs to include by default in that menu.
     *
     * Map keys correspond to Qgis::ProcessingMenu values.
     *
     * \since QGIS 4.4
     */
    static QMap< int, QStringList > defaultProcessingMenuEntries();
};


#endif // QGSPROCESSINGGUIUTILS_H
