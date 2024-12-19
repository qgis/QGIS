/***************************************************************************
                         qgsprocessinggui.h
                         ------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
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


#ifndef QGSPROCESSINGGUI_H
#define QGSPROCESSINGGUI_H

#include "qgis_gui.h"

/**
 * \class QgsProcessingGui
 * \ingroup gui
 *
 * \brief Contains general functions and values related to Processing GUI components.
 *
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingGui
{
  public:
    //! Types of dialogs which Processing widgets can be created for
    enum WidgetType
    {
      Standard, //!< Standard algorithm dialog
      Batch,    //!< Batch processing dialog
      Modeler,  //!< Modeler dialog
    };
};

#endif // QGSPROCESSINGGUI_H
