/***************************************************************************
                             qgswindowmanagerinterface.h
                             ---------------------------
    Date                 : September 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWINDOWMANAGERINTERFACE_H
#define QGSWINDOWMANAGERINTERFACE_H

#include "qgis.h"
#include "qgis_gui.h"

class QgsVectorLayer;
class QgsRasterLayer;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Interface for window manager.
 *
 * An implementation of the window manager interface is usually retrieved from
 * the QgsGui instance, via QgsGui::windowManager().
 *
 * \note This is not considered stable API and may change in future QGIS versions.
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsWindowManagerInterface
{
  public:

    //! Standard QGIS dialogs
    enum StandardDialog
    {
      DialogStyleManager = 0, //!< Style manager dialog
    };

    virtual ~QgsWindowManagerInterface() = default;

    /**
     * Opens an instance of a standard QGIS dialog. Depending on the window manager
     * implementation, this may either open a new instance of the dialog or bring an
     * existing instance to the foreground.
     *
     * Returns the dialog if shown, or nullptr if the dialog either could not be
     * created or is not supported by the window manager implementation.
     */
    virtual QWidget *openStandardDialog( StandardDialog dialog ) = 0;

};

///@endcond

#endif // QGSWINDOWMANAGERINTERFACE_H
