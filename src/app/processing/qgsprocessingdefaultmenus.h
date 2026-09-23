/***************************************************************************
    qgsprocessingdefaultmenus.h
    -------------------------
    begin                : September 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROCESSINGDEFAULTMENUS_H
#define QGSPROCESSINGDEFAULTMENUS_H

#include "qgis.h"

///@cond PRIVATE
class QgsProcessingDefaultMenus
{
  public:
    /**
   * Returns the map of Processing menus to a list of algorithm IDs to include by default in that menu.
   */
    static QMap< Qgis::ProcessingMenu, QStringList > defaultProcessingMenuEntries();
};

///@endcond

#endif // QGSPROCESSINGDEFAULTMENUS_H
