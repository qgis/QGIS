/***************************************************************************
                          qgisinterface.h 
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
#ifndef QGISINTERFACE_H
#define QGISINTERFACE_H

class QAction;
class QMenu;
class QToolBar;
class QDockWidget;
class QWidget;
#include <QObject>

#include <map>

class QgisApp;
class QgsMapLayer;
class QgsMapCanvas;
class QgsRasterLayer;
class QgsVectorLayer;

/** 
 * \class QgisInterface
 * \brief Abstract base class defining interfaces exposed by QgisApp and
 * made available to plugins.
 *
 * Only functionality exposed by QgisInterface can be used in plugins.
 * This interface has to be implemented with application specific details.
 *
 * QGIS implements it in QgisAppInterface class, 3rd party applications
 * could provide their own implementation to be able to use plugins.
 */

class GUI_EXPORT QgisInterface : public QObject
{
  Q_OBJECT;

  public:

    /** Constructor */
    QgisInterface();

    /** Virtual destructor */
    virtual ~QgisInterface();
    

  public slots: // TODO: do these functions really need to be slots?

    //! Zoom to full extent of map layers
    virtual void zoomFull()=0;
    //! Zoom to previous view extent
    virtual void zoomPrevious()=0;
    //! Zoome to extent of the active layer
    virtual void zoomActiveLayer()=0;

    //! Add a vector layer
    virtual QgsVectorLayer* addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)=0;
    //! Add a raster layer given a raster layer file name
    virtual QgsRasterLayer* addRasterLayer(QString rasterLayerPath, QString baseName = QString())=0;
    //! Add a WMS layer
    virtual QgsRasterLayer* addRasterLayer(const QString& url, const QString& layerName, const QString& providerKey, const QStringList& layers, \
					   const QStringList& styles, const QString& format, const QString& crs) = 0;

    //! Add a project
    virtual bool addProject(QString theProject)=0; 
    //! Start a blank project
    virtual void newProject(bool thePromptToSaveFlag=false)=0; 

    //! Get pointer to the active layer (layer selected in the legend)
    virtual QgsMapLayer *activeLayer()=0;

    //! Add an icon to the plugins toolbar
    virtual int addToolBarIcon(QAction *qAction) =0;
    //! Remove an action (icon) from the plugin toolbar
    virtual void removeToolBarIcon(QAction *qAction) = 0;
    //! Add toolbar with specified name
    virtual QToolBar * addToolBar(QString name)=0;
    /** Get the file toolbar - intended for use with plugins which
    *   add a new file type handler.
    */
    virtual QToolBar * fileToolBar()=0;
    // TODO: is this deprecated in favour of QgsContextHelp?
    /** Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to false.
     * @param url URL to open
     * @param useQgisDocDirectory If true, the URL will be formed by concatenating 
     * url to the QGIS documentation directory path (<prefix>/share/doc)
     */
    virtual void openURL(QString url, bool useQgisDocDirectory=true)=0;

    /** Return a pointer to the map canvas */
    virtual QgsMapCanvas * getMapCanvas()=0;

    /** Return a pointer to the main window (instance of QgisApp in case of QGIS) */
    virtual QWidget * getMainWindow()=0;

    /** Add action to the plugins menu */
    virtual void addPluginMenu(QString name, QAction* action)=0;
    /** Remove action from the plugins menu */
    virtual void removePluginMenu(QString name, QAction* action)=0;

    /** Add a dock widget to the main window */
    virtual void addDockWidget ( Qt::DockWidgetArea area, QDockWidget * dockwidget )=0;

    /** refresh the legend of a layer */
    virtual void refreshLegend(QgsMapLayer *l)=0;

  signals:
    /** Emited whenever current (selected) layer changes.
     *  The pointer to layer can be null if no layer is selected
     */
    void currentLayerChanged ( QgsMapLayer * layer );

};

// FIXME: also in core/qgis.h
#ifndef QGISEXTERN
#ifdef WIN32
#  define QGISEXTERN extern "C" __declspec( dllexport )
#  ifdef _MSC_VER
// do not warn about C bindings returing QString
#    pragma warning(disable:4190)
#  endif
#else
#  define QGISEXTERN extern "C"
#endif
#endif

#endif //#ifndef QGISINTERFACE_H
