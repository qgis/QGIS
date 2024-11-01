/***************************************************************************
    qgsapplicationexitblockerinterface.h
    ---------------------
    begin                : October 2020
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

#ifndef QGSAPPLICATIONEXITBLOCKERINTERFACE_H
#define QGSAPPLICATIONEXITBLOCKERINTERFACE_H

#include "qgis_gui.h"
#include <QStringList>
#include <QObject>

/**
 * \ingroup gui
 * \brief An interface that may be implemented to allow plugins or scripts to temporarily block
 * the QGIS application from exiting.
 *
 * This interface allows plugins to implement custom logic to determine whether it is safe
 * for the application to exit, e.g. by checking whether the plugin or script has any
 * unsaved changes which should be saved or discarded before allowing QGIS to exit.
 *
 * QgsApplicationExitBlockerInterface are registered via the iface object:
 *
 * ### Example
 *
 * \code{.py}
 *   class MyPluginExitBlocker(QgsApplicationExitBlockerInterface):
 *
 *      def allowExit(self):
 *          if self.has_unsaved_changes():
 *              # show a warning prompt
 *              # ...
 *              # prevent QGIS application from exiting
 *              return False
 *
 *          # allow QGIS application to exit
 *          return True
 *
 *   my_blocker = MyPluginExitBlocker()
 *   iface.registerApplicationExitBlocker(my_blocker)
 * \endcode
 *
 * \since QGIS 3.16
 */
class GUI_EXPORT QgsApplicationExitBlockerInterface
{
  public:
    virtual ~QgsApplicationExitBlockerInterface();

    /**
     * Called whenever the QGIS application has been asked to exit by a user.
     *
     * The subclass can use this method to implement custom logic handling whether it is safe
     * for the application to exit, e.g. by checking whether the plugin or script has any unsaved
     * changes which should be saved or discarded before allowing QGIS to exit.
     *
     * The implementation should return TRUE if it is safe for QGIS to exit, or FALSE if it
     * wishes to prevent the application from exiting.
     *
     * \note It is safe to use GUI widgets in implementations of this function, including message
     * boxes or custom dialogs with event loops.
    */
    virtual bool allowExit() = 0;
};

#endif // QgsCustomProjectOpenHandler_H
