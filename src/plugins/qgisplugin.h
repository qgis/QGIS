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

/**
 * QGIS - Plugin API
 *
 *  \section about  About QGIS Plugins
 * Plugins provide additional functionality to QGis. Plugins must
 * implement several required methods in order to be registered with
 * QGis. These methods include:
 * name:
 *
 * - version
 * - description
 *
 * All QGIS plugins must inherit from the abstract base class QgisPlugin.
 * This list will grow as the API is expanded.
 *
 * In addition, a plugin must implement the classFactory and unload
 * functions. Note that these functions must be declared as extern "C" in
 * order to be resolved properly and prevent C++ name mangling.
 */

#ifndef QGISPLUGIN_H
#define QGISPLUGIN_H

#define SIP_NO_FILE


#include <QString>

class QgisInterface;

//#include "qgisplugingui.h"

/**
 * \ingroup plugins
 * \class QgisPlugin
 * \brief Abstract base class from which all plugins must inherit
 * \note not available in Python bindings
 */
class QgisPlugin
{
  public:
    //! Interface to gui element collection object
    //virtual QgisPluginGui *gui()=0;
    //! Element types that can be added to the interface
#if 0
    enum Elements
    {
      MENU,
      MENU_ITEM,
      TOOLBAR,
      TOOLBAR_BUTTON,
    };

    \todo XXX this may be a hint that there should be subclasses
#endif

    enum PluginType
    {
      UI = 1,   //!< User interface plug-in
      MapLayer, //!< Map layer plug-in
      Renderer, //!< A plugin for a new renderer class
    };


    /**
     * Constructor for QgisPlugin
     */
    QgisPlugin( QString const &name = "", QString const &description = "", QString const &category = "", QString const &version = "", PluginType type = MapLayer )
      : mName( name )
      , mDescription( description )
      , mCategory( category )
      , mVersion( version )
      , mType( type )
    {}

    virtual ~QgisPlugin() = default;

    //! Gets the name of the plugin
    QString const &name() const
    {
      return mName;
    }

    QString &name()
    {
      return mName;
    }

    //! Version of the plugin
    QString const &version() const
    {
      return mVersion;
    }

    //! Version of the plugin
    QString &version()
    {
      return mVersion;
    }

    //! A brief description of the plugin
    QString const &description() const
    {
      return mDescription;
    }

    //! A brief description of the plugin
    QString &description()
    {
      return mDescription;
    }

    //! Plugin category
    QString const &category() const
    {
      return mCategory;
    }

    //! Plugin category
    QString &category()
    {
      return mCategory;
    }

    //! Plugin type, either UI or map layer
    QgisPlugin::PluginType const &type() const
    {
      return mType;
    }


    //! Plugin type, either UI or map layer
    QgisPlugin::PluginType &type()
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

    /// category
    QString mCategory;

    /// version
    QString mVersion;

    /// UI or MAPLAYER plug-in

    /**
     * \todo Really, might be indicative that this needs to split into
     * maplayer vs. ui plug-in vs. other kind of plug-in
     */
    PluginType mType;

}; // class QgisPlugin


// Typedefs used by qgis main app

//! Typedef for the function that returns a generic pointer to a plugin object
typedef QgisPlugin *create_t( QgisInterface * );

//! Typedef for the function to unload a plugin and free its resources
typedef void unload_t( QgisPlugin * );

//! Typedef for getting the name of the plugin without instantiating it
typedef const QString *name_t();

//! Typedef for getting the description without instantiating the plugin
typedef const QString *description_t();

//! Typedef for getting the category without instantiating the plugin
typedef const QString *category_t();

//! Typedef for getting the plugin type without instantiating the plugin
typedef int type_t();

//! Typedef for getting the plugin version without instantiating the plugin
typedef const QString *version_t();

//! Typedef for getting the plugin icon file name without instantiating the plugin
typedef const QString *icon_t();

//! Typedef for getting the experimental status without instantiating the plugin
typedef const QString *experimental_t();

//! Typedef for getting the create date without instantiating the plugin
typedef const QString *create_date_t();

//! Typedef for getting the update date status without instantiating the plugin
typedef const QString *update_date_t();

#endif // QGISPLUGIN_H
