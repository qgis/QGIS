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
#include <qwidget.h>
#include <qpainter.h>


#include "../../src/qgisapp.h"

/**
* \class Plugin
* \brief North Arrow plugin for QGIS
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
  // draw some arbitary text to the screen
  void renderNorthArrow(QPainter *);
  //! Run when the user has set a new rotation
  void rotationChanged(int);
  //! Refresh the map display using the mapcanvas exported via the plugin interface
  void refreshCanvas();
  //! unload the plugin
  void unload();
  //! show the help document
  void help();
  //! set north arrow placement
  void setPlacement(QString);
  //! enable or disable north arrow
  void setEnabled(bool);

    private:


  // The amount of rotation for the north arrow
  int mRotationInt;
  int pluginType;
  // enable or disable north arrow
  bool mEnable;
  // The placement string
  QString mPlacement;
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
  //! Pointer to the QAction object used in the menu and toolbar
  QAction *myQActionPointer;
};

#endif
