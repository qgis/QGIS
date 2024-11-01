/***************************************************************************
    qgscustomprojectopenhandler.h
    ---------------------
    begin                : April 2020
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

#ifndef QGSCUSTOMPROJECTOPENHANDLER_H
#define QGSCUSTOMPROJECTOPENHANDLER_H

#include "qgis_gui.h"
#include <QStringList>
#include <QObject>

/**
 * \ingroup gui
 * \brief Abstract base class that may be implemented to handle new project file types within
 * the QGIS application.
 *
 * This interface allows extending the QGIS interface by adding support for opening additional
 * (non QGS/QGZ) project files, e.g. allowing plugins to add support for opening other
 * vendor project formats (such as ArcGIS MXD documents or MapInfo WOR workspaces).
 *
 * Handler implementations should indicate the file types they support via their filters()
 * implementation, and then implement handleProjectOpen() to open the associated files.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsCustomProjectOpenHandler : public QObject
{
    Q_OBJECT

  public:
    /**
     * Called when the specified project \a file has been opened within QGIS. If TRUE
     * is returned, then the handler has accepted this file and it should not
     * be further processed (e.g. by other QgsCustomProjectOpenHandler).
     *
     * It it is the subclasses' responsibility to ignore file types it cannot handle
     * by returning FALSE for these.
     *
     * The base class implementation does nothing.
     */
    virtual bool handleProjectOpen( const QString &file ) = 0;

    /**
     * Returns file filters associated with this handler, e.g. "MXD Documents (*.mxd)", "MapInfo Workspaces (*.wor)".
     *
     * Each individual filter should be reflected as one entry in the returned list.
     */
    virtual QStringList filters() const = 0;

    /**
     * Returns TRUE if a document thumbnail should automatically be created after opening the project.
     *
     * The default behavior is to return FALSE.
     */
    virtual bool createDocumentThumbnailAfterOpen() const;

    /**
     * Returns a custom icon used to represent this handler.
     */
    virtual QIcon icon() const;
};

#endif // QgsCustomProjectOpenHandler_H
