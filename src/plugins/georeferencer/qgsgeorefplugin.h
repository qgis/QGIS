/***************************************************************************
*  File Name:               plugin.h
*
*  The georeferencer plugin is a tool for adding projection info to rasters
*
*--------------------------------------------------------------------------
*    begin                : Jan 21, 2004
*    copyright            : (C) 2004 by Tim Sutton
*    email                : tim@linfiniti.com
*
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/***************************************************************************
 *   QGIS Programming conventions:
 *
 *   mVariableName - a class level member variable
 *   sVariableName - a static class level member variable
 *   variableName() - accessor for a class member (no 'get' in front of name)
 *   setVariableName() - mutator for a class member (prefix with 'set')
 *
 *   Additional useful conventions:
 *
 *   theVariableName - a method parameter (prefix with 'the')
 *   myVariableName - a locally declared variable within a method ('my' prefix)
 *
 *   DO: Use mixed case variable names - myVariableName
 *   DON'T: separate variable names using underscores: my_variable_name (NO!)
 *
 * **************************************************************************/

#ifndef QGSGEOREFPLUGIN
#define QGSGEOREFPLUGIN

//
//QGIS Includes
//
#include <qgisplugin.h>
class QgisInterface;
class QgsGeorefPluginGui;

//
//QT Includes
//
#include <QWidget>
#include <QIcon>

/**
* \class Plugin
* \brief [name] plugin for QGIS
* [description]
*/
class QgsGeorefPlugin: public QObject, public QgisPlugin
{
  Q_OBJECT public:

    //////////////////////////////////////////////////////////////////////
    //
    //                MANDATORY PLUGIN METHODS FOLLOW
    //
    //////////////////////////////////////////////////////////////////////

    /**
    * Constructor for a plugin. The QgisApp and QgisIface pointers are passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param Pointer to the QgisApp object
    * @param Pointer to the QgisIface object.
     */
    explicit QgsGeorefPlugin( QgisInterface * );
    //! Destructor
    virtual ~ QgsGeorefPlugin();

  public slots:
    //! init the gui
    virtual void initGui() override;
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload() override;
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( const QString& theThemeName );
    QIcon getThemeIcon( const QString &theThemeName );

    //////////////////////////////////////////////////////////////////////
    //
    //                  END OF MANDATORY PLUGIN METHODS
    //
    //////////////////////////////////////////////////////////////////////

  private:

    ////////////////////////////////////////////////////////////////////
    //
    // MANDATORY PLUGIN MEMBER DECLARATIONS  .....
    //
    ////////////////////////////////////////////////////////////////////

    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //!pointer to the qaction for this plugin
    QAction * mActionRunGeoref;
    ////////////////////////////////////////////////////////////////////
    //
    // ADD YOUR OWN MEMBER DECLARATIONS AFTER THIS POINT.....
    //
    ////////////////////////////////////////////////////////////////////
    QgsGeorefPluginGui *mPluginGui;
};

#endif
