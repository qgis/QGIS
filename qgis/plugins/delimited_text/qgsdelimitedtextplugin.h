/***************************************************************************
                          qgsdelimitedtextplugin.h 
 Functions:
                             -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com
  
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /*  $Id$ */
#ifndef PLUGIN
#define PLUGIN
#include "../qgisplugin.h"
#include <qwidget.h>

#include "../../src/qgisapp.h"

/**
* \class QgsDelimitedTextPlugin
* \brief OpenModeller plugin for QGIS
*
*/
class QgsDelimitedTextPlugin:public QObject, public QgisPlugin
{
  Q_OBJECT public:
      /** 
       * Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
       * QGIS when it attempts to instantiate the plugin.
       * @param qgis Pointer to the QgisApp object
       * @param qI Pointer to the QgisIface object. 
       */
      QgsDelimitedTextPlugin(QgisApp * , QgisIface * );
  /**
   * Virtual function to return the name of the plugin. The name will be used when presenting a list 
   * of installable plugins to the user
   */
  virtual QString name();
  /**
   * Virtual function to return the version of the plugin. 
   */
  virtual QString version();
  /**
   * Virtual function to return a description of the plugins functions 
   */
  virtual QString description();
  /**
   * Return the plugin type
   */
  virtual int type();
  //! init the gui
  virtual void initGui();
  //! Destructor
  virtual ~ QgsDelimitedTextPlugin();
  public slots:
  //! Show the dialog box
  void run();
  //! Add a vector layer given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
  void drawVectorLayer(QString,QString,QString);
  //! unload the plugin
  void unload();
  //! show the help document
  void help();
    private:


  //! Name of the plugin
  QString pluginNameQString;
  //! Version
  QString pluginVersionQString;
  //! Descrption of the plugin
  QString pluginDescriptionQString;
  //! Plugin type as defined in Plugin::PLUGINTYPE
  int pluginType;
  //! Id of the plugin's menu. Used for unloading
  int menuIdInt;
  //! Pointer to our toolbar
  QToolBar *toolBarPointer;
  //! Pointer to our menu
  QMenuBar *menuBarPointer;
  //! Pionter to QGIS main application object
  QgisApp *qgisMainWindowPointer;
  //! Pointer to the QGIS interface object
  QgisIface *qGisInterface;
};

#endif
