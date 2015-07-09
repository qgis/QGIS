/***************************************************************************
                          qgscompassplugin.h
 Functions:
                             -------------------
    begin                : Jan 28, 2012
    copyright            : (C) 2012 by Marco Bernasocchi
    email                : marco@bernawebdesign.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef PLUGIN
#define PLUGIN
#include "../qgisplugin.h"
#include "ui_qgscompasspluginguibase.h"
#include "qgscompassplugingui.h"

class QgisInterface;

/**
* \class QgsCompassPlugin
*
*/
class QgsCompassPlugin: public QObject, public QgisPlugin, private Ui::QgsCompassPluginGuiBase
{
    Q_OBJECT
  public:
    /**
     * Constructor for a plugin. The QgisInterface pointer is passed by
     * QGIS when it attempts to instantiate the plugin.
     * @param qI Pointer to the QgisInterface object
     */
    QgsCompassPlugin( QgisInterface * );
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
     * Virtual function to return a plugin category
     */
    virtual QString category();
    /**
     * Return the plugin type
     */
    virtual int type();
    //! Destructor
    virtual ~ QgsCompassPlugin();
  public slots:
    //! init the gui
    virtual void initGui();
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload();
    //! show the help document
    void help();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );
    QIcon getThemeIcon( const QString &theThemeName );
    void about();
  private:


    //! Name of the plugin
    QString pluginNameQString;
    //! Version
    QString pluginVersionQString;
    //! Descrption of the plugin
    QString pluginDescriptionQString;
    //! Category of the plugin
    QString pluginCategoryQString;
    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //! Pointer to the QAction object used in the menu and toolbar
    QAction *mActionRunCompass;
    QAction *mActionAboutCompass;

    QDockWidget *mDock;
    QgsCompassPluginGui *mQgsCompassPluginGui;
};

#endif
