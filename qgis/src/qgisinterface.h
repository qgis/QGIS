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

#include <qwidget.h>
#include <map>
class QgisApp;
class QgsMapLayer;
class QPopupMenu;
class QgsMapCanvas;
class QgsRasterLayer;
class QgsMapLayerRegistry;

/** 
 * \class QgisInterface
 * \brief Abstract base class defining interfaces exposed by QgisApp and
 * made available to plugins.
 *
 */

class QgisInterface : public QWidget{
  Q_OBJECT
  public:
    /**
     * Constructor
     * @param qgis Pointer to the qgis application instance
     * @param name 
     */
    QgisInterface(QgisApp *qgis=0, const char *name=0) ;
    /**
     * Destructor
     */
    virtual ~QgisInterface() ;
    public slots:
      //! Zoom to full extent of map layers
      virtual void zoomFull()=0;
    //! Zoom to previous view extent
    virtual void zoomPrevious()=0;
    //! Zoome to extent of the active layer
    virtual void zoomActiveLayer()=0;
    //! Add a vector layer
    virtual bool addVectorLayer(QString vectorLayerPath, QString baseName, QString providerKey)=0;
    //! Add a raster layer given a raster layer file name
    virtual bool addRasterLayer(QString rasterLayerPath)=0;
    //! Add a raster layer given a QgsRasterLayer object
    virtual bool addRasterLayer(QgsRasterLayer * theRasterLayer, bool theForceRenderFlag=false)=0;
    //! Add a project
    virtual bool addProject(QString theProject)=0; 
    //! Start a blank project
    virtual void newProject(bool thePromptToSaveFlag=false)=0; 

    //! Get pointer to the active layer (layer selected in the legend)
    virtual QgsMapLayer *activeLayer()=0;
    //! add a menu item to the main menu, postioned to the left of the Help menu
    virtual int addMenu(QString menuText, QPopupMenu *menu) =0;
    /** Open a url in the users browser. By default the QGIS doc directory is used
     * as the base for the URL. To open a URL that is not relative to the installed
     * QGIS documentation, set useQgisDocDirectory to false.
     * @param url URL to open
     * @param useQgisDocDirectory If true, the URL will be formed by concatenating 
     * url to the QGIS documentation directory path (<prefix>/share/doc)
     */
    virtual void openURL(QString url, bool useQgisDocDirectory=true)=0;
    /** 
     * Get the menu info mapped by menu name (key is name, value is menu id)
     */
    virtual std::map<QString,int> menuMapByName()=0;
    /** 
     * Get the menu info mapped by menu id (key is menu id, value is name)
     */
    virtual std::map<int,QString> menuMapById()=0;
    /** Return a pointer to the map canvas used by qgisapp */
    virtual QgsMapCanvas * getMapCanvas()=0;
    /** Return a pointer to the map layer registry */
    virtual QgsMapLayerRegistry * getLayerRegistry()=0;	

  private:
    //QgisApp *qgis;
};

#endif //#ifndef QGISINTERFACE_H

