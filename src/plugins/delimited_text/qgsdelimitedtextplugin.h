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
#ifndef PLUGIN
#define PLUGIN
#include "../qgisplugin.h"
#include "ui_qgsdelimitedtextpluginguibase.h"

class QgisInterface;

/**
* \class QgsDelimitedTextPlugin
* \brief Delimited text plugin for QGIS
*
*/
class QgsDelimitedTextPlugin: public QObject, public QgisPlugin, private Ui::QgsDelimitedTextPluginGuiBase
{
  Q_OBJECT public:
    /**
     * Constructor for a plugin. The QgisInterface pointer is passed by
     * QGIS when it attempts to instantiate the plugin.
     * @param qI Pointer to the QgisInterface object
     */
    QgsDelimitedTextPlugin( QgisInterface * );
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
    //! Destructor
    virtual ~ QgsDelimitedTextPlugin();
  public slots:
    //! init the gui
    virtual void initGui();
    //! Show the dialog box
    void run();
    //! Add a vector layer given vectorLayerPath, baseName, providerKey ("ogr" or "postgres");
    void drawVectorLayer( QString, QString, QString );
    //! unload the plugin
    void unload();
    //! show the help document
    void help();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );
  private:


    //! Name of the plugin
    QString pluginNameQString;
    //! Version
    QString pluginVersionQString;
    //! Descrption of the plugin
    QString pluginDescriptionQString;
    //! Plugin type as defined in Plugin::PLUGINTYPE
    int pluginType;
    //! Pointer to the QGIS interface object
    QgisInterface *qGisInterface;
    //! Pointer to the QAction object used in the menu and toolbar
    QAction *myQActionPointer;
};

#endif
