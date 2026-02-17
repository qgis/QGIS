/***************************************************************************
    qgslayerchangescommitblockerinterface.h
    ---------------------
    begin                : October 2025
    copyright            : (C) 2025 by Seweryn Pajor
    email                : s3vdev@proton.me
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERCHANGESCOMMITBLOCKERINTERFACE_H
#define QGSLAYERCHANGESCOMMITBLOCKERINTERFACE_H

#include "qgis_gui.h"
#include "qgsmaplayer.h"

/**
 * \ingroup gui
 * \brief An interface that may be implemented to allow plugins or scripts to temporarily block
 * the ability to commit changes to a layer.
 *
 * This interface allows plugins to implement custom logic to determine whether it is safe
 * for the application to commit those changes.
 *
 * QgsLayerChangesCommitBlockerInterface are registered via the iface object:
 *
 * ### Example
 *
 * \code{.py}
 *   class MyPluginCommitBlocker(QgsLayerChangesCommitBlockerInterface):
 *
 *      def allowCommit(self, layer):
 *          if layer.something_is_wrong()
 *              # show a warning prompt
 *              # ...
 *              # prevent QGIS application from saving
 *              return False
 *
 *          # allow QGIS application to commit changes
 *          return True
 *
 *   my_blocker = MyPluginCommitBlocker()
 *   iface.registerApplicationExitBlocker(my_blocker)
 * \endcode
 *
 * \since QGIS 4.0
 */

class GUI_EXPORT QgsLayerChangesCommitBlockerInterface
{
  public:
    virtual ~QgsLayerChangesCommitBlockerInterface() = default;

    /**
     * Called whenever the QGIS application has been asked to commit changes to a layer.
     *
     * The subclass can use this method to implement custom logic handling whether it is safe
     * for the application to save changes.
     *
     * The implementation should return TRUE if it is safe for QGIS to commit, or FALSE if it
     * wishes to prevent the application from doing so.
     *
     * \note It is safe to use GUI widgets in implementations of this function, including message
     * boxes or custom dialogs with event loops.
    */
    virtual bool allowCommit( QgsMapLayer *layer ) = 0;
};

#endif // QGSLAYERCHANGESCOMMITBLOCKERINTERFACE_H
