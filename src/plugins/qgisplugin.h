/***************************************************************************
     qgisplugin.h
     --------------------------------------
    Date                 : Sun Sep 16 12:12:31 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*!  Quantum GIS - Plugin API
 *
 *  \section about  About QGis Plugins
 * Plugins provide additional functionality to QGis. Plugins must
 * implement several required methods in order to be registered with
 * QGis. These methods include:
 * <ul>name
 * <li>version
 * <li>description
 * </ul>
 *
 * All QGis plugins must inherit from the abstract base class QgisPlugin.
 * This list will grow as the API is expanded.
 *
 * In addition, a plugin must implement the classFactory and unload
 * functions. Note that these functions must be declared as extern "C" in
 * order to be resolved properly and prevent C++ name mangling.
 */

#ifndef QGISPLUGIN_H
#define QGISPLUGIN_H


#include <QString>

class QgisInterface;

//#include "qgisplugingui.h"

/*! \class QgisPlugin
 * \brief Abstract base class from which all plugins must inherit
 *
 */
class QgisPlugin
{
  public:

    //! Interface to gui element collection object
    //virtual QgisPluginGui *gui()=0;
    //! Element types that can be added to the interface
    /* enum ELEMENTS {
       MENU,
       MENU_ITEM,
       TOOLBAR,
       TOOLBAR_BUTTON,
       };

       @todo XXX this may be a hint that there should be subclasses
       */
    enum PLUGINTYPE
    {
      UI = 1,                     /* user interface plug-in */
      MAPLAYER,                    /* map layer plug-in */
      RENDERER,                     /*a plugin for a new renderer class*/
      VECTOR_OVERLAY                /*an overlay plugin. Added in version 1.1*/
    };


    QgisPlugin( QString const & name = "",
                QString const & description = "",
                QString const & version = "",
                PLUGINTYPE const & type = MAPLAYER )
        : mName( name ),
        mDescription( description ),
        mVersion( version ),
        mType( type )
    {}

    virtual ~QgisPlugin()
    {}

    //! Get the name of the plugin
    QString const & name() const
    {
      return mName;
    }

    QString       & name()
    {
      return mName;
    }

    //! Version of the plugin
    QString const & version() const
    {
      return mVersion;
    }

    //! Version of the plugin
    QString & version()
    {
      return mVersion;
    }

    //! A brief description of the plugin
    QString const & description() const
    {
      return mDescription;
    }

    //! A brief description of the plugin
    QString       & description()
    {
      return mDescription;
    }

    //! Plugin type, either UI or map layer
    QgisPlugin::PLUGINTYPE const & type() const
    {
      return mType;
    }


    //! Plugin type, either UI or map layer
    QgisPlugin::PLUGINTYPE       & type()
    {
      return mType;
    }

    /// function to initialize connection to GUI
    virtual void initGui() = 0;

    //! Unload the plugin and cleanup the GUI
    virtual void unload() = 0;

  private:

    /// plug-in name
    QString mName;

    /// description
    QString mDescription;

    /// version
    QString mVersion;

    /// UI or MAPLAYER plug-in
    /**
      @todo Really, might be indicative that this needs to split into
      maplayer vs. ui plug-in vs. other kind of plug-in
      */
    PLUGINTYPE mType;

}; // class QgisPlugin


// Typedefs used by qgis main app

//! Typedef for the function that returns a generic pointer to a plugin object
typedef QgisPlugin *create_t( QgisInterface * );

//! Typedef for the function to unload a plugin and free its resources
typedef void unload_t( QgisPlugin * );

//! Typedef for getting the name of the plugin without instantiating it
typedef QString name_t();

//! Typedef for getting the description without instantiating the plugin
typedef QString description_t();

//! Typedef for getting the plugin type without instantiating the plugin
typedef int type_t();

//! Typedef for getting the plugin version without instantiating the plugin
typedef QString version_t();

//! Typedef for getting the plugin icon file name without instantiating the plugin
typedef QString icon_t();


#endif //qgisplugin_h
