/***************************************************************************
                          qgsspatialqueryplugin.h
    A plugin that makes spatial queries on vector layers
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.comm

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*  $Id: qgsspatialqueryplugin.h 13377 2010-04-25 01:07:36Z jef $ */

#ifndef SPATIALQUERYPLUGIN_H
#define SPATIALQUERYPLUGIN_H

//
//QGIS Includes
//
#include "qgisplugin.h"

class QgisInterface;
class QgsSpatialQueryDialog;

//
//QT Includes
//
#include <QIcon>
#include <QObject>
class QAction;


/**
* \class QgsSpatialQueryPlugin
* \brief Spatial Query plugin for QGIS
*
*/
class QgsSpatialQueryPlugin: public QObject, public QgisPlugin
{
    Q_OBJECT
  public:
    /**
    * \brief Constructor for a plugin. The QgisInterface pointer is passed by
    * QGIS when it attempts to instantiate the plugin.
    * \param iface Pointer to the QgisInterface object.
    */
    QgsSpatialQueryPlugin( QgisInterface* iface );
    //! Destructor
    ~QgsSpatialQueryPlugin();

  public slots:
    //! init the gui
    void initGui();
    //! unload the plugin
    void unload();
    //! Show the dialog box
    void run();
    //! update the plugins theme when the app tells us its theme is changed
    void setCurrentTheme( QString theThemeName );
    QIcon getThemeIcon( const QString &theThemeName );

  private:

    QgsSpatialQueryDialog  *mDialog;
    //! Pointer to the QgisInterface object
    QgisInterface* mIface;
    //! Pointer to the QAction used in the menu and on the toolbar
    QAction* mSpatialQueryAction;


    void MsgDEBUG( QString sMSg );

};
#endif

