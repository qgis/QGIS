// $Id$
//////////////////////////////////////////////////////////////////////////////
//
// begin                : Aug 24, 2008
// copyright            : (C) 2008 by Mateusz Loskot
// email                : mateusz@loskot.net
//
//////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef QGIS_PLUGIN_OGRCONV_PLUGIN_H_INCLUDED
#define QGIS_PLUGIN_OGRCONV_PLUGIN_H_INCLUDED

// QGIS
#include "../qgisplugin.h"
// Qt4
#include <QObject>

// Forward declarations
class QAction;
class QToolBar;
class QgisInterface;

/**
* \class OgrPlugin
* \brief Translates vector layers between formats supported by OGR library.
*/
class OgrPlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:

    //////////////////////////////////////////////////////////////////////////
    //                MANDATORY PLUGIN METHODS FOLLOW
    //////////////////////////////////////////////////////////////////////////

    /**
    * Constructor for a plugin. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param theInterface Pointer to the QgisInterface object.
    */
    OgrPlugin( QgisInterface * theInterface );

    //! Destructor
    virtual ~OgrPlugin();

  public slots:

    /**
     * Initialize the GUI interface for the plugin.
     * This is only called once when the plugin is added to the plugin
     * registry in the QGIS application.
     */
    virtual void initGui();

    /**
     * Slot called when the menu item is triggered.
     * If you created more menu items / toolbar buttons in initiGui,
     * you should create a separate handler for each action - this
     * single run() method will not be enough.
     */
    void run();

    /**
     * Unload the plugin by cleaning up the GUI.
     */
    void unload();

    //! show the help document
    void help();

    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );

  private:

    //////////////////////////////////////////////////////////////////////////
    // MANDATORY PLUGIN PROPERTY DECLARATIONS
    //////////////////////////////////////////////////////////////////////////

    // FIXME: Is it used at all?
    int mPluginType;

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;

    //!pointer to the qaction for this plugin
    QAction * mQActionPointer;

    //////////////////////////////////////////////////////////////////////////
    // ADD YOUR OWN PROPERTY DECLARATIONS AFTER THIS POINT.....
    //////////////////////////////////////////////////////////////////////////

}; // OgrPlugin

#endif // QGIS_PLUGIN_OGRCONV_PLUGIN_H_INCLUDED
