/***************************************************************************
                          plugin.h 
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
#include "../../src/qgisapp.h"
#include <qwidget.h>
#include <qfont.h>
#include <qcolor.h>

/**
* \class Plugin
* \brief OpenModeller plugin for QGIS
*
*/
class Plugin:public QObject, public QgisPlugin
{
  Q_OBJECT public:
      /** 
       * Constructor for a plugin. The QgisApp and QgisIface pointers are passed by 
       * QGIS when it attempts to instantiate the plugin.
       * @param qgis Pointer to the QgisApp object
       * @param qI Pointer to the QgisIface object. 
       */
      Plugin(QgisApp * , QgisIface * );
  //! init the gui
  virtual void initGui();
  //! Destructor
  virtual ~ Plugin();
  public slots:
  //! Show the dialog box
  void run();
  //!draw a raster layer in the qui - not used!
  void drawRasterLayer(QString);
  //! Add a vector layer given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
  //  - not used!
  void drawVectorLayer(QString,QString,QString);
  // draw some arbitary text to the screen
  void renderLabel();
  //! Refresh the map display using the mapcanvas exported via the plugin interface
  void refreshCanvas();
  //! unload the plugin
  void unload();
  //! show the help document
  void help();
  //! change the copyright font
  void setFont(QFont);
  //! change the copyright text
  void setLabel(QString);
  //! change the copyright font colour
  void setColor(QColor);
  
    private:
  //! This is the font that will be used for the copyright label
  QFont mQFont;
  //! This is the string that will be used for the copyright label
  QString mLabelQString;
  //! This is the colour for the copyright label
  QColor mLabelQColor;
  
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
