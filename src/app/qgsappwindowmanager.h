/***************************************************************************
                             qgsappwindowmanager.h
                             ---------------------
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

#ifndef QGSAPPWINDOWMANAGER_H
#define QGSAPPWINDOWMANAGER_H

#include "qgis.h"
#include "qgswindowmanagerinterface.h"
#include <QPointer>

class QgsStyleManagerDialog;
class QgsLayoutManagerDialog;
class Qgs3DViewsManagerDialog;

/**
 * \ingroup gui
 * \brief Implementation of QgsWindowManagerInterface for the QGIS application.
 */
class QgsAppWindowManager : public QgsWindowManagerInterface
{
  public:

    //! Application-only QGIS dialogs
    enum ApplicationDialog
    {
      DialogLayoutManager = 0, //!< Layout manager dialog
      Dialog3DMapViewsManager = 1, //!< 3D map views manager dialog
    };

    QgsAppWindowManager() = default;
    ~QgsAppWindowManager();

    QWidget *openStandardDialog( QgsWindowManagerInterface::StandardDialog dialog ) override;

    /**
     * Opens an instance of a application QGIS dialog. Depending on the dialog,
     * this may either open a new instance of the dialog or bring an
     * existing instance to the foreground.
     *
     * Returns the dialog if shown, or NULLPTR if the dialog either could not be
     * created.
     */
    QWidget *openApplicationDialog( ApplicationDialog dialog );

  private:
    QPointer< QgsStyleManagerDialog > mStyleManagerDialog;
    QPointer< QgsLayoutManagerDialog > mLayoutManagerDialog;
    QPointer< Qgs3DViewsManagerDialog > m3DMapViewsManagerDialog;

};


#endif // QGSAPPWINDOWMANAGER_H
