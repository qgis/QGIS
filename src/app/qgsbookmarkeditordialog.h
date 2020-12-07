/***************************************************************************
                         qgsbookmarkeditordialog.h
                         -------------------------------------
    begin                : September 2019
    copyright            : (C) 2019 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBOOKMARKEDITORDIALOG_H
#define QGSBOOKMARKEDITORDIALOG_H

#include "ui_qgsbookmarkeditordialog.h"

#include "qgisapp.h"
#include "qgsbookmarkmanager.h"

#include <QDialog>

class QgsMapCanvas;


/**
 * \ingroup app
 * \brief a dialog for editing bookmarks.
 * \since QGIS 3.10
*/
class APP_EXPORT QgsBookmarkEditorDialog: public QDialog, private Ui::QgsBookmarkEditorDialog
{
    Q_OBJECT

  public:

    enum SaveLocation
    {
      ApplicationManager = 1, // Bookmark saved in the application bookmark manager
      ProjectManager          // Bookmark saved in the project bookmark manager
    };

    /**
     * Constructor for QgsBookmarkEditorDialog
     */
    QgsBookmarkEditorDialog( QgsBookmark bookmark, bool inProject = false, QWidget *parent = nullptr, QgsMapCanvas *mapCanvas = nullptr );

  private slots:

    void crsChanged( const QgsCoordinateReferenceSystem &crs );
    void onAccepted();
    void showHelp();

  private:

    QgsBookmark mBookmark;
    bool mInProject = false;

    QgsMapCanvas *mMapCanvas = nullptr;

};

#endif // QGSBOOKMARKEDITORDIALOG_H
