/***************************************************************************
                          qgisappinterface.h 
 Interface class for exposing functions in QgisApp for use by plugins
                             -------------------
  begin                : 2004-02-11
  copyright            : (C) 2004 by Gary E.Sherman
  email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGISIFACE_H
#define QGISIFACE_H

#include "qgisinterface.h"

class QgisApp;

/** \class QgisAppInterface
 * \brief Interface class to provide access to private methods in QgisApp
 * for use by plugins.
 * 
 * Only those functions "exposed" by QgisInterface can be called from within a
 * plugin.
 */
class QgisAppInterface : public QgisInterface
{
    Q_OBJECT;

    public:
        /**
         * Constructor.
         * @param qgis Pointer to the QgisApp object
         */
        QgisAppInterface(QgisApp *qgisapp);
        ~QgisAppInterface();

        /* Exposed functions */
        //! Zoom map to full extent
        void zoomFull();
        //! Zoom map to previous extent
        void zoomPrevious();
        //! Zoom to active layer
        void zoomActiveLayer();

        //! Add a vector layer
        QgsVectorLayer* addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey);
        //! Add a raster layer given its file name
        QgsRasterLayer* addRasterLayer(QString rasterLayerPath, QString baseName);
	//! Add a WMS layer
	QgsRasterLayer* addRasterLayer(const QString& url, const QString& baseName, const QString& providerKey, \
				       const QStringList& layers, const QStringList& styles, const QString& format, const QString& crs);

        //! Add a project
        bool addProject(QString theProjectName);
        //! Start a new blank project
        void newProject(bool thePromptToSaveFlag=false);

        //! Get pointer to the active layer (layer selected in the legend)
        QgsMapLayer *activeLayer();

        //! Add an icon to the plugins toolbar
        int addToolBarIcon(QAction *qAction);
        //! Remove an icon (action) from the plugin toolbar
        void removeToolBarIcon(QAction *qAction);
        //! Add toolbar with specified name
        QToolBar* addToolBar(QString name);
        /** Get the file toolbar - intended for use with plugins which
         *   add a new file type handler.
         */
        QToolBar * fileToolBar();

        /** Open a url in the users browser. By default the QGIS doc directory is used
         * as the base for the URL. To open a URL that is not relative to the installed
         * QGIS documentation, set useQgisDocDirectory to false.
         * @param url URL to open
         * @param useQgisDocDirectory If true, the URL will be formed by concatenating 
         * url to the QGIS documentation directory path (<prefix>/share/doc)
         */
        void openURL(QString url, bool useQgisDocDirectory=true);

        /** Return a pointer to the map canvas used by qgisapp */
        QgsMapCanvas * getMapCanvas();	

        /** Gives access to main QgisApp object

            Plugins don't need to know about QgisApp, as we pass it as QWidget,
            it can be used for connecting slots and using as widget's parent
        */
        QWidget * getMainWindow();

        /** Add action to the plugins menu */
        void addPluginMenu(QString name, QAction* action); 
        /** Remove action from the plugins menu */
        void removePluginMenu(QString name, QAction* action); 

        /** Return a pointer to the toolbox (where additional pages can be inserted) */
        virtual QToolBox* getToolBox();

	virtual void refreshLegend(QgsMapLayer *l);

    private:

        /// QgisInterface aren't copied
        QgisAppInterface( QgisAppInterface const & );
        
        /// QgisInterface aren't copied
        QgisAppInterface & operator=( QgisAppInterface const & );

        //! Pointer to the QgisApp object
        QgisApp *qgis;
};


#endif //#define QGISAPPINTERFACE_H
