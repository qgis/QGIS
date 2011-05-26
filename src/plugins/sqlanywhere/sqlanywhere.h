/***************************************************************************
  sqlanywhere.h
  Store vector layers within a SQL Anywhere database
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef SQLANYWHERE_H
#define SQLANYWHERE_H

//QT4 includes
#include <QObject>
#include <QIcon>

//QGIS includes
#include "../qgisplugin.h"

//forward declarations
class QAction;
class QToolBar;

class QgisInterface;

/**
* \class SqlAnywhere
* \brief SQL Anywhere plugin for QGIS
* Store vector layers within a SQL Anywhere database
*/
class SqlAnywhere: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:

    /**
    * Constructor for a plugin. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * @param theInterface Pointer to the QgisInterface object.
     */
    SqlAnywhere( QgisInterface * theInterface );
    //! Destructor
    virtual ~SqlAnywhere();

  public slots:
    //! init the gui
    virtual void initGui();
    //! Show the new SQL Anywhere layer dialogue box
    void addSqlAnywhereLayer();
    //! unload the plugin
    void unload();
    //! show the help document
    void help();

    //! Helper to get a theme icon. It will fall back to the
    //default theme if the active theme does not have the required
    //icon.
    static QIcon getThemeIcon( const QString theName );

  private:
    int mPluginType;
    QgisInterface *mQGisIface;
    QAction * mActionAddSqlAnywhereLayer;
};

#endif //SQLANYWHERE_H
