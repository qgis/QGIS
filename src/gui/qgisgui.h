/***************************************************************************
    qgisgui.h - Constants used throughout the QGIS GUI.
     --------------------------------------
    Date                 : 11-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGISGUI_H
#define QGISGUI_H

#include <Qt>

class QStringList;

/** \ingroup gui
 * /namespace QgisGui
 * The QgisGui namespace contains constants and helper functions used throughout the QGIS GUI.
 */
namespace QgisGui
{

  /*!
   * /var ModalDialogFlags
   * /brief Flags used to create a modal dialog (adapted from QMessageBox).
   *
   * Using these flags for all modal dialogs throughout QGIS ensures that
   * for platforms such as the Mac where modal and modeless dialogs have
   * different looks, QGIS modal dialogs will look the same as Qt modal
   * dialogs and all modal dialogs will look distinct from modeless dialogs.
   * Althought not the standard Mac modal look, it does lack the minimize
   * control which makes sense only for modeless dislogs.
   *
   * The Qt3 method of creating a true Mac modal dialog is deprecated in Qt4
   * and should not be used due to conflicts with QMessageBox style dialogs.
   *
   * Qt::WindowMaximizeButtonHint is included but will be ignored if
   * the dialog is a fixed size and does not have a size grip.
   */
  static const Qt::WFlags ModalDialogFlags =
#if QT_VERSION < 0x040200
    Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMaximizeButtonHint;
#else
    0;
#endif

  /**
    Open files, preferring to have the default file selector be the
    last one used, if any; also, prefer to start in the last directory
    associated with filterName.

    @param filterName the name of the filter; used for persistent store key
    @param filters    the file filters used for QFileDialog
    @param selectedFiles string list of selected files; will be empty if none selected
    @param enc        encoding?
    @param title      the title for the dialog
    @note

    Stores persistent settings under /UI/.  The sub-keys will be
    filterName and filterName + "Dir".

    Opens dialog on last directory associated with the filter name, or
    the current working directory if this is the first time invoked
    with the current filter name.

    This method returns true if cancel all was clicked, otherwise false

    @note added in 1.4
  */

  bool openFilesRememberingFilter( QString const &filterName,
                                   QString const &filters, QStringList & selectedFiles, QString& enc, QString &title,
                                   bool cancelAll = false );

}

#endif
