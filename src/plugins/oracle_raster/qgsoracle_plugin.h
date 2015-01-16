/***************************************************************************
    oracleplugin.h
    -------------------
    begin                : Oracle Spatial Plugin
    copyright            : (C) Ivan Lucena
    email                : ivan.lucena@pmldnet.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef OraclePlugin_H
#define OraclePlugin_H

// Qt Includes
#include <QObject>
#include <QAction>
#include <QToolBar>

// QGIS Includes
#include <qgisplugin.h>
#include <qgisinterface.h>
#include <qgisgui.h>

class QgsOraclePlugin : public QObject, public QgisPlugin
{
    Q_OBJECT

  public:

    /**
     * Constructor for a plugin. The QgisInterface pointer is passed by
     * QGIS when it attempts to instantiate the plugin.
     * @param theInterface Pointer to the QgisInterface object.
     */
    QgsOraclePlugin( QgisInterface * theInterface );
    //! Destructor
    virtual ~QgsOraclePlugin();

  public slots:

    //! init the gui
    virtual void initGui() override;
    //! Show the dialog box
    void run();
    //! unload the plugin
    void unload() override;
    //! show the help document
    void help();

  private:

    int mPluginType;
    //! Pointer to the QGIS interface object
    QgisInterface *mQGisIface;
    //!pointer to the qaction for this plugin
    QAction * mQActionPointer;

};

#endif //OraclePlugin_H
